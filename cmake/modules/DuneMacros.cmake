# Core DUNE module for CMake.
#
# Documentation of the public API defined in this module:
#
# .. cmake_function:: dune_project
#
#    Initialize a Dune module. This function needs to be run from every
#    top-level CMakeLists.txt file. It sets up the module, defines basic
#    variables and manages depedencies. Don't forget to call
#    :ref:`finalize_dune_project` afterwards.
#
# .. cmake_function:: finalize_dune_project
#
#    Finalize a Dune module. This function needs to be run at the end of
#    every top-level CMakeLists.txt file. Among other things it creates
#    the cmake package configuration files. Modules can add additional
#    entries to these files by setting the variable @${ProjectName}_INIT.
#
# .. cmake_function:: target_link_libraries
#
#    .. cmake_brief::
#
#       Overwrite of CMake's :code:`target_link_libraries`. If no interface key
#       word (like PUBLIC, INTERFACE, PRIVATE etc.) is given, PUBLIC is added.
#       This is to fix problems with CMP0023.
#
#    .. cmake_param:: basename
#
# .. cmake_function:: dune_add_library
#
#    .. cmake_brief::
#
#       Add a library to a Dune module!
#
#    .. cmake_param:: basename
#       :single:
#       :required:
#       :positional:
#
#       The basename for the library. On Unix this created :code:`lib<basename>.so`
#       and :code:`lib<basename>.a`
#
#    .. cmake_param:: NO_EXPORT
#       :option:
#
#       If omitted the library is exported for usage in other modules.
#
#    .. cmake_param:: ADD_LIBS
#       :multi:
#
#       A list of libraries that should be incorporated into this library.
#
#    .. cmake_param:: APPEND
#       :option:
#
#       Whether the library should be appended to the
#       exported libraries. If there a DUNE module must
#       make several libraries available, then first one
#       must not use this option but the others have to
#       use it. Otherwise only the last library will be
#       exported as the others will be overwritten.
#
#    .. cmake_param:: OBJECT
#       :option:
#
#       .. note::
#          This feature will very likely vanish in Dune 3.0
#
#    .. cmake_param:: SOURCES
#       :multi:
#       :required:
#
#       The source files from which to build the library.
#
#    .. cmake_param:: COMPILE_FLAGS
#       :single:
#
#       Any additional compile flags for building the library.
#
# .. cmake_function:: dune_target_link_libraries
#
#    .. cmake_param:: BASENAME
#
#    .. cmake_param:: LIBRARIES
#
#    Link libraries to the static and shared version of
#    library BASENAME
#
#
# .. cmake_function:: add_dune_all_flags
#
#    .. cmake_param:: targets
#       :single:
#       :required:
#       :positional:
#
#       The targets to add the flags of all external libraries to.
#
#    This function is superseded by :ref:`dune_target_enable_all_packages`.
#
# Documentation of internal macros in this module:
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
# dune_regenerate_config_cmake()
#
# Creates a new config_collected.h.cmake file in ${CMAKE_CURRENT_BINARY_DIR} that
# consists of entries from ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
# and includes non-private entries from the config.h.cmake files
# of all dependent modules.
# Finally config.h is created from config_collected.h.cmake.
#
include_guard(GLOBAL)

enable_language(C) # Enable C to skip CXX bindings for some tests.

# By default use -pthread flag. This option is set at the beginning to enforce it for
# find_package(Threads) everywhere
set(THREADS_PREFER_PTHREAD_FLAG TRUE CACHE BOOL "Prefer -pthread compiler and linker flag")

# Add a backport of cmakes FindPkgConfig module
if(${CMAKE_VERSION} VERSION_LESS "3.19.4")
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/FindPkgConfig")
endif()

include(FeatureSummary)
include(DuneEnableAllPackages)
include(DuneTestMacros)
include(OverloadCompilerFlags)
include(DuneSymlinkOrCopy)
include(DunePathHelper)
include(DuneExecuteProcess)

macro(target_link_libraries)
  # do nothing if not at least the two arguments target and scope are passed
  if(${ARGC} GREATER_EQUAL 2)
    target_link_libraries_helper(${ARGN})
  endif()
endmacro(target_link_libraries)

# helper for overwritten target_link_libraries to handle arguments more easily
macro(target_link_libraries_helper TARGET SCOPE)
  if(${SCOPE} MATCHES "^(PRIVATE|INTERFACE|PUBLIC|LINK_PRIVATE|LINK_PUBLIC|LINK_INTERFACE_LIBRARIES)$")
    _target_link_libraries(${TARGET} ${SCOPE} ${ARGN})
  else()
    message(DEPRECATION "Calling target_link_libraries without the <scope> argument is deprecated.")
    _target_link_libraries(${TARGET} PUBLIC ${SCOPE} ${ARGN})
  endif()
endmacro(target_link_libraries_helper)

# Converts a module name given by _module into an uppercase string
# _upper where all dashes (-) are replaced by underscores (_)
# Example: dune-common -> DUNE_COMMON
macro(dune_module_to_uppercase _upper _module)
  string(TOUPPER "${_module}" ${_upper})
  string(REPLACE "-" "_" ${_upper} "${${_upper}}")
