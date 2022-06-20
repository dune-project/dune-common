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
#    .. cmake_param:: CMAKE_METADATA_FLAGS
#       :multi:
#
#       A list of cmake flags to add to meta data file. For each flag given
#       an entry of the form "flagname:=value" is added. These flags are
#       then set in the CMakeLists.txt file of a generated dune-py module.
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
#    Finding the setup.py of the package follows the following procedure:
#      1. If PATH contains a `setup.py` file, such file will be used to make a `pip install` from the source directory
#      2. If PATH contains a `setup.py.in` file, such file configured and used tp `pip install` the package from the binary directory
#      3. Otherwise, this script will provide a template for `setup.py.in` and continue with 2.
#
# .. cmake_variable:: DUNE_PYTHON_ADDITIONAL_PIP_PARAMS
#
#    Use this variable to set additional flags for pip in this build. This can e.g.
#    be used to point pip to alternative package indices in restricted environments.
#
include_guard(GLOBAL)



# finds all the dependencies of a python package and installs them at configure time
function(dune_python_configure_dependencies)
  # Parse Arguments
  set(OPTION)
  set(SINGLE PATH RESULT)
  cmake_parse_arguments(PYINST "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYINST_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_configure_dependencies: This often indicates typos!")
  endif()

  # Check for the presence of the pip package
  if(NOT DUNE_PYTHON_pip_FOUND)
    message(WARNING "dune_python_configure_dependencies: Requested installations, but pip was not found!")
    set(DUNE_PYTHON_VENVSETUP FALSE CACHE BOOL "The internal venv setup failed due to missing pip")
    return()
  endif()

  if(IS_DIRECTORY ${DUNE_PYTHON_WHEELHOUSE})
    set(WHEEL_OPTION "--find-links=file://${DUNE_PYTHON_WHEELHOUSE}")
  endif()

  # generate egg_info requirement file from setup.py
  string(MD5 PACKAGE_HASH "${PYINST_PATH}")
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_HASH}")
  dune_execute_process(COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} setup.py
                        egg_info --egg-base "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_HASH}"
                        WORKING_DIRECTORY "${PYINST_PATH}"
                        RESULT_VARIABLE REQUIREMENTS_FAILED
                        ERROR_VARIABLE DEPENDENCIES_ERROR
                        WARNING_MESSAGE "python package requirements could not be generated\n${DEPENDENCIES_ERROR}"
                      )

  if(REQUIREMENTS_FAILED)
    set(${PYINST_RESULT} ${REQUIREMENTS_FAILED} PARENT_SCOPE)
    set(DUNE_PYTHON_VENVSETUP FALSE CACHE BOOL "The internal venv setup failed: some required packages could not be installed")
    return()
  else()
    set(DUNE_PYTHON_VENVSETUP TRUE CACHE BOOL "The internal venv setup successfull")
  endif()

  # find the generated egg-info folder
  file(GLOB EGG_INFO_PATH LIST_DIRECTORIES TRUE "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_HASH}/*.egg-info")
  if(EXISTS "${EGG_INFO_PATH}/requires.txt")
    file(READ "${EGG_INFO_PATH}/requires.txt" PACKAGE_REQUIREMENTS)
    string(REPLACE "\n" ";" PACKAGE_REQUIREMENTS ${PACKAGE_REQUIREMENTS})
    foreach(requirement IN LISTS PACKAGE_REQUIREMENTS)
      if("${requirement}" STREQUAL "")
        continue()
      endif()
      if (requirement MATCHES "^\\[")
        # sections (e.g. "[<section>]") in setuptools define the start of optional requirements
        set(OPTIONAL_PACKAGE ON)
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
                          WARNING_MESSAGE "python package requirement '${requirement}' could not be installed - possibly connection to the python package index failed\n${DEPENDENCIES_ERROR}"
                          )
      if(NOT OPTIONAL_PACKAGE)
        if(INSTALL_FAILED)
          set(${PYINST_RESULT} ${INSTALL_FAILED} PARENT_SCOPE)
          set(DUNE_PYTHON_VENVSETUP FALSE CACHE BOOL "The internal venv setup failed: some required packages could not be installed")
          return()
        else()
          set(DUNE_PYTHON_VENVSETUP TRUE CACHE BOOL "The internal venv setup successfull")
        endif()
      endif()
    endforeach()
  endif()

  set(${PYINST_RESULT} 0 PARENT_SCOPE)
