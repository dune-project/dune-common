# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DuneModuleInformation
---------------------

Collect information about Dune Modules.

.. cmake:command:: dune_module_information

  .. code-block:: cmake

    dune_module_information(MODULE_DIR [QUIET])

  Parse ``<MODULE_DIR>/dune.module`` and provide that information. If the
  second argument is ``<QUIET>`` no status message is printed.

#]=======================================================================]
include_guard(GLOBAL)

include(DuneUtilities)


# add dune-common version from dune.module to config.h
# optional second argument is verbosity
macro(dune_module_information MODULE_DIR)
  # find version strings
  extract_line("Version:" MODULE_LINE "${MODULE_DIR}/dune.module")
  if(NOT MODULE_LINE MATCHES ".+")
    message(FATAL_ERROR "${MODULE_DIR}/dune.module is missing a version.")
  endif()

  string(REGEX REPLACE ".*Version:[ ]*([^ \n]+).*" "\\1" DUNE_MOD_VERSION "${MODULE_LINE}")
  extract_major_minor_version("${DUNE_MOD_VERSION}" DUNE_VERSION)

  # find strings for module name, maintainer
  # 1. Check for line starting with Module
  extract_line("Module:" DUNE_MOD_NAME "${MODULE_DIR}/dune.module")
  if(NOT DUNE_MOD_NAME)
    message(FATAL_ERROR "${MODULE_DIR}/dune.module is missing a module name.")
  endif()

  # 2. Check for line starting with Maintainer
  extract_line("Maintainer:" DUNE_MAINTAINER "${MODULE_DIR}/dune.module")
  if(NOT DUNE_MAINTAINER)
    message(FATAL_ERROR "${MODULE_DIR}/dune.module is missing a maintainer.")
  endif()

  # 3. Check for line starting with Depends
  extract_line("Depends:" ${DUNE_MOD_NAME}_DEPENDS "${MODULE_DIR}/dune.module")
  if(${DUNE_MOD_NAME}_DEPENDS)
    split_module_version(${${DUNE_MOD_NAME}_DEPENDS} ${DUNE_MOD_NAME}_DEPENDS_MODULE ${DUNE_MOD_NAME}_DEPENDS_VERSION)
    foreach(_mod ${${DUNE_MOD_NAME}_DEPENDS})
      set(${_mod}_REQUIRED REQUIRED)
    endforeach()
    convert_deps_to_list(${DUNE_MOD_NAME}_DEPENDS)
    if(NOT ("${ARGV1}" STREQUAL QUIET))
      message(STATUS "Dependencies for ${DUNE_MOD_NAME}: ${${DUNE_MOD_NAME}_DEPENDS}")
    endif()
  endif()

  # 4. Check for line starting with Suggests
  extract_line("Suggests:" ${DUNE_MOD_NAME}_SUGGESTS "${MODULE_DIR}/dune.module")
  if(${DUNE_MOD_NAME}_SUGGESTS)
    split_module_version(${${DUNE_MOD_NAME}_SUGGESTS} ${DUNE_MOD_NAME}_SUGGESTS_MODULE ${DUNE_MOD_NAME}_SUGGESTS_VERSION)
    convert_deps_to_list(${DUNE_MOD_NAME}_SUGGESTS)
    if(NOT ("${ARGV1}" STREQUAL QUIET))
      message(STATUS "Suggestions for ${DUNE_MOD_NAME}: ${${DUNE_MOD_NAME}_SUGGESTS}")
    endif()
  endif()

  dune_module_to_uppercase(DUNE_MOD_NAME_UPPERCASE ${DUNE_MOD_NAME})

  # 5. Check for optional meta data
  extract_line("Author:" ${DUNE_MOD_NAME_UPPERCASE}_AUTHOR "${MODULE_DIR}/dune.module")
  extract_line("Description:" ${DUNE_MOD_NAME_UPPERCASE}_DESCRIPTION "${MODULE_DIR}/dune.module")
  extract_line("URL:" ${DUNE_MOD_NAME_UPPERCASE}_URL "${MODULE_DIR}/dune.module")
  extract_line("Python-Requires:" ${DUNE_MOD_NAME_UPPERCASE}_PYTHON_REQUIRES "${MODULE_DIR}/dune.module")

  # set module version
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION          "${DUNE_MOD_VERSION}")
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION_MAJOR    "${DUNE_VERSION_MAJOR}")
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION_MINOR    "${DUNE_VERSION_MINOR}")
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION_REVISION "${DUNE_VERSION_REVISION}")
endmacro(dune_module_information)