endmacro(dune_module_to_uppercase _upper _module)

macro(find_dune_package module)
  include(CMakeParseArguments)
  cmake_parse_arguments(DUNE_FIND "REQUIRED" "VERSION" "" ${ARGN})
  if(DUNE_FIND_REQUIRED)
    set(required REQUIRED)
    set_package_properties(${module} PROPERTIES TYPE REQUIRED)
    set(_warning_level "FATAL_ERROR")
  else()
    unset(required)
    set(_warning_level "WARNING")
    set_package_properties(${module} PROPERTIES TYPE OPTIONAL)
  endif()
  if(DUNE_FIND_VERSION MATCHES "(>=|=|<=).*")
    string(REGEX REPLACE "(>=|=|<=)(.*)" "\\1" DUNE_FIND_VERSION_OP ${DUNE_FIND_VERSION})
    string(REGEX REPLACE "(>=|=|<=)(.*)" "\\2" DUNE_FIND_VERSION_NUMBER ${DUNE_FIND_VERSION})
    string(STRIP ${DUNE_FIND_VERSION_NUMBER} DUNE_FIND_VERSION_NUMBER)
    extract_major_minor_version("${DUNE_FIND_VERSION_NUMBER}" DUNE_FIND_VERSION)
    set(DUNE_FIND_VERSION_STRING "${DUNE_FIND_VERSION_MAJOR}.${DUNE_FIND_VERSION_MINOR}.${DUNE_FIND_VERSION_REVISION}")
  else()
    set(DUNE_FIND_VERSION_STRING "0.0.0")
  endif()
  if(NOT ${module}_FOUND)
    if(NOT (${module}_DIR OR ${module}_ROOT OR
       "${CMAKE_PREFIX_PATH}" MATCHES ".*${module}.*"))
      string(REPLACE  ${ProjectName} ${module} ${module}_DIR
        ${PROJECT_BINARY_DIR})
    endif()
    find_package(${module} NO_CMAKE_PACKAGE_REGISTRY)
  endif()
  if(NOT ${module}_FOUND AND NOT CMAKE_DISABLE_FIND_PACKAGE_${module})
    message(STATUS "No full CMake package configuration support available."
      " Falling back to pkg-config.")
    # use pkg-config
    find_package(PkgConfig)
    if(NOT PKG_CONFIG_FOUND AND required)
      message(FATAL_ERROR "Could not find module ${module}. We tried to use"
        "pkg-config but could not find it. ")
    endif()
        pkg_check_modules (${module} ${required} ${module}${DUNE_FIND_VERSION})
    set(${module}_FAKE_CMAKE_PKGCONFIG TRUE)
  endif()
  if(${module}_FAKE_CMAKE_PKGCONFIG)
    # compute the path to the libraries
    if(${module}_LIBRARIES)
      unset(_module_lib)
      foreach(lib ${${module}_LIBRARIES})
        foreach(libdir ${${module}_LIBRARY_DIRS})
          if(EXISTS ${libdir}/lib${lib}.a)
            set(_module_lib ${libdir}/lib${lib}.a)
            set(_module_lib_static "STATIC")
          endif()
          if(EXISTS ${libdir}/lib${lib}.so)
            set(_module_lib ${libdir}/lib${lib}.so)
            set(_module_lib_static "")
          endif()
          if(_module_lib)
            #import library
            add_library(${lib} ${_module_lib_static} IMPORTED)
            set_property(TARGET ${lib} APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
            set_target_properties(${lib} PROPERTIES
              IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
              IMPORTED_LOCATION_NOCONFIG "${_module_lib}")
            break()
          endif()
        endforeach()
      endforeach()
    endif()
    if(NOT ${module}_MODULE_PATH)
      if(${module}_INCLUDE_DIRS)
        list(GET ${module}_INCLUDE_DIRS 0 _dir)
        if(EXISTS ${_dir}/../share/dune/cmake/modules)
          set(${module}_MODULE_PATH ${_dir}/../share/dune/cmake/modules)
        endif()
      endif()
    endif()
    unset(${module}_FAKE_CMAKE_PKGCONFIG)
  endif()
  if(${module}_FOUND)
    # parse other module's dune.module file to generate variables for config.h
    unset(${module}_dune_module)
    foreach(_dune_module_file
        ${${module}_PREFIX}/dune.module
        ${${module}_PREFIX}/lib/dunecontrol/${module}/dune.module
        ${${module}_PREFIX}/lib64/dunecontrol/${module}/dune.module)
      if(EXISTS ${_dune_module_file})
        get_filename_component(_dune_module_file_path ${_dune_module_file} PATH)
        dune_module_information(${_dune_module_file_path})# QUIET)
        set(${module}_dune_module 1)
        set(DUNE_FIND_MOD_VERSION_STRING "${DUNE_VERSION_MAJOR}.${DUNE_VERSION_MINOR}.${DUNE_VERSION_REVISION}")
        # check whether dependency mathes version requirement
        unset(module_version_wrong)
        if(DUNE_FIND_VERSION_OP MATCHES ">=")
          if(NOT (DUNE_FIND_MOD_VERSION_STRING VERSION_EQUAL DUNE_FIND_VERSION_STRING OR
                  DUNE_FIND_MOD_VERSION_STRING VERSION_GREATER DUNE_FIND_VERSION_STRING))
            set(module_version_wrong 1)
          endif()
        elseif(DUNE_FIND_VERSION_OP MATCHES "<=")
          if(NOT (DUNE_FIND_MOD_VERSION_STRING VERSION_EQUAL DUNE_FIND_VERSION_STRING OR
                  DUNE_FIND_MOD_VERSION_STRING VERSION_LESS DUNE_FIND_VERSION_STRING))
            set(module_version_wrong 1)
          endif()
        elseif(DUNE_FIND_VERSION_OP MATCHES "=" AND
           NOT (DUNE_FIND_MOD_VERSION_STRING VERSION_EQUAL DUNE_FIND_VERSION_STRING))
          set(module_version_wrong 1)
        endif()
      endif()
    endforeach()
    if(NOT ${module}_dune_module)
      message(${_warning_level} "Could not find dune.module file for module ${module} "
        "in ${${module}_PREFIX},  ${${module}_PREFIX}/lib/dunecontrol/${module}/, "
        "${${module}_PREFIX}/lib64/dunecontrol/${module}/dune.module")
      set(${module}_FOUND OFF)
    endif()
    if(module_version_wrong)
      message(${_warning_level} "Could not find requested version of module ${module}. "
        "Requested version was ${DUNE_FIND_VERSION}, found version is ${DUNE_FIND_MOD_VERSION_STRING}")
      set(${module}_FOUND OFF)
    endif()
  else(${module}_FOUND)
    if(required)
      message(FATAL_ERROR "Could not find required module ${module}.")
    endif()
  endif()
  set(DUNE_${module}_FOUND ${${module}_FOUND})
endmacro(find_dune_package module)

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

#
# split list of modules, potentially with version information
# into list of modules and list of versions
#
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

#
# Convert a string with spaces in a list which is a string with semicolon
#
function(convert_deps_to_list var)
  string(REGEX REPLACE "([a-zA-Z0-9\\)]) ([a-zA-Z0-9])" "\\1;\\2" ${var} ${${var}})
  set(${var} ${${var}} PARENT_SCOPE)
endfunction(convert_deps_to_list var)

#
# extracts major, minor, and revision from version string
#
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

macro(dune_process_dependency_leafs modules versions is_required next_level_deps
    next_level_sugs)
  # modules, and versions are not real variables, make them one
  set(mmodules ${modules})
  set(mversions ${versions})
  list(LENGTH mmodules mlength)
  list(LENGTH mversions vlength)
  if(NOT mlength EQUAL vlength)
    message(STATUS "mmodules=${mmodules} modules=${modules}")
    message(STATUS "mversions=${mversions} versions=${mversions}")
    message(FATAL_ERROR "List of modules and versions do not have the same length!")
  endif()
  if(mlength GREATER 0)
    math(EXPR length "${mlength}-1")
    foreach(i RANGE 0 ${length})
      list(GET mmodules ${i} _mod)
      list(GET mversions ${i} _ver)
      find_dune_package(${_mod} ${is_required} VERSION "${_ver}")
      set(${_mod}_SEARCHED ON)
      if(NOT "${is_required}" STREQUAL "")
        set(${_mod}_REQUIRED ON)
        set(${next_level_deps} ${${_mod}_DEPENDS} ${${next_level_deps}})
      else(NOT "${is_required}" STREQUAL "")
        set(${next_level_sugs} ${${_mod}_DEPENDS} ${${next_level_sugs}})
      endif()
      set(${next_level_sugs} ${${_mod}_SUGGESTS} ${${next_level_sugs}})
    endforeach()
  endif()
  if(${next_level_sugs})
    list(REMOVE_DUPLICATES ${next_level_sugs})
  endif()
  if(${next_level_deps})
    list(REMOVE_DUPLICATES ${next_level_deps})
  endif()
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
        endif()
      endif()
    endforeach()
  endif()
  set(${modules} ${${modules}} PARENT_SCOPE)
  set(${versions} ${${versions}} PARENT_SCOPE)
endfunction(remove_processed_modules modules versions is_required)

macro(dune_create_dependency_leafs depends depends_versions suggests suggests_versions)
  set(deps "")
  set(sugs "")
  #Process dependencies
  if(NOT "${depends}" STREQUAL "")
    dune_process_dependency_leafs("${depends}" "${depends_versions}" REQUIRED deps sugs)
  endif()
  # Process suggestions
  if(NOT "${suggests}" STREQUAL "")
    dune_process_dependency_leafs("${suggests}" "${suggests_versions}" "" deps sugs)
  endif()
  split_module_version("${deps}" next_mod_depends next_depends_versions)
  split_module_version("${sugs}" next_mod_suggests next_suggests_versions)
  set(ALL_DEPENDENCIES ${ALL_DEPENDENCIES} ${next_mod_depends} ${next_mod_suggests})
  # Move to next level
  if(next_mod_suggests OR next_mod_depends)
    dune_create_dependency_leafs("${next_mod_depends}" "${next_depends_versions}"
      "${next_mod_suggests}" "${next_suggests_versions}")
  endif()
endmacro(dune_create_dependency_leafs)

macro(dune_create_dependency_tree)
  if(dune-common_MODULE_PATH)
    list(REMOVE_ITEM CMAKE_MODULE_PATH "${dune-common_MODULE_PATH}")
  endif()
  list(FIND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules start)
  set(ALL_DEPENDENCIES "")
  if(${ProjectName}_DEPENDS_MODULE OR ${ProjectName}_SUGGESTS_MODULE)
    set(ALL_DEPENDENCIES ${${ProjectName}_DEPENDS_MODULE} ${${ProjectName}_SUGGESTS_MODULE})
    dune_create_dependency_leafs("${${ProjectName}_DEPENDS_MODULE}" "${${ProjectName}_DEPENDS_VERSION}"
      "${${ProjectName}_SUGGESTS_MODULE}" "${${ProjectName}_SUGGESTS_VERSION}")
  endif()
  set(_my_path "")
  if(ALL_DEPENDENCIES)
    # Reverse the order of the modules and remove duplicates
    # At end of this clause we have have a list modules
    # where for each entry all dependencies are before the
    # module in the list.
    set(NEW_ALL_DEPS "")
    list(LENGTH ALL_DEPENDENCIES length)
    if(length GREATER 0)
      math(EXPR length "${length}-1")
      list(GET ALL_DEPENDENCIES ${length} _mod)
      set(${_mod}_cmake_path_processed 1)
      set(_my_path ${${_mod}_MODULE_PATH})
      list(APPEND NEW_ALL_DEPS ${_mod})
      if(length GREATER 0)
        math(EXPR length "${length}-1")
        foreach(i RANGE ${length} 0 -1)
          list(GET ALL_DEPENDENCIES ${i} _mod)
          if(NOT ${_mod}_cmake_path_processed)
            set(${_mod}_cmake_path_processed 1)
            if(${_mod}_MODULE_PATH)
              list(INSERT _my_path 0 ${${_mod}_MODULE_PATH})
            endif()
            list(APPEND NEW_ALL_DEPS ${_mod})
          endif()
        endforeach()
      endif()
      list(LENGTH CMAKE_MODULE_PATH length)
      math(EXPR length "${length}-1")
      if(start EQUAL -1)
        list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules ${_my_path})
      else()
        if(start EQUAL ${length})
          list(APPEND CMAKE_MODULE_PATH ${_my_path})
        else()
          if(_my_path)
            list(INSERT CMAKE_MODULE_PATH ${start} ${_my_path})
          endif()
        endif()
      endif()
    endif()
    set(ALL_DEPENDENCIES ${NEW_ALL_DEPS})
  endif()
