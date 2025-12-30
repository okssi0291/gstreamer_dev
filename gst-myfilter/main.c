#include <gst/gst.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);

    /* If your plugin is NOT installed in a standard path,
       scan the folder where libgstmyfilter.so lives. */
    {
        GstRegistry *reg = gst_registry_get();
        GError *err = NULL;

        /* Change this to your plugin folder if needed */
        const char *plugin_dir = "/home/okssi/workspace/OkssiLab/woodscape/gstreamer_dev/gst-myfilter";

        gst_registry_scan_path(reg, plugin_dir);

        /* Optional: you can confirm myfilter exists now */
        GstElementFactory *f = gst_element_factory_find("myfilter");
        if (!f) {
            g_printerr("ERROR: Element factory 'myfilter' not found. Check plugin path.\n");
            return 1;
        }
        gst_object_unref(f);
    }

    GError *error = NULL;
    GstElement *pipeline = gst_parse_launch("videotestsrc ! myfilter ! fakesink", &error);
    if (!pipeline) {
        g_printerr("Failed to create pipeline: %s\n", error ? error->message : "unknown error");
        if (error) g_error_free(error);
        return 1;
    }

    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Failed to set pipeline to PLAYING\n");
        gst_object_unref(pipeline);
        return 1;
    }

    /* Wait until EOS or ERROR */
    GstBus *bus = gst_element_get_bus(pipeline);
    GstMessage *msg = gst_bus_timed_pop_filtered(
        bus,
        GST_CLOCK_TIME_NONE,
        (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS)
    );

    if (msg) {
        switch (GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_ERROR: {
                GError *err = NULL;
                gchar *dbg = NULL;
                gst_message_parse_error(msg, &err, &dbg);
                g_printerr("ERROR: %s\n", err->message);
                if (dbg) g_printerr("Debug info: %s\n", dbg);
                g_clear_error(&err);
                g_free(dbg);
                break;
            }
            case GST_MESSAGE_EOS:
                g_print("EOS received.\n");
                break;
            default:
                break;
        }
        gst_message_unref(msg);
    }

    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    return 0;
}
