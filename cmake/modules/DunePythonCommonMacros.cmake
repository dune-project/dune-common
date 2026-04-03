# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
DunePythonCommonMacros
----------------------

Main entry point for the Python extension of the DUNE CMake build system.

This module detects the Python installation, sets up Python-related helper
targets, configures the DUNE Python virtual environment, and includes the
helper modules that implement package installation and Python-based tests.

To disable all Python-related functionality, configure with
``-DCMAKE_DISABLE_FIND_PACKAGE_Python3=TRUE``.

If you use DUNE modules that provide Python functionality, keep in mind:

* CMake discovers the Python interpreter during configuration. Activate any
  desired virtual environment before configuring.
* Each module gets an ``install_python`` target for Python packages, and that
  installation is also part of ``make install``.
* Configure-time virtual environment management is provided by
  :doc:`DunePythonVirtualenv`.

After this module has run, Python-related configuration variables such as the
interpreter discovery results and ``DUNE_PYTHON_SYSTEM_IS_VIRTUALENV`` are
available to downstream modules.

.. cmake:variable:: DUNE_PYTHON_INSTALL_LOCATION

  Control where DUNE installs Python packages. Supported values are:

  * ``user`` for installation through ``pip --user``. This is incompatible
    with virtual environments.
  * ``system`` for installation into the standard paths of the interpreter
    found by CMake.
  * ``none`` to disable package installation.
  * ``--target <path>`` to forward a custom target directory to pip.

  The default depends on whether the selected interpreter already runs inside a
  virtual environment. In that case ``system`` is used, otherwise ``user`` is
  selected.

#]=======================================================================]
include_guard(GLOBAL)

# unless the user has defined the variable, unversioned names (like python3) are found
# first, to match what users most probably use later on to call the executable
if(NOT DEFINED Python3_FIND_UNVERSIONED_NAMES)
  set(Python3_FIND_UNVERSIONED_NAMES "FIRST")
endif()

# include code from CMake 3.20 to back-port using unversioned Python first
if(${CMAKE_VERSION} VERSION_LESS "3.20")
  list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_LIST_DIR}/FindPython3")
endif()

# Include all the other parts of the python extension to avoid that users need
# to explicitly include parts of our build system.
include(DunePythonFindPackage)
include(DunePythonInstallPackage)
include(DunePythonTestCommand)

# Find the Python Interpreter and libraries
find_package(Python3 COMPONENTS Interpreter Development)

# this option enables the build of Python bindings for DUNE modules per default
option(DUNE_ENABLE_PYTHONBINDINGS "Enable Python bindings for DUNE" ON)

# this option enables pre-compilation of certain modules in dune-common and dune-geometry
option(DUNE_ENABLE_PYTHONMODULE_PRECOMPILE "Enable module precompilation for certain Python modules in DUNE (can be set as environment variable)" $ENV{DUNE_ENABLE_PYTHONMODULE_PRECOMPILE})

# if python bindings are disabled then this option has to be disabled also
if(NOT DUNE_ENABLE_PYTHONBINDINGS)
  set(DUNE_ENABLE_PYTHONMODULE_PRECOMPILE OFF)
endif()

# helper message used below in various user messages
set(DUNE_PYTHON_BINDINGS_USER_NOTICE "If you do not plan to use the Dune Python bindings you can ignore this information")

