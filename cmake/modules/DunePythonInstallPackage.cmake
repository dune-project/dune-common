# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# This cmake module provides infrastructure for cmake installation rules concerning python packages.
#
# .. cmake_function:: dune_python_configure_dependencies
#
#    .. cmake_param:: PATH
#       :required:
#       :single:
#
#       Path to the python package source code containing a valid setup.py file.
#       In case of a relative path, it will be evaluated with respect to the :CMAKE_CURRENT_SOURCE_DIR:.
#
#    .. cmake_param:: RESULT
#       :single:
#
#       Variable where to store the result of the dependency configuration.
#       A non-zero result stands for a failure on the configuration of the dependencies.
#
#    .. cmake_param:: INSTALL_CONCRETE_DEPENDENCIES
#       :option:
#
#       This option forces the package dependencies to be installed with concrete dependencies
#       listed in the requirements.txt file
#
#     This function installs the dependencies of a python package at configure time.
#     The dependencies are extracted from the :setup.py: and :requiements.txt: file
#     A failure on the installation of the dependencies does will no trigger a
#     CMake fatal error but it will be reflected on the :RESULT: variable.
#
#
# .. cmake_function:: dune_link_dune_py
#
#    .. cmake_param:: PATH
#       :required:
#       :single:
#
#       Absolute path to the python package source code where to generate metadata.
#
#    .. cmake_param:: INSTALL_TARGET
#       :required:
#       :single:
#
#       Name of the target that generates the package metadata and triggers the :dune-py:
#       module configuration at installation time. This target will only be generated if
#       the build system is setup to install python packages :ref:`DUNE_PYTHON_INSTALL_LOCATION`.
#
#    .. cmake_param:: CMAKE_METADATA_FLAGS
#       :multi:
#
#       A list of cmake flags to add to meta data file. For each flag given
#       an entry of the form "flagname:=value" is added. These flags are
#       then set in the CMakeLists.txt file of a generated dune-py module.
#
#     This function generates the metadata required for Python package in order to be
#     used by the dune-py module. It essentially glues together python code generation
#     of dune-py (via CMake) with a python package. This is achiieved by generating a
#     filename that CMake should export some meta data about this build to.
#     The file will be installed together with the Python package. This mechanism
#     is used by the Python bindings to transport information from CMake to
#     the installed Python package. A module dune-mymodule that provides a Python
#     package dune.mymodule should set this to dune/mymodule/metadata.cmake
#
#    For historic reasons, this function installs the package at configure time rather than at
#    build time. This distinction is important because it means that the package dependencies
#    will be available to be used during configuration time.
#
# .. cmake_function:: dune_python_configure_package
#
#    .. cmake_param:: PATH
#       :required:
#       :single:
#
#       Path to the python package. In case of a relative path, it will be
#       evaluated with respect to the :CMAKE_CURRENT_SOURCE_DIR:.
#
#    .. cmake_param:: INSTALL_TARGET
#       :single:
#
#       Name of the target that installs the package on the install directory.
#
#    .. cmake_param:: RESULT
#       :single:
#
#       Variable where to store the result of the package configuration.
#       A non-zero result stands for a failure on the configuration of the package.
#
#    .. cmake_param:: ADDITIONAL_PIP_PARAMS
#       :multi:
#       :argname: param
#
#       Parameters to add to any :code:`pip install` call (appended).
#
#    .. cmake_param:: INSTALL_CONCRETE_DEPENDENCIES
#       :option:
#
#       See :dune_python_configure_dependencies:
#
#    This function installs the python package located at the given path:
#
#    * installs it to the location specified with :ref:`DUNE_PYTHON_INSTALL_LOCATION` during
#      :code:`make install_python` and during :code:`make install`.
#    * installs a wheel into the Dune wheelhouse during :code:`make install`.
#      This is necessary for mixing installed and non-installed Dune modules.
#
#    The package at the given location is expected to be a pip-installable package.
#    This function installs the package at configure time. This distinction is
#    important because it means that the package will be available to be used during
#    other CMake configureation tasks.
#
#
#
# .. cmake_function:: dune_python_configure_bindings
#
#    .. cmake_param:: PATH
#       :required:
#       :single:
#
#       Relative path to the given python package source code.
#
#    .. cmake_param:: PACKAGENAME
#       :single:
#
#       Name of the python package.
#
#    .. cmake_param:: ADDITIONAL_PIP_PARAMS
#       :multi:
#       :argname: param
#
#       Parameters to add to any :code:`pip install` call (appended).
#
#    .. cmake_param:: CMAKE_METADATA_FLAGS
#       :multi:
#
#       A list of cmake flags to add to meta data file. For each flag given
#       an entry of the form "flagname:=value" is added. These flags are
#       then set in the CMakeLists.txt file of a generated dune-py module.
#
#    This is a convenience function that performs the tasks of
#    :dune_python_configure_dependencies:, :dune_link_dune_py:, and :dune_python_configure_package:.
#    Additionally, it makes sure that a 'setup.py' is available with the following procedure:
#
#      1. If PATH contains a `setup.py` file, such file will be used to make a `pip install` from the source directory
#      2. If PATH contains a `setup.py.in` file, such file will be configured and used to `pip install` the package from the binary directory
#      3. Otherwise, this script will provide a template for `setup.py.in` and continue with 2.
#
# .. cmake_function:: dune_python_install_package
#
#    This function is deprecated, use :dune_python_configure_bindings: or
#    :dune_python_configure_package: according to the needed behavior.
#
# .. cmake_variable:: DUNE_PYTHON_ADDITIONAL_PIP_PARAMS
#
#    Use this variable to set additional flags for pip in this build. This can e.g.
#    be used to point pip to alternative package indices in restricted environments.
#
include_guard(GLOBAL)



