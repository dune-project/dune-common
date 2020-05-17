# This module provides functions to check for the existence of python packages on the host system.
#
# .. cmake_function:: dune_find_pybind11
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
#       Defaults to :code:`${PYTHON_EXECUTABLE}`, might differ when dealing with
#       the configure-time virtualenv set up with :ref:`DUNE_PYTHON_VIRTUALENV_SETUP`.
#
#    Find the python package for pybind11 on the system and store the
#    location of the header files in :code:PYBIND11_INCLUDE_DIRS.
#

function(dune_find_pybind11)
  # Parse Arguments
  set(OPTION REQUIRED EXACT)
  set(SINGLE VERSION INTERPRETER)
  set(MULTI)
  include(CMakeParseArguments)
  include(DunePythonFindPackage)
  cmake_parse_arguments(PYBIND "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYCHECK_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_find_package: This often indicates typos!")
  endif()

  # Do error checking on input and apply defaults
  if(NOT PYBIND_INTERPRETER)
    set(PYBIND_INTERPRETER "${PYTHON_EXECUTABLE}")
  endif()
  if(NOT PYBIND_REQUIRED)
    set(PYBIND_REQUIRED FALSE)
  endif()
  if(NOT PYBIND_EXACT)
    set(PYBIND_EXACT FALSE)
  endif()

  dune_python_find_package(PACKAGE "pybind11"
                           REQUIRED ${PYBIND_REQUIRED})
  #                         VERSION ${PYBIND_VERSION}
  #                         EXACT ${PYBIND_EXACT})

  if(DUNE_PYTHON_pybind11_FOUND)
    # pybind11 was found and we can check for the include path, note that
    # the check for version and 'required' was taken care of by
    # `dune_python_find_package` so we don't have to worry about checking
    # additional parameters
    # run the pybind11 script to find location of its header files
    execute_process(COMMAND "${PYBIND_INTERPRETER}" -m pybind11 --includes
                  RESULT_VARIABLE retcode
                  OUTPUT_VARIABLE PYBINDINC
                  OUTPUT_STRIP_TRAILING_WHITESPACE
                  ERROR_QUIET)

    set(PYBIND11_INCLUDE_DIRS ${PYBINDINC})
    # separate_arguments(PYBIND11_INCLUDE_DIRS)
    set(PYBIND11_INCLUDE_DIRS ${PYBIND11_INCLUDE_DIRS} PARENT_SCOPE)
    message(STATUS "pybind11 include dirs=${PYBIND11_INCLUDE_DIRS}")
    # dune_register_package_flags(COMPILE_DEFINITIONS "ENABLE_PYBIND11=1"
    #                             INCLUDE_DIRS "${PYBIND11_INCLUDE_DIRS}")
  endif()
endfunction()
