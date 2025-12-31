// gstnuscenesrc.c
#include <gst/gst.h>
#include <gst/base/gstpushsrc.h>
#include <json-glib/json-glib.h>
#include <glib.h>

#define GST_TYPE_NUSCENESRC (gst_nuscenesrc_get_type())
G_DECLARE_FINAL_TYPE(GstNuSceneSrc, gst_nuscenesrc, GST, NUSCENESRC, GstPushSrc)

struct _GstNuSceneSrc {
  GstPushSrc parent;

  gchar   *dataroot;     // dataset root (contains v1.0-test folder + samples/)
  gchar   *meta;         // meta folder name or full path (default: "v1.0-test")
  gchar   *cam;          // e.g., "CAM_BACK"
  gint     fps;          // e.g., 12
  gboolean loop;         // restart at end
  gint     scene_index;  // which scene to play

  GPtrArray *frames;     // array of gchar* full paths
  guint      frame_idx;  // current index

  GstClockTime next_pts;
  GstClockTime duration;
};

G_DEFINE_TYPE(GstNuSceneSrc, gst_nuscenesrc, GST_TYPE_PUSH_SRC)

enum {
  PROP_0,
  PROP_DATAROOT,
  PROP_META,
  PROP_CAM,
  PROP_FPS,
  PROP_LOOP,
  PROP_SCENE_INDEX,
};

static gchar* join_path_if_relative(const gchar *base, const gchar *maybe_rel) {
  if (!maybe_rel || !*maybe_rel) return NULL;
  if (g_path_is_absolute(maybe_rel)) return g_strdup(maybe_rel);
  return g_build_filename(base, maybe_rel, NULL);
}

static JsonArray* load_json_array_from_file(const gchar *path, GError **err) {
  JsonParser *parser = json_parser_new();
  if (!json_parser_load_from_file(parser, path, err)) {
    g_object_unref(parser);
    return NULL;
  }
  JsonNode *root = json_parser_get_root(parser);
  if (!JSON_NODE_HOLDS_ARRAY(root)) {
    g_set_error(err, g_quark_from_string("nuscenesrc"), 1,
                "Root JSON is not an array: %s", path);
    g_object_unref(parser);
    return NULL;
  }
  JsonArray *arr = json_node_get_array(root);
  // keep parser alive until we’re done copying data out; we copy what we need, so ok to unref
  g_object_unref(parser);
  return arr;
}

// Helper: sample_data rows sometimes use 'file_name' or 'filename'
static const gchar* sd_get_filename(JsonObject *sd) {
  if (json_object_has_member(sd, "file_name"))
    return json_object_get_string_member(sd, "file_name");
  if (json_object_has_member(sd, "filename"))
    return json_object_get_string_member(sd, "filename");
  return NULL;
}

