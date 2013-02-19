# set up project
project("dune-common" C CXX)

# general stuff
cmake_minimum_required(VERSION 2.8.6)
message("CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}|")
# make sure our own modules are found
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
message("CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")
# set the script dir for the macros.
set(DUNE_COMMON_SCRIPT_DIR "${CMAKE_SOURCE_DIR}/cmake/scripts")

#include the dune macros
include(DuneMacros)

# start a dune project with information from dune.module
dune_project()

# add subdirectories to execute CMakeLists.txt there
add_subdirectory("lib")
add_subdirectory("dune")
add_subdirectory("bin")
add_subdirectory("m4")
add_subdirectory("am")
add_subdirectory("doc")
add_subdirectory("cmake/modules")
add_subdirectory("cmake/scripts")

# finalize the dune project, e.g. generating config.h etc.
finalize_dune_project()
include(CPack)
