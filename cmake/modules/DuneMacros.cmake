# Core DUNE module for CMake.
#
# Provides the following macros:
#
# dune_module_to_uppercase(upper_name module_name)
#
# Converts a module name given by module_name into an uppercase string
# upper_name where all dashes (-) are replaced by underscores (_)
# Example: dune-common -> DUNE_COMMON
#
# dune_module_information(MODULE_DIR [QUIET])
#
# Parse ${MODULE_DIR}/dune.module and provide that information.
# If the second argument is QUIET no status message is printed.
#
#
# dune_project()
#
#  macro that should be called near the begin of the top level CMakeLists.txt.
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
# Creates a new config.h.cmake file in ${CMAKE_CURRENT_BINARY_DIR) that
# consists of entries from ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
# and includes non-private entries from the files config.h.cmake files
# of all dependent modules.
# Finally config.h is created from config.h.cmake.
#
#
# dune_add_library(<basename> [NO_EXPORT] [ADD_LIBS <lib1> [<lib2> ...]]
#   [OBJECT] SOURCES <source1> [<source2> ...] [COMPILE_FLAGS <flags>])
#
# Creates shared and static libraries with the same basename.
# <basename> is the basename of the library.
# On Unix this creates lib<basename>.so and lib<BASENAME>.a.
# Libraries that should be incorporate into this libary can
# be specified with the ADD_LIBS option.
# The libraries will be built in ${PROJECT_BINARY_DIR}/lib.
# If the option NO_EXPORT is omitted the library is exported
# for usage in other modules.
#
# Object libraries can now be created with dune_add_library(<target>
#  OBJECT <sources>). It will create a GLOBAL property
#  _DUNE_TARGET_OBJECTS:<target>_ that records the full path to the
#  source files. Theses can later be referenced by providing
#  _DUNE_TARGET_OBJECTS:<target>_ as one of the sources to dune_add_library
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

macro(find_dune_package module)
  if(NOT (${module}_DIR OR ${module}_ROOT OR
      "${CMAKE_PREFIX_PATH}" MATCHES ".*${module}.*"))
    string(REPLACE  ${ProjectName} ${module} ${module}_DIR
      ${PROJECT_BINARY_DIR})
  endif()
  find_package(${ARGV} NO_CMAKE_PACKAGE_REGISTRY)
endmacro(find_dune_package module)

macro(extract_line HEADER  OUTPUT FILE_CONTENT)
  set(REGEX "${HEADER}[ ]*[^\n]+")
  string(REGEX MATCH ${REGEX} OUTPUT1 "${FILE_CONTENT}")
  if(OUTPUT1)
    set(REGEX "^[ ]*${HEADER}[ ]*(.+)[ ]*$")
    string(REGEX REPLACE ${REGEX} "\\1" ${OUTPUT} "${OUTPUT1}")
  else(OUTPUT1)
    set(OUTPUT OUTPUT-NOTFOUND)
  endif(OUTPUT1)
endmacro(extract_line)

macro(split_module_version STRING MODULES VERSIONS)
  set(REGEX "[a-zA-Z-]+[ ]*(\\([ ]*([^ ]+)?[ ]*[^ ]+[ ]*\\))?")
  #set(REGEX "dune")
  string(REGEX MATCHALL "${REGEX}"  matches "${STRING}")
  set(${MODULES} "")
  set(${VERSIONS} "")
  foreach(i ${matches})
    string(REGEX REPLACE "^([a-zA-Z-]+).*$" "\\1" mod ${i})
    string(REGEX MATCH "\\([ ]*(([^ ]+)?[ ]*[^ ]+)[ ]*\\)" have_version
      ${i})
    if(have_version)
      string(REGEX REPLACE "^\\([ ]*([^ ]*[ ]*[^ ]+)[ ]*\\)$" "\\1"
        version ${have_version})
      else(have_version)
        set(version >=0.0)
      endif(have_version)
    list(APPEND ${MODULES} ${mod})
    list(APPEND ${VERSIONS} ${version})
  endforeach(i "${matches}")
endmacro(split_module_version)

function(convert_deps_to_list var)
  string(REGEX REPLACE "([a-zA-Z\\)]) ([a-zA-Z])" "\\1;\\2" ${var} ${${var}})
  set(${var} ${${var}} PARENT_SCOPE)