endmacro(dune_create_dependency_tree)

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
  endwhile()
  string(LENGTH "${_rest}" _length)
  string(SUBSTRING "${_rest}" 0 1 _first_letter)
  string(SUBSTRING "${_rest}" 1 -1 _rest)
  string(TOUPPER "${_first_letter}" _first_letter)
  set(${_macro_name} "${${_macro_name}}${_first_letter}${_rest}")
endmacro(dune_module_to_macro _macro_name _dune_module)

macro(dune_process_dependency_macros)
  foreach(_mod ${ALL_DEPENDENCIES} ${ProjectName})
    if(NOT ${_mod}_PROCESSED)
      # module not processed yet
      set(${_mod}_PROCESSED ${_mod})
      # Search for a cmake files containing tests and directives
      # specific to this module
      dune_module_to_macro(_cmake_mod_name "${_mod}")
      set(_macro "${_cmake_mod_name}Macros")
      set(_mod_cmake _mod_cmake-NOTFOUND) # Prevent false positives due to caching
      message(STATUS "Searching for macro file '${_macro}' for module '${_mod}'.")
      find_file(_mod_cmake
        NAMES "${_macro}.cmake"
        PATHS ${CMAKE_MODULE_PATH}
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH)
      if(_mod_cmake)
        message(STATUS "Performing tests specific to ${_mod} from file ${_mod_cmake}.")
        include(${_mod_cmake})
      else()
        message(STATUS "No module specific tests performed for module '${_mod}' because macro file '${_macro}.cmake' not found in ${CMAKE_MODULE_PATH}.")
      endif()
      dune_module_to_uppercase(_upper_case "${_mod}")
      if(${_mod}_INCLUDE_DIRS)
        message(STATUS "Setting ${_mod}_INCLUDE_DIRS=${${_mod}_INCLUDE_DIRS}")
        include_directories("${${_mod}_INCLUDE_DIRS}")
      endif()
      if(${_mod}_LIBRARIES)
        message(STATUS "Setting ${_mod}_LIBRARIES=${${_mod}_LIBRARIES}")
        foreach(_lib ${${_mod}_LIBRARIES})
          list(INSERT DUNE_DEFAULT_LIBS 0 "${_lib}")
          list(INSERT DUNE_LIBS 0 "${_lib}")
        endforeach()
      endif()

      # register dune module
      dune_register_package_flags(INCLUDE_DIRS "${${_mod}_INCLUDE_DIRS}")
    endif()
  endforeach()
