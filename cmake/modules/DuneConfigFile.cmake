#[=======================================================================[.rst:
DuneConfigFile
--------------

Generate the inclde header config.h

  dune_regenerate_config_cmake()

Creates a new config_collected.h.cmake file in ${CMAKE_CURRENT_BINARY_DIR} that
consists of entries from ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
and includes non-private entries from the config.h.cmake files
of all dependent modules.
Finally config.h is created from config_collected.h.cmake.

#]=======================================================================]

include_guard(GLOBAL)

include(DuneModuleInformation)
include(Utilities)

macro(dune_regenerate_config_cmake)
  set(CONFIG_H_CMAKE_FILE "${PROJECT_BINARY_DIR}/config_collected.h.cmake")
  if(EXISTS ${PROJECT_SOURCE_DIR}/config.h.cmake)
    file(READ ${PROJECT_SOURCE_DIR}/config.h.cmake _file)
    string(REGEX MATCH
      "/[\\*/][ ]*begin[ ]+${PROJECT_NAME}.*\\/[/\\*][ ]*end[ ]*${PROJECT_NAME}[^\\*]*\\*/"
      _myfile "${_file}")
  endif()
  # overwrite file with new content
  file(WRITE ${CONFIG_H_CMAKE_FILE} "/* config.h.  Generated from config_collected.h.cmake by CMake.
   It was generated from config_collected.h.cmake which in turn is generated automatically
   from the config.h.cmake files of modules this module depends on. */"
   )

 # define that we found this module
 set(${PROJECT_NAME}_FOUND 1)
 foreach(_dep ${PROJECT_NAME} ${ALL_DEPENDENCIES})
   dune_module_to_uppercase(upper ${_dep})
   set(HAVE_${upper} ${${_dep}_FOUND})
   file(APPEND ${CONFIG_H_CMAKE_FILE}
     "\n\n/* Define to 1 if you have module ${_dep} available */
#cmakedefine01 HAVE_${upper}\n")
 endforeach()

 # add previous module specific section
 foreach(_dep ${ALL_DEPENDENCIES})
   foreach(_mod_conf_file ${${_dep}_PREFIX}/config.h.cmake
       ${${_dep}_PREFIX}/share/${_dep}/config.h.cmake)
     if(EXISTS ${_mod_conf_file})
       file(READ "${_mod_conf_file}" _file)
       string(REGEX REPLACE
         ".*/\\*[ ]*begin[ ]+${_dep}[^\\*]*\\*/(.*)/[/\\*][ ]*end[ ]*${_dep}[^\\*]*\\*/" "\\1"
         _tfile "${_file}")
       # strip the private section
       string(REGEX REPLACE "(.*)/[\\*][ ]*begin private.*/[\\*][ ]*end[ ]*private[^\\*]\\*/(.*)" "\\1\\2" _ttfile "${_tfile}")

       # extract the bottom section
       string(REGEX MATCH "/[\\*][ ]*begin bottom.*/[\\*][ ]*end[ ]*bottom[^\\*]\\*/" _tbottom "${_ttfile}")
       string(REGEX REPLACE ".*/\\*[ ]*begin[ ]+bottom[^\\*]*\\*/(.*)/[/\\*][ ]*end[ ]*bottom[^\\*]*\\*/" "\\1" ${_dep}_CONFIG_H_BOTTOM "${_tbottom}" )
       string(REGEX REPLACE "(.*)/[\\*][ ]*begin bottom.*/[\\*][ ]*end[ ]*bottom[^\\*]\\*/(.*)" "\\1\\2" _file  "${_ttfile}")

       # append bottom section
       if(${_dep}_CONFIG_H_BOTTOM)
         set(CONFIG_H_BOTTOM "${CONFIG_H_BOTTOM} ${${_dep}_CONFIG_H_BOTTOM}")
       endif()

       file(APPEND ${CONFIG_H_CMAKE_FILE} "${_file}")
     endif()
   endforeach()
 endforeach()
 # parse again dune.module file of current module to set PACKAGE_* variables
 dune_module_information(${PROJECT_SOURCE_DIR} QUIET)
 file(APPEND ${CONFIG_H_CMAKE_FILE} "\n${_myfile}")
 # append CONFIG_H_BOTTOM section at the end if found
 if(CONFIG_H_BOTTOM)
    file(APPEND ${CONFIG_H_CMAKE_FILE} "${CONFIG_H_BOTTOM}")
 endif()
endmacro(dune_regenerate_config_cmake)
