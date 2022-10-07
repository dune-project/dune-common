# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Manage the creation of a configure-time virtual environment
#
# .. cmake_module::
#
#    This module manages the creation of virtual python environment during
#    configuration. The purpose of this virtual environment is to be able to run
#    python code from cmake in situations such as python-based code generation,
#    running postprocessing in python during testing etc.
#    If enabled the Python bindings are also installed (editable) into the
#    internal virtual environment.
#
#    The internal virtual environment is only generated if the
#    configuration is not run with an active virtual environment. If an
#    external environment is detected this will be used instead of the
#    internal environment.
#
#    Although designed for internal use, the internal (or detected external)
#    virtualenv can also be manually inspected. A symlink to the activation script is
#    placed in the top level build directory of all Dune modules in the stack.
#    To directly execute a command in the virtualenv, you can use the script
#    :code:`run-in-dune-env <command>`, which is also placed into every build directory.
#
#    All packages installed with :ref:`dune_python_install_package` are automatically
#    installed into the virtualenv.
#
#    After execution of this module, the following are available for use in
#    downstream modules:
#
#    * :code:`DUNE_PYTHON_VIRTUALENV_PATH` The path of the virtual environment
#    * :code:`DUNE_PYTHON_VIRTUALENV_EXECUTABLE` The python interpreter in the virtual environment
#
#    By default, the created virtualenv resides in the first non-installed Dune module of
#    the module stack (if no installation is performed: dune-common). Be aware
#    that mixing installed and non-installed modules may result in a situation,
#    where multiple such environments are created, although only one should.
#    You can change this behavior by either specifying a fixed path for the virtualenv
#    using :ref:`DUNE_PYTHON_VIRTUALENV_PATH` or by enabling
#    :ref:`DUNE_PYTHON_EXTERNAL_VIRTUALENV_FOR_ABSOLUTE_BUILDDIR` if you are using an
#    absolute build directory with dunecontrol. Note that this flag is enabled by default
#    starting from Dune 2.7.
#
# .. cmake_variable:: DUNE_PYTHON_VIRTUALENV_PATH
#
#    When the Dune build system has setup a virtualenv, this variable will contain its location.
#    You can also set this variable to a fixed path when CMake, and the virtualenv will be placed
#    at that location.
#
# .. cmake_variable:: DUNE_PYTHON_EXTERNAL_VIRTUALENV_FOR_ABSOLUTE_BUILDDIR
#
#    Before Dune 2.7, the virtualenv was always placed inside the build directory of  the first
#    non-installed Dune module that the current module depends on. When using installed core modules
#    or a multi-stage installation process, this can lead to situations where there are multiple
#    virtualenvs, making it impossible to find all Python modules installed by upstream modules.
#    In order to avoid this problem at least for builds using an absolute build directory (i.e., the
#    :code:`--builddir` option of dunecontrol refers to an absolute path), the build system will
#    place the virtualenv in a dedicated directory :code:`dune-python-env` inside that absolute
#    build directory, where it will be found by all Dune modules. If you want to disable this
#    behavior, set :code:`DUNE_PYTHON_EXTERNAL_VIRTUALENV_FOR_ABSOLUTE_BUILDDIR=0`.
#
# .. cmake_variable:: DUNE_PYTHON_ALLOW_GET_PIP
#
#    The Dune build system will try to build a virtualenv with pip installed into it,
#    but this can fail in some situations, in particular on Debian and Ubuntu distributions.
#    In this case, you will see a warning message in the CMake output. If you are on Debian
#    or Ubuntu, try installing the :code:`python3-venv` (for Python 3) and / or
#    :code:`python-virtualenv` packages, delete your build directory and try configuring
#    again.
#
#    If that still does not help, set this variable to allow the Dune build system to download
#    :code:`get-pip.py` from https://bootstrap.pypa.io/get-pip.py at configure time and execute
#    it to install pip into the freshly set up virtual environment. While this should normally
#    not be necessary anymore, see https://bugs.launchpad.net/debian/+source/python3.4/+bug/1290847
#    for more information about the underlying distribution bug.
#
# .. cmake_variable:: DUNE_PYTHON_WHEELHOUSE
#
#    The place where python wheels are stored. Notice that this wheelhouse directory shall be
#    the same for all dune installations.
#
include_guard(GLOBAL)

# pre-populate DUNE_PYTHON_SYSTEM_IS_VIRTUALENV
set(DUNE_PYTHON_SYSTEM_IS_VIRTUALENV "" CACHE PATH
  "Running in an external activated virtual environment"
  )
# pre-populate DUNE_PYTHON_VIRTUALENV_PATH
set(DUNE_PYTHON_VIRTUALENV_PATH "" CACHE PATH
  "Location of Python virtualenv created by the Dune build system"
  )
# pre-populate DUNE_PYTHON_EXTERNAL_VIRTUALENV_FOR_ABSOLUTE_BUILDDIR
set(DUNE_PYTHON_EXTERNAL_VIRTUALENV_FOR_ABSOLUTE_BUILDDIR ON CACHE BOOL
  "Place Python virtualenv in top-level directory \"dune-python-env\" when using an absolute build directory"
  )