endmacro(dune_process_dependency_macros)

# macro that should be called near the begin of the top level CMakeLists.txt.
# Namely it sets up the module, defines basic variables and manages
# depedencies.
# Don't forget to call finalize_dune_project afterwards.
macro(dune_project)

  # check if CXX flag overloading has been enabled (see OverloadCompilerFlags.cmake)
  initialize_compiler_script()

  # extract information from dune.module
  dune_module_information(${PROJECT_SOURCE_DIR})
  set(ProjectName            "${DUNE_MOD_NAME}")
  set(ProjectVersion         "${DUNE_MOD_VERSION}")
  set(ProjectVersionString   "${DUNE_VERSION_MAJOR}.${DUNE_VERSION_MINOR}.${DUNE_VERSION_REVISION}")
  set(ProjectVersionMajor    "${DUNE_VERSION_MAJOR}")
  set(ProjectVersionMinor    "${DUNE_VERSION_MINOR}")
  set(ProjectVersionRevision "${DUNE_VERSION_REVISION}")
  set(ProjectMaintainerEmail "${DUNE_MAINTAINER}")
  set(ProjectDescription     "${${DUNE_MOD_NAME_UPPERCASE}_DESCRIPTION}")
  set(ProjectAuthor          "${${DUNE_MOD_NAME_UPPERCASE}_AUTHOR}")
  set(ProjectUrl             "${${DUNE_MOD_NAME_UPPERCASE}_URL}")
  set(ProjectPythonRequires  "${${DUNE_MOD_NAME_UPPERCASE}_PYTHON_REQUIRES}")

  # check whether this module has been explicitly disabled through the cmake flags.
  # If so, stop the build. This is necessary because dunecontrol does not parse
  # the given CMake flags for a disabled Dune modules.
  if(CMAKE_DISABLE_FIND_PACKAGE_${ProjectName})
    message("Module ${ProjectName} has been explicitly disabled through the cmake flags. Skipping build.")
    return()
  endif()

  define_property(GLOBAL PROPERTY DUNE_MODULE_LIBRARIES
        BRIEF_DOCS "List of libraries of the module. DO NOT EDIT!"
        FULL_DOCS "List of libraries of the module. Used to generate CMake's package configuration files. DO NOT EDIT!")
  dune_create_dependency_tree()

  # assert the project names matches
  if(NOT (ProjectName STREQUAL PROJECT_NAME))
    message(FATAL_ERROR "Module name from dune.module does not match the name given in CMakeLists.txt.")
  endif()

  # As default request position independent code if shared libraries are built
  # This should allow DUNE modules to use CMake's object libraries.
  # This can be overwritten for targets by setting the target property
  # POSITION_INDEPENDENT_CODE to false/OFF
  include(CMakeDependentOption)
  cmake_dependent_option(CMAKE_POSITION_INDEPENDENT_CODE "Build position independent code" ON "NOT BUILD_SHARED_LIBS" ON)

  # check for C++ features, set compiler flags for C++14 or C++11 mode
  include(CheckCXXFeatures)

  # set include path and link path for the current project.
  include_directories("${PROJECT_BINARY_DIR}")
  include_directories("${PROJECT_SOURCE_DIR}")
  include_directories("${CMAKE_CURRENT_BINARY_DIR}")
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
  add_definitions(-DHAVE_CONFIG_H)

  # Create custom target for building the documentation
  # and provide macros for installing the docs and force
  # building them before.
  include(DuneDoc)

  # activate pkg-config
  include(DunePkgConfig)

  # Process the macros provided by the dependencies and ourself
  dune_process_dependency_macros()

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
      "Installation directory for CMake modules. Default is \${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/modules when not set explicitely")
    set(DUNE_INSTALL_MODULEDIR ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/modules)
  endif()
  if(NOT DUNE_INSTALL_NONOBJECTLIBDIR)
    set(DUNE_INSTALL_NONOBJECTLIBDIR ""
      CACHE PATH
      "Installation directory for libraries that are not architecture dependent. Default is lib when not set explicitely")
    set(DUNE_INSTALL_NONOBJECTLIBDIR lib)
  endif()
  # set up make headercheck
  include(Headercheck)
  setup_headercheck()