endfunction(convert_deps_to_list var)

# add dune-common version from dune.module to config.h
# optional second argument is verbosity
macro(dune_module_information MODULE_DIR)
  file(READ "${MODULE_DIR}/dune.module" DUNE_MODULE)

  # find version string
  extract_line("Version:" MODULE_LINE "${DUNE_MODULE}")
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
  extract_line("Module:" DUNE_MOD_NAME "${DUNE_MODULE}")
  if(NOT DUNE_MOD_NAME)
    message(FATAL_ERROR "${MODULE_DIR}/dune.module is missing a module name.")
  endif(NOT DUNE_MOD_NAME)

  # 2. Check for line starting with Maintainer
  extract_line("Maintainer:" DUNE_MAINTAINER "${DUNE_MODULE}")
  if(NOT DUNE_MAINTAINER)
    message(FATAL_ERROR "${MODULE_DIR}/dune.module is missing a maintainer.")
  endif(NOT DUNE_MAINTAINER)

  # 3. Check for line starting with Depends
  extract_line("Depends:" DUNE_DEPENDS "${DUNE_MODULE}")
  if(DUNE_DEPENDS)
    split_module_version(${DUNE_DEPENDS} DEPENDS_MODULE DEPENDS_VERSIONS)
    foreach(_mod ${DUNE_DEPENDS})
      set(${_mod}_REQUIRED REQUIRED)
    endforeach(_mod ${DUNE_DEPENDS})
    convert_deps_to_list(DUNE_DEPENDS)
    if(NOT ("${ARGV1}" STREQUAL QUIET))
      message(STATUS "Dependencies: ${DEPENDS_MODULE} (versions: ${DEPENDS_VERSIONS}) DUNE_DEPENDS=${DUNE_DEPENDS}")
    endif(NOT ("${ARGV1}" STREQUAL QUIET))
  endif(DUNE_DEPENDS)

  # 4. Check for line starting with Suggests
  extract_line("Suggests:" DUNE_SUGGESTS "${DUNE_MODULE}")
  if(DUNE_SUGGESTS)
    split_module_version(${DUNE_SUGGESTS} SUGGESTS_MODULE SUGGESTS_VERSION)
    convert_deps_to_list(DUNE_SUGGESTS)
    if(NOT ("${ARGV1}" STREQUAL QUIET))
      message(STATUS "Suggestions: ${SUGGESTS_MODULE} (versions: ${SUGGESTS_VERSIONS}) DUNE_SUGGESTS=${DUNE_SUGGESTS}")
    endif(NOT ("${ARGV1}" STREQUAL QUIET))
  endif(DUNE_SUGGESTS)

  dune_module_to_uppercase(DUNE_MOD_NAME_UPPERCASE ${DUNE_MOD_NAME})

  # set module version
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION          "${DUNE_MOD_VERSION}")
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION_MAJOR    "${DUNE_VERSION_MAJOR}")
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION_MINOR    "${DUNE_VERSION_MINOR}")
  set(${DUNE_MOD_NAME_UPPERCASE}_VERSION_REVISION "${DUNE_VERSION_REVISION}")
endmacro(dune_module_information)

macro(dune_process_dependency_leafs modules versions is_required next_level_deps
    next_level_sugs)
  # modules is not a real variable, make it one
  set(mmodules ${modules})
  list(LENGTH mmodules mlength)
  if(mlength GREATER 0)
    math(EXPR length "${mlength}-1")
    foreach(i RANGE 0 ${length})
      list(GET mmodules ${i} _mod)
      find_dune_package(${_mod} ${REQUIRED})
      if(${_mod}_MODULE_PATH)
        list(APPEND CMAKE_MODULE_PATH ${${_mod}_MODULE_PATH})
      endif(${_mod}_MODULE_PATH)
      set(${_mod}_SEARCHED ON)
      if(NOT "${is_required}" STREQUAL "")
        set(${_mod}_REQUIRED ON)
        set(${next_level_deps} ${${_mod}_DEPENDS} ${${next_level_deps}})
      else(NOT "${is_required}" STREQUAL "")
        set(${next_level_sugs} ${${_mod}_DEPENDS} ${${next_level_sugs}})
      endif(NOT "${is_required}" STREQUAL "")
      set(${next_level_sugs} ${${_mod}_SUGGESTS} ${${next_level_sugs}})
    endforeach(i RANGE 0 ${length})
  endif(mlength GREATER 0)
