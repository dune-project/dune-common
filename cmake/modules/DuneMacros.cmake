MACRO(dune_module_to_uppercase _upper _module)
  string(TOUPPER "${_module}" ${_upper})
  string(REPLACE "-" "_" ${_upper} "${${_upper}}")
ENDMACRO(dune_module_to_uppercase _upper _module)

# add dune-common version from dune.module to config.h
macro(dune_module_information MODULE_DIR)
  file(READ "${MODULE_DIR}/dune.module" DUNE_MODULE)

  # find version string
  string(REGEX MATCH "Version:[ ]*[^ \n]+" MODULE_LINE "${DUNE_MODULE}")
  if(NOT MODULE_LINE)
    message("${MODULE_DIR}/dune.module is missing a version." FATAL_ERROR)
  endif(NOT MODULE_LINE)

  string(REGEX REPLACE ".*Version:[ ]*([^ \n]+).*" "\\1" DUNE_MOD_VERSION "${MODULE_LINE}")
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
    string(REGEX REPLACE ".*Depends:[ ]*([^ \n]+).*" "\\1" DUNE_DEPENDS "${MODULE_LINE}")
  endif(MODULE_LINE)

  dune_module_to_uppercase(DUNE_MOD_NAME_UPPERCASE ${DUNE_MOD_NAME})

  # set module version
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION         "${DUNE_MOD_VERSION}")
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION_MAJOR    "${DUNE_VERSION_MAJOR}")
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION_MINOR    "${DUNE_VERSION_MINOR}")
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION_REVISION "${DUNE_VERSION_REVISION}")
endmacro(dune_module_information)

MACRO(dune_create_dependency_tree)
  # TODO Create full dependency tree from ${DEPENDENCIES}
  set(DEPENDENCY_TREE ${DUNE_DEPENDS})
ENDMACRO(dune_create_dependency_tree _immediates)

macro(dune_module_to_macro _macro_name _dune_module)
  set(${_macro_name} "")
  set(_rest "${_dune_module}")
  string(FIND "${_rest}" "-" _found)
  while(_found GREATER -1)
    string(REGEX REPLACE "([^-]*)-.*" "\\1" _first_part
      "${_rest}")
    string(REGEX REPLACE "[^-]*-(.*)" "\\1" _rest
      "${_rest}")
    string(SUBSTRING "${_first_part}" 0 1 _first_letter)
    string(SUBSTRING "${_first_part}" 1 -1 _rest_first_part)
    string(TOUPPER "${_first_letter}" _first_letter)
    set(${_macro_name} "${${_macro_name}}${_first_letter}${_rest_first_part}")
    string(FIND "${_rest}" "-" _found)
  endwhile(_found GREATER -1)
  string(LENGTH "${_rest}" _length)
  string(SUBSTRING "${_rest}" 0 1 _first_letter)
  string(SUBSTRING "${_rest}" 1 -1 _rest)
  string(TOUPPER "${_first_letter}" _first_letter)
  set(${_macro_name} "${${_macro_name}}${_first_letter}${_rest}")
endmacro(dune_module_to_macro _macro_name _dune_module)

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
  include_directories("/\${CMAKE_CURRENT_SOURCE_DIR}")
  add_definitions(-DHAVE_CONFIG_H)

  # Search for MPI and set the relevant variables.
  include(DuneMPI)


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
  include(DuneDoc)

  # activate testing the DUNE way
  include(DuneTests)

  # activate pkg-config
  include(DunePkgConfig)

  dune_create_dependency_tree()

  foreach(_mod ${DEPENDENCY_TREE})
    # Search for a cmake files containing tests and directives
    # specific to this module
    dune_module_to_macro(_cmake_mod_name "${_mod}")
    set(_macro "${_cmake_mod_name}Macros")
    set(_mod_cmake _mod_cmake-NOTFOUND) # Prevent false positives due to caching
    find_file(_mod_cmake ${_macro}.cmake ${CMAKE_MODULE_PATH}
      NO_DEFAULT_PATH)
    if(_mod_cmake)
      message(STATUS "Performing tests specific to ${DUNE_MOD_NAME} from file ${_mod_cmake}.")
      include(${_mod_cmake})
    endif(_mod_cmake)
    # Find the module
    find_package(${_cmake_mod_name})
    # set includes
    dune_module_to_uppercase(_upper_case "${_mod}")
    include_directories("${${_cmake_mod_name}_INCLUDE_DIRS}")
    message(STATUS "Setting ${_cmake_mod_name}_LIBRARIES=${${_cmake_mod_name}_LIBRARIES}")
    if(${_cmake_mod_name}_LIBRARIES)
      foreach(_lib ${${_cmake_mod_name}_LIBRARIES})
	list(APPEND DUNE_DEFAULT_LIBS "${_lib}")
      endforeach(_lib ${${_cmake_mod_name}_LIBRARIES})
    endif(${_cmake_mod_name}_LIBRARIES)
  endforeach(_mod DEPENDENCY_TREE)

  # Search for cmake files containing tests and directives
  # specific to this module
  dune_module_to_macro(_macro ${DUNE_MOD_NAME})
  set(DUNE_MOD_NAME_CMAKE "${_macro}")
  set(_macro "${_macro}Macros")
  set(_mod_cmake _mod_cmake-NOTFOUND)  # Prevent false positives due to caching
  find_file(_mod_cmake ${_macro}.cmake ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/cmake/modules NO_DEFAULT_PATH)
  if(_mod_cmake)
    message(STATUS " Performing tests specific to ${DUNE_MOD_NAME} from file ${_mod_cmake}.")
    include(${_mod_cmake})
  else(_mod_cmake)
    message(STATUS "There are no tests for module ${DUNE_MOD_NAME}.")
  endif(_mod_cmake)
