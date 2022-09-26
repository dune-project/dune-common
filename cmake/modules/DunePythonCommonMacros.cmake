# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# The python extension of the Dune cmake build system
#
# .. cmake_module::
#
#    This module is the main entry point for the python extension of the Dune cmake
#    build system. It handles the detection of the python installation, defines installation
#    rules for python packages in Dune modules and provides virtual environments to
#    run python code from cmake.
#
#    To disable any Python-related features use -DCMAKE_DISABLE_FIND_PACKAGE_Python3=TRUE
#
#    If you want to use Dune modules that provide Python functionality, you should be aware
#    of some facts:
#
#    * CMake looks for your python interpreter during configure. If you want to have it
#      work with a virtual environment, you should activate your virtualenv before configure.
#    * Each module has an additional target :code:`make install_python`, that installs python packages
#      defined in the Dune module. You can customize the install location with
#      :ref:`DUNE_PYTHON_INSTALL_LOCATION`. This is also included in :code:`make install`.
#    * There is additional functionality, that automatically sets up a virtual environment
#      at configure time, you can read more at :ref:`DunePythonVirtualenv`.
#
#    After the module :code:`DunePythonCommonMacros` is run (which happens automatically when
#    configuring dune-common) the following python-related variables will be set and available
#    for use in downstream modules:
#
#    * All variables set by :code:`FindPythonInterp.cmake` and :code:`FindPythonLibs.cmake`
#    * :code:`DUNE_PYTHON_SYSTEM_IS_VIRTUALENV`: True if the given system interpreter resides in
#      virtual environment.
#
#    For documentation on how to customize the build process, check the input variable
#    reference for any variables prefixed with :code:`DUNE_PYTHON`. To learn how to write build
#    system code for Dune modules shipping python, have a look at the command reference for
#    commands prefixed :code:`dune_python`.
#
# .. cmake_variable:: DUNE_PYTHON_INSTALL_LOCATION
#
#    This variable can be used to control where Dune should install python
#    packages. Possible values are:
#
#    * :code:`user`: installs into the users home directory through :code:`pip --user`. Note, that
#      this is incompatible with using virtual environments (as per pip docs).
#    * :code:`system`: into the standard paths of the interpreter which was found
#      by cmake.
#    * :code:`none`: Never install any python packages.
#
#    The default value in use depends on the system interpreter to run in a virtual environment
#    or not: If it does, :code:`system` is the default, if it does not :code:`none` is the default.
#    This rather unintuitive default originates from the strong belief, that installing
#    python packages into the system locations at :code:`/usr/...` should be discouraged.
#
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
include(DunePythonDeprecations)
include(DunePythonFindPackage)
include(DunePythonInstallPackage)
include(DunePythonTestCommand)

# Find the Python Interpreter and libraries
find_package(Python3 COMPONENTS Interpreter Development)

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

  # this option enables the build of Python bindings for DUNE modules
  option(DUNE_ENABLE_PYTHONBINDINGS "Enable Python bindings for DUNE" ON)

  if( DUNE_ENABLE_PYTHONBINDINGS )
    if(NOT Python3_Interpreter_FOUND)
      message(WARNING "Python bindings require a Python 3 interpreter")
      set(DUNE_ENABLE_PYTHONBINDINGS OFF)
      return()
    endif()
    if(NOT Python3_INCLUDE_DIRS)
      message(WARNING "Found a Python interpreter but the Python bindings also requires the Python "
                      "libraries (a package named like python-dev package or python3-devel)")
      set(DUNE_ENABLE_PYTHONBINDINGS OFF)
      return()
    endif()

    include_directories("${Python3_INCLUDE_DIRS}")

    function(add_python_targets base)
      include(DuneSymlinkOrCopy)
      if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
        message(WARNING "Source and binary dir are the same, skipping symlink!")
      else()
        foreach(file ${ARGN})
          dune_symlink_to_source_files(FILES ${file}.py)
        endforeach()
      endif()
    endfunction()

    include(DuneAddPybind11Module)
  endif()


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

endif()