# ------------------------------------------------------------------------
# Internal macros and functions
# ------------------------------------------------------------------------


# Read and parse a line in the dune.module file
macro(extract_line HEADER  OUTPUT FILE_NAME)
  set(REGEX "^${HEADER}[ ]*[^\\n]+")
  file(STRINGS "${FILE_NAME}" OUTPUT1 REGEX "${REGEX}")
  if(OUTPUT1)
    set(REGEX "^[ ]*${HEADER}[ ]*(.+)[ ]*$")
    string(REGEX REPLACE ${REGEX} "\\1" ${OUTPUT} "${OUTPUT1}")
  else(OUTPUT1)
    set(OUTPUT OUTPUT-NOTFOUND)
  endif()
endmacro(extract_line)


# Convert a string with spaces in a list which is a string with semicolon
function(convert_deps_to_list var)
  string(REGEX REPLACE "([a-zA-Z0-9\\)]) ([a-zA-Z0-9])" "\\1;\\2" ${var} ${${var}})
  set(${var} ${${var}} PARENT_SCOPE)
endfunction(convert_deps_to_list var)


# split list of modules, potentially with version information
# into list of modules and list of versions
macro(split_module_version STRING MODULES VERSIONS)
  set(REGEX "[a-zA-Z0-9-]+[ ]*(\\([ ]*([^ ]+)?[ ]*[^ ]+[ ]*\\))?")
  string(REGEX MATCHALL "${REGEX}" matches "${STRING}")
  set(${MODULES} "")
  set(${VERSIONS} "")
  foreach(i ${matches})
    string(REGEX REPLACE "^([a-zA-Z0-9-]+).*$" "\\1" mod ${i})
    string(REGEX MATCH "\\([ ]*(([^ ]+)?[ ]*[^ ]+)[ ]*\\)" have_version
      ${i})
    if(have_version)
      string(REGEX REPLACE "^\\([ ]*([^ ]*[ ]*[^ ]+)[ ]*\\)$" "\\1"
        version ${have_version})
      else()
        set(version " ") # Mark as no version requested.
        # Having a space is mandatory as we will append it to a list
        # and an empty string will not be treated as entry we append to it.
      endif()
    list(APPEND ${MODULES} ${mod})
    list(APPEND ${VERSIONS} ${version})
  endforeach()
endmacro(split_module_version)


# extracts major, minor, and revision from version string
function(extract_major_minor_version version_string varname)
  string(REGEX REPLACE "([0-9]+).*" "\\1" ${varname}_MAJOR "${version_string}")
  string(REGEX REPLACE "[0-9]+\\.([0-9]+).*" "\\1" ${varname}_MINOR "${version_string}")
  string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" ${varname}_REVISION "${version_string}")
  set(${varname}_MAJOR "${${varname}_MAJOR}" PARENT_SCOPE) # make variable accessible in parent scope

  # remove false matches in version string and export to parent scop
  string(REGEX MATCH "[^0-9]" NON_NUMBER_CHARACTER "${${varname}_MINOR}")
  if(NON_NUMBER_CHARACTER)
    set(${varname}_MINOR "0" PARENT_SCOPE)
  else()
    set(${varname}_MINOR ${${varname}_MINOR} PARENT_SCOPE)
  endif()
  string(REGEX MATCH "[^0-9]" NON_NUMBER_CHARACTER "${${varname}_REVISION}")
  if(NON_NUMBER_CHARACTER)
    set(${varname}_REVISION "0"  PARENT_SCOPE)
  else()
    set(${varname}_REVISION ${${varname}_REVISION} PARENT_SCOPE)
  endif()
endfunction(extract_major_minor_version version_string varname)
