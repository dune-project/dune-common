# The python extension of the Dune cmake build system
#
# .. cmake_module::
#
#    This module is the main entry point for the python extension of the Dune cmake
#    build system. It handles the detection of the python installation, defines installation
#    rules for python packages in Dune modules and provides virtual environments to
#    run python code from cmake.
#
#    After this module is run (which happens automatically when configuring dune-common)
#    the following python-related variables will be set and available for use in downstream
#    modules:
#
#    * All variables set by `FindPythonInterp.cmake` and `FindPythonLibs.cmake`
#    * `DUNE_PYTHON_IS_VIRTUALENV`: True if the given system interpreter resides in
#      virtual environment.
#
#    For documentation on how to customize the build process, check the input variable
#    reference for any variables prefixed with `DUNE_PYTHON`. To learn how to write build
#    system code for Dune modules shipping python, have a look at the command reference for
#    commands prefixed `dune_python`.
#


# Include all the other parts of the python extension to avoid that users need
# to explicitly include parts of our build system.
include(DunePythonFindPackage)
include(DunePythonRequireVersion)

# Find the Python Interpreter
set(_VERSION_STRING "")
if(DUNE_PYTHON_FORCE_PYTHON2 AND DUNE_PYTHON_FORCE_PYTHON3)
  message(FATAL_ERROR "Cannot enforce both python2 *and* python3")
endif()
if(DUNE_PYTHON_FORCE_PYTHON2)
  set(_VERSION_STRING "2")
endif()
if(DUNE_PYTHON_FORCE_PYTHON3)
  set(_VERSION_STRING "3")
endif()
find_package(PythonInterp ${_VERSION_STRING})

# Determine whether the given interpreter is running inside a virtualenv
if(PYTHONINTERP_FOUND)
  include(DuneExecuteProcess)
  include(DunePathHelper)
  dune_module_path(MODULE dune-common
                   RESULT scriptdir
                   SCRIPT_DIR)

  dune_execute_process(COMMAND "${PYTHON_EXECUTABLE}" "${scriptdir}/envdetect.py"
                       RESULT_VARIABLE DUNE_PYTHON_SYSTEM_IS_VIRTUALENV
                       )
endif()

# Check presence of python packages required by the buildsystem
dune_python_find_package(PACKAGE pip)

# Add python related meta targets
add_custom_target(test_python)
add_custom_target(install_python)

# Set up the installation script for pip installations
# Background: Using self-defined functions in "install(CODE ...)" requires
#             to import those commands. To avoid too much duplication, we do
#             this once here to make sure that the includes are put into the
#             cmake_install.cmake!
install(CODE "set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
              include(DuneExecuteProcess)
             ")