option(DUNE_RUNNING_IN_CI "This is turned on if running in dune gitlab ci" OFF)

if(DUNE_RUNNING_IN_CI)
  set(DUNE_PIP_INDEX "--index-url=https://gitlab.dune-project.org/api/v4/projects/133/packages/pypi/simple")
else()
  set(DUNE_PIP_INDEX "")
endif()

# Construct the wheel house installation option string
# First set the path to a Dune wheelhouse that is to be used during installation
# NB: Right now, the same logic is used to retrieve the location of the
#     wheelhouse (which means that you have to use the same CMAKE_INSTALL_PREFIX
#     when *using* installed modules, you used when *installing* them.
#     TODO: Replace this with a better mechanism (like writing the location into
#           dune-commons package config file)
set(DUNE_PYTHON_WHEELHOUSE "${CMAKE_INSTALL_PREFIX}/share/dune/wheelhouse" CACHE PATH "The place where the wheels will be stored")

# Determine whether the given interpreter is running inside a virtualenv
dune_execute_process(COMMAND "${Python3_EXECUTABLE}" "${scriptdir}/venvpath.py"
                     RESULT_VARIABLE DUNE_PYTHON_SYSTEM_IS_VIRTUALENV
                     OUTPUT_VARIABLE DUNE_PYTHON_VIRTUALENV_PATH
                     OUTPUT_STRIP_TRAILING_WHITESPACE
                     )

# If the user has not specified an absolute, we look through the dependency tree of this module
# for a build directory that already contains a virtual environment.
# if we haven't found it yet, check in the current build directory - this might be a reconfigure
if(NOT DUNE_PYTHON_VIRTUALENV_PATH)
  foreach(mod ${ALL_DEPENDENCIES} CMAKE_BINARY)
    if(IS_DIRECTORY ${${mod}_DIR}/dune-env)
      set(DUNE_PYTHON_VIRTUALENV_PATH ${${mod}_DIR}/dune-env)
      break()
    endif()
  endforeach()
endif()

# We didn't find anything, so figure out the correct location for building the virtualenv
if(NOT DUNE_PYTHON_VIRTUALENV_PATH)
  if(DUNE_PYTHON_EXTERNAL_VIRTUALENV_FOR_ABSOLUTE_BUILDDIR AND DUNE_BUILD_DIRECTORY_ROOT_PATH)
    # Use a dedicated directory not associated with any module
    set(DUNE_PYTHON_VIRTUALENV_PATH "${DUNE_BUILD_DIRECTORY_ROOT_PATH}/dune-python-env")
  else()
    # Create the virtualenv inside our build directory
    set(DUNE_PYTHON_VIRTUALENV_PATH ${CMAKE_BINARY_DIR}/dune-env)
  endif()
endif()

# If it does not yet exist, set it up!
if(NOT IS_DIRECTORY "${DUNE_PYTHON_VIRTUALENV_PATH}")
  # Check for presence of the virtualenv/venv package
  dune_python_find_package(PACKAGE virtualenv)
  dune_python_find_package(PACKAGE venv)
  if(NOT(DUNE_PYTHON_virtualenv_FOUND OR DUNE_PYTHON_venv_FOUND))
    message(WARNING "One of the python packages virtualenv/venv is needed on the host system! "
                    "If you are using Debian or Ubuntu, consider installing python3-venv "
                    "and/or python-virtualenv")
    set(DUNE_ENABLE_PYTHONBINDINGS OFF)
    return()
  endif()

  # Set some options depending on which virtualenv package is used
  if(DUNE_PYTHON_venv_FOUND)
    set(VIRTUALENV_PACKAGE_NAME venv)
    set(NOPIP_OPTION --without-pip)
    set(INTERPRETER_OPTION "")
  endif()
  if(DUNE_PYTHON_virtualenv_FOUND)
    set(VIRTUALENV_PACKAGE_NAME virtualenv)
    set(NOPIP_OPTION --no-pip)
    set(INTERPRETER_OPTION -p "${Python3_EXECUTABLE}")
  endif()

  if(DUNE_PYTHON_venv_FOUND AND DUNE_PYTHON_SYSTEM_IS_VIRTUALENV)
    message(WARNING "You are using a system python interpreter which is a virtualenv and the venv "
                    "package. You might want to consider installing the virtualenv package if you "
                    "experience inconveniences.")
  endif()

  # Set up the env itself
  message(STATUS "Building a virtualenv in ${DUNE_PYTHON_VIRTUALENV_PATH}")

  # First, try to build it with pip installed, but only if the user has not set DUNE_PYTHON_ALLOW_GET_PIP
  if(NOT DUNE_PYTHON_ALLOW_GET_PIP)
    dune_execute_process(COMMAND ${Python3_EXECUTABLE}
                                  -m ${VIRTUALENV_PACKAGE_NAME}
                                  ${INTERPRETER_OPTION}
                                  "${DUNE_PYTHON_VIRTUALENV_PATH}"
                         RESULT_VARIABLE venv_install_result
                         )
  endif()

  if(NOT "${venv_install_result}" STREQUAL "0")
    if(NOT DUNE_PYTHON_ALLOW_GET_PIP)
      # we attempted the default installation before, so issue a warning
      message(WARNING "Failed to build a virtual env with pip installed, trying again without "
                      "pip. If you are using Debian or Ubuntu, consider installing python3-venv "
                      "and/or python-virtualenv")
    endif()

    # remove the remainder of a potential first attempt
    file(REMOVE_RECURSE "${DUNE_PYTHON_VIRTUALENV_PATH}")

    # try to build the env without pip
    dune_execute_process(COMMAND ${Python3_EXECUTABLE}
                                  -m ${VIRTUALENV_PACKAGE_NAME}
                                  ${INTERPRETER_OPTION}
                                  ${NOPIP_OPTION}
                                  "${DUNE_PYTHON_VIRTUALENV_PATH}"
                         RESULT_VARIABLE venv_install_result2)
    if(NOT "${venv_install_result2}" STREQUAL "0")
      message(WARNING "Failed to build a virtual env without pip.")
      set(DUNE_ENABLE_PYTHONBINDINGS OFF)
      return()
    endif()
  endif()