static gboolean build_frame_list(GstNuSceneSrc *self, GError **err) {
  g_return_val_if_fail(self->frames == NULL, FALSE);

  gchar *meta_path = join_path_if_relative(self->dataroot, self->meta);
  if (!meta_path) {
    g_set_error(err, g_quark_from_string("nuscenesrc"), 2, "Invalid meta path");
    return FALSE;
  }

  gchar *scene_json      = g_build_filename(meta_path, "scene.json", NULL);
  gchar *sample_json     = g_build_filename(meta_path, "sample.json", NULL);
  gchar *sample_data_json= g_build_filename(meta_path, "sample_data.json", NULL);

  JsonArray *scenes = load_json_array_from_file(scene_json, err);
  if (!scenes) goto fail;

  JsonArray *samples = load_json_array_from_file(sample_json, err);
  if (!samples) goto fail;

  JsonArray *sample_data = load_json_array_from_file(sample_data_json, err);
  if (!sample_data) goto fail;

  guint n_scenes = json_array_get_length(scenes);
  if (n_scenes == 0) {
    g_set_error(err, g_quark_from_string("nuscenesrc"), 3, "No scenes found");
    goto fail;
  }
  if (self->scene_index < 0 || (guint)self->scene_index >= n_scenes) {
    g_set_error(err, g_quark_from_string("nuscenesrc"), 4,
                "scene-index out of range: %d (0..%u)",
                self->scene_index, n_scenes - 1);
    goto fail;
  }

  // --- Build sample_by_token: token -> next_token
  GHashTable *sample_next = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

  for (guint i = 0; i < json_array_get_length(samples); i++) {
    JsonObject *s = json_array_get_object_element(samples, i);
    if (!json_object_has_member(s, "token")) continue;
    const gchar *token = json_object_get_string_member(s, "token");
    const gchar *next  = NULL;
    if (json_object_has_member(s, "next"))
      next = json_object_get_string_member(s, "next");
    g_hash_table_insert(sample_next, g_strdup(token), g_strdup(next ? next : ""));
  }

  // --- Build sd_by_sample_token for the chosen cam
  // Prefer sd["channel"] match; fallback to path contains "/CAM_BACK/" style.
  GHashTable *sd_fn_by_sample = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

  for (guint i = 0; i < json_array_get_length(sample_data); i++) {
    JsonObject *sd = json_array_get_object_element(sample_data, i);
    const gchar *fn = sd_get_filename(sd);
    if (!fn || !g_str_has_suffix(fn, ".jpg")) continue;

    const gchar *sample_token = json_object_has_member(sd, "sample_token")
      ? json_object_get_string_member(sd, "sample_token")
      : NULL;
    if (!sample_token) continue;

    gboolean keep = FALSE;

    if (json_object_has_member(sd, "channel")) {
      const gchar *ch = json_object_get_string_member(sd, "channel");
      if (ch && self->cam && g_strcmp0(ch, self->cam) == 0)
        keep = TRUE;
    }
    if (!keep && self->cam && g_strstr_len(fn, -1, self->cam)) {
      // fallback (less strict): path contains CAM_BACK etc.
      keep = TRUE;
    }
    if (!keep) continue;

    // store relative filename (we’ll join with dataroot later)
    g_hash_table_insert(sd_fn_by_sample, g_strdup(sample_token), g_strdup(fn));
  }

  // --- Pick scene and walk sample chain
  JsonObject *scene_obj = json_array_get_object_element(scenes, self->scene_index);
  if (!scene_obj || !json_object_has_member(scene_obj, "first_sample_token")) {
    g_set_error(err, g_quark_from_string("nuscenesrc"), 5, "Scene missing first_sample_token");
    goto fail_tables;
  }

  const gchar *first_sample_token = json_object_get_string_member(scene_obj, "first_sample_token");
  if (!first_sample_token || !*first_sample_token) {
    g_set_error(err, g_quark_from_string("nuscenesrc"), 6, "Invalid first_sample_token");
    goto fail_tables;
  }

  self->frames = g_ptr_array_new_with_free_func(g_free);
  const gchar *cur = first_sample_token;

  while (cur && *cur) {
    gchar *rel_fn = g_hash_table_lookup(sd_fn_by_sample, cur);
    if (rel_fn) {
      gchar *full = g_build_filename(self->dataroot, rel_fn, NULL);
      g_ptr_array_add(self->frames, full);
    }
    gchar *next = g_hash_table_lookup(sample_next, cur);
    if (!next || !*next) break;
    cur = next;
  }

  g_hash_table_destroy(sd_fn_by_sample);
  g_hash_table_destroy(sample_next);

  g_free(meta_path);
  g_free(scene_json);
  g_free(sample_json);
  g_free(sample_data_json);

  if (self->frames->len == 0) {
    g_set_error(err, g_quark_from_string("nuscenesrc"), 7,
                "No frames collected (cam=%s). Check CAM name / dataset.",
                self->cam ? self->cam : "(null)");
    return FALSE;
  }

  return TRUE;

fail_tables:
  g_hash_table_destroy(sd_fn_by_sample);
  g_hash_table_destroy(sample_next);
fail:
  g_free(meta_path);
  g_free(scene_json);
  g_free(sample_json);
  g_free(sample_data_json);
  return FALSE;
}

