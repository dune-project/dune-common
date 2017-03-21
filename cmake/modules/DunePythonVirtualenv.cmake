# Manage the creation of a configure-time virtual environment
#
# .. cmake_variable:: DUNE_PYTHON_ALLOW_GET_PIP
#
#    Set this variable to allow the Dune build system to download get-pip.py
#    from https://bootstrap.pypa.io/get-pip.py at configure time and execute it
#    to install pip into the freshly set up virtual environment. This step became
#    necessary because of a debian bug:
#    https://bugs.launchpad.net/debian/+source/python3.4/+bug/1290847
#
#    If you do not want the Dune build system to download get-pip.py, you can
#    manually activate the virtual environment, install pip through your favorite
#    method and reconfigure.
#

# First, we look through the dependency tree of this module for a build directory
# that already contains a virtual environment.
set(DUNE_PYTHON_VIRTUALENV_PATH)
foreach(mod ${ALL_DEPENDENCIES})
  if(IS_DIRECTORY ${${mod}_DIR}/python-env)
    set(DUNE_PYTHON_VIRTUALENV_PATH ${${mod}_DIR}/python-env)
  endif()
  # check in the current build directory - this might be a reconfigure
  if(IS_DIRECTORY ${CMAKE_BINARY_DIR}/python-env)
    set(DUNE_PYTHON_VIRTUALENV_PATH ${CMAKE_BINARY_DIR}/python-env)
  endif()
endforeach()

# If it does not yet exist, set it up!
if(NOT DUNE_PYTHON_VIRTUALENV_PATH)
  # Check for presence of the virtualenv/venv package
  dune_python_find_package(PACKAGE virtualenv)
  dune_python_find_package(PACKAGE venv)
  if(NOT(DUNE_PYTHON_virtualenv_FOUND OR DUNE_PYTHON_venv_FOUND))
    message(FATAL_ERROR "One of the python packages virtualenv/venv is needed on the host system!")
  endif()

  # Set some options depending on which virtualenv package is used
  if(DUNE_PYTHON_virtualenv_FOUND)
    set(VIRTUALENV_PACKAGE_NAME virtualenv)
    set(NOPIP_OPTION --no-pip)
  endif()
  if(DUNE_PYTHON_venv_FOUND)
    set(VIRTUALENV_PACKAGE_NAME venv)
    set(NOPIP_OPTION --without-pip)
  endif()

  # Set up the env itself
  message("-- Building a virtual env in ${CMAKE_BINARY_DIR}/python-env...")
  dune_execute_process(COMMAND ${PYTHON_EXECUTABLE}
                                -m ${VIRTUALENV_PACKAGE_NAME}
                                ${NOPIP_OPTION}
                                ${CMAKE_BINARY_DIR}/python-env
                       ERROR_MESSAGE "Fatal error when setting up a virtualenv."
                       )

  # And set the path to it
  set(DUNE_PYTHON_VIRTUALENV_PATH ${CMAKE_BINARY_DIR}/python-env)
endif()

# Also store the virtual env interpreter directly
set(DUNE_PYTHON_VIRTUALENV_EXECUTABLE ${DUNE_PYTHON_VIRTUALENV_PATH}/bin/python)

# Write a symlink for activation of the environment into all the
# build directories of the Dune stack
dune_execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${DUNE_PYTHON_VIRTUALENV_PATH}/bin/activate ${CMAKE_BINARY_DIR}/activate)

# We previously omitted pip from the env, because of this Debian bug:
# https://bugs.launchpad.net/debian/+source/python3.4/+bug/1290847
# We now, need to install pip. Easiest way is to download the get-pip
# script. We ask users for permission to do so, or we allow them to
# set it up themselves.
dune_python_find_package(PACKAGE pip
                         RESULT pippresent
                         INTERPRETER ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE}
                         )
if(NOT pippresent)
  if(DUNE_PYTHON_ALLOW_GET_PIP)
    message("-- Installing pip using https://bootstrap.pypa.io/get-pip.py...")
    file(DOWNLOAD https://bootstrap.pypa.io/get-pip.py ${CMAKE_CURRENT_BINARY_DIR}/get-pip.py)
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