# Core DUNE module for CMake.
#
# Provides the following macros:
#
# dune_module_to_upper_case
#
# Converts a module name given by _module into an uppercase string
# _upper where all dashes (-) are replaced by underscores (_)
# Example: dune-common -> DUNE_COMMON
#
# dune_module_information(MODULE_DIR)
#
# Parse ${MODULE_DIR}/dune.module and provide that information.
#
#
# dune_project()
#
#  macro that should be called near the beginning of the top level
# CMakeLists.txt. Please make sure to call
#
# project(dune-module-name)
#
# at the very beginning of that file.
# Namely it sets up the module, defines basic variables and manages
# depedencies.
# Don't forget to call finalize_dune_project afterwards.
#
#
# dune_create_dependency_tree()
#
# Creates the dependency tree of the module.
#
# dune_module_to_macro(_macro_name, _dune_module)
#
# Converts a module name given by _dune_module into a string _macro_name
# where all dashes (-) are removed and letters after a dash are capitalized
# Example: dune-grid-howto -> DuneGridHowto
#
# _macro_name: variable where the name will be stored.
# _dune_module: the name of the dune module.
#
#
# dune_regenerate_config_cmake()
#
# Create a new config.h.cmake file in ${CMAKE_CURRENT_BINARY_DIR) that consists
# of entries from ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake and includes non-private
# entries from the files config.h.cmake files of all dependent modules.
# Finally config.h is created from config.h.cmake.
#
#
# dune_add_library(BASENAME)
#
# Creates shared and static libraries with the same basename.
# BASENAME is the basename of the library.
# On Unix this creates lib<BASENAME>.so and lib<BASENAME>.a.
# The libraries will be built in ${PROJECT_BINARY_DIR}/lib
# and exported for usage in other modules.
#
#
# finalize_dune_project()
#
# macro that should be called at the end of the top level CMakeLists.txt.
# Namely it creates config.h and the cmake-config files,
# some install directives and exports the module.
#
#
# dune_target_link_libraries(BASENAME, LIBRARIES)
#
# Link libraries to the static and shared version of
# library BASENAME
#

enable_language(C) # Enable C to skip CXX bindings for some tests.

# Converts a module name given by _module into an uppercase string
# _upper where all dashes (-) are replaced by underscores (_)
# Example: dune-common -> DUNE_COMMON
macro(dune_module_to_uppercase _upper _module)
  string(TOUPPER "${_module}" ${_upper})
  string(REPLACE "-" "_" ${_upper} "${${_upper}}")
endmacro(dune_module_to_uppercase _upper _module)

# add dune-common version from dune.module to config.h
macro(dune_module_information MODULE_DIR)
  file(READ "${MODULE_DIR}/dune.module" DUNE_MODULE)

  # find version string
  string(REGEX MATCH "Version:[ ]*[^ \n]+" MODULE_LINE "${DUNE_MODULE}")
  if(NOT MODULE_LINE)
    message(FATAL_ERROR "${MODULE_DIR}/dune.module is missing a version.")
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
    message(FATAL_ERROR "${MODULE_DIR}/dune.module is missing a module name.")
  endif(NOT MODULE_LINE)

  # extract Module name
  string(REGEX REPLACE ".*Module:[ ]*([^ \n]+).*" "\\1" DUNE_MOD_NAME "${MODULE_LINE}")

  # 2. Check for line starting with Maintainer
  string(REGEX MATCH "Maintainer:[ ]*[^ \n]+" MODULE_LINE "${DUNE_MODULE}")
  if(NOT MODULE_LINE)
    message(FATAL_ERROR "${MODULE_DIR}/dune.module is missing a maintainer.")
  endif(NOT MODULE_LINE)

  # extract Maintainer
  string(REGEX REPLACE ".*Maintainer:[ ]*([^ \n]+).*" "\\1" DUNE_MAINTAINER "${MODULE_LINE}")

  # 3. Check for line starting with Depends
  string(REGEX MATCH "Depends:[ ]*[^ \n]+" MODULE_LINE "${DUNE_MODULE}")

  # extract dependencies if the line exists.
  if(MODULE_LINE)
    string(REGEX REPLACE ".*Depends:[ ]*([^ \n]+).*" "\\1" DUNE_DEPENDS "${MODULE_LINE}")
  endif(MODULE_LINE)

  string(REGEX REPLACE ".*Suggests:[ ]*([^ \n]+).*" "\\1" DUNE_SUGGESTS "${MODULE_LINE}")

  dune_module_to_uppercase(DUNE_MOD_NAME_UPPERCASE ${DUNE_MOD_NAME})

  # set module version
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION         "${DUNE_MOD_VERSION}")
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION_MAJOR    "${DUNE_VERSION_MAJOR}")
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION_MINOR    "${DUNE_VERSION_MINOR}")
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION_REVISION "${DUNE_VERSION_REVISION}")
endmacro(dune_module_information)

