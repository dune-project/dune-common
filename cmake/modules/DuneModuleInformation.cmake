#[=======================================================================[.rst:
DuneModuleInformation
---------------------

Initialize information about the dune module.

  dune_module_information(<module_dir> [QUIET])

Parse ``<module_dir>/dune.module`` and provide that information.
If the second argument is QUIET no status message is printed.

#]=======================================================================]

include_guard(GLOBAL)

include(Utilities)

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

  # set module version
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION          "${DUNE_MOD_VERSION}")
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION_MAJOR    "${DUNE_VERSION_MAJOR}")
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION_MINOR    "${DUNE_VERSION_MINOR}")
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION_REVISION "${DUNE_VERSION_REVISION}")

  # configure CPack
  set(CPACK_PACKAGE_NAME "${DUNE_MOD_NAME}")
  set(CPACK_PACKAGE_VERSION "${DUNE_VERSION_MAJOR}.${DUNE_VERSION_MINOR}.${DUNE_VERSION_REVISION}")
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
  set(CPACK_SOURCE_IGNORE_FILES "${CMAKE_BINARY_DIR}" "\\\\.svn" "\\\\.git" ".*/*\\\\.gitignore")
endmacro(dune_module_information)
