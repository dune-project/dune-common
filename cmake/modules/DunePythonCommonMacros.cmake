# The python extension of the Dune cmake build system
#
# .. cmake_module::
#
#    This module is the main entry point for the python extension of the Dune cmake
#    build system. It handles the detection of the python installation, defines installation
#    rules for python packages in Dune modules and provides virtual environments to
#    run python code from cmake.
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
# .. cmake_variable:: DUNE_PYTHON_VIRTUALENV_SETUP
#
#    Set this variable to allow the Dune build system to set up a virtualenv at
#    configure time. Such virtual environment is very useful, whenever python code
#    is to be run at configure time, i.e. to implement code generation in Python or
#    to use Python wrappers in testing. Some downstream modules will *require* you
#    to set this variable. When setting this variable, you allow the Dune buildsystem
#    to install packages through :code:`pip` into a virtualenv, that resides in a cmake
#    build directory. For all the information on this virtualenv, see :ref:`DunePythonVirtualenv`.
#
# .. cmake_function:: dune_python_require_virtualenv_setup
#
#    Call this function from a downstream module, if that module relies on the
#    the presence of the configure time virtualenv described in :ref:`DunePythonVirtualenv`.
#
include_guard(GLOBAL)

# unless the user has defined the variable, unversioned names (like python3) are found
# first, to match what users most probably use later on to call the executable
if(NOT DEFINED Python3_FIND_UNVERSIONED_NAMES)
  set(Python3_FIND_UNVERSIONED_NAMES "FIRST")
endif()

# include code from CMake 3.20 to back-port using unversioned Python first
if(${CMAKE_VERSION} VERSION_LESS "3.20")
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/FindPython3")
endif()

# Include all the other parts of the python extension to avoid that users need
# to explicitly include parts of our build system.
include(DunePythonFindPackage)
include(DunePythonInstallPackage)
include(DunePythonTestCommand)

# Find the Python Interpreter and libraries
find_package(Python3 COMPONENTS Interpreter Development)


# Determine whether the given interpreter is running inside a virtualenv
if(Python3_Interpreter_FOUND)
  include(DuneExecuteProcess)
  include(DunePathHelper)
  dune_module_path(MODULE dune-common
                   RESULT scriptdir
                   SCRIPT_DIR)

  dune_execute_process(COMMAND "${Python3_EXECUTABLE}" "${scriptdir}/envdetect.py"
                       RESULT_VARIABLE DUNE_PYTHON_SYSTEM_IS_VIRTUALENV
                       )
endif()

# Determine where to install python packages
if(NOT DUNE_PYTHON_INSTALL_LOCATION)
  if(DUNE_PYTHON_SYSTEM_IS_VIRTUALENV)
    set(DUNE_PYTHON_INSTALL_LOCATION "system")
  else()
    set(DUNE_PYTHON_INSTALL_LOCATION "none")
  endif()
endif()
if(NOT(("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "user") OR
       ("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "system") OR
       ("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "none")))
  message(FATAL_ERROR "DUNE_PYTHON_INSTALL_LOCATION must be user|system|none.")
endif()
if(("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "user") AND
   DUNE_PYTHON_SYSTEM_IS_VIRTUALENV)
  message(FATAL_ERROR "Specifying 'user' as install location is incomaptible with using virtual environments (as per pip docs)")
endif()

# Check presence of python packages required by the buildsystem
dune_python_find_package(PACKAGE pip)

# Add python related meta targets
add_custom_target(test_python)
add_custom_target(install_python)

# Set the path to a Dune wheelhouse that is to be used during installation
# NB: Right now, the same logic is used to retrieve the location of the
#     wheelhouse (which means that you have to use the same CMAKE_INSTALL_PREFIX
#     when *using* installed modules, you used when *installing* them.
#     TODO: Replace this with a better mechanism (like writing the location into
#           dune-commons package config file)
set(DUNE_PYTHON_WHEELHOUSE ${CMAKE_INSTALL_PREFIX}/share/dune/wheelhouse)

# Have make install do the same as make install_python
install(CODE "set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
              set(DUNE_PYTHON_WHEELHOUSE ${DUNE_PYTHON_WHEELHOUSE})
              include(DuneExecuteProcess)
              dune_execute_process(COMMAND \"${CMAKE_COMMAND}\" --build . --target install_python --config $<CONFIG>)
              ")

# Implement a check for the presence of the virtualenv
function(dune_python_require_virtualenv_setup)
  if(NOT DUNE_PYTHON_VIRTUALENV_SETUP)
    message(FATAL_ERROR "\n
    ${PROJECT_NAME} relies on a configure-time virtual environment being
    set up by the Dune python build system. You have to set the CMake variable
    DUNE_PYTHON_VIRTUALENV_SETUP to allow that.\n
    ")
  endif()
endfunction()

# If requested, switch into DunePythonVirtualenv.cmake and setup the virtualenv.
if(DUNE_PYTHON_VIRTUALENV_SETUP)
  include(DunePythonVirtualenv)
endif()

# marcos used for the Python bindings
include(DunePythonMacros)
