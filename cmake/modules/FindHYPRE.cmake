# FindHYPRE.cmake
#
# Finds the HYPRE library
#
# This will define the following variables
#
#    HYPRE_FOUND
#    HYPRE_INCLUDE_DIR
#    HYPRE_LIBRARIES
#
# and the following imported targets
#
#     HYPRE::HYPRE
#
# Author: Simon Praetorius <simon.praetorius@tu-dresden.de>


mark_as_advanced(HYPRE_FOUND HYPRE_INCLUDE_DIR HYPRE_LIBRARIES)

find_path(HYPRE_INCLUDE_DIR HYPRE.h
  PATHS
    ENV EBROOTHYPRE
  PATH_SUFFIXES
    hypre include include/hypre
  NO_DEFAULT_PATH)
find_path(HYPRE_INCLUDE_DIR HYPRE.h
  PATH_SUFFIXES hypre include include/hypre)

find_library(HYPRE_LIBRARY HYPRE
  PATHS
    ENV EBROOTHYPRE
  PATH_SUFFIXES
    lib
  NO_DEFAULT_PATH)
find_library(HYPRE_LIBRARY HYPRE
  PATH_SUFFIXES lib)

if (HYPRE_LIBRARY)
  set(HYPRE_LIBRARIES ${HYPRE_LIBRARY})
  get_filename_component(HYPRE_LIBRARY_DIR ${HYPRE_LIBRARY} DIRECTORY)

  file(GLOB HYPRE_LIB_NAMES LIST_DIRECTORIES false RELATIVE ${HYPRE_LIBRARY_DIR} "${HYPRE_LIBRARY_DIR}/*HYPRE_*.*")
  foreach(_lib_name_long ${HYPRE_LIB_NAMES})
    string(REGEX REPLACE "^(lib)?HYPRE_([a-zA-Z_]+)([0-9.-]*)[.][a-zA-Z]+$" "\\2" _lib_name "${_lib_name_long}")
    set(LIB_VAR "_lib_${_lib_name}")
    find_library(${LIB_VAR} "HYPRE_${_lib_name}" HINTS ${HYPRE_LIBRARY_DIR} NO_DEFAULT_PATH)
    if (${LIB_VAR})
      list(APPEND HYPRE_LIBRARIES ${${LIB_VAR}})
    endif ()
  endforeach()
endif (HYPRE_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HYPRE
  REQUIRED_VARS HYPRE_INCLUDE_DIR HYPRE_LIBRARIES
  )

# text for feature summary
include(FeatureSummary)
set_package_properties("HYPRE" PROPERTIES
  DESCRIPTION "high performance preconditioners"
  )


if(HYPRE_FOUND AND NOT TARGET HYPRE::HYPRE)
  add_library(HYPRE::HYPRE INTERFACE IMPORTED)

  set_target_properties(HYPRE::HYPRE PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${HYPRE_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES "${HYPRE_LIBRARIES}"
  )
endif()
