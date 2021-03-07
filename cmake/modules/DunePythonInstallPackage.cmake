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
#       :argname: param
#
#       Parameters to add to any :code:`pip install` call (appended).
#
#    This function installs the python package located at the given path. It
#
#    * installs it to the location specified with :ref:`DUNE_PYTHON_INSTALL_LOCATION` during
#      :code:`make install_python` and during :code:`make install`.
#    * installs a wheel into the Dune wheelhouse during :code:`make install`.
#      This is necessary for mixing installed and non-installed Dune modules.
#
#    The package at the given location is expected to be a pip-installable package.
#
# .. cmake_variable:: DUNE_PYTHON_INSTALL_EDITABLE
#
#    Set this variable to have all installations of python packages use
#    :code:`pip --editable`.
#
#
# .. cmake_variable:: DUNE_PYTHON_ADDITIONAL_PIP_PARAMS
#
#    Use this variable to set additional flags for pip in this build. This can e.g.
#    be used to point pip to alternative package indices in restricted environments.
#
include_guard(GLOBAL)

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

  set(PYINST_FULLPATH ${CMAKE_CURRENT_SOURCE_DIR}/${PYINST_PATH})
  if(EXISTS ${PYINST_FULLPATH}/setup.py.in)
    configure_file(${PYINST_PATH}/setup.py.in ${PYINST_PATH}/setup.py)
    set(PYINST_FULLPATH ${CMAKE_CURRENT_BINARY_DIR}/${PYINST_PATH})
    set(PYINST_PUREPYTHON FALSE)
  elseif(EXISTS ${PYINST_FULLPATH}/setup.py)
    set(PYINST_PUREPYTHON TRUE)
  else()
    message(FATAL_ERROR "dune_python_install_package: Requested installations, but neither setup.py nor setup.py.in found!")
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
  if("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "user")
    set(INSTALL_OPTION "--user")
  endif()

  set(INSTALL_CMDLINE -m pip install
                      "${INSTALL_OPTION}" --upgrade "${WHEEL_OPTION}" "${EDIT_OPTION}" ${PYINST_ADDITIONAL_PIP_PARAMS} ${DUNE_PYTHON_ADDITIONAL_PIP_PARAMS}
                      "${PYINST_FULLPATH}")


  # Leave this function if no installation rules are required
  if("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "none" AND NOT DUNE_PYTHON_VIRTUALENV_SETUP)
    return()
  endif()

  # Check for the presence of the pip package
  if(NOT DUNE_PYTHON_pip_FOUND)
    message(FATAL_ERROR "dune_python_install_package: Requested installations, but pip was not found!")
  endif()

  #
  # If requested, install into the configure-time Dune virtualenv
  #

  if(PYINST_PUREPYTHON AND DUNE_PYTHON_VIRTUALENV_SETUP)
    message("-- Installing python package at ${CMAKE_CURRENT_SOURCE_DIR}/${PYINST_PATH} into the virtualenv...")
    dune_execute_process(COMMAND "${DUNE_PYTHON_VIRTUALENV_EXECUTABLE}" "${INSTALL_CMDLINE}"
                         ERROR_MESSAGE "dune_python_install_package: Error installing into virtualenv!")
  endif()

  #
  # Now define rules for `make install_python`.
  #

  # Leave this function if no installation rules are required
  if("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "none")
    return()
  endif()

  dune_module_path(MODULE dune-common
                   RESULT scriptdir
                   SCRIPT_DIR)

  # Determine a target name for installing this package
  string(REPLACE "/" "_" targetname "install_python_${CMAKE_CURRENT_SOURCE_DIR}_${PYINST_PATH}")

  # Add a custom target that globally installs this package if requested
  add_custom_target(${targetname}
                    COMMAND ${Python3_EXECUTABLE} ${INSTALL_CMDLINE}
                    COMMENT "Installing the python package at ${PYINST_FULLPATH}"
                    )

  add_dependencies(install_python ${targetname})

  # Define rules for `make install` that install a wheel into a central wheelhouse
  #
  # NB: This is necessary, to allow mixing installed and non-installed modules
  #     with python packages. The wheelhouse will allow to install any missing
  #     python packages into a virtual environment.
  #

  # Construct the wheel installation commandline
  set(WHEEL_COMMAND ${Python3_EXECUTABLE} -m pip wheel -w ${DUNE_PYTHON_WHEELHOUSE} ${PYINST_ADDITIONAL_PIP_PARAMS} ${DUNE_PYTHON_ADDITIONAL_PIP_PARAMS} ${PYINST_FULLPATH})

  # Add the installation rule
  install(CODE "message(\"Installing wheel for python package at ${PYINST_FULLPATH}...\")
                dune_execute_process(COMMAND ${WHEEL_COMMAND}
                                     )"
          )
endfunction()
