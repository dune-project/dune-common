# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DunePythonFindPackage
---------------------

Helpers for checking whether Python packages are available for a given
interpreter.

.. cmake:command:: dune_python_find_package

  Check whether a Python package can be imported and optionally whether it
  satisfies a version constraint.

  .. code-block:: cmake

    dune_python_find_package(
      PACKAGE <package>
      [RESULT <var>]
      [REQUIRED]
      [VERSION <version>]
      [EXACT]
      [INTERPRETER <python>]
    )

  ``PACKAGE``
    Name of the Python package to look for.

  ``RESULT``
    Variable that receives the result in the calling scope. If omitted, the
    default is ``DUNE_PYTHON_<package>_FOUND``. The package name is case
    sensitive and is typically lowercase.

  ``REQUIRED``
    If given, configuration fails when the package cannot be found.

  ``VERSION``
    Minimum version required for the package.

  ``EXACT``
    Require the version given by ``VERSION`` to match exactly.

  ``INTERPRETER``
    Python interpreter whose search path is used. The default is
    ``Python3::Interpreter``. This can differ from the system interpreter when
    using DUNE's configure-time virtual environment.

  The package check is performed by importing the module with the selected
  interpreter. If the import succeeds, the package version is queried through
  the dune-common helper script ``pyversion.py``.

#]=======================================================================]
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
    set(PYPACKAGE_INTERPRETER Python3::Interpreter)
  endif()
  if(PYPACKAGE_EXACT AND NOT PYPACKAGE_VERSION)
    message(FATAL_ERROR "dune_python_find_package: EXACT given, but no VERSION specified.")
  endif()

  # Find interpreter path location
  set(PYPACKAGE_LOCATION ${PYPACKAGE_INTERPRETER})
  if(TARGET ${PYPACKAGE_INTERPRETER})
    get_target_property(PYPACKAGE_LOCATION ${PYPACKAGE_INTERPRETER} LOCATION)
  endif()

  # Do the actual check
  execute_process(COMMAND "${PYPACKAGE_LOCATION}" -c "import ${PYPACKAGE_PACKAGE}"
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
    execute_process(COMMAND "${PYPACKAGE_LOCATION}" "${scriptdir}/pyversion.py" "${PYPACKAGE_PACKAGE}"
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
      message(FATAL_ERROR "The python package ${PYPACKAGE_PACKAGE} could not be found! (for interpreter in ${PYPACKAGE_LOCATION})")
    endif()
  endif()

  # Set the result variable and print the result
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(${PYPACKAGE_PACKAGE}_${PYPACKAGE_INTERPRETER}
                                    "Failed to find the python package ${PYPACKAGE_PACKAGE} with interpreter in ${PYPACKAGE_LOCATION}."
                                    ${PYPACKAGE_RESULT}
                                    )
  set(${PYPACKAGE_RESULT} ${${PYPACKAGE_RESULT}} PARENT_SCOPE)
endfunction()