endmacro(dune_project MODULE_DIR)

MACRO(dune_regenerate_config_cmake)
  set(CONFIG_H_CMAKE_FILE "${CMAKE_BINARY_DIR}/config.h.cmake")
  if(EXISTS ${CMAKE_SOURCE_DIR}/config.h.cmake)
    file(READ ${CMAKE_SOURCE_DIR}/config.h.cmake _file)
    string(REGEX MATCH
      "/[\\*/][ ]*begin[ ]+${DUNE_MOD_NAME}.*\\/[/\\*][ ]*end[ ]*${DUNE_MOD_NAME}[^\\*]*\\*/"
      _tfile "${_file}")
  endif(EXISTS ${CMAKE_SOURCE_DIR}/config.h.cmake)
  # overwrite file with new content
  file(WRITE ${CONFIG_H_CMAKE_FILE} "/* config.h.  Generated from config.h.cmake by CMake.
   It was generated from config.h.cmake which in turn is generated automatically
   from the config.h.cmake files of modules this module depends on. */"
   )

 # add previous module specific section
 file(APPEND ${CONFIG_H_CMAKE_FILE} "\n${_tfile}")
 foreach(_dep  ${DEPENDENCY_TREE})
   dune_module_to_macro(_dep_macro ${_dep})
   foreach(_mod_conf_file ${${_dep_macro}_PREFIX}/config.h.cmake
       ${${_dep_macro}_PREFIX}/share/${_dep}/config.h.cmake)
     if(EXISTS ${_mod_conf_file})
       file(READ "${_mod_conf_file}"  _file)
       string(REGEX REPLACE
	 ".*/\\*[ ]*begin[ ]+${_dep}[^\\*]*\\*/(.*)/[/\\*][ ]*end[ ]*${_dep}[^\\*]*\\*/" "\\1"
	 _tfile "${_file}")
       # strip the private section
       string(REGEX REPLACE "(.*)/[\\*][ ]*begin private.*/[\\*][ ]*end[ ]*private[^\\*]\\*/(.*)" "\\1\\2" _file "${_tfile}")
       file(APPEND ${CONFIG_H_CMAKE_FILE} "${_file}")
     endif(EXISTS ${_mod_conf_file})
   endforeach()
 endforeach(_dep  DEPENDENCY_TREE)
ENDMACRO(dune_regenerate_config_cmake)

# macro that should be called at the end of the top level CMakeLists.txt.
# Namely it creates  config.h and the cmake-config files,
# some install directives and export th module.
MACRO(finalize_dune_project)

  #create cmake-config files
  configure_file(
    ${PROJECT_SOURCE_DIR}/${DUNE_MOD_NAME_CMAKE}Config.cmake.in
    ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME_CMAKE}Config.cmake @ONLY)

  configure_file(
    ${PROJECT_SOURCE_DIR}/${DUNE_MOD_NAME_CMAKE}Version.cmake.in
    ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME_CMAKE}Version.cmake @ONLY)

  #install dune.module file
  install(FILES dune.module DESTINATION lib/dunecontrol/${DUNE_MOD_NAME})

  #install cmake-config files
  install(FILES ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME_CMAKE}Config.cmake
    ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME_CMAKE}Version.cmake
    DESTINATION lib/cmake/${DUNE_MOD_NAME_CMAKE})

  #install config.h
  install(FILES config.h.cmake DESTINATION share/${DUNE_MOD_NAME})
  export(PACKAGE ${DUNE_MOD_NAME})

  if("${ARGC}" EQUAL "1")
    message("Adding custom target for config.h generation")
    dune_regenerate_config_cmake()
    # add a target to generate config.h.cmake
    add_custom_target(OUTPUT config.h.cmake
      COMMAND dune_regenerate_config_cmake()
      DEPENDS stamp-regenerate-config-h)
    # actually write the config.h file to disk
    # using generated file
    configure_file(${CMAKE_CURRENT_BINARY_DIR}/config.h.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/config.h)
  else("${ARGC}" EQUAL "1")
    message("Not adding custom target for config.h generation")
    # actually write the config.h file to disk
    configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
  endif("${ARGC}" EQUAL "1")
ENDMACRO(finalize_dune_project)

MACRO(target_link_dune_default_libraries _target)
  message("libs ${DUNE_DEFAULT_LIBS}")
  foreach(_lib ${DUNE_DEFAULT_LIBS})
    message("lib=${_lib}")
    target_link_libraries(${_target} ${_lib})
  endforeach(_lib ${DUNE_DEFAULT_LIBS})
ENDMACRO(target_link_dune_default_libraries)


MACRO(dune_common_script_dir _script_dir)
  if("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
    set(${_script_dir} ${CMAKE_SOURCE_DIR}/cmake/scripts)
  else("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
    set(${_script_dir} ${DuneCommon_SCRIPT_DIR})
  endif("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
ENDMACRO(dune_common_script_dir)