static GstCaps* gst_nuscenesrc_get_caps(GstBaseSrc *bsrc, GstCaps *filter) {
  GstNuSceneSrc *self = GST_NUSCENESRC(bsrc);
  GstCaps *caps = gst_caps_new_simple(
    "image/jpeg",
    "framerate", GST_TYPE_FRACTION, (self->fps > 0 ? self->fps : 12), 1,
    NULL
  );
  if (filter) {
    GstCaps *icaps = gst_caps_intersect_full(filter, caps, GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref(caps);
    return icaps;
  }
  return caps;
}

static gboolean gst_nuscenesrc_start(GstBaseSrc *bsrc) {
  GstNuSceneSrc *self = GST_NUSCENESRC(bsrc);

  if (!self->dataroot || !*self->dataroot) {
    GST_ERROR_OBJECT(self, "dataroot property is required");
    return FALSE;
  }

  if (!self->meta) self->meta = g_strdup("v1.0-test");
  if (!self->cam)  self->cam  = g_strdup("CAM_BACK");
  if (self->fps <= 0) self->fps = 12;

  self->duration = gst_util_uint64_scale_int(GST_SECOND, 1, self->fps);
  self->next_pts = 0;
  self->frame_idx = 0;

  GError *err = NULL;
  if (!build_frame_list(self, &err)) {
    GST_ERROR_OBJECT(self, "Failed to build frame list: %s", err ? err->message : "unknown");
    if (err) g_error_free(err);
    return FALSE;
  }

  // This source is NOT live; it pushes with timestamps.
  gst_base_src_set_live(bsrc, FALSE);
  gst_base_src_set_format(bsrc, GST_FORMAT_TIME);

  return TRUE;
}

static gboolean gst_nuscenesrc_stop(GstBaseSrc *bsrc) {
  GstNuSceneSrc *self = GST_NUSCENESRC(bsrc);
  if (self->frames) {
    g_ptr_array_free(self->frames, TRUE);
    self->frames = NULL;
  }
  self->frame_idx = 0;
  self->next_pts = 0;
  return TRUE;
}

static GstFlowReturn gst_nuscenesrc_create(GstPushSrc *psrc, GstBuffer **outbuf) {
  GstNuSceneSrc *self = GST_NUSCENESRC(psrc);

  if (!self->frames || self->frames->len == 0) {
    return GST_FLOW_EOS;
  }

  if (self->frame_idx >= self->frames->len) {
    if (self->loop) {
      self->frame_idx = 0;
      self->next_pts = 0;
    } else {
      return GST_FLOW_EOS;
    }
  }

  const gchar *path = g_ptr_array_index(self->frames, self->frame_idx);
  self->frame_idx++;

  gchar *data = NULL;
  gsize size = 0;
  GError *err = NULL;

  if (!g_file_get_contents(path, &data, &size, &err)) {
    GST_WARNING_OBJECT(self, "Failed to read %s: %s", path, err ? err->message : "unknown");
    if (err) g_error_free(err);
    // skip this frame (try next)
    return GST_FLOW_OK;
  }

  GstBuffer *buf = gst_buffer_new_allocate(NULL, size, NULL);
  GstMapInfo map;
  gst_buffer_map(buf, &map, GST_MAP_WRITE);
  memcpy(map.data, data, size);
  gst_buffer_unmap(buf, &map);

  g_free(data);

  GST_BUFFER_PTS(buf) = self->next_pts;
  GST_BUFFER_DTS(buf) = GST_CLOCK_TIME_NONE;
  GST_BUFFER_DURATION(buf) = self->duration;

  self->next_pts += self->duration;

  *outbuf = buf;
  return GST_FLOW_OK;
}

static void gst_nuscenesrc_set_property(GObject *object, guint prop_id,
                                       const GValue *value, GParamSpec *pspec) {
  GstNuSceneSrc *self = GST_NUSCENESRC(object);

  switch (prop_id) {
    case PROP_DATAROOT:
      g_free(self->dataroot);
      self->dataroot = g_value_dup_string(value);
      break;
    case PROP_META:
      g_free(self->meta);
      self->meta = g_value_dup_string(value);
      break;
    case PROP_CAM:
      g_free(self->cam);
      self->cam = g_value_dup_string(value);
      break;
    case PROP_FPS:
      self->fps = g_value_get_int(value);
      break;
    case PROP_LOOP:
      self->loop = g_value_get_boolean(value);
      break;
    case PROP_SCENE_INDEX:
      self->scene_index = g_value_get_int(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void gst_nuscenesrc_get_property(GObject *object, guint prop_id,
                                       GValue *value, GParamSpec *pspec) {
  GstNuSceneSrc *self = GST_NUSCENESRC(object);

  switch (prop_id) {
    case PROP_DATAROOT:
      g_value_set_string(value, self->dataroot);
      break;
    case PROP_META:
      g_value_set_string(value, self->meta);
      break;
    case PROP_CAM:
      g_value_set_string(value, self->cam);
      break;
    case PROP_FPS:
      g_value_set_int(value, self->fps);
      break;
    case PROP_LOOP:
      g_value_set_boolean(value, self->loop);
      break;
    case PROP_SCENE_INDEX:
      g_value_set_int(value, self->scene_index);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void gst_nuscenesrc_finalize(GObject *object) {
  GstNuSceneSrc *self = GST_NUSCENESRC(object);
  g_free(self->dataroot);
  g_free(self->meta);
  g_free(self->cam);
  G_OBJECT_CLASS(gst_nuscenesrc_parent_class)->finalize(object);
}

static void gst_nuscenesrc_class_init(GstNuSceneSrcClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS(klass);
  GstBaseSrcClass *basesrc_class = GST_BASE_SRC_CLASS(klass);
  GstPushSrcClass *pushsrc_class = GST_PUSH_SRC_CLASS(klass);

  gobject_class->set_property = gst_nuscenesrc_set_property;
  gobject_class->get_property = gst_nuscenesrc_get_property;
  gobject_class->finalize = gst_nuscenesrc_finalize;

  g_object_class_install_property(gobject_class, PROP_DATAROOT,
    g_param_spec_string("dataroot", "Dataset root",
                        "nuScenes dataset root folder",
                        NULL,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(gobject_class, PROP_META,
    g_param_spec_string("meta", "Meta folder",
                        "Meta folder name (e.g. v1.0-test) or absolute path",
                        "v1.0-test",
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(gobject_class, PROP_CAM,
    g_param_spec_string("cam", "Camera channel",
                        "Camera channel (e.g. CAM_BACK)",
                        "CAM_BACK",
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(gobject_class, PROP_FPS,
    g_param_spec_int("fps", "FPS",
                     "Output framerate",
                     1, 120, 12,
                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(gobject_class, PROP_LOOP,
    g_param_spec_boolean("loop", "Loop",
                         "Loop playback",
                         FALSE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(gobject_class, PROP_SCENE_INDEX,
    g_param_spec_int("scene-index", "Scene index",
                     "Which scene to play (0-based index in scene.json)",
                     0, 1000000, 0,
                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_static_metadata(element_class,
    "nuScenes JPEG source", "Source/Video",
    "Pushes nuScenes camera JPEG frames as a timed stream",
    "You <you@example.com>");

  // src pad template
  GstCaps *caps = gst_caps_new_simple("image/jpeg", NULL);
  gst_element_class_add_static_pad_template(element_class,
    & (GstStaticPadTemplate) GST_STATIC_PAD_TEMPLATE(
      "src", GST_PAD_SRC, GST_PAD_ALWAYS,
      GST_STATIC_CAPS("image/jpeg")
    )
  );
  gst_caps_unref(caps);

  basesrc_class->start = gst_nuscenesrc_start;
  basesrc_class->stop  = gst_nuscenesrc_stop;
  basesrc_class->get_caps = gst_nuscenesrc_get_caps;

  pushsrc_class->create = gst_nuscenesrc_create;
}

static void gst_nuscenesrc_init(GstNuSceneSrc *self) {
  self->fps = 12;
  self->loop = FALSE;
  self->scene_index = 0;
  self->frames = NULL;
  self->frame_idx = 0;
  self->next_pts = 0;
  self->duration = GST_SECOND / 12;
}

// plugin entry
static gboolean plugin_init(GstPlugin *plugin) {
  return gst_element_register(plugin, "nuscenesrc", GST_RANK_NONE, GST_TYPE_NUSCENESRC);
}

GST_PLUGIN_DEFINE(
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  nuscenesrc,
  "nuScenes JPEG Source",
  plugin_init,
  "1.0",
  "LGPL",
  "nuscenesrc",
  "local"
)
