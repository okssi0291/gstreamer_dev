// main.cpp
#include <gst/gst.h>
#include <iostream>

int main(int argc, char *argv[]) {
  gst_init(&argc, &argv);

  // If you build the plugin locally (libgstnuscenesrc.so) and don't install it system-wide,
  // set GST_PLUGIN_PATH in your shell before running:
  //   export GST_PLUGIN_PATH=/path/to/your/plugin/build/dir

  GError *err = nullptr;

  // const char *pipeline_desc =
  //     "nuscenesrc "
  //     "dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset "
  //     "meta=v1.0-test cam=CAM_BACK fps=12 scene-index=-1 loop=true "
  //     "! jpegdec "
  //     "! videoconvert "
  //     "! autovideosink sync=false";
  
  const char *pipeline_desc = "compositor name=comp background=black \
    sink_0::xpos=0    sink_0::ypos=0   \
    sink_1::xpos=640  sink_1::ypos=0   \
    sink_2::xpos=1280 sink_2::ypos=0   \
    sink_3::xpos=0    sink_3::ypos=360 \
    sink_4::xpos=640  sink_4::ypos=360 \
    sink_5::xpos=1280 sink_5::ypos=360 \
  ! videoconvert ! autovideosink sync=false \
  \
  nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_FRONT fps=12 scene-index=-1 loop=true \
  ! jpegdec ! videoconvert ! videoscale \
  ! video/x-raw,width=640,height=360,pixel-aspect-ratio=1/1 \
  ! queue ! comp.sink_0 \
  \
  nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_FRONT_LEFT fps=12 scene-index=-1 loop=true \
  ! jpegdec ! videoconvert ! videoscale \
  ! video/x-raw,width=640,height=360,pixel-aspect-ratio=1/1 \
  ! queue ! comp.sink_1 \
  \
  nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_FRONT_RIGHT fps=12 scene-index=-1 loop=true \
  ! jpegdec ! videoconvert ! videoscale \
  ! video/x-raw,width=640,height=360,pixel-aspect-ratio=1/1 \
  ! queue ! comp.sink_2 \
  \
  nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_BACK fps=12 scene-index=-1 loop=true \
  ! jpegdec ! videoconvert ! videoscale \
  ! video/x-raw,width=640,height=360,pixel-aspect-ratio=1/1 \
  ! queue ! comp.sink_3 \
  \
  nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_BACK_LEFT fps=12 scene-index=-1 loop=true \
  ! jpegdec ! videoconvert ! videoscale \
  ! video/x-raw,width=640,height=360,pixel-aspect-ratio=1/1 \
  ! queue ! comp.sink_4 \
  \
  nuscenesrc dataroot=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset meta=v1.0-test cam=CAM_BACK_RIGHT fps=12 scene-index=-1 loop=true \
  ! jpegdec ! videoconvert ! videoscale \
  ! video/x-raw,width=640,height=360,pixel-aspect-ratio=1/1 \
  ! queue ! comp.sink_5";

  
  // const char *pipeline_desc =
  //   "gst-launch-1.0 -v "
  //   "multifilesrc location=/home/okssi/workspace/dataset_sdb1/nuscenes/dataset/samples/CAM_FRONT/n015-2018-07-25-16-15-50+0800__CAM_FRONT__1532506690912460.jpg loop=true "
  //   "! jpegdec "
  //   "! videoconvert "
  //   "! autovideosink sync=false";

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
