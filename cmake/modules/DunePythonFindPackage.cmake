# This module provides functions to check for the existence of python packages on the host system.
#
# .. cmake_function:: dune_python_find_package
#
#    .. cmake_param:: PACKAGE
#       :required:
#       :single:
#
#       The package name to look for.
#
#    .. cmake_param: RESULT
#       :single:
#
#       The variable to store the result of the check in
#       in the calling scope. Defaults to DUNE_PYTHON_<package>_FOUND
#       Note that the package name is case sensitive and will
#       usually be lowercase.
#
#    .. cmake_param:: REQUIRED
#       :option:
#
#       If set, the function will error out if the package is not
#       found.
#
#    .. cmake_param:: VERSION
#       :single:
#
#       The minimum version of the package that is required.
#
#    .. cmake_param:: EXACT
#       :option:
#
#       Whether the given version requirement has to be matched exactly.
#
#
#    Find a given python package on the host system. Note, that you should
#    only use this macro in the context of dune-python if you really need the
#    package to be present on the host. Any dependencies of your python packages
#    will instead be installed into the dune-python virtualenv. dune-python
#    uses this module to check for the existence of the virtualenv and pip packages.

function(dune_python_find_package)
  # Parse Arguments
  set(OPTION REQUIRED EXACT)
  set(SINGLE PACKAGE RESULT VERSION)
  set(MULTI)
  include(CMakeParseArguments)
  cmake_parse_arguments(PYPACKAGE "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYCHECK_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_find_package: This often indicates typos!")
  endif()

  # Do error checking on input and apply defaults
  if(NOT PYPACKAGE_RESULT)
    set(PYPACKAGE_RESULT DUNE_PYTHON_${PYPACKAGE_PACKAGE}_FOUND)
  endif()
  if(PYPACKAGE_EXACT AND NOT PYPACKAGE_VERSION)
    message(FATAL_ERROR "dune_python_find_package: EXACT given, but no VERSION specified.")
  endif()

  # Do the actual check
  execute_process(COMMAND "${PYTHON_EXECUTABLE}" -c "import ${PYPACKAGE_PACKAGE}"
                  RESULT_VARIABLE PYPACKAGE_RETURN
                  ERROR_QUIET)

  # Perform additional checks
  if(PYPACKAGE_RETURN STREQUAL "0")
    include(DunePathHelper)
    dune_module_path(MODULE dune-common
                     RESULT scriptdir
                     SCRIPT_DIR)

    # Check the found version of the given python package
    # We cannot use find_package_handle_standard_args for that, as it is too
    # closely tied to using find_package(), which we cannot use for variable package
    # name...
    execute_process(COMMAND "${PYTHON_EXECUTABLE}" "${scriptdir}/pyversion.py" "${PYPACKAGE_PACKAGE}"
                    RESULT_VARIABLE retcode
                    OUTPUT_VARIABLE VERSION_STRING
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    OUTPUT_QUIET
                    )

    if("${retcode}" STREQUAL "0")
      if(("${VERSION_STRING}" VERSION_LESS "${PYPACKAGE_VERSION}") OR
         (PYPACKAGE_EXACT AND NOT ("${VERSION_STRING}" VERSION_EQUAL "${PYPACKAGE_VERSION}")))
        set(fail TRUE)
      endif()
    else()
      set(VERSION_STRING "unknown version")
      if(PYPACKAGE_VERSION)
        set(fail TRUE)
      endif()
    endif()
  else()
    set(fail TRUE)
    if(PYPACKAGE_REQUIRED)
      message(FATAL_ERROR "The python package ${PYCHECK_PACKAGE} could not be found! (for interpreter ${PYTHON_EXECUTABLE})")
    endif()
  endif()

  # Set the result variable and print the result
  include(FindPackageHandleStandardArgs)
  if(fail)
    set(${PYPACKAGE_RESULT} FALSE)
  else()
    set(${PYPACKAGE_RESULT} TRUE)
  endif()
  find_package_handle_standard_args(${PYPACKAGE_PACKAGE}
                                    "Failed to find the python package ${PYPACKAGE_PACKAGE} with interpreter ${PYTHON_EXECUTABLE}."
                                    ${PYPACKAGE_RESULT}
                                    )
  set(${PYPACKAGE_RESULT} ${${PYPACKAGE_RESULT}} PARENT_SCOPE)
endfunction()
