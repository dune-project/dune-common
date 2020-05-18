# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
DuneCheckCXXSourceCompiles
----------------------

Check if given C++ source compiles and links into an executable with the compiel and
link properties derived from a given target.

.. command:: dune_check_cxx_source_compiles

  .. code-block:: cmake

    dune_check_cxx_source_compiles(<target> <code> <resultVar>)

  Check that the source supplied in ``<code>`` can be compiled as a C++ source
  file and linked as an executable (so it must contain at least a ``main()``
  function). The result will be stored in the internal cache variable specified
  by ``<resultVar>``, with a boolean true value for success and boolean false
  for failure.

  The underlying check is performed by the :command:`try_compile` command. The
  compile and link commands are incluences by the target properties of ``<target>``.

  The check is only performed once, with the result cached in the variable
  named by ``<resultVar>``. Every subsequent CMake run will re-use this cached
  value rather than performing the check again, even if the ``<code>`` changes.
  In order to force the check to be re-evaluated, the variable named by
  ``<resultVar>`` must be manually removed from the cache.

#]=======================================================================]

include_guard(GLOBAL)

# Get the property of the TARGET or return the DEFAULT value
macro(get_target_property_opt OUT_VAR TARGET PROPERTY DEFAULT)
  get_target_property(${OUT_VAR}_tmp ${TARGET} ${PROPERTY})

  set(${OUT_VAR} ${DEFAULT})
  if (${OUT_VAR}_tmp)
    set(${OUT_VAR} ${${OUT_VAR}_tmp})
  endif ()
endmacro(get_target_property_opt)

# Get the properties of the TARGET and combine the resulting lists or return an empty list
# All requested properties are listed after the TARGET argument
macro(get_target_properties_opt OUT_VAR TARGET)
  set(${OUT_VAR} "")
  foreach (PROP ${ARGN})
    get_target_property(${OUT_VAR}_${PROP} ${TARGET} ${PROP})
    if (${OUT_VAR}_${PROP})
      list(APPEND ${OUT_VAR} ${${OUT_VAR}_${PROP}})
    endif ()
  endforeach ()
endmacro(get_target_properties_opt)

# Check whether SOURCE compiles when compiled and linked with the corresponding properties of given TARGET
macro(dune_check_cxx_source_compiles TARGET SOURCE VAR)
  if(NOT DEFINED "${VAR}")
    # write source to file
    file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.cxx"
      "${SOURCE}\n")

    # get compile and link properties from target
    get_target_properties_opt(ARG_INCLUDE_DIRECTORIES ${TARGET} INCLUDE_DIRECTORIES INTERFACE_INCLUDE_DIRECTORIES)
    get_target_properties_opt(ARG_COMPILE_OPTIONS ${TARGET} COMPILE_OPTIONS INTERFACE_COMPILE_OPTIONS)
    get_target_properties_opt(ARG_COMPILE_DEFINITIONS ${TARGET} COMPILE_DEFINITIONS INTERFACE_COMPILE_DEFINITIONS)
    get_target_properties_opt(ARG_LINK_LIBRARIES ${TARGET} LINK_LIBRARIES INTERFACE_LINK_LIBRARIES)
    get_target_properties_opt(ARG_LINK_OPTIONS ${TARGET} LINK_OPTIONS INTERFACE_LINK_OPTIONS)

    list(TRANSFORM ARG_COMPILE_DEFINITIONS PREPEND "-D")
    if (${ARG_COMPILE_OPTIONS})
      set(ARG_COMPILE_DEFINITIONS "${ARG_COMPILE_DEFINITIONS};${ARG_COMPILE_OPTIONS}")
    endif ()

    get_target_property_opt(ARG_CXX_STANDARD ${TARGET} CXX_STANDARD "17")
    get_target_property_opt(ARG_CXX_STANDARD_REQUIRED ${TARGET} CXX_STANDARD_REQUIRED "TRUE")
    get_target_property_opt(ARG_CXX_EXTENSIONS ${TARGET} CXX_EXTENSIONS "TRUE")

    if(NOT CMAKE_REQUIRED_QUIET)
      message(CHECK_START "Performing Test ${VAR}")
    endif()

    # try to compile the source
    try_compile(${VAR}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.cxx
      COMPILE_DEFINITIONS ${ARG_COMPILE_DEFINITIONS}
      LINK_LIBRARIES ${ARG_LINK_LIBRARIES}
      LINK_OPTIONS ${ARG_LINK_OPTIONS}
      CXX_STANDARD ${ARG_CXX_STANDARD}
      CXX_STANDARD_REQUIRED ${ARG_CXX_STANDARD_REQUIRED}
      CXX_EXTENSIONS ${ARG_CXX_EXTENSIONS}
      CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${ARG_INCLUDE_DIRECTORIES}"
      OUTPUT_VARIABLE OUTPUT)

    # evaluate the result
    if(${VAR})
      set(${VAR} 1 CACHE INTERNAL "Test ${VAR}")
      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_PASS "Success")
      endif()
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Performing C++ SOURCE FILE Test ${VAR} succeeded with the following output:\n"
        "${OUTPUT}\n"
        "Source file was:\n${SOURCE}\n")
    else()
      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_FAIL "Failed")
      endif()
      set(${VAR} "" CACHE INTERNAL "Test ${VAR}")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Performing C++ SOURCE FILE Test ${VAR} failed with the following output:\n"
        "${OUTPUT}\n"
        "Source file was:\n${SOURCE}\n")
    endif()
  endif()
endmacro()