macro(dune_create_dependency_tree)
  # TODO Create full dependency tree from ${DEPENDENCIES}
  set(DEPENDENCY_TREE ${DUNE_DEPENDS})
endmacro(dune_create_dependency_tree _immediates)

# Converts a module name given by _dune_module into a string _macro_name
# where all dashes (-) are removed and letters after a dash are capitalized
# Example: dune-grid-howto -> DuneGridHowto
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

# macro that should be called near the begin of the top level CMakeLists.txt.
# Namely it sets up the module, defines basic variables and manages
# depedencies.
# Don't forget to call finalize_dune_project afterwards.
macro(dune_project)
  # Set the flags
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
  set(CMAKE_C_FLAGS_DEBUG "-g -O0")
  set(CMAKE_CXX_FLAGS_RELEASE "-funroll-loops -O3")
  set(CMAKE_C_FLAGS_RELEASE "-funroll-loops -O3")

  # extract information from dune.module
  dune_module_information(${CMAKE_SOURCE_DIR})
  set(ProjectName            "${DUNE_MOD_NAME}")
  set(ProjectVersion         "${DUNE_MOD_VERSION}")
  set(ProjectMaintainerEmail "${DUNE_MAINTAINER}")

  # assert the project names matches
  if(NOT (ProjectName STREQUAL CMAKE_PROJECT_NAME))
    message(FATAL_ERROR "Module name from dune.module does not match the name given in CMakeLists.txt.")
  endif(NOT (ProjectName STREQUAL CMAKE_PROJECT_NAME))

  # optional Fortran support
  include(LanguageSupport)
  workaround_9220(Fortran Fortran_Works)
  if(Fortran_Works)
    enable_language(Fortran OPTIONAL)
  endif(Fortran_Works)

  option(DUNE_USE_ONLY_STATIC_LIBS "If set to ON, we will force static linkage everywhere" OFF)
  if(DUNE_USE_ONLY_STATIC_LIBS)
    set(_default_enable_shared OFF)
    set(_default_enable_static ON)
  else(DUNE_USE_ONLY_STATIC_LIBS)
    set(_default_enable_shared ON)
    set(_default_enable_static OFF)
  endif(DUNE_USE_ONLY_STATIC_LIBS)
  option(BUILD_SHARED_LIBS "If set to ON, shared libs will be built" ${_default_enable_shared})
  option(DUNE_BUILD_BOTH_LIBS "If set to ON, shared and static libs will be built"
    ${_default_enable_static})

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
  endif(DUNE_USE_ONLY_STATIC_LIBS)

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
    find_package(${_mod})
    # set includes
    dune_module_to_uppercase(_upper_case "${_mod}")
    include_directories("${${_mod}_INCLUDE_DIRS}")
    message(STATUS "Setting ${_mod}_LIBRARIES=${${_mod}_LIBRARIES}")
    if(${_mod}_LIBRARIES)
      foreach(_lib ${${_mod}_LIBRARIES})
        list(APPEND DUNE_DEFAULT_LIBS "${_lib}")
      endforeach(_lib ${${_mod}_LIBRARIES})
    endif(${_mod}_LIBRARIES)
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

# create a new config.h file and overwrite the existing one
macro(dune_regenerate_config_cmake)
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
   foreach(_mod_conf_file ${${_dep}_PREFIX}/config.h.cmake
       ${${_dep_macro}_PREFIX}/share/${_dep}/config.h.cmake)
     if(EXISTS ${_mod_conf_file})
       file(READ "${_mod_conf_file}" _file)
       string(REGEX REPLACE
         ".*/\\*[ ]*begin[ ]+${_dep}[^\\*]*\\*/(.*)/[/\\*][ ]*end[ ]*${_dep}[^\\*]*\\*/" "\\1"
         _tfile "${_file}")
       # strip the private section
       string(REGEX REPLACE "(.*)/[\\*][ ]*begin private.*/[\\*][ ]*end[ ]*private[^\\*]\\*/(.*)" "\\1\\2" _file "${_tfile}")
       file(APPEND ${CONFIG_H_CMAKE_FILE} "${_file}")
     endif(EXISTS ${_mod_conf_file})
   endforeach()
 endforeach(_dep DEPENDENCY_TREE)
endmacro(dune_regenerate_config_cmake)