if(Python3_Interpreter_FOUND)
  include(DuneExecuteProcess)
  include(DunePathHelper)
  dune_module_path(MODULE dune-common
                   RESULT scriptdir
                   SCRIPT_DIR)

  # Check presence of python packages required by the buildsystem
  dune_python_find_package(PACKAGE pip)

  # Add python related meta targets
  add_custom_target(test_python COMMAND ctest CMD_ARGS -L python)
  add_custom_target(install_python)

  ##### Python bindings specific part begin ################
  # first we test if all requirements are satisfied, if not, Python bindings are
  # disabled and the user gets an informative message explaining why
  if((DUNE_ENABLE_PYTHONBINDINGS) AND (NOT ${Python3_Development.Module_FOUND}))
    message(STATUS "Python bindings disabled")
    message(NOTICE
      "   ----------------------------------------------------------------------------------------\n"
      "   Found a Python interpreter but the Python bindings also requires the Python libraries.\n"
      "   On Linux systems they may be installed in form of a package like python3-dev, python3-devel, python-dev or python-devel (depending on your distribution).\n"
      "   ${DUNE_PYTHON_BINDINGS_USER_NOTICE}.\n"
      "   ----------------------------------------------------------------------------------------\n"
    )
    set(DUNE_ENABLE_PYTHONBINDINGS OFF CACHE BOOL "Disabled Python bindings (requirements not satisfied)" FORCE)
    return()
  endif()

  # the Python bindings currently require the following minimum Python version
  set(DUNE_PYTHON_BINDINGS_MIN_PYTHON_VERSION 3.7)
  if((DUNE_ENABLE_PYTHONBINDINGS) AND (Python3_VERSION VERSION_LESS ${DUNE_PYTHON_BINDINGS_MIN_PYTHON_VERSION}))
    message(STATUS "Python bindings disabled")
    message(NOTICE
      "   ----------------------------------------------------------------------------------------\n"
      "   Python bindings require at least Python version ${DUNE_PYTHON_BINDINGS_MIN_PYTHON_VERSION} but only version ${Python3_VERSION} was found.\n"
      "   ${DUNE_PYTHON_BINDINGS_USER_NOTICE}.\n"
      "   ----------------------------------------------------------------------------------------\n"
    )
    set(DUNE_ENABLE_PYTHONBINDINGS OFF CACHE BOOL "Disabled Python bindings (requirements not satisfied)" FORCE)
    return()
  endif()

  if(DUNE_ENABLE_PYTHONBINDINGS)
    include_directories("${Python3_INCLUDE_DIRS}")
    include(DuneAddPybind11Module)
  endif()
  ##### Python bindings end ################

  # Set up the Dune-internal virtualenv
  include(DunePythonVirtualenv)

  # Determine where to install python packages
  if(DUNE_PYTHON_SYSTEM_IS_VIRTUALENV)
    set(DUNE_PYTHON_INSTALL_LOCATION "system" CACHE STRING "Location of where to install python packages")
  else()
    set(DUNE_PYTHON_INSTALL_LOCATION "user" CACHE STRING "Location of where to install python packages")
  endif()

  if(NOT(("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "user") OR
         ("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "system") OR
         ("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "none") OR
         ("${DUNE_PYTHON_INSTALL_LOCATION}" MATCHES "--target")   # this allows to provide a folder, i.e., --target /home/foo/site-packages
    ))
    message(FATAL_ERROR "DUNE_PYTHON_INSTALL_LOCATION must be user|system|none|--target <target>.")
  endif()

  if(("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "user") AND DUNE_PYTHON_SYSTEM_IS_VIRTUALENV)
    message(FATAL_ERROR "Specifying 'user' as install location is incompatible with using virtual environments (as per pip docs)")
  endif()

else()
  message(STATUS "Python bindings disabled")
  message(NOTICE
        "   ----------------------------------------------------------------------------------------\n"
        "   Python bindings require a Python3 interpreter.\n"
        "   ${DUNE_PYTHON_BINDINGS_USER_NOTICE}.\n"
        "   ----------------------------------------------------------------------------------------\n"
  )
  set(DUNE_ENABLE_PYTHONBINDINGS OFF CACHE BOOL "Disabled Python bindings (requirements not satisfied)" FORCE)
endif()

include(DuneSymlinkOrCopy)

# TODO: This function should have dune prefix and use named arguments
function(add_python_targets base)
  if(Python3_Interpreter_FOUND)
    if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
      message(WARNING "Source and binary dir are the same, skipping symlink!")
    else()
      foreach(file ${ARGN})
        dune_symlink_to_source_files(FILES ${file}.py)
      endforeach()
    endif()
  endif()
endfunction()
