# Core DUNE module for CMake.
#
# Documentation of the public API defined in this module:
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

include_guard(GLOBAL)

enable_language(C) # Enable C to skip CXX bindings for some tests.

include(FeatureSummary)
include(DuneEnableAllPackages)
include(DuneTestMacros)
include(OverloadCompilerFlags)
include(DuneSymlinkOrCopy)
include(DunePathHelper)
include(DuneExecuteProcess)
include(DuneModuleInformation)
include(DuneModuleDependencies)
include(DuneProject)
include(FinalizeDuneProject)
include(GNUInstallDirs)
include(Headercheck)
include(TargetOptionalCompileFeatures)
include(TargetSourcesLocal)

##
# Documentation here!
##
macro(target_link_dune_default_libraries _target)
  foreach(_lib ${DUNE_DEFAULT_LIBS})
    target_link_libraries(${_target} ${_lib})
  endforeach()
endmacro(target_link_dune_default_libraries)


##
# Documentation here!
##
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


##
# Creates shared and static libraries with the same basename.
# More docu can be found at the top of this file.
##
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
    set_property(GLOBAL APPEND PROPERTY DUNE_MODULE_LIBRARIES ${basename})
    # link with specified libraries.
    target_link_libraries(${basename} PRIVATE ${PROJECT_NAME})
    if(DUNE_LIB_ADD_LIBS)
      target_link_libraries(${basename} PRIVATE "${DUNE_LIB_ADD_LIBS}")
    endif()
    if(DUNE_LIB_COMPILE_FLAGS)
      target_compile_flags(${basename} "${DUNE_LIB_COMPILE_FLAGS}")
    endif()
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
        target_link_libraries(${basename}-static PRIVATE ${PROJECT_NAME})
        if(DUNE_LIB_ADD_LIBS)
          target_link_libraries(${basename}-static PRIVATE "${DUNE_LIB_ADD_LIBS}")
        endif()
        if(DUNE_LIB_COMPILE_FLAGS)
          target_compile_flags(${basename}-static PRIVATE "${DUNE_LIB_COMPILE_FLAGS}")
        endif()
      else()
        #create shared libs
        add_library(${basename}-shared SHARED  ${DUNE_LIB_SOURCES})
        set_target_properties(${basename}-shared PROPERTIES
          OUTPUT_NAME ${basename}
          LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
        # link with specified libraries.
        target_link_libraries(${basename}-shared PRIVATE ${PROJECT_NAME})
        if(DUNE_LIB_ADD_LIBS)
          target_link_libraries(${basename}-shared PRIVATE "${DUNE_LIB_ADD_LIBS}")
        endif()
        if(DUNE_LIB_COMPILE_FLAGS)
          target_compile_flags(${basename}-shared PRIVATE "${DUNE_LIB_COMPILE_FLAGS}")
        endif()
        list(APPEND _created_libs ${basename}-shared)
      endif()
    endif()

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
      install(TARGETS ${_created_libs}
        EXPORT ${PROJECT_NAME}-legacy-targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
      install(EXPORT ${PROJECT_NAME}-legacy-targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

      # export libraries for use in build tree
      export(TARGETS ${_created_libs} ${_append}
        FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-legacy-targets.cmake)
    endif()
  endif()
endmacro(dune_add_library basename sources)


##
# Documentation here!
##
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


##
# Documentation here!
##
function(dune_target_link_libraries basename)
  target_link_libraries(${basename} PUBLIC ${ARGN})
  if(DUNE_BUILD_BOTH_LIBS)
    if(BUILD_SHARED_LIBS)
      target_link_libraries(${basename}-static PUBLIC ${ARGN})
    else()
      target_link_libraries(${basename}-shared PUBLIC ${ARGN})
    endif()
  endif()
endfunction(dune_target_link_libraries)


##
# Documentation here!
##
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


##
# Documentation here!
##
macro(add_dune_all_flags targets)
  get_property(incs GLOBAL PROPERTY ALL_PKG_INCS)
  get_property(defs GLOBAL PROPERTY ALL_PKG_DEFS)
  get_property(libs GLOBAL PROPERTY ALL_PKG_LIBS)
  get_property(opts GLOBAL PROPERTY ALL_PKG_OPTS)
  foreach(target ${targets})
    set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES ${incs})
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_DEFINITIONS ${defs})
    target_link_libraries(${target} PUBLIC ${libs})
    target_compile_options(${target} PUBLIC ${opts})
  endforeach()
endmacro(add_dune_all_flags targets)