function(dune_python_configure_dependencies)
  # Parse Arguments
  set(OPTION)
  set(SINGLE PATH RESULT INSTALL_CONCRETE_DEPENDENCIES)
  cmake_parse_arguments(PYCONFDEPS "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYCONFDEPS_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_configure_dependencies: This often indicates typos!")
  endif()

  # Check for the presence of the pip package
  if(NOT DUNE_PYTHON_pip_FOUND)
    message(WARNING "dune_python_configure_dependencies: Requested installations, but pip was not found!")
    set(DUNE_PYTHON_VENVSETUP FALSE CACHE BOOL "The internal venv setup failed due to missing pip")
    return()
  endif()

  if(NOT IS_ABSOLUTE "${PYCONFDEPS_PATH}")
    set(PYCONFDEPS_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PYCONFDEPS_PATH}")
  endif()

  if(NOT EXISTS "${PYCONFDEPS_PATH}/setup.py")
    message(FATAL_ERROR "Directory '${PYCONFDEPS_PATH}' does not contain a configuration file 'setup.py'")
  endif()

  if(IS_DIRECTORY ${DUNE_PYTHON_WHEELHOUSE})
    set(WHEEL_OPTION "--find-links=file://${DUNE_PYTHON_WHEELHOUSE}")
  endif()

  if(PYCONFDEPS_INSTALL_CONCRETE_DEPENDENCIES)
    # if requirements file exists, install them directly
    message(STATUS "Installing python package concrete requirements at ${PYPKGCONF_PATH}/requirements.txt")
    # Install requirements (e.g. not dune packages) once at configure stage
    dune_execute_process(COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m pip install
                                  "${WHEEL_OPTION}"
                                  # we can't use the same additional parameters for both internal
                                  # install and normal install so not including these flags at the moment
                                  "${DUNE_PIP_INDEX}"          # stopgap solution until ci repo fixed
                                  -r requirements.txt
                        WORKING_DIRECTORY "${PYCONFDEPS_PATH}"
                        RESULT_VARIABLE INSTALL_FAILED
                        ERROR_VARIABLE DEPENDENCIES_ERROR
                        WARNING_MESSAGE "python package requirements could not be installed - possibly connection to the python package index failed\n${DEPENDENCIES_ERROR}"
                        )
    if(NOT OPTIONAL_PACKAGE)
      if(INSTALL_FAILED)
        set(${PYCONFDEPS_RESULT} ${INSTALL_FAILED} PARENT_SCOPE)
        set(DUNE_PYTHON_VENVSETUP FALSE CACHE BOOL "The internal venv setup failed: some required packages could not be installed")
        return()
      else()
        set(DUNE_PYTHON_VENVSETUP TRUE CACHE BOOL "The internal venv setup successful")
      endif()
    endif()
  endif()

  # generate egg_info requirement file from setup.py
  string(MD5 PACKAGE_HASH "${PYCONFDEPS_PATH}")
  set(EGG_INFO_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_HASH}")
  file(MAKE_DIRECTORY "${EGG_INFO_PATH}")
  dune_execute_process(COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} setup.py
                        egg_info --egg-base "${EGG_INFO_PATH}"
                        WORKING_DIRECTORY "${PYCONFDEPS_PATH}"
                        RESULT_VARIABLE REQUIREMENTS_FAILED
                        ERROR_VARIABLE DEPENDENCIES_ERROR
                        WARNING_MESSAGE "python package requirements could not be generated\n${DEPENDENCIES_ERROR}"
                      )

  if(REQUIREMENTS_FAILED)
    set(${PYCONFDEPS_RESULT} ${REQUIREMENTS_FAILED} PARENT_SCOPE)
    set(DUNE_PYTHON_VENVSETUP FALSE CACHE BOOL "The internal venv setup failed: some required packages could not be installed")
    return()
  else()
    set(DUNE_PYTHON_VENVSETUP TRUE CACHE BOOL "The internal venv setup successful")
  endif()

  # find the generated egg-info folder and install each dependency listed on the requires.txt file
  file(GLOB EGG_INFO_PATH LIST_DIRECTORIES TRUE "${EGG_INFO_PATH}/*.egg-info")
  if(EXISTS "${EGG_INFO_PATH}/requires.txt")
    file(READ "${EGG_INFO_PATH}/requires.txt" PACKAGE_REQUIREMENTS)
    string(REPLACE "\n" ";" PACKAGE_REQUIREMENTS ${PACKAGE_REQUIREMENTS})
    string(REPLACE ";" " " PACKAGE_REQUIREMENTS_STR "${PACKAGE_REQUIREMENTS}")
    message(STATUS "Installing python package abstract requirements: " ${PACKAGE_REQUIREMENTS_STR})
    foreach(requirement IN LISTS PACKAGE_REQUIREMENTS)
      if("${requirement}" STREQUAL "")
        continue()
      endif()
      if (requirement MATCHES "^\\[")
        # sections (e.g. "[<section>]") in setuptools define the start of optional requirements
        set(OPTIONAL_PACKAGE ON)
        set(PYCONFDEPS_OPT "(optional) ")
        continue()
      endif()
      # Install requirements (e.g. not dune packages) once at configure stage
      dune_execute_process(COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m pip install
                                    "${WHEEL_OPTION}"
                                    # we can't use the same additional parameters for both internal
                                    # install and normal install so not including these flags at the moment
                                    "${DUNE_PIP_INDEX}"          # stopgap solution until ci repo fixed
                                    "${requirement}"
                          RESULT_VARIABLE INSTALL_FAILED
                          ERROR_VARIABLE DEPENDENCIES_ERROR
                          WARNING_MESSAGE "Python ${PYCONFDEPS_OPT}package requirement '${requirement}' could not be installed - possibly connection to the python package index failed\n${DEPENDENCIES_ERROR}"
                          )
      if(NOT OPTIONAL_PACKAGE)
        if(INSTALL_FAILED)
          set(${PYCONFDEPS_RESULT} ${INSTALL_FAILED} PARENT_SCOPE)
          set(DUNE_PYTHON_VENVSETUP FALSE CACHE BOOL "The internal venv setup failed: some required packages could not be installed")
          return()
        else()
          set(DUNE_PYTHON_VENVSETUP TRUE CACHE BOOL "The internal venv setup successful")
        endif()
      endif()
    endforeach()
  endif()

  set(${PYCONFDEPS_RESULT} 0 PARENT_SCOPE)
