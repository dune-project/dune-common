# add dune-common version from dune.module to config.h
macro(dune_module_information MODULE_DIR)
  file(READ "${MODULE_DIR}/dune.module" DUNE_MODULE)

  # find version string
  string(REGEX REPLACE ".*Version:[ ]*([^ \n]+).*" "\\1" DUNE_VERSION_STRING "${DUNE_MODULE}")
  string(REGEX REPLACE "([0-9]).*" "\\1" DUNE_VERSION_MAJOR "${DUNE_VERSION_STRING}")
  string(REGEX REPLACE "[0-9]*\\.([0-9]).*" "\\1" DUNE_VERSION_MINOR "${DUNE_VERSION_STRING}")
  string(REGEX REPLACE "[0-9]*\\.[0-9]*\\.([0-9]).*" "\\1" DUNE_VERSION_REVISION "${DUNE_VERSION_STRING}")

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
  string(REGEX REPLACE ".*Module:[ ]*([^ \n]+).*" "\\1" DUNE_MODULE_NAME "${DUNE_MODULE}")
  string(REGEX REPLACE ".*Maintainer:[ ]*([^ \n]+).*" "\\1" DUNE_MAINTAINER "${DUNE_MODULE}")
endmacro(dune_module_information)
