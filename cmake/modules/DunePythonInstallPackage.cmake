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
#    .. cmake_param:: DEPENDS
#       :multi:
#       :argname: dep
#
#       Other CMake targets that the installation of this target depends on.
#
#    .. cmake_param:: CMAKE_METADATA_FILE
#       :single:
#       :argname: filename
#
#       A filename that CMake should export some meta data about this build to.
#       The file will be installed together with the Python package. This mechanism
#       is used by the Python bindings to transport information from CMake to
#       the installed Python package. A module dune-mymodule that provides a Python
#       package dune.mymodule should set this to dune/mymodule/metadata.cmake
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
# .. cmake_variable:: DUNE_PYTHON_ADDITIONAL_PIP_PARAMS
#
#    Use this variable to set additional flags for pip in this build. This can e.g.
#    be used to point pip to alternative package indices in restricted environments.
#
include_guard(GLOBAL)

function(dune_python_install_package)
  # Parse Arguments
  set(OPTION)
  set(SINGLE PATH CMAKE_METADATA_FILE)
  set(MULTI ADDITIONAL_PIP_PARAMS DEPENDS)
  cmake_parse_arguments(PYINST "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYINST_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_install_package: This often indicates typos!")
  endif()

  # Configure setup.py.in if present
  set(PYINST_FULLPATH ${CMAKE_CURRENT_SOURCE_DIR}/${PYINST_PATH})
  if(EXISTS ${PYINST_FULLPATH}/setup.py.in)
    configure_file(${PYINST_PATH}/setup.py.in ${PYINST_PATH}/setup.py)
    set(PYINST_FULLPATH ${CMAKE_CURRENT_BINARY_DIR}/${PYINST_PATH})
  endif()

  # Error out if setup.py is missing
  if(NOT EXISTS ${PYINST_FULLPATH})
    message(FATAL_ERROR "dune_python_install_package: Requested installations, but neither setup.py nor setup.py.in found!")
  endif()

  # Construct the wheel house installation option string
  set(WHEEL_OPTION "")
  if(IS_DIRECTORY ${DUNE_PYTHON_WHEELHOUSE})
    set(WHEEL_OPTION "--find-links=file://${DUNE_PYTHON_WHEELHOUSE}")
  endif()

  # Check for the presence of the pip package
  if(NOT DUNE_PYTHON_pip_FOUND)
    message(FATAL_ERROR "dune_python_install_package: Requested installations, but pip was not found!")
  endif()

  #
  # Define build rules that install the Python package into the Dune virtualenv at the build stage
  #

  # Install the Python Package into the Dune virtual environment in the build stage
  string(REPLACE "/" "_" envtargetname "env_install_python_${CMAKE_CURRENT_SOURCE_DIR}_${PYINST_PATH}")
  add_custom_target(
    ${envtargetname}
    ALL
    COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m pip install
      --upgrade               # TODO: Check with Andreas whether we really need --upgrade
      "${WHEEL_OPTION}"
      --editable              # Installations into the internal env are always editable
      ${PYINST_ADDITIONAL_PIP_PARAMS} ${DUNE_PYTHON_ADDITIONAL_PIP_PARAMS}
      "${PYINST_FULLPATH}"
    COMMENT "Installing Python package at ${PYINST_FULLPATH} into Dune virtual environment..."
    DEPENDS ${PYINST_DEPENDS}
  )

  #
  # Now define rules for `make install_python`.
  #

  # Only add installation rules if it was requested
  if(NOT "${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "none")
    # Construct the installation location option string
    set(USER_INSTALL_OPTION "")
    if("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "user")
      set(USER_INSTALL_OPTION "--user")
    endif()

    # Add a custom target that globally installs this package if requested
    string(REPLACE "/" "_" targetname "install_python_${CMAKE_CURRENT_SOURCE_DIR}_${PYINST_PATH}")
    add_custom_target(${targetname}
                      COMMAND ${Python3_EXECUTABLE} -m pip install
                        "${USER_INSTALL_OPTION}"
                        --upgrade
                        "${WHEEL_OPTION}"
                        ${PYINST_ADDITIONAL_PIP_PARAMS} ${DUNE_PYTHON_ADDITIONAL_PIP_PARAMS}
                        "${PYINST_FULLPATH}"
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
  endif()

  #
  # Add some CMake-exported metadata to the package
  #

  if(PYINST_CMAKE_METADATA_FILE)
    # Locate the cmake/scripts directory of dune-common
    dune_module_path(MODULE dune-common
                     RESULT scriptdir
                     SCRIPT_DIR)

    # Add the metadata file to MANIFEST.in to enable its installation
    file(
      APPEND ${PYINST_FULLPATH}/MANIFEST.in
      "include ${PYINST_CMAKE_METADATA_FILE}\n"
    )

    # Determine full path of the meta data file
    set(metadatafile ${PYINST_FULLPATH}/${PYINST_CMAKE_METADATA_FILE})

    # Collect some variables that we would like to export
    set(_export_builddirs ${CMAKE_BINARY_DIR})
    foreach(mod ${ALL_DEPENDENCIES})
      list(APPEND _export_builddirs ${${mod}_DIR})
    endforeach()

    # Make sure to generate the metadata for the build stage
    add_custom_target(
      metadata_${envtargetname}
      COMMAND ${CMAKE_COMMAND}
        -Dmetadatafile=${metadatafile}
        -DDEPBUILDDIRS=${_export_builddirs}
        -DDEPS="${PROJECT_NAME};${ALL_DEPENDENCIES}"
        -DMODULENAME=${PROJECT_NAME}
        -P ${scriptdir}/WritePythonCMakeMetadata.cmake
      COMMENT "Generating the CMake metadata file at ${PYINST_CMAKE_METADATA_FILE}"
    )
    add_dependencies(${envtargetname} metadata_${envtargetname})

    # Make sure to generate the metadata for the install stage
    if(NOT "${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "none")
      add_custom_target(
        metadata_${targetname}
        COMMAND ${CMAKE_COMMAND}
          -Dmetadatafile=${metadatafile}
          -DDEPS="${PROJECT_NAME};${ALL_DEPENDENCIES}"
          -DDEPBUILDDIRS=${_export_builddirs}
          -DMODULENAME=${PROJECT_NAME}
          -DINSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
          -P ${scriptdir}/WritePythonCMakeMetadata.cmake
        COMMENT "Generating the CMake metadata file at ${PYINST_CMAKE_METADATA_FILE}"
      )
      add_dependencies(${targetname} metadata_${targetname})
    endif()
  endif()

endfunction()
