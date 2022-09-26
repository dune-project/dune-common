# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# .. cmake_variable:: ENABLE_HEADERCHECK
#
#    Set this variable to TRUE if you want to use the CMake
#    reimplementation of the old autotools feaure :code:`make headercheck`.
#    There has been a couple of issues with this implementation in
#    the past, so it was deactivated by default.
#
include_guard(GLOBAL)

# sets up a global property with the names of all header files
# in the module and a global target depending on all checks
macro(setup_headercheck)
  #glob for headers
  file(GLOB_RECURSE all_headers "*.hh")
  # strip hidden files
  string(REGEX REPLACE "[^;]*/\\.[^;/]*\\.hh;?" "" headers "${all_headers}")
  set_property(GLOBAL PROPERTY headercheck_list ${headers})

  #define headercheck target
  dune_module_path(MODULE dune-common RESULT scriptdir SCRIPT_DIR)
  if(NOT TARGET headercheck)
    add_custom_target(headercheck ${CMAKE_COMMAND}
      -DENABLE_HEADERCHECK=${ENABLE_HEADERCHECK}
      -P ${scriptdir}/FinalizeHeadercheck.cmake
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  endif()
endmacro(setup_headercheck)

# these macros are used to exclude headers from make headercheck
# call this from a CMakeLists.txt file with a list of headers in that directory
macro(exclude_from_headercheck)
  #make this robust to argument being passed with or without ""
  string(REGEX REPLACE "[\ \n]+([^\ ])" ";\\1" list ${ARGV0})
  set(list "${list};${ARGV}")
  get_property(headerlist GLOBAL PROPERTY headercheck_list)
  foreach(item ${list})
    list(REMOVE_ITEM headerlist "${CMAKE_CURRENT_SOURCE_DIR}/${item}")
  endforeach()
  set_property(GLOBAL PROPERTY headercheck_list ${headerlist})
endmacro(exclude_from_headercheck)

macro(exclude_dir_from_headercheck)
  file(GLOB list RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.hh")
  exclude_from_headercheck(${list})
endmacro(exclude_dir_from_headercheck)

macro(exclude_subdir_from_headercheck DIRNAME)
  file(GLOB_RECURSE exlist "${CMAKE_CURRENT_SOURCE_DIR}/${DIRNAME}/*.hh")
  get_property(headerlist GLOBAL PROPERTY headercheck_list)
  foreach(item ${exlist})
    list(REMOVE_ITEM headerlist "${item}")
  endforeach()
  set_property(GLOBAL PROPERTY headercheck_list ${headerlist})
endmacro(exclude_subdir_from_headercheck)

macro(exclude_all_but_from_headercheck)
  file(GLOB excllist RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.hh")
  #make this robust to argument being passed with or without ""
  string(REGEX REPLACE "[\ \n]+([^\ \n])" ";\\1" list ${ARGV0})
  set(list "${list};${ARGV}")
  foreach(item ${list})
    list(REMOVE_ITEM excllist ${item})
  endforeach()
  exclude_from_headercheck(${excllist})
endmacro(exclude_all_but_from_headercheck)

# configure all headerchecks
macro(finalize_headercheck)
  if(ENABLE_HEADERCHECK)
    get_property(headerlist GLOBAL PROPERTY headercheck_list)
    foreach(header ${headerlist})
      #do some name conversion
      string(REGEX REPLACE ".*/([^/]*)" "\\1" simple ${header})
      string(REPLACE ${PROJECT_SOURCE_DIR} "" rel ${header})
      string(REGEX REPLACE "(.*)/[^/]*" "\\1" relpath ${rel})
      string(REGEX REPLACE "/" "_" targname ${rel})

      #generate the headercheck .cc file
      file(WRITE ${CMAKE_BINARY_DIR}/headercheck/${rel}.cc "#ifdef HAVE_CONFIG_H\n#include<config.h>\n#endif\n#include<${simple}>\n#include<${simple}>\nint main(){return 0;}")

      # add target for the check of current header, this is implemented as a library
      # to prevent CMake from automatically trying to link the target, functionality
      # of macro try_compile() is unfortunately not available due to it not being scriptable.
      add_library(headercheck_${targname} STATIC EXCLUDE_FROM_ALL
        ${CMAKE_BINARY_DIR}/headercheck/${rel}.cc)
      add_dependencies(headercheck headercheck_${targname})

      #add PKG_ALL_FLAGS and the directory where the header is located
      set_property(TARGET headercheck_${targname}
        APPEND_STRING PROPERTY COMPILE_FLAGS "-DHEADERCHECK -I${PROJECT_SOURCE_DIR}${relpath} -I${CMAKE_BINARY_DIR}")
      set_property(TARGET headercheck_${targname} PROPERTY ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/headercheck/${relpath}")
      add_dune_all_flags(headercheck_${targname})
      unset(headercheck_${targname}_LIB_DEPENDS CACHE)
    endforeach(header ${headerlist})
  endif()
endmacro(finalize_headercheck)
