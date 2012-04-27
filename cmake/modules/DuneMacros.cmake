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

  # Set the flags
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
  set(CMAKE_C_FLAGS_DEBUG "-g -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "-funroll-loops -O3")
  set(CMAKE_C_FLAGS_RELEASE "-funroll-loops -O3")

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

  option(DUNE_USE_ONLY_STATIC_LIBS "If set to ON, we will force static linkage everywhere" OFF)
  if(DUNE_USE_ONLY_STATIC_LIBS)
    set(_default_enable_shared OFF)
  else(DUNE_USE_ONLY_STATIC_LIBS)
    set(_default_enable_shared ON)
  endif(DUNE_USE_ONLY_STATIC_LIBS)
  option(BUILD_SHARED_LIBS "If set to ON, shared libs will be built" ${_default_enable_shared})

  if(DUNE_USE_ONLY_STATIC_LIBS)
    # Use only static libraries.
    # We do this by overriding the library suffixes.
    set( BLA_STATIC 1)
    set( _dune_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
    if (WIN32)
      set(CMAKE_FIND_LIBRARY_SUFFIXES .lib ${CMAKE_FIND_LIBRARY_SUFFIXES})
    endif (WIN32)
    if (APPLE)
      set(CMAKE_FIND_LIBRARY_SUFFIXES .lib ${CMAKE_FIND_LIBRARY_SUFFIXES})
    else (APPLE)
      set(CMAKE_FIND_LIBRARY_SUFFIXES .a )
    endif (APPLE)
  endif()

  # set required compiler flags for C++11 (former C++0x)
  find_package(CXX11Features)

  # search for headers
  include(CheckIncludeFile)
  include(CheckIncludeFileCXX)
  check_include_file("malloc.h" HAVE_MALLOC_H)
  check_include_file("stdint.h" HAVE_STDINT_H)
  check_include_file_cxx("memory" HAVE_MEMORY)
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -DHAVE_MEMORY=${HAVE_MEMORY}")

  # set include path and link path for the current project.
  include_directories("${CMAKE_SOURCE_DIR}")
  link_directories("${CMAKE_SOURCE_DIR}/lib")
  include_directories("${CMAKE_CURRENT_BINARY_DIR}")
  add_definitions(-DHAVE_CONFIG_H)

  # Search for MPI and set the relevant variables.
  include(DUNEMPI)


  # Make calling fortran routines from C/C++ possible
  if(Fortran_Works)
    include(FortranCInterface)
    FortranCInterface_VERIFY(CXX)
    # Write FC.h header containing information about
    # how to call fortran routined.
    # It will be included in config.h
    FortranCInterface_HEADER(FC.h MACRO_NAMESPACE "FC_")
  else(Fortran_Works)
    # Write empty FC.h header
    file(WRITE ${CMAKE_BINARY_DIR}/FC.h "")
  endif(Fortran_Works)

  # Create custom target for building the documentation
  # and provide macros for installing the docs and force
  # building them before.
  include(DUNEDoc)

  # activate testing the DUNE way
  include(DuneTests)

  # activate pkg-config
  include(DunePkgConfig)

  # Search for a cmake files containing tests and directives
  # specific to this module
  find_file(_mod_cmake DuneCommonMacros.cmake ${CMAKE_MODULE_PATH}
    NO_DEFAULT_PATH)
  if(_mod_cmake)
    include(DuneCommonMacros)
  endif(_mod_cmake)
endmacro(dune_project MODULE_DIR)

# macro that should be called at the end of the top level CMakeLists.txt.
# Namely it creates  config.h and the cmake-config files,
# some install directives and export th module.
MACRO(finalize_dune_project)
  # actually write the config.h file to disk
  configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

  #create cmake-config files
  configure_file(
    ${PROJECT_SOURCE_DIR}/${DUNE_MOD_NAME}-config.cmake.in
    ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}-config.cmake @ONLY)

  configure_file(
    ${PROJECT_SOURCE_DIR}/${DUNE_MOD_NAME}-version.cmake.in
    ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}-version.cmake @ONLY)

  #install dune.module file
  install(FILES dune.module DESTINATION lib/dunecontrol/${DUNE_MOD_NAME})

  #install cmake-config files
  install(FILES ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}-config.cmake
    ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}-version.cmake
    DESTINATION lib/cmake)

  export(PACKAGE ${DUNE_MOD_NAME})
ENDMACRO(finalize_dune_project)
