// main.cpp
#include <gst/gst.h>
#include <iostream>

int main(int argc, char *argv[]) {
  gst_init(&argc, &argv);

  // If you build the plugin locally (libgstnuscenesrc.so) and don't install it system-wide,
  // set GST_PLUGIN_PATH in your shell before running:
  //   export GST_PLUGIN_PATH=/path/to/your/plugin/build/dir

  GError *err = nullptr;

  const char *pipeline_desc =
      "nuscenesrc "
      "dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset "
      "meta=v1.0-test cam=CAM_BACK fps=12 scene-index=0 loop=true "
      "! jpegdec "
      "! videoconvert "
      "! autovideosink sync=false";

  GstElement *pipeline = gst_parse_launch(pipeline_desc, &err);
  if (!pipeline) {
    std::cerr << "Failed to create pipeline: " << (err ? err->message : "unknown") << "\n";
    if (err) g_error_free(err);
    return 1;
  }
  if (err) { // gst_parse_launch can return a pipeline with a non-fatal error
    std::cerr << "Pipeline parse warning: " << err->message << "\n";
    g_error_free(err);
    err = nullptr;
  }

  GstBus *bus = gst_element_get_bus(pipeline);

  // Start playing
  GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    std::cerr << "Failed to set pipeline to PLAYING\n";
    gst_object_unref(bus);
    gst_object_unref(pipeline);
    return 1;
  }

  // Wait until error or EOS
  bool running = true;
  while (running) {
    GstMessage *msg = gst_bus_timed_pop_filtered(
        bus, GST_CLOCK_TIME_NONE,
        (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    if (!msg) continue;

    switch (GST_MESSAGE_TYPE(msg)) {
      case GST_MESSAGE_ERROR: {
        GError *gerr = nullptr;
        gchar *debug = nullptr;
        gst_message_parse_error(msg, &gerr, &debug);
        std::cerr << "ERROR from " << GST_OBJECT_NAME(msg->src) << ": "
                  << (gerr ? gerr->message : "unknown") << "\n";
        if (debug) std::cerr << "Debug: " << debug << "\n";
        if (gerr) g_error_free(gerr);
        g_free(debug);
        running = false;
        break;
      }
      case GST_MESSAGE_EOS:
        std::cout << "EOS\n";
        running = false;
        break;
      default:
        break;
    }
    gst_message_unref(msg);
  }

  // Cleanup
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(bus);
  gst_object_unref(pipeline);
  return 0;
}
