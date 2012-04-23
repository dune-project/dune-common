# add dune-common version from dune.module to config.h
macro(dune_module_information MODULE_DIR)
  file(READ "${MODULE_DIR}/dune.module" DUNE_MODULE)

  # find version string
  string(REGEX MATCH "Version:[ ]*[^ \n]+" MODULE_LINE "${DUNE_MODULE}")
  if(NOT MODULE_LINE)
    message("${MODULE_DIR}/dune.module is missing a version." FATAL_ERROR)
  endif(NOT MODULE_LINE)

  string(REGEX REPLACE ".*Version:[ ]*([^ \n]+).*" "\\1" MODULE_LINE "${DUNE_MODULE}")
  string(REGEX REPLACE "([0-9]).*" "\\1" DUNE_VERSION_MAJOR "${DUNE_MOD_VERSION}")
  string(REGEX REPLACE "[0-9]*\\.([0-9]).*" "\\1" DUNE_VERSION_MINOR "${DUNE_MOD_VERSION}")
  string(REGEX REPLACE "[0-9]*\\.[0-9]*\\.([0-9]).*" "\\1" DUNE_VERSION_REVISION "${DUNE_MOD_VERSION}")

  # remove false matches in version string
  string(REGEX MATCH "[^0-9]" NON_NUMBER_CHARACTER "${DUNE_VERSION_MINOR}")
  if(NON_NUMBER_CHARACTER)
    set(DUNE_VERSION_MINOR "0")
  endif(NON_NUMBER_CHARACTER)
  string(REGEX MATCH "[^0-9]" NON_NUMBER_CHARACTER "${DUNE_VERSION_REVISION}")
  if(NON_NUMBER_CHARACTER)
    set(DUNE_VERSION_REVISION "0")
  endif(NON_NUMBER_CHARACTER)

  # find strings for module name, maintainer
  # 1. Check for line starting with Module
  string(REGEX MATCH "Module:[ ]*[^ \n]+" MODULE_LINE "${DUNE_MODULE}")
  if(NOT MODULE_LINE)
    message("${MODULE_DIR}/dune.module is missing a module name." FATAL_ERROR)
  endif(NOT MODULE_LINE)

  # extract Module name
  string(REGEX REPLACE ".*Module:[ ]*([^ \n]+).*" "\\1" DUNE_MOD_NAME "${MODULE_LINE}")

  # 2. Check for line starting with Maintainer
  string(REGEX MATCH "Maintainer:[ ]*[^ \n]+" MODULE_LINE "${DUNE_MODULE}")
  if(NOT MODULE_LINE)
    message("${MODULE_DIR}/dune.module is missing a maintainer." FATAL_ERROR)
  endif(NOT MODULE_LINE)

  # extract Maintainer
  string(REGEX REPLACE ".*Maintainer:[ ]*([^ \n]+).*" "\\1" DUNE_MAINTAINER "${MODULE_LINE}")

  # 3. Check for line starting with Depends
  string(REGEX MATCH "Depends:[ ]*[^ \n]+" MODULE_LINE "${DUNE_MODULE}")

  # extract dependencies if the line exists.
  if(MODULE_LINE)
    string(REGEX REPLACE ".*Depends:[ ]*([^ \n]+).*" "\\1" DUNE_DEPENDS "${DUNE_MODULE}")
  endif(MODULE_LINE)

  # set module version
  set("${DUNE_MOD_NAME}_VERSION"          "${DUNE_MOD_VERSION}")
  set("${DUNE_MOD_NAME}_VERSION_MAJOR"    "${DUNE_VERSION_MAJOR}")
  set("${DUNE_MOD_NAME}_VERSION_MINOR"    "${DUNE_VERSION_MINOR}")
  set("${DUNE_MOD_NAME}_VERSION_REVISION" "${DUNE_VERSION_REVISION}")

  #foreach( module $DEPENDENCIES
endmacro(dune_module_information)

macro(dune_project)
  dune_module_information(${CMAKE_SOURCE_DIR})
  set(ProjectName            "${DUNE_MOD_NAME}")
  set(ProjectVersion         "${DUNE_MOD_VERSION}")
  set(ProjectMaintainerEmail "${DUNE_MAINTAINER}")
  project(${ProjectName} C CXX)# Fortran)
  include(LanguageSupport)
  workaround_9220(Fortran Fortran_Works)
  if(Fortran_Works)
    enable_language(Fortran OPTIONAL)
  endif(Fortran_Works)
endmacro(dune_project MODULE_DIR)
