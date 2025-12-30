cmake_minimum_required(VERSION 3.16)
project(gst_myfilter_cpp LANGUAGES C CXX)

# ----------------------------
# Build type (Debug/Release)
# ----------------------------
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------
# Find GStreamer (via pkg-config)
# ----------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-base-1.0)

message(STATUS "GStreamer include dirs: ${GST_INCLUDE_DIRS}")
message(STATUS "GStreamer library dirs: ${GST_LIBRARY_DIRS}")
message(STATUS "GStreamer libs: ${GST_LIBRARIES}")

# ----------------------------
# Common compile flags
# ----------------------------
add_compile_options(-Wall -Wextra)

# Useful debug flags (only Debug)
add_compile_options("$<$<CONFIG:Debug>:-O0;-g3;-DDEBUG>")

# ----------------------------
# Plugin: libgstmyfilter.so
# ----------------------------
add_library(gstmyfilter SHARED gstmyfilter.cpp)

target_include_directories(gstmyfilter PRIVATE ${GST_INCLUDE_DIRS})
target_link_directories(gstmyfilter PRIVATE ${GST_LIBRARY_DIRS})
target_link_libraries(gstmyfilter PRIVATE ${GST_LIBRARIES})

# Needed when not using autotools/meson: these are used by GST_PLUGIN_DEFINE
target_compile_definitions(gstmyfilter PRIVATE
  PACKAGE="myfilter"
  PACKAGE_NAME="myfilter"
  GST_PACKAGE_ORIGIN="example.com"
)

# Ensure the output name is exactly libgstmyfilter.so
set_target_properties(gstmyfilter PROPERTIES
  OUTPUT_NAME "gstmyfilter"
  PREFIX "lib"
)

# ----------------------------
# App: run_pipeline
# ----------------------------
add_executable(run_pipeline main.cpp)

target_include_directories(run_pipeline PRIVATE ${GST_INCLUDE_DIRS})
target_link_directories(run_pipeline PRIVATE ${GST_LIBRARY_DIRS})
target_link_libraries(run_pipeline PRIVATE ${GST_LIBRARIES})

# ----------------------------
# Convenience: run target
# ----------------------------
# Runs with GST_PLUGIN_PATH pointing at the build directory
add_custom_target(run
  COMMAND ${CMAKE_COMMAND} -E env
          GST_PLUGIN_PATH=$<TARGET_FILE_DIR:gstmyfilter>
          $<IF:$<CONFIG:Debug>,GST_DEBUG=myfilter:7,GST_DEBUG=>
          $<TARGET_FILE:run_pipeline>
  DEPENDS gstmyfilter run_pipeline
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Running pipeline with GST_PLUGIN_PATH set to build output dir"
)
