#[=======================================================================[.rst:
DuneAddLibrary
--------------

Add a library to a Dune module.

.. cmake:command:: dune_add_library

  .. code-block:: cmake

    dune_add_library(<basename>
      [SOURCES <sources>...]
      [ADD_LIBS <targets>...]
      [COMPILE_FLAGS "<flags>;..."]
      [APPEND]
      [NO_EXPORT]
      [NO_MODULE_LIBRARY]
      [OBJECT]
    )

  Create a new library target with ``<basename>`` for the library name. On Unix
  this created ``lib<basename>.so`` or ``lib<basename>.a``.

  ``SOURCES``
    The source files from which to build the library.

  ``ADD_LIBS``
    A list of libraries that should be incorporated into this library.

  ``COMPILE_FLAGS``
    Any additional compile flags for building the library.

  ``APPEND``
    Whether the library should be appended to the exported libraries. If
    a DUNE module must make several libraries available, then first one
    must not use this option but the others have to use it. Otherwise only
    the last library will be exported as the others will be overwritten.

  ``NO_EXPORT``
    If omitted the library is exported for usage in other modules.

  ``NO_MODULE_LIBRARY``
    If omitted the library is added to the global property ``DUNE_MODULE_LIBRARIES``

  ``OBJECT``
    This feature will very likely vanish in Dune 3.0

#]=======================================================================]
include_guard(GLOBAL)


macro(dune_add_library basename)
  cmake_parse_arguments(DUNE_LIB "APPEND;NO_EXPORT;NO_MODULE_LIBRARY;OBJECT" "COMPILE_FLAGS"
    "ADD_LIBS;SOURCES" ${ARGN})
  list(APPEND DUNE_LIB_SOURCES ${DUNE_LIB_UNPARSED_ARGUMENTS})
  if(DUNE_LIB_OBJECT)
    if(DUNE_LIB_${basename}_SOURCES)
      message(FATAL_ERROR "There is already a library with the name ${basename}, "
        "but only one is allowed!")
    else()
      foreach(source ${DUNE_LIB_SOURCES})
        list(APPEND full_path_sources ${CMAKE_CURRENT_SOURCE_DIR}/${source})
      endforeach()
      # register sources, libs and flags for building the library later
      define_property(GLOBAL PROPERTY DUNE_LIB_${basename}_SOURCES
        BRIEF_DOCS "Convenience property with sources for library ${basename}. DO NOT EDIT!"
        FULL_DOCS "Convenience property with sources for library ${basename}. DO NOT EDIT!")
      set_property(GLOBAL PROPERTY DUNE_LIB_${basename}_SOURCES
        "${full_path_sources}")
      define_property(GLOBAL PROPERTY DUNE_LIB_${basename}_ADD_LIBS
        BRIEF_DOCS "Convenience property with libraries for library ${basename}. DO NOT EDIT!"
        FULL_DOCS "Convenience property with libraries for library ${basename}. DO NOT EDIT!")
      set_property(GLOBAL PROPERTY DUNE_LIB_${basename}_ADD_LIBS
        "${DUNE_LIB_ADD_LIBS}")
      define_property(GLOBAL PROPERTY DUNE_LIB_${basename}_COMPILE_FLAGS
        BRIEF_DOCS "Convenience property with compile flags for library ${basename}. DO NOT EDIT!"
        FULL_DOCS "Convenience property with compile flags for library ${basename}. DO NOT EDIT!")
      set_property(GLOBAL PROPERTY DUNE_LIB_${basename}_COMPILE_FLAGS
        "${DUNE_LIB_COMPILE_FLAGS}")
    endif()
  else(DUNE_LIB_OBJECT)
    dune_expand_object_libraries(DUNE_LIB_SOURCES DUNE_LIB_ADD_LIBS DUNE_LIB_COMPILE_FLAGS)
    #create lib
    add_library(${basename} ${DUNE_LIB_SOURCES})
    # add library to DUNE_MODULE_LIBRARIES
    if(NOT DUNE_LIB_NO_MODULE_LIBRARY)
      get_property(_prop GLOBAL PROPERTY DUNE_MODULE_LIBRARIES)
      set_property(GLOBAL PROPERTY DUNE_MODULE_LIBRARIES ${_prop} ${basename})
    endif()
    # link with specified libraries.
    if(DUNE_LIB_ADD_LIBS)
      target_link_libraries(${basename} PUBLIC "${DUNE_LIB_ADD_LIBS}")
    endif()
    if(DUNE_LIB_COMPILE_FLAGS)
      set_property(${basename} APPEND_STRING COMPILE_FLAGS
        "${DUNE_LIB_COMPILE_FLAGS}")
    endif()
    # Build library in ${PROJECT_BINARY_DIR}/lib
    set_target_properties(${basename} PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
      ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

    if(NOT DUNE_LIB_NO_EXPORT)
      # The following allows for adding multiple libs in the same
      # directory or below with passing the APPEND keyword.
      # If there are additional calls to dune_add_library in other
      # modules then you have to use APPEND or otherwise only the
      # last lib will get exported as a target.
      if(NOT _MODULE_EXPORT_USED)
        set(_MODULE_EXPORT_USED ON)
        set(_append "")
      else()
        set(_append APPEND)
      endif()
      # Allow to explicitly pass APPEND
      if(DUNE_LIB_APPEND)
        set(_append APPEND)
      endif()

      # install targets to use the libraries in other modules.
      install(TARGETS ${basename}
        EXPORT ${ProjectName}-targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
      install(EXPORT ${ProjectName}-targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${ProjectName})

      # export libraries for use in build tree
      export(TARGETS ${basename} ${_append}
        FILE ${PROJECT_BINARY_DIR}/${ProjectName}-targets.cmake)
    endif()
  endif()
endmacro(dune_add_library)


# ------------------------------------------------------------------------
# Internal macros and functions
# ------------------------------------------------------------------------


function(dune_expand_object_libraries _SOURCES_var _ADD_LIBS_var _COMPILE_FLAGS_var)
  set(_new_SOURCES "")
  set(_new_ADD_LIBS "${${_ADD_LIBS_var}}")
  set(_new_COMPILE_FLAGS "${${_COMPILE_FLAGS_var}}")
  set(_regex "_DUNE_TARGET_OBJECTS:([a-zA-Z0-9_-]+)_")
  foreach(_source ${${_SOURCES_var}})
    string(REGEX MATCH ${_regex} _matched "${_source}")
    if(_matched)
      string(REGEX REPLACE "${_regex}" "\\1" _basename  "${_source}")
      foreach(var _SOURCES _ADD_LIBS _COMPILE_FLAGS)
        get_property(_prop GLOBAL PROPERTY DUNE_LIB_${_basename}${var})
        list(APPEND _new${var} "${_prop}")
      endforeach()
    else()
      list(APPEND _new_SOURCES "${_source}")
    endif()
  endforeach()

  foreach(var _SOURCES _ADD_LIBS _COMPILE_FLAGS)
    set(${${var}_var} "${_new${var}}" PARENT_SCOPE)
  endforeach()
endfunction(dune_expand_object_libraries)
