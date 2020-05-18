# FindMProtect.cmake
#
# Finds the MProtect library
#
# This will define the following variables
#
#    MProtect_FOUND
#
# and the following imported targets
#
#     MProtect::MProtect
#
# Author: Simon Praetorius <simon.praetorius@tu-dresden.de>

# text for feature summary
include(FeatureSummary)
set_package_properties("MProtect" PROPERTIES
  DESCRIPTION "Set protection on a region of memory"
)

enable_language(C)
mark_as_advanced(MMAN_INCLUDE_DIR)

find_path(MMAN_INCLUDE_DIR sys/mman.h
  NO_DEFAULT_PATH)
find_path(MMAN_INCLUDE_DIR sys/mman.h)

if (MMAN_INCLUDE_DIR)
  include(CheckCSourceCompiles)

  set(CMAKE_REQUIRED_INCLUDES ${MMAN_INCLUDE_DIR})
  check_c_source_compiles("
    #include <sys/mman.h>
    int main(void){
      mprotect(0,0,PROT_NONE);
    }" MPROTECT_WORKS)
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MProtect
  DEFAULT_MSG
  MMAN_INCLUDE_DIR MPROTECT_WORKS
)

if (MProtect_FOUND AND NOT TARGET MProtect::MProtect)
  add_library(MProtect::MProtect INTERFACE IMPORTED)
  target_include_directories(MProtect::MProtect INTERFACE ${MMAN_INCLUDE_DIR})
endif ()