endfunction()


function(dune_link_dune_py)
  # Parse Arguments
  set(OPTION)
  set(SINGLE PATH INSTALL_TARGET PACKAGENAME)
  set(MULTI CMAKE_METADATA_FLAGS)
  cmake_parse_arguments(LINKDUNEPY "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(LINKDUNEPY_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_install_package: This often indicates typos!")
  endif()


  # check the package name argument
  if("${LINKDUNEPY_PACKAGENAME}" STREQUAL "")
    message(FATAL_ERROR "PACKAGENAME cannot be empty!")
  endif()

  # set the meta data file path for this package
  set(LINKDUNEPY_CMAKE_METADATA_FILE "${LINKDUNEPY_PACKAGENAME}/data/${ProjectName}.cmake")

  # Locate the cmake/scripts directory of dune-common
  dune_module_path(MODULE dune-common
                    RESULT scriptdir
                    SCRIPT_DIR)


  if(NOT IS_ABSOLUTE "${LINKDUNEPY_PATH}")
    message(FATAL_ERROR "'PATH=${LINKDUNEPY_PATH}' argument is not an absulte path")
  endif()

  if(NOT EXISTS "${LINKDUNEPY_PATH}/setup.py")
    message(Warning "Directory '${LINKDUNEPY_PATH}' does not contain a configuration file 'setup.py'. Link for dune-py module may not work properly")
  endif()

  # Add the metadata file to MANIFEST.in to enable its installation
  file(
    APPEND "${LINKDUNEPY_PATH}/MANIFEST.in"
    "include ${LINKDUNEPY_CMAKE_METADATA_FILE}\n"
  )

  # Determine full path of the meta data file
  set(metadatafile ${LINKDUNEPY_PATH}/${LINKDUNEPY_CMAKE_METADATA_FILE})

  # Collect some variables that we would like to export
  set(_deps ${ProjectName})
  set(_export_builddirs "${CMAKE_BINARY_DIR}")
  foreach(mod ${ALL_DEPENDENCIES})
    string(APPEND _deps " ${mod}")
    string(APPEND _export_builddirs "\;${${mod}_DIR}")
  endforeach()

  # add the list of HAVE_{MODULE} flags to the meta data
  set(_cmake_flags "")
  foreach(_dep ${ProjectName} ${ALL_DEPENDENCIES})
    dune_module_to_uppercase(upper ${_dep})
    if(DEFINED HAVE_${upper})
      list(APPEND _cmake_flags "HAVE_${upper}:=${HAVE_${upper}}")
    endif()
  endforeach()

  # automatically add DUNE_OPTS_FILE
  list(APPEND _cmake_flags "DUNE_OPTS_FILE:=${DUNE_OPTS_FILE}")

  # handle all manually added flags
  foreach(flags_loop IN ITEMS ${LINKDUNEPY_CMAKE_METADATA_FLAGS})
    if(${flags_loop})
      set(value ${${flags_loop}})
      # need to make sure not to use the script generated by the CXX_OVERWRITE
      # because dune-py should not depend on a file in the build dir
      if(DEFINED DEFAULT_CXX_COMPILER AND "${flags_loop}" STREQUAL "CMAKE_CXX_COMPILER")
        set(value "${DEFAULT_CXX_COMPILER}")
      endif()
      if(DEFINED DEFAULT_CXXFLAGS AND "${flags_loop}" STREQUAL "CMAKE_CXX_FLAGS")
        set(value "${DEFAULT_CXXFLAGS}")
      endif()
      list(APPEND _cmake_flags "${flags_loop}:=\"${value}\"")
    endif()
  endforeach()
  # transform the list into an escaped string
  string(REPLACE ";" "<SEP>" _cmake_flags "${_cmake_flags}")

  #
  # Generate metadata - note that there is a metadata target for the
  # build stage and one for the install stage so changes need to be made
  # to both.
  #

  # check consistency of the builddir when using an external venv
  if(DUNE_PYTHON_SYSTEM_IS_VIRTUALENV)
    message(STATUS "Checking if the modules used to confiugre this module match those from any installed dune packages")
    dune_execute_process(COMMAND "${CMAKE_COMMAND}" -E echo "configured for interpreter ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE}"
                         COMMAND "${DUNE_PYTHON_VIRTUALENV_EXECUTABLE}" "${scriptdir}/checkvenvconf.py"
                                  checkbuilddirs \"${PROJECT_NAME};${ALL_DEPENDENCIES}\" "${_export_builddirs}"
                        )
  endif()

  # Make sure to generate the metadata for the build stage
  # Issue: parameter forwarding fails when using dune_execute_process so
  # reverting to 'execute_process' for now. Alternative would be to extra escape the ';':
  # e.g. string(REPLACE ";" "\\\\\\\;" _export_builddirs # "${_export_builddirs}")
  # and the same for the DEPS argument.
  if(SKBUILD)
    # this is the only version of the metadata we need for the package installation
    message(STATUS "Generating the CMake metadata file at ${LINKDUNEPY_CMAKE_METADATA_FILE}")
    execute_process(
      COMMAND ${CMAKE_COMMAND}
        -Dmetadatafile=${metadatafile}
        -DDEPS=${_deps}
        -DMODULENAME=${PROJECT_NAME}
        -DCMAKE_FLAGS=${_cmake_flags}
        -P ${scriptdir}/WritePythonCMakeMetadata.cmake
    )
    # don't need an 'install' target for the metadata since we can use the build version
    # but we need to make sure that skbuild correctly installs the
    # existing metadata file into the site-package
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LINKDUNEPY_CMAKE_METADATA_FILE} DESTINATION python/dune/data)
  else()
    # this is the build version - keep in mind there is an install version further down
    message(STATUS "Generating the CMake metadata file at ${LINKDUNEPY_CMAKE_METADATA_FILE}")
    execute_process(
      COMMAND ${CMAKE_COMMAND}
        -Dmetadatafile=${metadatafile}
        -DDEPBUILDDIRS=${_export_builddirs}
        -DDEPS=${_deps}
        -DMODULENAME=${PROJECT_NAME}
        -DCMAKE_FLAGS=${_cmake_flags}
        -DINSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -P ${scriptdir}/WritePythonCMakeMetadata.cmake
    )
    #  WARNING_MESSAGE "Writing metadata failed"

    if(NOT "${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "none")
      add_custom_target(
        metadata_${LINKDUNEPY_INSTALL_TARGET}
        COMMAND ${CMAKE_COMMAND}
          -Dmetadatafile=${metadatafile}
          -DDEPS=${_deps}
          -DMODULENAME=${PROJECT_NAME}
          -DCMAKE_FLAGS="${_cmake_flags}"
          -DINSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
          -P ${scriptdir}/WritePythonCMakeMetadata.cmake
        COMMENT "Generating the CMake metadata file at ${LINKDUNEPY_CMAKE_METADATA_FILE}"
      )
    endif()
  endif()

  # Trigger the configuration of dune-py
  dune_execute_process(COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m dune configure)

  # Add a custom command that triggers the configuration of dune-py when installing package
  if(NOT "${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "none")
    dune_execute_process(COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m dune configure)
  endif()

endfunction()


function(dune_python_configure_package)
  # Parse Arguments
  set(SINGLE PATH RESULT INSTALL_TARGET INSTALL_CONCRETE_DEPENDENCIES)
  set(MULTI ADDITIONAL_PIP_PARAMS)
  cmake_parse_arguments(PYPKGCONF "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYPKGCONF_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_install_package: This often indicates typos!")
  endif()

  if(NOT IS_ABSOLUTE "${PYPKGCONF_PATH}")
    set(PYPKGCONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${PYPKGCONF_PATH})
  endif()

  dune_python_configure_dependencies(
       PATH ${PYPKGCONF_PATH}
       RESULT PYTHON_DEPENDENCIES_FAILED
       ${PYPKGCONF_INSTALL_CONCRETE_DEPENDENCIES}
  )

  if (PYTHON_DEPENDENCIES_FAILED)
    set(${PYPKGCONF_RESULT} ${PYTHON_DEPENDENCIES_FAILED} PARENT_SCOPE)
    return()
  endif()

  if(IS_DIRECTORY ${DUNE_PYTHON_WHEELHOUSE})
    set(WHEEL_OPTION "--find-links=file://${DUNE_PYTHON_WHEELHOUSE}")
  endif()

  # installation command for dune package into local env - external requirements are already sorted and we want this step to not require
  # internet access. Dune packages need to be installed at this stage and should not be obtained from pypi (those packages include the C++ part
  # of the module which we don't want to install. So only use available wheels.
  if(NOT SKBUILD)
    message(STATUS "Installing python package at ${PYPKGCONF_PATH} into Dune virtual environment ${DUNE_PIP_INDEX}")
    dune_execute_process(
      COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m pip install
        --no-build-isolation      # avoid looking for packages during 'make' if they in the internal venv from previous 'make'
        --no-warn-script-location # suppress warnings that dune-env/bin is not in path
        --no-index
        "${WHEEL_OPTION}"
        # we can't use the same additional parameters for both internal
        # install and normal install so not including these flags at the moment
        # ${PYPKGCONF_ADDITIONAL_PIP_PARAMS} ${DUNE_PYTHON_ADDITIONAL_PIP_PARAMS}
        --editable                  # Installations into the internal env are always editable
        .
      WORKING_DIRECTORY "${PYPKGCONF_PATH}"
      RESULT_VARIABLE PYTHON_INSTALL_FAILED
      ERROR_VARIABLE PYTHON_INSTALL_ERROR
      WARNING_MESSAGE "python package at '${PYPKGCONF_PATH}' could not be installed - possibly connection to the python package index failed\n${PYTHON_INSTALL_ERROR}"
    )
    set(${PYPKGCONF_RESULT} ${PYTHON_INSTALL_FAILED} PARENT_SCOPE)
    if (PYTHON_INSTALL_FAILED)
      return()
    endif()
  endif()

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
    if("${DUNE_PYTHON_INSTALL_LOCATION}" MATCHES "--target")
      set(USER_INSTALL_OPTION "${DUNE_PYTHON_INSTALL_LOCATION}")
    endif()

    # Add a custom target that globally installs this package if requested
    if (NOT PYPKGCONF_INSTALL_TARGET)
      string(REPLACE "/" "_" PYPKGCONF_INSTALL_TARGET "install_python_${CMAKE_CURRENT_SOURCE_DIR}_${PYPKGCONF_PATH}")
    endif()
    # TODO this creates an egg-info folder in the source directory
    add_custom_target(${PYPKGCONF_INSTALL_TARGET}
                      COMMAND ${Python3_EXECUTABLE} -m pip install
                        "${USER_INSTALL_OPTION}"
                       "${DUNE_PIP_INDEX}"
                        # --use-feature=in-tree-build
                        --upgrade
                        "${WHEEL_OPTION}"
                        ${PYPKGCONF_ADDITIONAL_PIP_PARAMS} ${DUNE_PYTHON_ADDITIONAL_PIP_PARAMS}
                        .
                        WORKING_DIRECTORY "${PYPKGCONF_PATH}"
                      COMMENT "Installing the python package at ${PYPKGCONF_PATH} (location ${USER_INSTALL_OPTION})"
                      )

    # during package installation we don't want the package to be installed
    # since skbuild takes care of that.
    if(NOT SKBUILD)
      add_dependencies(install_python ${PYPKGCONF_INSTALL_TARGET})
    endif()
  endif()

  # Construct the wheel installation commandline
  # TODO should the wheel be build for the internal env setup or for the external one?
  set(WHEEL_COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m pip wheel -w ${DUNE_PYTHON_WHEELHOUSE}
                        "${DUNE_PIP_INDEX}"
                        # --use-feature=in-tree-build
                        "${WHEEL_OPTION}"
                        ${PYPKGCONF_ADDITIONAL_PIP_PARAMS} ${DUNE_PYTHON_ADDITIONAL_PIP_PARAMS}
                        "${PYPKGCONF_PATH}")
  #
  # Have make install do the same as make install_python plus
  # install a wheel into a central wheelhouse
  # NB: This is necessary, to allow mixing installed and non-installed modules
  #     with python packages. The wheelhouse will allow to install any missing
  #     python packages into a virtual environment.
  #

  if(NOT SKBUILD)
    install(CODE "set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
                  set(DUNE_PYTHON_WHEELHOUSE ${DUNE_PYTHON_WHEELHOUSE})
                  include(DuneExecuteProcess)
                  message(\"Installing python package\")
                  dune_execute_process(COMMAND \"${CMAKE_COMMAND}\" --build .  --target install_python --config $<CONFIG>
                                       WARNING_MESSAGE \"python package installation failed - ignored\")
                  message(\"Installing wheel for python package at ${PYPKGCONF_PATH} into ${DUNE_PYTHON_WHEELHOUSE}...\")
                  dune_execute_process(COMMAND ${WHEEL_COMMAND}
                                       WARNING_MESSAGE \"wheel installation failed - ignored\")"
            )
  endif()

endfunction()


function(dune_python_configure_bindings)
  # Parse Arguments
  set(SINGLE PATH PACKAGENAME)
  set(MULTI ADDITIONAL_PIP_PARAMS CMAKE_METADATA_FLAGS)
  cmake_parse_arguments(PYCONFBIND "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYCONFBIND_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_configure_bindings: This often indicates typos!")
  endif()

  # set the new package name
  if("${PYCONFBIND_PACKAGENAME}" STREQUAL "")
    set(PYCONFBIND_PACKAGENAME "dune")
  endif()

  if ("${DUNE_BINDINGS_PACKAGENAME}" STREQUAL "")
    set(DUNE_BINDINGS_PACKAGENAME ${PYCONFBIND_PACKAGENAME})
  else()
    message(FATAL_ERROR "DUNE Python bindings can only be configured once per project!")
  endif()

  if(IS_ABSOLUTE "${PYCONFBIND_PATH}")
    message(FATAL_ERROR "'PATH=${PYCONFBIND_PATH}' is not a relative path")
  endif()

  # try to find setup.py. if not found, provide it from a template
  set(PYCONFBIND_FULLPATH ${CMAKE_CURRENT_SOURCE_DIR}/${PYCONFBIND_PATH})

  if(NOT EXISTS ${PYCONFBIND_FULLPATH}/setup.py)
    # 'RequiredPythonModules' is added to the builddir/python/setup.py.in
    #    This also contains the dependent dune modules which can be available
    #    in the dune wheelhouse (e.g. used in the nightly-build)
    set(RequiredPythonModules "${ProjectPythonRequires}")

    foreach(mod ${ALL_DEPENDENCIES})
      if(${${mod}_HASPYTHON}) # module found and has python bindings
        string(APPEND RequiredPythonModules " ${mod}")
      endif()
    endforeach()

    # Configure setup.py.in if present
    if(EXISTS ${PYCONFBIND_FULLPATH}/setup.py.in)
      configure_file(${PYCONFBIND_PATH}/setup.py.in ${PYCONFBIND_PATH}/setup.py)
      set(PYCONFBIND_FULLPATH ${CMAKE_CURRENT_BINARY_DIR}/${PYCONFBIND_PATH})
    else()
      configure_file(${scriptdir}/setup.py.in ${PYCONFBIND_PATH}/setup.py)
      set(PYCONFBIND_FULLPATH ${CMAKE_CURRENT_BINARY_DIR}/${PYCONFBIND_PATH})
    endif()
  endif()

  if(NOT EXISTS ${PYCONFBIND_FULLPATH})
    message(FATAL_ERROR "dune_python_install_package: ${PYCONFBIND_FULLPATH} does not exists")
  endif()

  dune_python_configure_package(
    PATH ${PYCONFBIND_FULLPATH}
    ADDITIONAL_PIP_PARAMS ${PYCONFBIND_ADDITIONAL_PIP_PARAMS}
    RESULT PYTHON_PACKAGE_FAILED
    INSTALL_TARGET install_python_package_${PYCONFBIND_PACKAGENAME}
  )

  # we could actually add metadata even if the configuration of the venv
  # failed - some people set the PYTHON_PATH env variable instead of using
  # a venv.
  # At the moment the metadata is added to target that only exists if
  # configuration succeeded. When moving to pure configure time this could
  # be changed.
  if(NOT PYTHON_PACKAGE_FAILED)
    dune_link_dune_py(
      PATH ${CMAKE_CURRENT_BINARY_DIR}/${PYCONFBIND_PATH}
      PACKAGENAME ${PYCONFBIND_PACKAGENAME}
      INSTALL_TARGET install_python_package_${PYCONFBIND_PACKAGENAME}
      CMAKE_METADATA_FLAGS ${PYCONFBIND_CMAKE_METADATA_FLAGS}
    )
  else()
    message(WARNING "python binding configuration failed - no linking done")
  endif()

endfunction()


function(dune_python_install_package)
  # Parse Arguments
  set(SINGLE PATH PACKAGENAME)
  set(MULTI ADDITIONAL_PIP_PARAMS DEPENDS CMAKE_METADATA_FLAGS)
  cmake_parse_arguments(PYINST "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYINST_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_install_package: This often indicates typos!")
  endif()

  message(DEPRECATION "This function is deprecated. Use 'dune_python_configure_bindings' for python binding packages or 'dune_python_configure_package' for installable python packages")

  if(PYINST_DEPENDS)
    message(DEPRECATION "Argument DEPENDS is deprecated and will be ignored!")
  endif()

  dune_python_configure_bindings(
    PATH ${PYINST_PATH}
    PACKAGENAME ${PYINST_PACKAGENAME}
    ADDITIONAL_PIP_PARAMS ${PYINST_ADDITIONAL_PIP_PARAMS}
    CMAKE_METADATA_FLAGS ${PYINST_CMAKE_METADATA_FLAGS}
    INSTALL_TARGET install_python_package_${PYINST_PACKAGENAME}
  )

endfunction()
