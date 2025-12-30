PLUGIN_NAME    := myfilter
PLUGIN_LIB     := libgstmyfilter.so
APP_NAME       := run_pipeline

PLUGIN_SRC     := gstmyfilter.cpp
APP_SRC        := main.cpp

CXX            := g++
CXXFLAGS       := -fPIC -Wall -Wextra -O2 -std=c++17
LDFLAGS        := -shared

GST_CFLAGS     := $(shell pkg-config --cflags gstreamer-1.0 gstreamer-base-1.0)
GST_LIBS       := $(shell pkg-config --libs gstreamer-1.0 gstreamer-base-1.0)

DEFS := \
	-DPACKAGE=\"$(PLUGIN_NAME)\" \
	-DPACKAGE_NAME=\"$(PLUGIN_NAME)\" \
	-DGST_PACKAGE_ORIGIN=\"example.com\"

all: $(PLUGIN_LIB) $(APP_NAME)

$(PLUGIN_LIB): $(PLUGIN_SRC)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) \
		$(DEFS) \
		$(GST_CFLAGS) \
		-o $@ $< \
		$(GST_LIBS)

$(APP_NAME): $(APP_SRC)
	$(CXX) $(CXXFLAGS) \
		$(GST_CFLAGS) \
		-o $@ $< \
		$(GST_LIBS)

run: all
	GST_PLUGIN_PATH=$(PWD) ./$(APP_NAME)

clean:
	rm -f $(PLUGIN_LIB) $(APP_NAME)

.PHONY: all run clean
