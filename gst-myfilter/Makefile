# =====================================================
# Project settings
# =====================================================

PLUGIN_NAME    := myfilter
PLUGIN_LIB     := libgstmyfilter.so
APP_NAME       := run_pipeline

PLUGIN_SRC     := gstmyfilter.c
APP_SRC        := main.c

CC             := gcc
CFLAGS         := -fPIC -Wall -Wextra -O2
LDFLAGS        := -shared

GST_CFLAGS     := $(shell pkg-config --cflags gstreamer-1.0 gstreamer-base-1.0)
GST_LIBS       := $(shell pkg-config --libs gstreamer-1.0 gstreamer-base-1.0)

# Required by GST_PLUGIN_DEFINE when not using autotools/meson
DEFS := \
	-DPACKAGE=\"$(PLUGIN_NAME)\" \
	-DPACKAGE_NAME=\"$(PLUGIN_NAME)\" \
	-DGST_PACKAGE_ORIGIN=\"example.com\"

# =====================================================
# Targets
# =====================================================

all: $(PLUGIN_LIB) $(APP_NAME)

# -----------------------------------------------------
# Build GStreamer plugin
# -----------------------------------------------------
$(PLUGIN_LIB): $(PLUGIN_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) \
		$(DEFS) \
		$(GST_CFLAGS) \
		-o $@ $< \
		$(GST_LIBS)

# -----------------------------------------------------
# Build application
# -----------------------------------------------------
$(APP_NAME): $(APP_SRC)
	$(CC) -Wall -Wextra -O2 \
		$(GST_CFLAGS) \
		-o $@ $< \
		$(GST_LIBS)

# -----------------------------------------------------
# Run application (with plugin path set)
# -----------------------------------------------------
run: all
	GST_PLUGIN_PATH=$(PWD) ./$(APP_NAME)

# -----------------------------------------------------
# Clean
# -----------------------------------------------------
clean:
	rm -f $(PLUGIN_LIB) $(APP_NAME)

.PHONY: all clean run