endfunction()


# links python package to be used in dune-py
function(dune_link_dune_py)

  # Parse Arguments
  set(OPTION)
  set(SINGLE PATH BUILD_TARGET INSTALL_TARGET)
  set(MULTI CMAKE_METADATA_FLAGS)
  cmake_parse_arguments(PYINST "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYINST_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_install_package: This often indicates typos!")
  endif()

  # set the meta data file path for this package
  set(PYINST_CMAKE_METADATA_FILE "dune/data/${ProjectName}.cmake")


  #
  # Add some CMake-exported metadata to the package
  #

  # Locate the cmake/scripts directory of dune-common
  dune_module_path(MODULE dune-common
                    RESULT scriptdir
                    SCRIPT_DIR)

  # Add the metadata file to MANIFEST.in to enable its installation
  file(
    APPEND "${PYINST_PATH}/MANIFEST.in"
    "include ${PYINST_CMAKE_METADATA_FILE}\n"
  )

  # Determine full path of the meta data file
  set(metadatafile ${PYINST_PATH}/${PYINST_CMAKE_METADATA_FILE})

  # Collect some variables that we would like to export
  set(_export_builddirs "${CMAKE_BINARY_DIR}")
  foreach(mod ${ALL_DEPENDENCIES})
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
  foreach(flags_loop IN ITEMS ${PYINST_CMAKE_METADATA_FLAGS})
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
  string(REPLACE ";" "\;" _cmake_flags "${_cmake_flags}")

  #
  # Generate metadata - note that there is a metadata target for the
  # build stage and one for the install stage so changes need to be made
  # to both.
  #

  # Make sure to generate the metadata for the build stage
  if(SKBUILD)
    # this is the only version of the metadata we need for the package insallation
    add_custom_target(
      metadata_${PYINST_BUILD_TARGET}
      COMMAND ${CMAKE_COMMAND}
        -Dmetadatafile=${metadatafile}
        -DDEPS="${PROJECT_NAME};${ALL_DEPENDENCIES}"
        -DMODULENAME=${PROJECT_NAME}
        -DCMAKE_FLAGS="${_cmake_flags}"
        -P ${scriptdir}/WritePythonCMakeMetadata.cmake
      COMMENT "Generating the CMake metadata file at ${PYINST_CMAKE_METADATA_FILE}"
    )
    # don't need an 'install' target for the metadata since we can use the build version
    # but we need to make sure that skbuild correctly installs the
    # existing metadata file into the site-package
    get_filename_component(PYINST_CMAKE_METADATA_PATH ${PYINST_CMAKE_METADATA_FILE} DIRECTORY)
    # todo: not use 'python' here but somehow use PATH parameter?
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PYINST_CMAKE_METADATA_FILE} DESTINATION python/${PYINST_CMAKE_METADATA_PATH})
  else()
    # this is the build version - keep in mind there is an install version further down
    add_custom_target(
      metadata_${PYINST_BUILD_TARGET}
      COMMAND ${CMAKE_COMMAND}
        -Dmetadatafile=${metadatafile}
        -DDEPBUILDDIRS="${_export_builddirs}"
        -DDEPS="${PROJECT_NAME};${ALL_DEPENDENCIES}"
        -DMODULENAME=${PROJECT_NAME}
        -DCMAKE_FLAGS="${_cmake_flags}"
        -DINSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -P ${scriptdir}/WritePythonCMakeMetadata.cmake
      COMMENT "Generating the CMake metadata file at ${PYINST_CMAKE_METADATA_FILE}"
    )
    if(NOT "${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "none")
      add_custom_target(
        metadata_${PYINST_INSTALL_TARGET}
        COMMAND ${CMAKE_COMMAND}
          -Dmetadatafile=${metadatafile}
          -DDEPS="${PROJECT_NAME};${ALL_DEPENDENCIES}"
          -DMODULENAME=${PROJECT_NAME}
          -DCMAKE_FLAGS="${_cmake_flags}"
          -DINSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
          -P ${scriptdir}/WritePythonCMakeMetadata.cmake
        COMMENT "Generating the CMake metadata file at ${PYINST_CMAKE_METADATA_FILE}"
      )
      add_dependencies(${PYINST_INSTALL_TARGET} metadata_${PYINST_INSTALL_TARGET})
    endif()
  endif()
  add_dependencies(${PYINST_BUILD_TARGET} metadata_${PYINST_BUILD_TARGET})

  # check consistency of the builddir when using an external venv
  if(DUNE_PYTHON_SYSTEM_IS_VIRTUALENV)
    add_custom_command(TARGET metadata_${PYINST_BUILD_TARGET} PRE_BUILD
                      COMMAND "${CMAKE_COMMAND}" -E echo "configured for interpreter ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE}"
                      COMMAND "${DUNE_PYTHON_VIRTUALENV_EXECUTABLE}" "${scriptdir}/checkvenvconf.py"
                                checkbuilddirs \"${PROJECT_NAME};${ALL_DEPENDENCIES}\" "${_export_builddirs}"
                      COMMENT checking if the modules used to confiugre this module match those from any installed dune packages
                      )
  endif()

  # Add a custom command that triggers the configuration of dune-py
  add_custom_command(TARGET ${PYINST_BUILD_TARGET} POST_BUILD
                      COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m dune configure
                    )


  # Add a custom command that triggers the configuration of dune-py when installing package
  if(NOT "${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "none")
    add_custom_command(TARGET ${PYINST_INSTALL_TARGET} POST_BUILD
                        COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m dune configure
                      )
  endif()

endfunction()

function(dune_python_configure_package)
  # Parse Arguments
  set(SINGLE PATH RESULT BUILD_TARGET INSTALL_TARGET)
  set(MULTI ADDITIONAL_PIP_PARAMS DEPENDS)
  cmake_parse_arguments(PYINST "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYINST_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_install_package: This often indicates typos!")
  endif()

  # try to find setup.py. if not found, provide it from a template
  set(PYINST_FULLPATH ${CMAKE_CURRENT_SOURCE_DIR}/${PYINST_PATH})

  if(NOT EXISTS ${PYINST_FULLPATH}/setup.py)
    # 'RequiredPythonModules' is added to the builddir/python/setup.py.in
    #    This also contains the dependent dune modules which can be available
    #    in the dune wheelhouse (e.g. used in the nightly-build)
    set(RequiredPythonModules "${ProjectPythonRequires}")
    if (NOT ("${RequiredPythonModules}" STREQUAL ""))
      message(DEPRECATION "The variable ProjectPythonRequires is deprecated!")
    endif()

    foreach(mod ${ALL_DEPENDENCIES})
      if(${${mod}_HASPYTHON}) # module found and has python bindings
        string(APPEND RequiredPythonModules " ${mod}")
      endif()
    endforeach()

    # Configure setup.py.in if present
    if(EXISTS ${PYINST_FULLPATH}/setup.py.in)
      configure_file(${PYINST_PATH}/setup.py.in ${PYINST_PATH}/setup.py)
      set(PYINST_FULLPATH ${CMAKE_CURRENT_BINARY_DIR}/${PYINST_PATH})
    else()
      configure_file(${scriptdir}/setup.py.in ${PYINST_PATH}/setup.py)
      set(PYINST_FULLPATH ${CMAKE_CURRENT_BINARY_DIR}/${PYINST_PATH})
    endif()
  endif()

  if(NOT EXISTS ${PYINST_FULLPATH})
    message(FATAL_ERROR "dune_python_install_package: ${PYINST_FULLPATH} does not exists")
  endif()

  dune_python_configure_dependencies(PATH ${PYINST_FULLPATH} RESULT PYTHON_DEPENDENCIES_FAILED)

  #
  # Define build rules that install the Python package into the Dune virtualenv at the build stage
  #
  set(PACKAGE_INDEX "${DUNE_PIP_INDEX}")

  # Install the Python Package into the Dune virtual environment in the build stage
  if (NOT PYINST_BUILD_TARGET)
    string(REPLACE "/" "_" PYINST_BUILD_TARGET "env_install_python_${CMAKE_CURRENT_SOURCE_DIR}_${PYINST_PATH}")
  endif()

  if(IS_DIRECTORY ${DUNE_PYTHON_WHEELHOUSE})
    set(WHEEL_OPTION "--find-links=file://${DUNE_PYTHON_WHEELHOUSE}")
  endif()

  # installation target for dune package into local env - external requirements are already sorted and we want this step to not require
  # internet access. Dune packages need to be installed at this stage and should not be obtained from pypi (those packages include the C++ part
  # of the module which we don't want to install. So only use available wheels.
  add_custom_target(
    ${PYINST_BUILD_TARGET}
    ALL
    COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m pip install
      --no-build-isolation      # avoid looking for packages during 'make' if they in the internal venv from previous 'make'
      --no-warn-script-location # suppress warnings that dune-env/bin is not in path
      --no-index
      "${WHEEL_OPTION}"
      # we can't use the same additional parameters for both internal
      # install and normal install so not including these flags at the moment
      # ${PYINST_ADDITIONAL_PIP_PARAMS} ${DUNE_PYTHON_ADDITIONAL_PIP_PARAMS}
      --editable                  # Installations into the internal env are always editable
      .
    WORKING_DIRECTORY "${PYINST_FULLPATH}"
    COMMENT "Installing Python package at ${PYINST_FULLPATH} into Dune virtual environment (${PACKAGE_INDEX})."
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
    if("${DUNE_PYTHON_INSTALL_LOCATION}" MATCHES "--target")
      set(USER_INSTALL_OPTION "${DUNE_PYTHON_INSTALL_LOCATION}")
    endif()

    # Add a custom target that globally installs this package if requested
    if (NOT PYINST_INSTALL_TARGET)
      string(REPLACE "/" "_" PYINST_INSTALL_TARGET "install_python_${CMAKE_CURRENT_SOURCE_DIR}_${PYINST_PATH}")
    endif()
    # TODO this creates an egg-info folder in the source directory
    add_custom_target(${PYINST_INSTALL_TARGET}
                      COMMAND ${Python3_EXECUTABLE} -m pip install
                        "${USER_INSTALL_OPTION}"
                       "${PACKAGE_INDEX}"
                        # --use-feature=in-tree-build
                        --upgrade
                        "${WHEEL_OPTION}"
                        ${PYINST_ADDITIONAL_PIP_PARAMS} ${DUNE_PYTHON_ADDITIONAL_PIP_PARAMS}
                        .
                        WORKING_DIRECTORY "${PYINST_FULLPATH}"
                      COMMENT "Installing the python package at ${PYINST_FULLPATH} (location ${USER_INSTALL_OPTION})"
                      )

    # during package installation we don't want the package to be installed
    # since skbuild takes care of that.
    if(NOT SKBUILD)
      add_dependencies(install_python ${PYINST_INSTALL_TARGET})
    endif()
  endif()

  # Construct the wheel installation commandline
  # TODO should the wheel be build for the internal env setup or for the external one?
  set(WHEEL_COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m pip wheel -w ${DUNE_PYTHON_WHEELHOUSE}
                        "${PACKAGE_INDEX}"
                        --use-feature=in-tree-build
                        "${WHEEL_OPTION}"
                        ${PYINST_ADDITIONAL_PIP_PARAMS} ${DUNE_PYTHON_ADDITIONAL_PIP_PARAMS}
                        "${PYINST_FULLPATH}")
  #
  # Have make install do the same as make install_python plus
  # install a wheel into a central wheelhouse
  # NB: This is necessary, to allow mixing installed and non-installed modules
  #     with python packages. The wheelhouse will allow to install any missing
  #     python packages into a virtual environment.
  #

  install(CODE "set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
                set(DUNE_PYTHON_WHEELHOUSE ${DUNE_PYTHON_WHEELHOUSE})
                include(DuneExecuteProcess)
                message(\"Installing python package\")
                dune_execute_process(COMMAND \"${CMAKE_COMMAND}\" --build .  --target install_python --config $<CONFIG>
                                     WARNING_MESSAGE \"python package installation failed - ignored\")
                message(\"Installing wheel for python package at ${PYINST_FULLPATH} into ${DUNE_PYTHON_WHEELHOUSE}...\")
                dune_execute_process(COMMAND ${WHEEL_COMMAND}
                                     WARNING_MESSAGE \"wheel installation failed - ignored\")"
          )

  if (PYINST_RESULT)
    set(${PYINST_RESULT} ${PYTHON_DEPENDENCIES_FAILED} PARENT_SCOPE)
  endif()
endfunction()


function(dune_python_configure_bindings)
  # Parse Arguments
  set(SINGLE PATH PACKAGENAME)
  set(MULTI ADDITIONAL_PIP_PARAMS DEPENDS CMAKE_METADATA_FLAGS)
  cmake_parse_arguments(PYINST "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYINST_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_install_package: This often indicates typos!")
  endif()

  # set the new package name
  if("${PYINST_PACKAGENAME}" STREQUAL "")
      set(PYINST_PACKAGENAME "dune")
  endif()

  if ("${DUNE_BINDINGS_PACKAGENAME}" STREQUAL "")
    set(DUNE_BINDINGS_PACKAGENAME ${PYINST_PACKAGENAME})
  else()
    message(FATAL_ERROR "DUNE Python bindings can only be configured once per project!")
  endif()

  dune_python_configure_package(
    PATH ${PYINST_PATH}
    DEPENDS ${PYINST_DEPENDS}
    ADDITIONAL_PIP_PARAMS ${PYINST_ADDITIONAL_PIP_PARAMS}
    RESULT PYTHON_PACKAGE_FAILED
    BUILD_TARGET build_python_package_${PYINST_PACKAGENAME}
    INSTALL_TARGET install_python_package_${PYINST_PACKAGENAME}
  )

  if(NOT PYTHON_PACKAGE_FAILED)
    dune_link_dune_py(
      PATH ${CMAKE_CURRENT_BINARY_DIR}/${PYINST_PATH}
      CMAKE_METADATA_FLAGS ${PYINST_CMAKE_METADATA_FLAGS}
      BUILD_TARGET build_python_package_${PYINST_PACKAGENAME}
      INSTALL_TARGET install_python_package_${PYINST_PACKAGENAME}
    )
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

  dune_python_configure_bindings(
    PATH ${PYINST_PATH}
    PACKAGENAME ${PYINST_PACKAGENAME}
    DEPENDS ${PYINST_DEPENDS}
    ADDITIONAL_PIP_PARAMS ${PYINST_ADDITIONAL_PIP_PARAMS}
    CMAKE_METADATA_FLAGS ${PYINST_CMAKE_METADATA_FLAGS}
    BUILD_TARGET build_python_package_${PYINST_PACKAGENAME}
    INSTALL_TARGET install_python_package_${PYINST_PACKAGENAME}
  )

endfunction()
