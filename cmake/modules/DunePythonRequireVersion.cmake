# This module provides functions to implement constraints on the version of the python interpreter.
#
# .. cmake_function:: dune_python_require_version
#
#    .. cmake_param:: version
#       :positional:
#       :single:
#       :required:
#
#       The minimum required version.
#
#    Error out, if the python interpreter found by cmake is older than the given version.
#    Python2 and Python3 versions are not compared to each other, so you can call this
#    macro multiple times to enforce minimal versions on a python2 and python3 interpreter
#    independently.
#
# .. cmake_function:: dune_python_force_version
#
#    .. cmake_param:: version
#       :positional:
#       :single:
#       :required:
#
#       The major python version: 2 or 3
#
#    Enforce the major version of the python interpreter to be either 2 or 3.
#
#    .. note::
#
#       This is the developer-facing interface to enforce a major python version.
#       A user can set the input variable :ref:`DUNE_PYTHON_FORCE_PYTHON2` or
#       :ref:`DUNE_PYTHON_FORCE_PYTHON3` to achieve the same result for a given build only.
#
# .. cmake_variable:: DUNE_PYTHON_FORCE_PYTHON2
#
#     Set this variable to TRUE to force usage of a python2 interpreter. This is
#     the *user-facing* interface, developers of Dune modules, may force the python
#     major version through :ref:`dune_python_force_version`.
#
#     .. note::
#        This does not check for the interpreter requirements of your python packages.
#        If you set it and one of your packages requires python3, you will get an error.
#
#
# .. cmake_variable:: DUNE_PYTHON_FORCE_PYTHON3
#
#     Set this variable to TRUE to force usage of a python3 interpreter. This is
#     the *user-facing* interface, developers of Dune modules, may force the python
#     major version through :ref:`dune_python_force_version`.
#
#     .. note::
#        This does not check for the interpreter requirements of your python packages.
#        If you set it and one of your packages requires python2, you will get an error.
#


function(dune_python_require_version)
  string(REPLACE "." ";" versionlist ${ARGV0})
  list(GET versionlist 0 major)
  if("${major}" STREQUAL "2")
    # This is a python2 requirement.
    if("${PYTHON_VERSION_MAJOR}" STREQUAL "2" AND PYTHON_VERSION_STRING VERSION_LESS ${ARGV0})
      message(FATAL_ERROR "${CMAKE_PROJECT_NAME} requires at least python ${ARGV0}")
    endif()
  else()
    # This is a python3 requirement.
    if("${PYTHON_VERSION_MAJOR}" STREQUAL "3" AND PYTHON_VERSION_STRING VERSION_LESS ${ARGV0})
      message(FATAL_ERROR "${CMAKE_PROJECT_NAME} requires at least python ${ARGV0}")
    endif()
  endif()
endfunction()


function(dune_python_force_version)
  if(NOT "${PYTHON_VERSION_MAJOR}" STREQUAL "${ARGV0}")
    message(FATAL_ERROR "${CMAKE_PROJECT_NAME} requires python ${ARGV0}!")
  endif()
endfunction()
