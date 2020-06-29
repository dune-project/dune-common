#[=======================================================================[.rst:
DuneModuleDependencies
----------------------

Definition of internal functions to manage and extract dune module dependencies

  dune_create_dependency_tree()
  dune_process_dependency_macros()

Some more private functions are defined in this file:

  dune_module_to_macro(<macro_name> <dune_module>)
  dune_process_dependency_leafs(...)
  remove_processed_modules(...)
  dune_create_dependency_leafs(...)

#]=======================================================================]

include_guard(GLOBAL)

include(DuneEnableAllPackages)
include(FindDunePackage)
include(Utilities)

##
# Converts a module name given by _dune_module into a string _macro_name
# where all dashes (-) are removed and letters after a dash are capitalized
# Example: dune-grid-howto -> DuneGridHowto
##
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


##
# Documentation here!
##
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


##
# Documentation here!
##
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


##
# Documentation here!
##
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


##
# Creates the dependency tree of the module.
##
macro(dune_create_dependency_tree)
  if(dune-common_MODULE_PATH)
    list(REMOVE_ITEM CMAKE_MODULE_PATH "${dune-common_MODULE_PATH}")
  endif()
  list(FIND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules start)
  set(ALL_DEPENDENCIES "")
  if(${PROJECT_NAME}_DEPENDS_MODULE OR ${PROJECT_NAME}_SUGGESTS_MODULE)
    set(ALL_DEPENDENCIES ${${PROJECT_NAME}_DEPENDS_MODULE} ${${PROJECT_NAME}_SUGGESTS_MODULE})
    dune_create_dependency_leafs("${${PROJECT_NAME}_DEPENDS_MODULE}" "${${PROJECT_NAME}_DEPENDS_VERSION}"
      "${${PROJECT_NAME}_SUGGESTS_MODULE}" "${${PROJECT_NAME}_SUGGESTS_VERSION}")
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


##
# Process the Dune<module>.cmake file
##
macro(dune_process_dependency_macros)
  foreach(_mod ${ALL_DEPENDENCIES} ${PROJECT_NAME})
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

  # link dependencies
  foreach(_mod ${ALL_DEPENDENCIES})
    if(TARGET Dune::${_mod})
      target_link_libraries(${PROJECT_NAME} PUBLIC Dune::${_mod})
    endif()
  endforeach(_mod)

endmacro(dune_process_dependency_macros)