# macro that should be called at the end of the top level CMakeLists.txt.
# Namely it creates config.h and the cmake-config files,
# some install directives and exports the module.
macro(finalize_dune_project)
  #create cmake-config files for build tree
  configure_file(
    ${PROJECT_SOURCE_DIR}/${DUNE_MOD_NAME}-config.cmake.in
    ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}-config.cmake @ONLY)

  #create cmake-config files for installation tree
  configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/pkg/${DUNE_MOD_NAME}-config.cmake.in
    ${PROJECT_BINARY_DIR}/cmake/pkg/${DUNE_MOD_NAME}-config.cmake @ONLY)

  configure_file(
    ${PROJECT_SOURCE_DIR}/${DUNE_MOD_NAME}-version.cmake.in
    ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}-version.cmake @ONLY)

  #install dune.module file
  install(FILES dune.module DESTINATION lib/dunecontrol/${DUNE_MOD_NAME})

  #install cmake-config files
  install(FILES ${PROJECT_BINARY_DIR}/cmake/pkg/${DUNE_MOD_NAME}-config.cmake
    ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}-version.cmake
    DESTINATION lib/cmake/${DUNE_MOD_NAME})

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

  test_dep()
endmacro(finalize_dune_project)

macro(target_link_dune_default_libraries _target)
  message("libs ${DUNE_DEFAULT_LIBS}")
  foreach(_lib ${DUNE_DEFAULT_LIBS})
    message("lib=${_lib}")
    target_link_libraries(${_target} ${_lib})
  endforeach(_lib ${DUNE_DEFAULT_LIBS})
endmacro(target_link_dune_default_libraries)

# Gets path to the common Dune CMake scripts
macro(dune_common_script_dir _script_dir)
  if("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
    set(${_script_dir} ${CMAKE_SOURCE_DIR}/cmake/scripts)
  else("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
    set(${_script_dir} ${dune-common_SCRIPT_DIR})
  endif("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
endmacro(dune_common_script_dir)

# Gets path to the common Dune CMake scripts source
macro(dune_common_script_source_dir _script_dir)
  if("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
    set(${_script_dir} ${CMAKE_SOURCE_DIR}/cmake/scripts)
  else("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
    set(${_script_dir} ${dune-ommon_SCRIPT_SOURCE_DIR})
  endif("${CMAKE_PROJECT_NAME}" STREQUAL "dune-common")
endmacro(dune_common_script_source_dir)

# Creates shared and static libraries with the same basename.
# dune_add_lib(basename)
# Basename is the basename of the library.
# On Unix this creates lib<basename>.so and lib<basename>.a.
# The libraries will be built in ${PROJECT_BINARY_DIR}/lib
# and exported for usage in other modules.
macro(dune_add_library basename)
  #create lib
  add_library(${basename} ${ARGN})

  # Build library in ${PROJECT_BINARY_DIR}/lib
  set_target_properties(${basename} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

  set(_created_libs ${basename})

  if(DUNE_BUILD_BOTH_LIBS)
    if(BUILD_SHARED_LIBS)
      #create static lib
      add_library(${basename}-static STATIC ${ARGN})
      # make sure both libs have the same name.
      set_target_properties(${basename}-static PROPERTIES
        OUTPUT_NAME ${basename}
        ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
      list(APPEND _created_libs ${basename}-static)
    else(BUILD_SHARED_LIBS)
      #create shared libs
      add_library(${basename}-shared SHARED ${ARGN})
      set_target_properties(${basename}-shared PROPERTIES
        OUTPUT_NAME ${basename}
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
      list(APPEND _created_libs ${basename}-shared)
    endif(BUILD_SHARED_LIBS)
  endif(DUNE_BUILD_BOTH_LIBS)

  # install targets to use the libraries in other modules.
  install(TARGETS ${_created_libs}
    EXPORT ${DUNE_MOD_NAME}-targets DESTINATION lib)
  install(EXPORT ${DUNE_MOD_NAME}-targets
    DESTINATION lib/cmake)

  # export libraries for use in build tree
  export(TARGETS ${_created_libs}
    FILE ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}-targets.cmake)
endmacro(dune_add_library basename sources)

macro(dune_target_link_libraries basename libraries)
  target_link_libraries(${basename} ${libraries})
  if(DUNE_BUILD_BOTH_LIBS)
    if(BUILD_SHARED_LIBS)
      target_link_libraries(${basename}-static ${libraries})
    else(BUILD_SHARED_LIBS)
      target_link_libraries(${basename}-shared ${libraries})
    endif(BUILD_SHARED_LIBS)
  endif(DUNE_BUILD_BOTH_LIBS)
endmacro(dune_target_link_libraries basename libraries)
