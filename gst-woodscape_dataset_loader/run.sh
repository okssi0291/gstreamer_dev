export GST_PLUGIN_PATH="$PWD:$GST_PLUGIN_PATH"
gst-inspect-1.0 myfilter
gst-launch-1.0 videotestsrc ! myfilter ! fakesink