endmacro(dune_process_dependency_leafs)

function(remove_processed_modules modules versions is_required)
  list(LENGTH ${modules} mlength)
  if(mlength GREATER 0)
    math(EXPR length "${mlength}-1")
    foreach(i RANGE ${length} 0 -1)
      list(GET ${modules} ${i} _mod)
      if(${_mod}_SEARCHED)
        list(REMOVE_AT ${modules} ${i})
        list(REMOVE_AT ${versions} ${i})
        if(is_required AND NOT ${_mod}_REQUIRED AND NOT ${_mod}_FOUND)
          message(FATAL_ERROR "Required module ${_mod} not found!")
        endif(is_required AND NOT ${_mod}_REQUIRED AND NOT ${_mod}_FOUND)
      endif(${_mod}_SEARCHED)
    endforeach(i RANGE 0 ${length})
  endif(mlength GREATER 0)
  set(${modules} ${${modules}} PARENT_SCOPE)
  set(${versions} ${${versions}} PARENT_SCOPE)
endfunction(remove_processed_modules modules versions is_required)

macro(dune_create_dependency_leafs depends depends_versions suggests suggests_versions
    global_depends global_suggests)
  set(deps "")
  set(sugs "")
  #Process dependencies
  if(NOT "${depends}" STREQUAL "")
    dune_process_dependency_leafs("${depends}" "${depends_versions}" REQUIRED deps sugs)
  endif(NOT "${depends}" STREQUAL "")
  # Process suggestions
  if(NOT "${suggests}" STREQUAL "")
    dune_process_dependency_leafs("${suggests}" "${suggests_versions}" "" deps sugs)
  endif(NOT "${suggests}" STREQUAL "")
  split_module_version("${deps}" next_mod_depends next_depends_versions)
  #remove_processed_modules(next_mod_depends next_depends_versions REQUIRED)
  set(${global_depends} "${${global_depends}}" ${next_mod_depends})
  split_module_version("${sugs}" next_mod_suggests next_suggests_versions)
  #remove_processed_modules(next_mod_suggests next_suggests_versions "")
  set(${global_suggests} "${${global_suggests}}" ${next_mod_suggests})
  # Move to next level
  if(next_mod_suggests OR next_mod_depends)
    dune_create_dependency_leafs("${next_mod_depends}" "${next_depends_versions}"
      "${next_mod_suggests}" "${next_suggests_versions}" global_depends global_suggests)
  endif(next_mod_suggests OR next_mod_depends)
endmacro(dune_create_dependency_leafs)

macro(dune_create_dependency_tree)
  set(ALL_DEPENDENCIES "")
  if(DEPENDS_MODULE OR SUGGESTS_MODULE)
    set(global_depends ${DEPENDS_MODULE})
    set(global_suggests ${SUGGESTS_MODULE})
    foreach(_mod ${DEPENDS_MODULE})
      find_dune_package(${_mod} REQUIRED)
      if(${_mod}_MODULE_PATH)
        list(APPEND CMAKE_MODULE_PATH ${${_mod}_MODULE_PATH})
      endif(${_mod}_MODULE_PATH)
      set(${_mod}_REQUIRED ON)
    endforeach(_mod ${DEPENDS_MODULE})
    foreach(_mod ${SUGGESTS_MODULE})
      find_dune_package(${_mod})
      if(${_mod}_MODULE_PATH)
        list(APPEND CMAKE_MODULE_PATH ${${_mod}_MODULE_PATH})
      endif(${_mod}_MODULE_PATH)
      set(${_mod}_REQUIRED ON)
    endforeach(_mod ${SUGGESTS_MODULE})
    dune_create_dependency_leafs("${DEPENDS_MODULE}" "${DEPENDS_VERSIONS}"
      "${SUGGESTS_MODULE}" "${SUGGESTS_VERSIONS}" global_depends
      global_suggests)
    set(ALL_DEPENDENCIES "${global_depends}" "${global_suggests}")
  endif(DEPENDS_MODULE OR SUGGESTS_MODULE)
  # reverse ALL_DEPENDENCIES
  list(REVERSE ALL_DEPENDENCIES)
  list(REMOVE_DUPLICATES ALL_DEPENDENCIES)
  list(REMOVE_DUPLICATES CMAKE_MODULE_PATH)
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

