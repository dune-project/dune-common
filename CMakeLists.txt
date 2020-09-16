cmake_minimum_required(VERSION 3.13)
project(dune-common LANGUAGES C CXX)

# make sure our own modules are found
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

# set the script dir for the macros.
set(DUNE_COMMON_SCRIPT_DIR ${PROJECT_SOURCE_DIR}/cmake/scripts)

#include the dune macros
include(DuneMacros)

# start a dune project with information from dune.module
dune_project()

# add subdirectories to execute CMakeLists.txt there
add_subdirectory(bin)
add_subdirectory(cmake)
add_subdirectory(doc)
add_subdirectory(dune)
add_subdirectory(lib)
add_subdirectory(share)

# if Python bindings are enabled, include necessary sub directories.
if(DUNE_ENABLE_PYTHONBINDINGS)
  add_subdirectory(python)
  dune_python_install_package(PATH "python")
endif()

# finalize the dune project, e.g. generating config.h etc.
finalize_dune_project()
