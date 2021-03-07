# Manage the creation of a configure-time virtual environment
#
# .. cmake_module::
#
#    This module manages the creation of virtual python environment during
#    configuration. Execution of this module must be explicitly enabled by
#    setting the variable :ref:`DUNE_PYTHON_VIRTUALENV_SETUP`. Note that some
#    downstream modules will require you to set this variable. The purpose
#    of this virtual environment is to be able to run python code from cmake
#    in situations such as python-based code generation, running postprocessing
#    in python during testing etc.
#
#    Although designed for internal use, this virtualenv can also be manually
#    inspected. A symlink to the activation script is placed in the top level
#    build directory of all Dune modules in the stack. To directly execute a
#    command in the virtualenv, you can use the script :code:`run-in-dune-env <command>`,
#    which is also placed into every build directory.
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
#    In this case, you will se a warning message in the CMake output. If you are on Debian
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
include_guard(GLOBAL)

# If the user has not specified an absolute, we look through the dependency tree of this module
# for a build directory that already contains a virtual environment.

set(DUNE_PYTHON_VIRTUALENV_PATH "" CACHE PATH
  "Location of Python virtualenv created by the Dune build system"
  )

# pre-populate DUNE_PYTHON_EXTERNAL_VIRTUALENV_FOR_ABSOLUTE_BUILDDIR
set(DUNE_PYTHON_EXTERNAL_VIRTUALENV_FOR_ABSOLUTE_BUILDDIR ON CACHE BOOL
  "Place Python virtualenv in top-level directory \"dune-python-env\" when using an absolute build directory"
  )

if(DUNE_PYTHON_VIRTUALENV_PATH STREQUAL "")
  foreach(mod ${ALL_DEPENDENCIES})
    if(IS_DIRECTORY ${${mod}_DIR}/dune-env)
      set(DUNE_PYTHON_VIRTUALENV_PATH ${${mod}_DIR}/dune-env)
      break()
    endif()
  endforeach()

  # if we haven't found it yet, check in the current build directory - this might be a reconfigure
  if(DUNE_PYTHON_VIRTUALENV_PATH STREQUAL "")
    if(IS_DIRECTORY ${CMAKE_BINARY_DIR}/dune-env)
      set(DUNE_PYTHON_VIRTUALENV_PATH ${CMAKE_BINARY_DIR}/dune-env)
    endif()
  endif()
endif()


if(DUNE_PYTHON_VIRTUALENV_PATH STREQUAL "")
  # We didn't find anything, so figure out the correct location for building the virtualenv

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
    message(FATAL_ERROR "One of the python packages virtualenv/venv is needed on the host system!")
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

  if(("${VIRTUALENV_PACKAGE_NAME}" STREQUAL "venv") AND DUNE_PYTHON_SYSTEM_IS_VIRTUALENV)
    message("-- WARNING: You are using a system interpreter which is a virtualenv and the venv package.")
    message("            You might want to consider installing the virtualenv package if you experience inconveniences.")
  endif()

  # Set up the env itself
  message("-- Building a virtualenv in ${DUNE_PYTHON_VIRTUALENV_PATH}")
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
      message("-- WARNING: Failed to build a virtual env with pip installed, trying again without pip")
      message("-- If you are using Debian or Ubuntu, consider installing python3-venv and / or python-virtualenv")
    endif()

    # remove the remainder of a potential first attempt
    file(REMOVE_RECURSE "${DUNE_PYTHON_VIRTUALENV_PATH}")

    # try to build the env without pip
    dune_execute_process(COMMAND ${Python3_EXECUTABLE}
                                  -m ${VIRTUALENV_PACKAGE_NAME}
                                  ${INTERPRETER_OPTION}
                                  ${NOPIP_OPTION}
                                  "${DUNE_PYTHON_VIRTUALENV_PATH}"
                         ERROR_MESSAGE "Fatal error when setting up a virtualenv."
                         )
  endif()

else()
  message("-- Using existing virtualenv in ${DUNE_PYTHON_VIRTUALENV_PATH}")
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
    message("-- Installing pip using https://bootstrap.pypa.io/get-pip.py...")
    file(DOWNLOAD https://bootstrap.pypa.io/get-pip.py ${CMAKE_CURRENT_BINARY_DIR}/get-pip.py)

    # Verify that the script was successfully fetched
    file(READ ${CMAKE_CURRENT_BINARY_DIR}/get-pip.py verify LIMIT 1)
    if(NOT verify)
      message(FATAL_ERROR "
      Fetching get-pip.py failed. This often happens when CMake is built from source without SSL/TLS support.
      Consider using a different cmake version or fall back to manually installing pip into the virtualenv.
                          ")
    endif()

    # Execute the script
    dune_execute_process(COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/get-pip.py
                         ERROR_MESSAGE "Fatal error when installing pip into the virtualenv."
                         )
  else()
    message(FATAL_ERROR "dune-common set up a virtualenv, but needs pip to be installed into it.
                         You can either install it yourself manually activating the virtualenv with
                         the activate script in your build directory ${CMAKE_BINARY_DIR} or you set
                         the CMake variable DUNE_PYTHON_ALLOW_GET_PIP to allow Dune to use get-pip.py
                         from https://bootstrap.pypa.io/get-pip.py")
  endif()
endif()