macro(dune_process_dependency_tree DEPENDS DVERSIONS SUGGESTS SVERSIONS)
  foreach(_mod ${ALL_DEPENDENCIES})
    if(NOT ${_mod}_PROCESSED)
      # module not processed yet
      set(${_mod}_PROCESSED ${_mod})
      # Search for a cmake files containing tests and directives
      # specific to this module
      dune_module_to_macro(_cmake_mod_name "${_mod}")
      set(_macro "${_cmake_mod_name}Macros")
      set(_mod_cmake _mod_cmake-NOTFOUND) # Prevent false positives due to caching
      find_file(_mod_cmake
        ${_macro}.cmake
        ${CMAKE_MODULE_PATH}
        NO_DEFAULT_PATH)
      if(_mod_cmake)
        message(STATUS "Performing tests specific to ${_mod} from file ${_mod_cmake}.")
        include(${_mod_cmake})
      endif(_mod_cmake)
      # Find the module
      #find_package(${_mod})
      # set includes
      dune_module_to_uppercase(_upper_case "${_mod}")
      if(${_mod}_INCLUDE_DIRS)
        message(STATUS "Setting ${_mod}_INCLUDE_DIRS=${${_mod}_INCLUDE_DIRS}")
        include_directories("${${_mod}_INCLUDE_DIRS}")
      endif(${_mod}_INCLUDE_DIRS)
      if(${_mod}_LIBRARIES)
        message(STATUS "Setting ${_mod}_LIBRARIES=${${_mod}_LIBRARIES}")
        foreach(_lib ${${_mod}_LIBRARIES})
          list(INSERT DUNE_DEFAULT_LIBS 0 "${_lib}")
          list(INSERT DUNE_LIBS 0 "${_lib}")
        endforeach(_lib ${${_mod}_LIBRARIES})
      endif(${_mod}_LIBRARIES)
      message(STATUS "Dependencies for ${_mod}: ${${_mod}_DEPENDENCIES}")
    endif(NOT ${_mod}_PROCESSED)
  endforeach(_mod DEPENDENCIES)
endmacro(dune_process_dependency_tree)

# macro that should be called near the begin of the top level CMakeLists.txt.
# Namely it sets up the module, defines basic variables and manages
# depedencies.
# Don't forget to call finalize_dune_project afterwards.
macro(dune_project)
  # extract information from dune.module
  dune_module_information(${CMAKE_SOURCE_DIR})
  set(ProjectName            "${DUNE_MOD_NAME}")
  set(ProjectVersion         "${DUNE_MOD_VERSION}")
  set(ProjectMaintainerEmail "${DUNE_MAINTAINER}")

  dune_create_dependency_tree()

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
  find_package(CXX11Conditional)

  include(DuneCxaDemangle)

  # search for headers
  include(CheckIncludeFile)
  include(CheckIncludeFileCXX)
  check_include_file("malloc.h" HAVE_MALLOC_H)
  check_include_file("stdint.h" HAVE_STDINT_H)
  check_include_file_cxx("memory" HAVE_MEMORY)
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -DHAVE_MEMORY=${HAVE_MEMORY}")

  # set include path and link path for the current project.
  include_directories("${CMAKE_BINARY_DIR}")
  include_directories("${CMAKE_SOURCE_DIR}")
  link_directories("${CMAKE_SOURCE_DIR}/lib")
  include_directories("${CMAKE_CURRENT_BINARY_DIR}")
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
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

  dune_process_dependency_tree("${DEPENDS_MODULE}" "${DEPENDS_VERSION}"
    "${SUGGESTS_MODULE}" "${SUGGESTS_VERSION}")

  # Search for cmake files containing tests and directives
  # specific to this module
  dune_module_to_macro(_macro ${DUNE_MOD_NAME})
  set(DUNE_MOD_NAME_CMAKE "${_macro}")
  set(_macro "${_macro}Macros")
  set(_mod_cmake _mod_cmake-NOTFOUND)  # Prevent false positives due to caching
  find_file(_mod_cmake
    ${_macro}.cmake
    ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/cmake/modules
    NO_DEFAULT_PATH)
  if(_mod_cmake)
    set(${DUNE_MOD_NAME_CMAKE}_FOUND FOUND)
    message(STATUS " Performing tests specific to ${DUNE_MOD_NAME} from file ${_mod_cmake}.")
    include(${_mod_cmake})
  else(_mod_cmake)
    message(STATUS "There are no tests for module ${DUNE_MOD_NAME}.")
  endif(_mod_cmake)
  include(GNUInstallDirs)
  # Set variable where the cmake modules will be installed.
  # Thus the user can override it and for example install
  # directly into the CMake installation. We use a cache variable
  # that is overridden by a local variable of the same name if
  # the user does not explicitely set a value for it. Thus the value
  # will automatically change if the user changes CMAKE_INSTALL_DATAROOTDIR
  # or CMAKE_INSTALL_PREFIX
  if(NOT DUNE_INSTALL_MODULEDIR)
    set(DUNE_INSTALL_MODULEDIR ""
      CACHE PATH
      "Installation directory for CMake modules. Default is \${CMAKE_INSTALL_DATAROOTDIR}/cmake/modules when not set explicitely")
    set(DUNE_INSTALL_MODULEDIR ${CMAKE_INSTALL_DATAROOTDIR}/cmake/modules)
  endif(NOT DUNE_INSTALL_MODULEDIR)
