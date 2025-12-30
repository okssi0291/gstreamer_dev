#include <gst/gst.h>
#include <iostream>

int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);

    // Scan current directory so GStreamer can discover ./libgstmyfilter.so
    GstRegistry *reg = gst_registry_get();
    gst_registry_scan_path(reg, ".");

    // Confirm element exists
    GstElementFactory *f = gst_element_factory_find("myfilter");
    if (!f) {
        std::cerr << "ERROR: Element factory 'myfilter' not found. "
                     "Check GST_PLUGIN_PATH or scan path.\n";
        return 1;
    }
    gst_object_unref(f);

    GError *error = nullptr;
    GstElement *pipeline = gst_parse_launch("videotestsrc ! myfilter ! fakesink", &error);
    if (!pipeline) {
        std::cerr << "Failed to create pipeline: "
                  << (error ? error->message : "unknown error") << "\n";
        if (error) g_error_free(error);
        return 1;
    }

    auto ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "Failed to set pipeline to PLAYING\n";
        gst_object_unref(pipeline);
        return 1;
    }

    GstBus *bus = gst_element_get_bus(pipeline);
    GstMessage *msg = gst_bus_timed_pop_filtered(
        bus,
        GST_CLOCK_TIME_NONE,
        (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS)
    );

    if (msg) {
        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
            GError *err = nullptr;
            gchar *dbg = nullptr;
            gst_message_parse_error(msg, &err, &dbg);
            std::cerr << "ERROR: " << (err ? err->message : "unknown") << "\n";
            if (dbg) std::cerr << "Debug info: " << dbg << "\n";
            if (err) g_error_free(err);
            g_free(dbg);
        } else {
            std::cout << "EOS received.\n";
        }
        gst_message_unref(msg);
    }

    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    return 0;
}
