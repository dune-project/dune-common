# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

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
#       in the calling scope. Defaults to :code:`DUNE_PYTHON_<package>_FOUND`
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
#    .. cmake_param:: INTERPRETER
#       :single:
#
#       The python interpreter, whose paths are searched for the package.
#       Defaults to :code:`${Python3_EXECUTABLE}`, might differ when dealing with
#       the configure-time virtualenv set up with :ref:`DUNE_PYTHON_VIRTUALENV_SETUP`.
#
#    Find a given python package on the system.
#
include_guard(GLOBAL)

function(dune_python_find_package)
  # Parse Arguments
  set(OPTION REQUIRED EXACT)
  set(SINGLE PACKAGE RESULT VERSION INTERPRETER)
  cmake_parse_arguments(PYPACKAGE "${OPTION}" "${SINGLE}" "" ${ARGN})
  if(PYCHECK_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_find_package: This often indicates typos!")
  endif()

  # Do error checking on input and apply defaults
  if(NOT PYPACKAGE_RESULT)
    set(PYPACKAGE_RESULT DUNE_PYTHON_${PYPACKAGE_PACKAGE}_FOUND)
  endif()
  if(NOT PYPACKAGE_INTERPRETER)
    set(PYPACKAGE_INTERPRETER "${Python3_EXECUTABLE}")
  endif()
  if(PYPACKAGE_EXACT AND NOT PYPACKAGE_VERSION)
    message(FATAL_ERROR "dune_python_find_package: EXACT given, but no VERSION specified.")
  endif()

  # Do the actual check
  execute_process(COMMAND "${PYPACKAGE_INTERPRETER}" -c "import ${PYPACKAGE_PACKAGE}"
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
    execute_process(COMMAND "${PYPACKAGE_INTERPRETER}" "${scriptdir}/pyversion.py" "${PYPACKAGE_PACKAGE}"
                    RESULT_VARIABLE retcode
                    OUTPUT_VARIABLE VERSION_STRING
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    )

    set(${PYPACKAGE_RESULT} TRUE)
    if("${retcode}" STREQUAL "0")
      if(("${VERSION_STRING}" VERSION_LESS "${PYPACKAGE_VERSION}") OR
         (PYPACKAGE_EXACT AND NOT ("${VERSION_STRING}" VERSION_EQUAL "${PYPACKAGE_VERSION}")))
        set(${PYPACKAGE_RESULT} FALSE)
      endif()
    else()
      set(VERSION_STRING "unknown version")
      if(PYPACKAGE_VERSION)
        set(${PYPACKAGE_RESULT} FALSE)
      endif()
    endif()
  else()
    set(${PYPACKAGE_RESULT} FALSE)
    if(PYPACKAGE_REQUIRED)
      message(FATAL_ERROR "The python package ${PYPACKAGE_PACKAGE} could not be found! (for interpreter ${PYPACKAGE_INTERPRETER})")
    endif()
  endif()

  # Set the result variable and print the result
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(${PYPACKAGE_PACKAGE}_${PYPACKAGE_INTERPRETER}
                                    "Failed to find the python package ${PYPACKAGE_PACKAGE} with interpreter ${PYPACKAGE_INTERPRETER}."
                                    ${PYPACKAGE_RESULT}
                                    )
  set(${PYPACKAGE_RESULT} ${${PYPACKAGE_RESULT}} PARENT_SCOPE)
endfunction()