endmacro(dune_project)

# create a new config.h file and overwrite the existing one
macro(dune_regenerate_config_cmake)
  set(CONFIG_H_CMAKE_FILE "${CMAKE_BINARY_DIR}/config.h.cmake")
  if(EXISTS ${CMAKE_SOURCE_DIR}/config.h.cmake)
    file(READ ${CMAKE_SOURCE_DIR}/config.h.cmake _file)
    string(REGEX MATCH
      "/[\\*/][ ]*begin[ ]+${DUNE_MOD_NAME}.*\\/[/\\*][ ]*end[ ]*${DUNE_MOD_NAME}[^\\*]*\\*/"
      _myfile "${_file}")
  endif(EXISTS ${CMAKE_SOURCE_DIR}/config.h.cmake)
  # overwrite file with new content
  file(WRITE ${CONFIG_H_CMAKE_FILE} "/* config.h.  Generated from config.h.cmake by CMake.
   It was generated from config.h.cmake which in turn is generated automatically
   from the config.h.cmake files of modules this module depends on. */"
   )

 # define that we found this module
 set(${DUNE_MOD_NAME}_FOUND 1)

 foreach(_dep ${DUNE_MOD_NAME} ${ALL_DEPENDENCIES})
   dune_module_to_uppercase(upper ${_dep})
   set(HAVE_${upper} ${${_dep}_FOUND})
   file(APPEND ${CONFIG_H_CMAKE_FILE}
     "\n\n/* Define to 1 if you have module ${_dep} available */
#cmakedefine01 HAVE_${upper}\n")
 endforeach(_dep ${DUNE_MOD_NAME} ${ALL_DEPENDENCIES})

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
       string(REGEX REPLACE "(.*)/[\\*][ ]*begin private.*/[\\*][ ]*end[ ]*private[^\\*]\\*/(.*)" "\\1\\2" _file "${_tfile}")
       file(APPEND ${CONFIG_H_CMAKE_FILE} "${_file}")
     endif(EXISTS ${_mod_conf_file})
   endforeach()
   # parse other modules dune.module file to generate variables for config.h
   foreach(_dune_module_file ${${_dep}_PREFIX}/dune.module
       ${${_dep}_PREFIX}/share/${_dep}/dune.module)
     if(EXISTS ${_dune_module_file})
       get_filename_component(_dune_module_file_path ${_dune_module_file} PATH)
       dune_module_information(${_dune_module_file_path} QUIET)
     endif(EXISTS ${_dune_module_file})
   endforeach()
 endforeach(_dep ${ALL_DEPENDENCIES})
 # parse again dune.module file of current module to set PACKAGE_* variables
 dune_module_information(${CMAKE_SOURCE_DIR} QUIET)
 file(APPEND ${CONFIG_H_CMAKE_FILE} "\n${_myfile}")
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
  list(LENGTH DEPENDS_MODULE mlength)
  math(EXPR len2 "${mlength}-1")
  if(mlength GREATER 0)
    foreach(i RANGE 0 ${len2})
      list(GET DEPENDS_MODULE ${i} _mod)
      dune_module_to_macro(_macro ${_mod})
      list(GET DEPENDS_VERSION ${i} _ver)
      #file(APPEND ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}-config.cmake
      #"find_package(${_mod})\n")#include(${_macro}Macros)\n")
      #file(APPEND ${PROJECT_BINARY_DIR}/cmake/pkg/${DUNE_MOD_NAME}-config.cmake
      #"find_package(${_mod})\n")#include(${_macro}Macros)\n")
    endforeach(i RANGE 0 ${mlength})
  endif(mlength GREATER 0)

  if(${DUNE_MOD_NAME_CMAKE}_FOUND)
    # This module hast its own tests.
    # Execute them during find_package
    #file(APPEND ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}-config.cmake
    #  "include(${DUNE_MOD_NAME_CMAKE}Macros)\n")
    #file(APPEND ${PROJECT_BINARY_DIR}/cmake/pkg/${DUNE_MOD_NAME}-config.cmake
    #  "\ninclude(${DUNE_MOD_NAME_CMAKE}Macros)\n")
  endif(${DUNE_MOD_NAME_CMAKE}_FOUND)
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
  if(EXISTS ${CMAKE_SOURCE_DIR}/config.h.cmake)
  install(FILES config.h.cmake DESTINATION share/${DUNE_MOD_NAME})
  endif(EXISTS ${CMAKE_SOURCE_DIR}/config.h.cmake)

  if("${ARGC}" EQUAL "1")
    message(STATUS "Adding custom target for config.h generation")
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
    message(STATUS "Not adding custom target for config.h generation")
    # actually write the config.h file to disk
    configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
  endif("${ARGC}" EQUAL "1")

  test_dep()
endmacro(finalize_dune_project)

macro(target_link_dune_default_libraries _target)
  foreach(_lib ${DUNE_DEFAULT_LIBS})
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
      endforeach(var _SOURCES _ADD_LIBS _COMPILE_FLAGS)
    else(_matched)
      list(APPEND _new_SOURCES "${_source}")
    endif(_matched)
  endforeach(_source ${${_SOURCES_var}})

  foreach(var _SOURCES _ADD_LIBS _COMPILE_FLAGS)
    set(${${var}_var} "${_new${var}}" PARENT_SCOPE)
  endforeach(var _SOURCES _ADD_LIBS _COMPILE_FLAGS)
endfunction(dune_expand_object_libraries)

# Creates shared and static libraries with the same basename.
# More docu can be found at the top of this file.
macro(dune_add_library basename)
  include(CMakeParseArguments)
  cmake_parse_arguments(DUNE_LIB "NO_EXPORT;OBJECT" "COMPILE_FLAGS"
    "ADD_LIBS;SOURCES" ${ARGN})
  if(DUNE_LIB_OBJECT)
    if(DUNE_LIB_${basename}_SOURCES)
      message(FATAL_ERROR "There is already a library with the name ${basename}, "
        "but only one is allowed!")
    else(DUNE_LIB_${basename}_SOURCES)
      foreach(source ${DUNE_LIB_UNPARSED_ARGUMENTS})
        list(APPEND full_path_sources ${CMAKE_CURRENT_SOURCE_DIR}/${source})
      endforeach(source ${DUNE_LIB_UNPARSED_ARGUMENTS})
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
    endif(DUNE_LIB_${basename}_SOURCES)
  else(DUNE_LIB_OBJECT)
    list(APPEND DUNE_LIB_SOURCES ${DUNE_LIB_UNPARSED_ARGUMENTS})
    dune_expand_object_libraries(DUNE_LIB_SOURCES DUNE_LIB_ADD_LIBS DUNE_LIB_COMPILE_FLAGS)
    #create lib
    add_library(${basename} ${DUNE_LIB_SOURCES})
    # link with specified libraries.
    if(DUNE_LIB_ADD_LIBS)
      dune_target_link_libraries(${basename} ${DUNE_LIB_ADD_LIBS})
    endif(DUNE_LIB_ADD_LIBS)
    if(DUNE_LIB_COMPILE_FLAGS)
      setproperty(${basename} APPEND_STRING COMPILE_FLAGS
        "${DUNE_LIB_COMPILE_FLAGS}")
    endif(DUNE_LIB_COMPILE_FLAGS)
    # Build library in ${PROJECT_BINARY_DIR}/lib
    set_target_properties(${basename} PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
      ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

    set(_created_libs ${basename})

    if(DUNE_BUILD_BOTH_LIBS)
      if(BUILD_SHARED_LIBS)
        #create static lib
        add_library(${basename}-static STATIC ${DUNE_LIB_SOURCES})
        # make sure both libs have the same name.
        set_target_properties(${basename}-static PROPERTIES
          OUTPUT_NAME ${basename}
          ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
        list(APPEND _created_libs ${basename}-static)
        # link with specified libraries.
        if(DUNE_LIB_ADD_LIBS)
          dune_target_link_libraries(${basename}-static ${DUNE_LIB_ADD_LIBS})
        endif(DUNE_LIB_ADD_LIBS)
        if(DUNE_LIB_COMPILE_FLAGS)
          setproperty(${basename}-static APPEND_STRING COMPILE_FLAGS
            "${DUNE_LIB_COMPILE_FLAGS}")
        endif(DUNE_LIB_COMPILE_FLAGS)
      else(BUILD_SHARED_LIBS)
        #create shared libs
        add_library(${basename}-shared SHARED  ${DUNE_LIB_SOURCES})
        set_target_properties(${basename}-shared PROPERTIES
          OUTPUT_NAME ${basename}
          LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
        # link with specified libraries.
        if(DUNE_LIB_ADD_LIBS)
          dune_target_link_libraries(${basename}-shared ${DUNE_LIB_ADD_LIBS})
        endif(DUNE_LIB_ADD_LIBS)
        if(DUNE_LIB_COMPILE_FLAGS)
          setproperty(${basename}-shared APPEND_STRING COMPILE_FLAGS
            "${DUNE_LIB_COMPILE_FLAGS}")
        endif(DUNE_LIB_COMPILE_FLAGS)
        list(APPEND _created_libs ${basename}-shared)
      endif(BUILD_SHARED_LIBS)
    endif(DUNE_BUILD_BOTH_LIBS)

    if(NOT DUNE_LIB_NO_EXPORT)
      if(NOT _MODULE_EXPORT_USED)
        set(_MODULE_EXPORT_USED ON)
        set(_append "")
      else(NOT _MODULE_EXPORT_USED)
        set(_append APPEND)
      endif(NOT _MODULE_EXPORT_USED)
      # install targets to use the libraries in other modules.
      install(TARGETS ${_created_libs}
        EXPORT ${DUNE_MOD_NAME}-targets DESTINATION lib)
      install(EXPORT ${DUNE_MOD_NAME}-targets
        DESTINATION lib/cmake)

      # export libraries for use in build tree
      export(TARGETS ${_created_libs} ${_append}
        FILE ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}-targets.cmake)
    endif(NOT DUNE_LIB_NO_EXPORT)
  endif(DUNE_LIB_OBJECT)
endmacro(dune_add_library basename sources)

macro(replace_properties_for_one)
  get_property(properties ${option_command} ${_target}
    PROPERTY ${REPLACE_PROPERTY})
  if(NOT properties)
    # property not set. set it directly
    foreach(i RANGE 0 ${hlength})
      math(EXPR idx "(2 * ${i}) + 1")
      list(GET REPLACE_UNPARSED_ARGUMENTS ${idx} repl)
      list(APPEND replacement ${repl})
    endforeach(i RANGE 0 ${hlength})
    list(REMOVE_DUPLICATES replacement)
    set_property(${option_command} ${_target} ${REPLACE_APPEND}
      ${REPLACE_APPEND_STRING} PROPERTY ${REPLACE_PROPERTY} ${replacement})
  else(NOT properties)
    foreach(prop ${properties})
      set(matched FALSE)
      foreach(i RANGE 0 ${hlength})
        math(EXPR regexi "2 * ${i}")
        math(EXPR repli  "${regexi} +1")
        list(GET REPLACE_UNPARSED_ARGUMENTS ${regexi} regex)
        list(GET REPLACE_UNPARSED_ARGUMENTS ${repli} replacement)
        string(REGEX MATCH ${regex} match ${prop})

        if(match)
          list(APPEND new_props ${replacement})
          set(matched TRUE)
        endif(match)
      endforeach(i RANGE 0 ${hlength})

      if(NOT matched)
        list(APPEND new_props ${prop})
      endif(NOT matched)
    endforeach(prop ${properties})
    list(REMOVE_DUPLICATES new_props)
    set_property(${option_command} ${_target}
      PROPERTY ${REPLACE_PROPERTY} ${new_props})
  endif(NOT properties)
  get_property(properties ${option_command} ${_target} PROPERTY ${REPLACE_PROPERTY})
endmacro(replace_properties_for_one)

function(dune_target_link_libraries basename libraries)
  target_link_libraries(${basename} ${libraries})
  if(DUNE_BUILD_BOTH_LIBS)
    if(BUILD_SHARED_LIBS)
      target_link_libraries(${basename}-static ${libraries})
    else(BUILD_SHARED_LIBS)
      target_link_libraries(${basename}-shared ${libraries})
    endif(BUILD_SHARED_LIBS)
  endif(DUNE_BUILD_BOTH_LIBS)
endfunction(dune_target_link_libraries basename libraries)

function(replace_properties)
  include(CMakeParseArguments)
  set(_first_opts "GLOBAL;DIRECTORY;TARGET;SOURCE;CACHE")
  cmake_parse_arguments(REPLACE "GLOBAL"
    "DIRECTORY;PROPERTY" "TARGET;SOURCE;TEST;CACHE" ${ARGN})

  set(MY_DIRECTORY TRUE)
  foreach(i ${_first_opts})
    if(REPLACE_${i})
      set(MY_DIRECTORY FALSE)
    endif(REPLACE_${i})
  endforeach(i ${_first_opts})
  if(NOT MY_DIRECTORY)
    list(FIND REPLACE_UNPARSED_ARGUMENTS DIRECTORY _found)
    if(_found GREATER -1)
      list(REMOVE_AT REPLACE_UNPARSED_ARGUMENTS ${_found})
      set(MY_DIRECTORY TRUE)
      set(REPLACE_DIRECTORY "")
    endif(_found GREATER -1)
  else(NOT MY_DIRECTORY)
    #set(REPLACE_PROPERTY
  endif(NOT MY_DIRECTORY)

  #setup options
  if(REPLACE_GLOBAL)
    set(option_command GLOBAL)
  elseif(MY_DIRECTORY)
    set(option_command DIRECTORY)
  elseif(REPLACE_DIRECTORY)
    set(option_command DIRECTORY)
    set(option_arg ${REPLACE_DIRECTORY})
  elseif(REPLACE_TARGET)
    set(option_command TARGET)
    set(option_arg ${REPLACE_TARGET})
  elseif(REPLACE_SOURCE)
    set(option_command SOURCE)
    set(option_arg ${REPLACE_SOURCE})
  elseif(REPLACE_TEST)
    set(option_command TEST)
    set(option_arg${REPLACE_TEST})
  elseif(REPLACE_CACHE)
    set(option_command CACHE)
    set(option_arg ${REPLACE_CACHE})
  endif()

  if(NOT (REPLACE_CACHE OR REPLACE_TEST OR REPLACE_SOURCE
      OR REPLACE_TARGET OR REPLACE_DIRECTORY OR REPLACE_GLOBAL
      OR MY_DIRECTORY))
    message(ERROR "One of GLOBAL, DIRECTORY, TARGET, SOURCE, TEST, or CACHE"
      " has to be present")
  endif()

  list(LENGTH REPLACE_UNPARSED_ARGUMENTS length)
#  if(NOT (REPLACE_GLOBAL AND REPLACE_TARGET AND
#        REPLACE_SOURCE AND REPLACE
  math(EXPR mlength "${length} % 2 ")
  math(EXPR hlength "${length} / 2 - 1")

  if(NOT ${mlength} EQUAL 0)
    message(ERROR "You need to specify pairs consisting of a regular expression and a replacement string.")
  endif(NOT ${mlength} EQUAL 0)

  if(NOT length GREATER 0)
    message(ERROR "You need to specify at least on pair consisting of a regular expression
and a replacement string. ${REPLACE_UNPARSED_ARGUMENTS}")
  endif(NOT length GREATER 0)

  foreach(_target ${option_arg})
    replace_properties_for_one()
  endforeach(_target ${option_arg})

  list(LENGTH option_arg _length)
  if(_length EQUAL 0)
    replace_properties_for_one()
  endif(_length EQUAL 0)
endfunction(replace_properties)