endmacro(dune_project)

# create a new config.h file and overwrite the existing one
macro(dune_regenerate_config_cmake)
  set(CONFIG_H_CMAKE_FILE "${PROJECT_BINARY_DIR}/config_collected.h.cmake")
  if(EXISTS ${PROJECT_SOURCE_DIR}/config.h.cmake)
    file(READ ${PROJECT_SOURCE_DIR}/config.h.cmake _file)
    string(REGEX MATCH
      "/[\\*/][ ]*begin[ ]+${ProjectName}.*\\/[/\\*][ ]*end[ ]*${ProjectName}[^\\*]*\\*/"
      _myfile "${_file}")
  endif()
  # overwrite file with new content
  file(WRITE ${CONFIG_H_CMAKE_FILE} "/* config.h.  Generated from config_collected.h.cmake by CMake.
   It was generated from config_collected.h.cmake which in turn is generated automatically
   from the config.h.cmake files of modules this module depends on. */"
   )

 # define that we found this module
 set(${ProjectName}_FOUND 1)
 foreach(_dep ${ProjectName} ${ALL_DEPENDENCIES})
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

# macro that should be called at the end of the top level CMakeLists.txt.
# Namely it creates config.h and the cmake-config files,
# some install directives and exports the module.
macro(finalize_dune_project)
  if(DUNE_SYMLINK_TO_SOURCE_TREE)
    dune_symlink_to_source_tree()
  endif()

  #configure all headerchecks
  finalize_headercheck()

  #create cmake-config files for installation tree
  include(CMakePackageConfigHelpers)
  include(GNUInstallDirs)
  set(DOXYSTYLE_DIR ${CMAKE_INSTALL_DATAROOTDIR}/dune-common/doc/doxygen/)
  set(SCRIPT_DIR ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/scripts)
  # Set the location where the doc sources are installed.
  # Needed by custom package configuration
  # file section of dune-grid.
  set(DUNE_MODULE_SRC_DOCDIR "\${${ProjectName}_PREFIX}/${CMAKE_INSTALL_DOCDIR}")

  if(NOT EXISTS ${PROJECT_SOURCE_DIR}/cmake/pkg/${ProjectName}-config.cmake.in)
    # Generate a standard cmake package configuration file
    file(WRITE ${PROJECT_BINARY_DIR}/CMakeFiles/${ProjectName}-config.cmake.in
"if(NOT ${ProjectName}_FOUND)
# Whether this module is installed or not
set(${ProjectName}_INSTALLED @MODULE_INSTALLED@)

# Settings specific to the module
@${ProjectName}_INIT@
# Package initialization
@PACKAGE_INIT@

#report other information
set_and_check(${ProjectName}_PREFIX \"\${PACKAGE_PREFIX_DIR}\")
set_and_check(${ProjectName}_INCLUDE_DIRS \"@PACKAGE_CMAKE_INSTALL_INCLUDEDIR@\")
set(${ProjectName}_CXX_FLAGS \"${CMAKE_CXX_FLAGS}\")
set(${ProjectName}_CXX_FLAGS_DEBUG \"${CMAKE_CXX_FLAGS_DEBUG}\")
set(${ProjectName}_CXX_FLAGS_MINSIZEREL \"${CMAKE_CXX_FLAGS_MINSIZEREL}\")
set(${ProjectName}_CXX_FLAGS_RELEASE \"${CMAKE_CXX_FLAGS_RELEASE}\")
set(${ProjectName}_CXX_FLAGS_RELWITHDEBINFO \"${CMAKE_CXX_FLAGS_RELWITHDEBINFO}\")
set(${ProjectName}_DEPENDS \"@${ProjectName}_DEPENDS@\")
set(${ProjectName}_SUGGESTS \"@${ProjectName}_SUGGESTS@\")
set(${ProjectName}_MODULE_PATH \"@PACKAGE_DUNE_INSTALL_MODULEDIR@\")
set(${ProjectName}_LIBRARIES \"@DUNE_MODULE_LIBRARIES@\")

# Lines that are set by the CMake build system via the variable DUNE_CUSTOM_PKG_CONFIG_SECTION
${DUNE_CUSTOM_PKG_CONFIG_SECTION}

#import the target
if(${ProjectName}_LIBRARIES)
  get_filename_component(_dir \"\${CMAKE_CURRENT_LIST_FILE}\" PATH)
  include(\"\${_dir}/${ProjectName}-targets.cmake\")
endif()
endif()")
      set(CONFIG_SOURCE_FILE ${PROJECT_BINARY_DIR}/CMakeFiles/${ProjectName}-config.cmake.in)
  else()
    set(CONFIG_SOURCE_FILE ${PROJECT_SOURCE_DIR}/cmake/pkg/${ProjectName}-config.cmake.in)
  endif()
  get_property(DUNE_MODULE_LIBRARIES GLOBAL PROPERTY DUNE_MODULE_LIBRARIES)

  # compute under which libdir the package configuration files are to be installed.
  # If the module installs an object library we use CMAKE_INSTALL_LIBDIR
  # to capture the multiarch triplet of Debian/Ubuntu.
  # Otherwise we fall back to DUNE_INSTALL_NONOBJECTLIB which is lib
  # if not set otherwise.
  get_property(DUNE_MODULE_LIBRARIES GLOBAL PROPERTY DUNE_MODULE_LIBRARIES)
  if(DUNE_MODULE_LIBRARIES)
    set(DUNE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
  else()
    set(DUNE_INSTALL_LIBDIR ${DUNE_INSTALL_NONOBJECTLIBDIR})
  endif()

  # Set the location of the doc file source. Needed by custom package configuration
  # file section of dune-grid.
  set(DUNE_MODULE_SRC_DOCDIR "${PROJECT_SOURCE_DIR}/doc")
  set(MODULE_INSTALLED ON)

  configure_package_config_file(${CONFIG_SOURCE_FILE}
    ${PROJECT_BINARY_DIR}/cmake/pkg/${ProjectName}-config.cmake
    INSTALL_DESTINATION  ${DUNE_INSTALL_LIBDIR}/cmake/${ProjectName}
    PATH_VARS CMAKE_INSTALL_DATAROOTDIR DUNE_INSTALL_MODULEDIR CMAKE_INSTALL_INCLUDEDIR
    DOXYSTYLE_DIR SCRIPT_DIR)


  #create cmake-config files for build tree
  set(PACKAGE_CMAKE_INSTALL_INCLUDEDIR ${PROJECT_SOURCE_DIR})
  set(PACKAGE_CMAKE_INSTALL_DATAROOTDIR ${PROJECT_BINARY_DIR})
  set(PACKAGE_DOXYSTYLE_DIR ${PROJECT_SOURCE_DIR}/doc/doxygen)
  set(PACKAGE_SCRIPT_DIR ${PROJECT_SOURCE_DIR}/cmake/scripts)
  set(PACKAGE_DUNE_INSTALL_MODULEDIR ${PROJECT_SOURCE_DIR}/cmake/modules)
  set(PACKAGE_PREFIX_DIR ${PROJECT_BINARY_DIR})
  set(PACKAGE_INIT "# Set prefix to source dir
set(PACKAGE_PREFIX_DIR ${PROJECT_SOURCE_DIR})
macro(set_and_check _var _file)
  set(\${_var} \"\${_file}\")
  if(NOT EXISTS \"\${_file}\")
    message(FATAL_ERROR \"File or directory \${_file} referenced by variable \${_var} does not exist !\")
  endif()
endmacro()")
  set(MODULE_INSTALLED OFF)
  configure_file(
    ${CONFIG_SOURCE_FILE}
    ${PROJECT_BINARY_DIR}/${ProjectName}-config.cmake @ONLY)

  if(NOT EXISTS ${PROJECT_SOURCE_DIR}/${ProjectName}-config-version.cmake.in)
    file(WRITE ${PROJECT_BINARY_DIR}/CMakeFiles/${ProjectName}-config-version.cmake.in
"set(PACKAGE_VERSION \"${ProjectVersionString}\")

if(\"\${PACKAGE_FIND_VERSION_MAJOR}\" EQUAL \"${ProjectVersionMajor}\" AND
     \"\${PACKAGE_FIND_VERSION_MINOR}\" EQUAL \"${ProjectVersionMinor}\")
  set (PACKAGE_VERSION_COMPATIBLE 1) # compatible with newer
  if (\"\${PACKAGE_FIND_VERSION}\" VERSION_EQUAL \"${ProjectVersionString}\")
    set(PACKAGE_VERSION_EXACT 1) #exact match for this version
  endif()
endif()
")
    set(CONFIG_VERSION_FILE ${PROJECT_BINARY_DIR}/CMakeFiles/${ProjectName}-config-version.cmake.in)
  else()
    set(CONFIG_VERSION_FILE ${PROJECT_SOURCE_DIR}/${ProjectName}-config-version.cmake.in)
  endif()
  configure_file(
    ${CONFIG_VERSION_FILE}
    ${PROJECT_BINARY_DIR}/${ProjectName}-config-version.cmake @ONLY)

  # install dune.module file
  install(FILES dune.module DESTINATION ${DUNE_INSTALL_NONOBJECTLIBDIR}/dunecontrol/${ProjectName})

  # install cmake-config files
  install(FILES ${PROJECT_BINARY_DIR}/cmake/pkg/${ProjectName}-config.cmake
    ${PROJECT_BINARY_DIR}/${ProjectName}-config-version.cmake
    DESTINATION ${DUNE_INSTALL_LIBDIR}/cmake/${ProjectName})

  # install config.h
  if(EXISTS ${PROJECT_SOURCE_DIR}/config.h.cmake)
    install(FILES config.h.cmake DESTINATION share/${ProjectName})
  endif()

  # install pkg-config files
  create_and_install_pkconfig(${DUNE_INSTALL_LIBDIR})

  if("${ARGC}" EQUAL "1")
    message(STATUS "Adding custom target for config.h generation")
    dune_regenerate_config_cmake()
    # add a target to generate config.h.cmake
    if(NOT TARGET OUTPUT)
      add_custom_target(OUTPUT config_collected.h.cmake
        COMMAND dune_regenerate_config_cmake())
    endif()
    # actually write the config.h file to disk
    # using generated file
    configure_file(${CMAKE_CURRENT_BINARY_DIR}/config_collected.h.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/config.h)
  else()
    message(STATUS "Not adding custom target for config.h generation")
    # actually write the config.h file to disk
    configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
  endif()

  if(PROJECT_NAME STREQUAL CMAKE_PROJECT_NAME)
    feature_summary(WHAT ALL)
  endif()

  # check if CXX flag overloading has been enabled
  # and write compiler script (see OverloadCompilerFlags.cmake)
  finalize_compiler_script()
endmacro(finalize_dune_project)

macro(target_link_dune_default_libraries _target)
  foreach(_lib ${DUNE_DEFAULT_LIBS})
    target_link_libraries(${_target} PUBLIC ${_lib})
  endforeach()
endmacro(target_link_dune_default_libraries)

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
      endforeach()
    else()
      list(APPEND _new_SOURCES "${_source}")
    endif()
  endforeach()

  foreach(var _SOURCES _ADD_LIBS _COMPILE_FLAGS)
    set(${${var}_var} "${_new${var}}" PARENT_SCOPE)
  endforeach()
endfunction(dune_expand_object_libraries)

# Creates shared and static libraries with the same basename.
# More docu can be found at the top of this file.
macro(dune_add_library basename)
  include(CMakeParseArguments)
  cmake_parse_arguments(DUNE_LIB "APPEND;NO_EXPORT;OBJECT" "COMPILE_FLAGS"
    "ADD_LIBS;SOURCES" ${ARGN})
  list(APPEND DUNE_LIB_SOURCES ${DUNE_LIB_UNPARSED_ARGUMENTS})
  if(DUNE_LIB_OBJECT)
    if(DUNE_LIB_${basename}_SOURCES)
      message(FATAL_ERROR "There is already a library with the name ${basename}, "
        "but only one is allowed!")
    else()
      foreach(source ${DUNE_LIB_SOURCES})
        list(APPEND full_path_sources ${CMAKE_CURRENT_SOURCE_DIR}/${source})
      endforeach()
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
    endif()
  else(DUNE_LIB_OBJECT)
    dune_expand_object_libraries(DUNE_LIB_SOURCES DUNE_LIB_ADD_LIBS DUNE_LIB_COMPILE_FLAGS)
    #create lib
    add_library(${basename} ${DUNE_LIB_SOURCES})
    get_property(_prop GLOBAL PROPERTY DUNE_MODULE_LIBRARIES)
    set_property(GLOBAL PROPERTY DUNE_MODULE_LIBRARIES ${_prop} ${basename})
    # link with specified libraries.
    if(DUNE_LIB_ADD_LIBS)
      target_link_libraries(${basename} PUBLIC "${DUNE_LIB_ADD_LIBS}")
    endif()
    if(DUNE_LIB_COMPILE_FLAGS)
      set_property(${basename} APPEND_STRING COMPILE_FLAGS
        "${DUNE_LIB_COMPILE_FLAGS}")
    endif()
    # Build library in ${PROJECT_BINARY_DIR}/lib
    set_target_properties(${basename} PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
      ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

    if(NOT DUNE_LIB_NO_EXPORT)
      # The following allows for adding multiple libs in the same
      # directory or below with passing the APPEND keyword.
      # If there are additional calls to dune_add_library in other
      # modules then you have to use APPEND or otherwise only the
      # last lib will get exported as a target.
      if(NOT _MODULE_EXPORT_USED)
        set(_MODULE_EXPORT_USED ON)
        set(_append "")
      else()
        set(_append APPEND)
      endif()
      # Allow to explicitly pass APPEND
      if(DUNE_LIB_APPEND)
        set(_append APPEND)
      endif()

      # install targets to use the libraries in other modules.
      install(TARGETS ${basename}
        EXPORT ${ProjectName}-targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
      install(EXPORT ${ProjectName}-targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${ProjectName})

      # export libraries for use in build tree
      export(TARGETS ${basename} ${_append}
        FILE ${PROJECT_BINARY_DIR}/${ProjectName}-targets.cmake)
    endif()
  endif()
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
    endforeach()
    list(REMOVE_DUPLICATES replacement)
    set_property(${option_command} ${_target} ${REPLACE_APPEND}
      ${REPLACE_APPEND_STRING} PROPERTY ${REPLACE_PROPERTY} ${replacement})
  else()
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
        endif()
      endforeach()

      if(NOT matched)
        list(APPEND new_props ${prop})
      endif()
    endforeach()
    list(REMOVE_DUPLICATES new_props)
    set_property(${option_command} ${_target}
      PROPERTY ${REPLACE_PROPERTY} ${new_props})
  endif()
  get_property(properties ${option_command} ${_target} PROPERTY ${REPLACE_PROPERTY})
endmacro(replace_properties_for_one)

function(dune_target_link_libraries basename libraries)
  target_link_libraries(${basename} PUBLIC ${libraries})
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
    endif()
  endforeach()
  if(NOT MY_DIRECTORY)
    list(FIND REPLACE_UNPARSED_ARGUMENTS DIRECTORY _found)
    if(_found GREATER -1)
      list(REMOVE_AT REPLACE_UNPARSED_ARGUMENTS ${_found})
      set(MY_DIRECTORY TRUE)
      set(REPLACE_DIRECTORY "")
    endif()
  endif()

  # setup options
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
  endif()

  if(NOT length GREATER 0)
    message(ERROR "You need to specify at least on pair consisting of a regular expression
and a replacement string. ${REPLACE_UNPARSED_ARGUMENTS}")
  endif()

  foreach(_target ${option_arg})
    replace_properties_for_one()
  endforeach()

  list(LENGTH option_arg _length)
  if(_length EQUAL 0)
    replace_properties_for_one()
  endif()
endfunction(replace_properties)

macro(add_dune_all_flags targets)
  get_property(incs GLOBAL PROPERTY ALL_PKG_INCS)
  get_property(defs GLOBAL PROPERTY ALL_PKG_DEFS)
  get_property(libs GLOBAL PROPERTY ALL_PKG_LIBS)
  get_property(opts GLOBAL PROPERTY ALL_PKG_OPTS)
  foreach(target ${targets})
    set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES ${incs})
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_DEFINITIONS ${defs})
    target_link_libraries(${target} PUBLIC ${DUNE_LIBS} ${libs})
    target_compile_options(${target} PUBLIC ${opts})
  endforeach()
endmacro(add_dune_all_flags targets)
