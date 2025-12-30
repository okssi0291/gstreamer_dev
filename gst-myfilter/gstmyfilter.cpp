#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include <iostream>

/* When compiling as C++ with GCC/Clang, this avoids missing macro warnings */
#ifndef PACKAGE
#define PACKAGE "myfilter"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "myfilter"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "example.com"
#endif

/* ============================================================
 * GObject type definitions (still GObject-style)
 * ============================================================ */

typedef struct _GstMyFilter {
    GstBaseTransform parent;
} GstMyFilter;

typedef struct _GstMyFilterClass {
    GstBaseTransformClass parent_class;
} GstMyFilterClass;

G_DEFINE_TYPE(GstMyFilter, gst_myfilter, GST_TYPE_BASE_TRANSFORM)

/* ============================================================
 * Pad templates
 * ============================================================ */

static GstStaticPadTemplate sink_template =
GST_STATIC_PAD_TEMPLATE(
    "sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("video/x-raw, format=RGB")
);

static GstStaticPadTemplate src_template =
GST_STATIC_PAD_TEMPLATE(
    "src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("video/x-raw, format=RGB")
);

/* ============================================================
 * Buffer processing (this is where you can do real C++ work)
 * ============================================================ */

static GstFlowReturn
gst_myfilter_transform_ip(GstBaseTransform * /*base*/, GstBuffer *buf)
{
    GstMapInfo map;
    if (!gst_buffer_map(buf, &map, GST_MAP_READWRITE))
        return GST_FLOW_ERROR;

    // map.data: uint8_t*
    // map.size: size_t

    // Example: no-op (do nothing)
    // Put your C++ processing here.
    std::cout << "Processing buffer of size " << map.size << " bytes.\n";

    gst_buffer_unmap(buf, &map);
    return GST_FLOW_OK;
}

/* ============================================================
 * Class initialization
 * ============================================================ */

static void
gst_myfilter_class_init(GstMyFilterClass *klass)
{
    auto *element_class = GST_ELEMENT_CLASS(klass);
    auto *base_class    = GST_BASE_TRANSFORM_CLASS(klass);

    gst_element_class_add_pad_template(
        element_class,
        gst_static_pad_template_get(&sink_template)
    );
    gst_element_class_add_pad_template(
        element_class,
        gst_static_pad_template_get(&src_template)
    );

    gst_element_class_set_static_metadata(
        element_class,
        "MyFilter",
        "Filter/Video",
        "Simple custom GStreamer video filter (C++ build)",
        "You <you@email.com>"
    );

    base_class->transform_ip = GST_DEBUG_FUNCPTR(gst_myfilter_transform_ip);
    base_class->passthrough_on_same_caps = TRUE;
}

/* ============================================================
 * Instance initialization
 * ============================================================ */

static void
gst_myfilter_init(GstMyFilter * /*self*/)
{
    // per-instance init
}

/* ============================================================
 * Plugin entry point
 * ============================================================ */

static gboolean
plugin_init(GstPlugin *plugin)
{
    return gst_element_register(
        plugin,
        "myfilter",
        GST_RANK_NONE,
        gst_myfilter_get_type()
    );
}

/* ============================================================
 * Plugin definition
 * ============================================================ */

GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    myfilter,
    "My custom GStreamer filter (C++ build)",
    plugin_init,
    "1.0",
    "LGPL",
    PACKAGE_NAME,
    GST_PACKAGE_ORIGIN
)
