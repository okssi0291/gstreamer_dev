cmake_minimum_required(VERSION 3.16)

project(OkssiLab LANGUAGES C CXX)

# -------------------------------------------------
# Global settings (apply to all subprojects)
# -------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Optional: enable compile_commands.json (nice for VSCode, clangd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -------------------------------------------------
# Add subprojects
# -------------------------------------------------
add_subdirectory(gst-myfilter)
add_subdirectory(gst-woodscape_dataset_loader)

# -------------------------------------------------
# (Future) Add more subprojects here
# -------------------------------------------------
# add_subdirectory(another_project)
# add_subdirectory(libs/some_lib)
