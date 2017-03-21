# This cmake module provides infrastructure for cmake installation rules concerning python packages.
#
# .. cmake_function:: dune_python_install_package
#
#    .. cmake_param:: PATH
#       :required:
#       :single:
#
#       Relative path to the given python package source code.
#
#    .. cmake_param:: ADDITIONAL_PIP_PARAMS
#       :multi:
#
#       Parameters to add to any :code:`pip install` call (appended).
#
#    This function installs the python package located at the given path. It
#
#    * installs it to the location specified with :ref:`DUNE_PYTHON_INSTALL_LOCATION` during
#      :code:`make install-python` and during :code:`make install`.
#    * installs a wheel into the Dune wheelhouse during :code:`make install`.
#      This is necessary for mixing installed and non-installed Dune modules.
#
#    The package at the given location is expected to be a pip-installable package.
#
# .. cmake_variable:: DUNE_PYTHON_INSTALL_LOCATION
#
#    This variable can be used to control where Dune should install python
#    packages. Possible values are:
#    * `user`: installs into the users home directory through `pip --user`
#    * `system`: into the standard paths of the interpreter which was found
#      by cmake.
#    * `none`: Never install any python packages.
#
#    The default value in use depends on the system interpreter to run in a virtual environment
#    or not: If it does, `system` is the default, if it does not `none` is the default.
#    This rather unintuitive default originates from the strong belief, that installing
#    python packages into the system locations at `/usr/...` should be discouraged.
#
# .. cmake_variable:: DUNE_PYTHON_INSTALL_EDITABLE
#
#    Set this variable to have all installations of python packages use
#    :code:`pip --editable`.
#

function(dune_python_install_package)
  # Parse Arguments
  set(OPTION)
  set(SINGLE PATH)
  set(MULTI ADDITIONAL_PIP_PARAMS)
  include(CMakeParseArguments)
  cmake_parse_arguments(PYINST "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYINST_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_install_package: This often indicates typos!")
  endif()

  # Determine where to install python packages
  set(INSTALL_LOCATION ${DUNE_PYTHON_INSTALL_LOCATION})
  if(NOT INSTALL_LOCATION)
    if(DUNE_PYTHON_SYSTEM_IS_VIRTUALENV)
      set(INSTALL_LOCATION "system")
    else()
      set(INSTALL_LOCATION "none")
    endif()
  endif()
  if(NOT(("${INSTALL_LOCATION}" STREQUAL "user") OR
         ("${INSTALL_LOCATION}" STREQUAL "system") OR
         ("${INSTALL_LOCATION}" STREQUAL "none")))
    message(FATAL_ERROR "DUNE_PYTHON_INSTALL_LOCATION must be user|system|none.")
  endif()

  # Leave this function if no installation rules are required
  if("${INSTALL_LOCATION}" STREQUAL "none")
    return()
  endif()

  # Find out whether we should install in editable mode
  set(INSTALL_EDITABLE ${DUNE_PYTHON_INSTALL_EDITABLE})

  # Construct the wheel house installation option string
  set(WHEEL_OPTION "")
  if(IS_DIRECTORY ${DUNE_PYTHON_WHEELHOUSE})
    set(WHEEL_OPTION "--find-links=${DUNE_PYTHON_WHEELHOUSE}")
    #
    # The following line is a bummer!
    # We cannot have editable packages once we start using global installations!
    # This is related to the nightmare that is https://github.com/pypa/pip/issues/3
    #
    set(INSTALL_EDITABLE FALSE)
  endif()

  # Construct the editable option string
  set(EDIT_OPTION "")
  if(INSTALL_EDITABLE)
    set(EDIT_OPTION "-e")
  endif()

  # Construct the installation location option string
  set(INSTALL_OPTION "")
  if("${INSTALL_LOCATION}" STREQUAL "user")
    set(INSTALL_OPTION "--user")
  endif()

  set(INSTALL_CMDLINE "${PYTHON_EXECUTABLE}" -m pip install
                      "${INSTALL_OPTION}" "${WHEEL_OPTION}" "${EDIT_OPTION}" ${PYINST_ADDITIONAL_PIP_PARAMS}
                      "${CMAKE_CURRENT_SOURCE_DIR}/${PYINST_PATH}")


  #
  # Now define rules for `make install_python`.
  #

  dune_module_path(MODULE dune-common
                   RESULT scriptdir
                   SCRIPT_DIR)

  # Determine a target name for installing this package
  string(REPLACE "/" "_" targetname "install_python_${PYINST_PATH}")

  # Add a custom target that globally installs this package if requested
  add_custom_target(${targetname}
                    COMMAND ${CMAKE_COMMAND}
                            -DCMAKE_MODULE_PATH="${CMAKE_MODULE_PATH}"
                            -DCMDLINE="${INSTALL_CMDLINE}"
                            -DPACKAGE_PATH="${PYINST_PATH}"
                             -P ${scriptdir}/install_python_package.cmake
                    COMMENT "Installing the python package at ${CMAKE_CURRENT_SOURCE_DIR}/${PYINST_PATH}"
                    )

  add_dependencies(install_python ${targetname})

  # Define rules for `make install` that install a wheel into a central wheelhouse
  #
  # NB: This is necessary, to allow mixing installed and non-installed modules
  #     with python packages. The wheelhouse will allow to install any missing
  #     python packages into a virtual environment.
  #

  # Construct the wheel installation commandline
  set(WHEEL_COMMAND ${PYTHON_EXECUTABLE} -m pip wheel -w ${DUNE_PYTHON_WHEELHOUSE} ${WHEEL_OPTION} ${CMAKE_CURRENT_SOURCE_DIR}/${PYINST_PATH})

  # Add the installation rule
  install(CODE "message(\"Installing wheel for python package at ${CMAKE_CURRENT_SOURCE_DIR}/${PYINST_PATH}...\")
                dune_execute_process(COMMAND ${WHEEL_COMMAND}
                                     ERROR_MESSAGE \"Error installing wheel for python package at ${CMAKE_CURRENT_SOURCE_DIR}/${PYINST_PATH}\"
                                     )"
          )
endfunction()