else()
  message(STATUS "Using existing virtualenv in ${DUNE_PYTHON_VIRTUALENV_PATH}")
endif()

# Also store the virtual env interpreter directly
set(DUNE_PYTHON_VIRTUALENV_EXECUTABLE ${DUNE_PYTHON_VIRTUALENV_PATH}/bin/python)

# Write a symlink for activation of the environment into all the
# build directories of the Dune stack
dune_execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${DUNE_PYTHON_VIRTUALENV_PATH}/bin/activate ${CMAKE_BINARY_DIR}/activate)

# Also write a small wrapper script 'run-in-dune-env' into the build directory
# This is necessary to execute installed python scripts (the bin path of a virtualenv
# is *not* in the sys path, so a simple `python scriptname` does not work.
if(UNIX)
  find_package(UnixCommands QUIET)
  dune_module_path(MODULE dune-common
                   RESULT scriptdir
                   SCRIPT_DIR)
  configure_file(${scriptdir}/run-in-dune-env.sh.in
                 ${CMAKE_BINARY_DIR}/run-in-dune-env
                 @ONLY)
else()
  message(WARNING "Writing script 'run-in-dune-env' not implemented on your platform!")
endif()

# The virtualenv might not contain pip due to the distribution bug described in
# https://bugs.launchpad.net/debian/+source/python3.4/+bug/1290847
# We need to install pip, so if pip is missing, we offer to download and run the get-pip
# script. We ask users for permission to do so, or we allow them to set it up themselves.

dune_python_find_package(PACKAGE pip
                         RESULT pippresent
                         INTERPRETER ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE}
                         )
if(NOT pippresent)
  if(DUNE_PYTHON_ALLOW_GET_PIP)
    # Fetch the get-pip.py script
    message(STATUS "Installing pip using https://bootstrap.pypa.io/get-pip.py...")
    file(DOWNLOAD https://bootstrap.pypa.io/get-pip.py ${CMAKE_CURRENT_BINARY_DIR}/get-pip.py)

    # Verify that the script was successfully fetched
    file(READ ${CMAKE_CURRENT_BINARY_DIR}/get-pip.py verify LIMIT 1)
    if(NOT verify)
      message(WARNING "Fetching get-pip.py failed. This often happens when CMake is built from "
                      "source without SSL/TLS support. Consider using a different cmake version or "
                      "fall back to manually installing pip into the virtualenv.")
      set(DUNE_ENABLE_PYTHONBINDINGS OFF)
      return()
    endif()

    # Execute the script
    dune_execute_process(COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/get-pip.py RESULT_VARIABLE pip_install_result)
    if(NOT "${pip_install_result}" STREQUAL "0")
      message(WARNING "Fatal error when installing pip into the virtualenv.")
      set(DUNE_ENABLE_PYTHONBINDINGS OFF)
      return()
    endif()
  else()
    message(WARNING "dune-common set up a virtualenv, but needs pip to be installed into it. "
                    "You can either install it yourself manually activating the virtualenv with "
                    "the activate script in your build directory ${CMAKE_BINARY_DIR} or you set "
                    "the CMake variable DUNE_PYTHON_ALLOW_GET_PIP to allow Dune to use get-pip.py "
                    "from https://bootstrap.pypa.io/get-pip.py")
    set(DUNE_ENABLE_PYTHONBINDINGS OFF)
    return()
  endif()
endif()

# if pip was not found before then we can set it here since it was now found
set(DUNE_PYTHON_pip_FOUND ON)

# install setuptools into the venv (needed to find dependencies later on)
dune_execute_process(COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m pip install
      "${DUNE_PIP_INDEX}"
      setuptools>=41 ninja
  WARNING_MESSAGE "python 'setuptools' package could not be installed - possibly connection to the python package index failed"
  )
