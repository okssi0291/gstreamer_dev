cmake_minimum_required(VERSION 3.16)
project(gst_woodscape_dataset_loader_cpp LANGUAGES C CXX)

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
# Plugin: libgstwoodscape_dataset_loader.so
# ----------------------------
add_library(gstwoodscape_dataset_loader SHARED gstwood_dataset_loader.cpp)

target_include_directories(gstwoodscape_dataset_loader PRIVATE ${GST_INCLUDE_DIRS})
target_link_directories(gstwoodscape_dataset_loader PRIVATE ${GST_LIBRARY_DIRS})
target_link_libraries(gstwoodscape_dataset_loader PRIVATE ${GST_LIBRARIES})

# Needed when not using autotools/meson: these are used by GST_PLUGIN_DEFINE
target_compile_definitions(gstwoodscape_dataset_loader PRIVATE
  PACKAGE="woodscape_dataset_loader"
  PACKAGE_NAME="woodscape_dataset_loader"
  GST_PACKAGE_ORIGIN="example.com"
)

# Ensure the output name is exactly libgstwoodscape_dataset_loader.so
set_target_properties(gstwoodscape_dataset_loader PROPERTIES
  OUTPUT_NAME "gstwoodscape_dataset_loader"
  PREFIX "lib"
)

# ----------------------------
# App: woodscape_run_pipeline
# ----------------------------
add_executable(woodscape_run_pipeline main.cpp)

target_include_directories(woodscape_run_pipeline PRIVATE ${GST_INCLUDE_DIRS})
target_link_directories(woodscape_run_pipeline PRIVATE ${GST_LIBRARY_DIRS})
target_link_libraries(woodscape_run_pipeline PRIVATE ${GST_LIBRARIES})

# ----------------------------
# Convenience: run target
# ----------------------------
# Runs with GST_PLUGIN_PATH pointing at the build directory
add_custom_target(woodscape_run
  COMMAND ${CMAKE_COMMAND} -E env
          GST_PLUGIN_PATH=$<TARGET_FILE_DIR:gstwoodscape_dataset_loader>
          $<IF:$<CONFIG:Debug>,GST_DEBUG=woodscape_dataset_loader:7,GST_DEBUG=>
          $<TARGET_FILE:run_pipeline>
  DEPENDS gstwoodscape_dataset_loader woodscape_run_pipeline
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Running pipeline with GST_PLUGIN_PATH set to build output dir"
)
