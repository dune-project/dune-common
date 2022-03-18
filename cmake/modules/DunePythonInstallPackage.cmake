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
# .. cmake_variable:: DUNE_PYTHON_ADDITIONAL_PIP_PARAMS
#
#    Use this variable to set additional flags for pip in this build. This can e.g.
#    be used to point pip to alternative package indices in restricted environments.
#
include_guard(GLOBAL)

function(dune_python_install_package)
  # Parse Arguments
  set(OPTION)
  set(SINGLE PATH CMAKE_METADATA_FILE PACKAGENAME)
  set(MULTI ADDITIONAL_PIP_PARAMS DEPENDS CMAKE_METADATA_FLAGS)
  cmake_parse_arguments(PYINST "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})
  if(PYINST_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dune_python_install_package: This often indicates typos!")
  endif()

  # set the new package name
  if("${PYINST_PACKAGENAME}" STREQUAL "")
      set(PYINST_PACKAGENAME "dune")
  endif()

  # This argument has been introduced after release 2.8 so no deprecation is necessary
  # To ease the transition in downstream modules we print a warning. However, this
  # can be removed anytime _before_ release 2.9 once most downstream modules have been adapted.
  if(PYINST_CMAKE_METADATA_FILE)
    message(WARNING "Argument CMAKE_METADATA_FILE is ignored and should be removed. The path is set to ${PYINST_PACKAGENAME}/data/${ProjectName}.cmake.")
  endif()

  # set the meta data file path for this package
  set(PYINST_CMAKE_METADATA_FILE "${PYINST_PACKAGENAME}/data/${ProjectName}.cmake")

  # 'ProjectPythonRequires' is used for pip install below
  # 'RequiredPythonModules' is added to the builddir/python/setup.py
  #    This also contains the dependent dune modules which can be available
  #    in the dune wheelhouse (e.g. used in the nightly-build)
  set(RequiredPythonModules "${ProjectPythonRequires}")
  # Configure setup.py.in if present
  foreach(mod ${ALL_DEPENDENCIES})
    if(${${mod}_HASPYTHON}) # module found and has python bindings
      string(APPEND RequiredPythonModules " ${mod}")
      string(APPEND ProjectPythonRequires " ${${mod}_PYTHONREQUIRES}")
    endif()
  endforeach()

  # if MPI is found dune-common will be linked to MPI
  # in that case we require mpi4py for MPI support from the Python side
  # This will not install mpi4py when installing in build-isolation but
  # will install it into the (internal) venv during a source build.
  # The case of a package installation is taken care of in
  # `python/dune/common/__init__.py'.
  if(HAVE_MPI AND NOT SKBUILD)
    message(STATUS "Adding mpi4py to the Python requirements")
    string(APPEND ProjectPythonRequires " mpi4py")
  endif()

  set(PYINST_FULLPATH ${CMAKE_CURRENT_SOURCE_DIR}/${PYINST_PATH})
  if(EXISTS ${PYINST_FULLPATH})
    if(EXISTS ${PYINST_FULLPATH}/setup.py.in)
      configure_file(${PYINST_PATH}/setup.py.in ${PYINST_PATH}/setup.py)
    else()
      configure_file(${scriptdir}/setup.py.in ${PYINST_PATH}/setup.py)
    endif()
    set(PYINST_FULLPATH ${CMAKE_CURRENT_BINARY_DIR}/${PYINST_PATH})
  else()
    # Error out if setup.py is missing
    message(FATAL_ERROR "dune_python_install_package: Requested installations, but neither setup.py nor setup.py.in found!")
  endif()

  # Construct the wheel house installation option string
  # First set the path to a Dune wheelhouse that is to be used during installation
  # NB: Right now, the same logic is used to retrieve the location of the
  #     wheelhouse (which means that you have to use the same CMAKE_INSTALL_PREFIX
  #     when *using* installed modules, you used when *installing* them.
  #     TODO: Replace this with a better mechanism (like writing the location into
  #           dune-commons package config file)
  set(DUNE_PYTHON_WHEELHOUSE ${CMAKE_INSTALL_PREFIX}/share/dune/wheelhouse)

  set(WHEEL_OPTION "")
  if(IS_DIRECTORY ${DUNE_PYTHON_WHEELHOUSE})
    set(WHEEL_OPTION "--find-links=file://${DUNE_PYTHON_WHEELHOUSE}")
  endif()

  # Check for the presence of the pip package
  if(NOT DUNE_PYTHON_pip_FOUND)
    message(WARNING "dune_python_install_package: Requested installations, but pip was not found!")
    set(DUNE_PYTHON_VENVSETUP FALSE CACHE BOOL "The internal venv setup failed due to missing pip")
    return()
  endif()

  option(DUNE_RUNNING_IN_CI "This is turned on if running in dune gitlab ci" OFF)
  if(DUNE_RUNNING_IN_CI)
      set(PACKAGE_INDEX "--index-url=https://gitlab.dune-project.org/api/v4/projects/133/packages/pypi/simple")
  else()
      set(PACKAGE_INDEX "")
  endif()

  # Install external requirements (i.e. not dune packages) once at configure stage - install of package is
  # only carried out if this succeeded and with --no-index, i.e., without using any package indices but only local wheels
  # Installing python modules here can lead to issues with versions of module source packages and pypi packages
  # and possible unexpected version downgrades
  string(REPLACE " " ";" RequiredPypiModules "${ProjectPythonRequires}")
  dune_execute_process(COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m pip install
                                "${WHEEL_OPTION}"
                                # we can't use the same additional parameters for both internal
                                # install and normal install so not including these flags at the moment
                                "${PACKAGE_INDEX}"          # stopgap solution until ci repo fixed
                                "${RequiredPypiModules}"
                       RESULT_VARIABLE DUNE_PYTHON_DEPENDENCIES_FAILED
                       WARNING_MESSAGE "python package requirements could not be installed - possibly connection to the python package index failed"
                      )
  if(DUNE_PYTHON_DEPENDENCIES_FAILED)
    set(DUNE_PYTHON_VENVSETUP FALSE CACHE BOOL "The internal venv setup failed: some required packages could not be installed")
    return()
  endif()

  set(DUNE_PYTHON_VENVSETUP TRUE CACHE BOOL "The internal venv setup successfull")

  #
  # Define build rules that install the Python package into the Dune virtualenv at the build stage
  #

  # Install the Python Package into the Dune virtual environment in the build stage
  string(REPLACE "/" "_" envtargetname "env_install_python_${CMAKE_CURRENT_SOURCE_DIR}_${PYINST_PATH}")

  # installation target for dune package into local env - external requirements are already sorted and we want this step to not require
  # internet access. Dune packages need to be installed at this stage and should not be optained from pypi (those packages include the C++ part
  # of the module which we don't want to install. So only use available wheels.
  add_custom_target(
    ${envtargetname}
    ALL
    COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m pip install
      --no-build-isolation      # avoid looking for packages during 'make' if they in the internal venv from previous 'make'
      --no-warn-script-location # supress warnings that dune-env/bin not in path
      --no-index
      "${WHEEL_OPTION}"
      # we can't use the same additional parameters for both internal
      # install and normal install so not including these flags at the moment
      # ${PYINST_ADDITIONAL_PIP_PARAMS} ${DUNE_PYTHON_ADDITIONAL_PIP_PARAMS}
      --editable                  # Installations into the internal env are always editable
      "${PYINST_FULLPATH}"
    COMMENT "Installing Python package at ${PYINST_FULLPATH} into Dune virtual environment (${PACKAGE_INDEX})."
    DEPENDS ${PYINST_DEPENDS}
  )

  #
  # Now define rules for `make install_python`.
  #

  # Determine where to install python packages
  if(NOT DUNE_PYTHON_INSTALL_LOCATION)
    if(DUNE_PYTHON_SYSTEM_IS_VIRTUALENV)
      set(DUNE_PYTHON_INSTALL_LOCATION "system")
    else()
      set(DUNE_PYTHON_INSTALL_LOCATION "user")
    endif()
  endif()

  if(NOT(("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "user") OR
         ("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "system") OR
         ("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "none") OR
         ("${DUNE_PYTHON_INSTALL_LOCATION}" MATCHES "--target")   # this allow to provide a folder, i.e., --target /home/foo/site-packages
    ))
    message(FATAL_ERROR "DUNE_PYTHON_INSTALL_LOCATION must be user|system|none.")
  endif()
  if(("${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "user") AND
     DUNE_PYTHON_SYSTEM_IS_VIRTUALENV)
    message(FATAL_ERROR "Specifying 'user' as install location is incomaptible with using virtual environments (as per pip docs)")
  endif()

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
    string(REPLACE "/" "_" targetname "install_python_${CMAKE_CURRENT_SOURCE_DIR}_${PYINST_PATH}")
    add_custom_target(${targetname}
                      COMMAND ${Python3_EXECUTABLE} -m pip install
                        "${USER_INSTALL_OPTION}"
                       "${PACKAGE_INDEX}"
                        # --use-feature=in-tree-build
                        --upgrade
                        "${WHEEL_OPTION}"
                        ${PYINST_ADDITIONAL_PIP_PARAMS} ${DUNE_PYTHON_ADDITIONAL_PIP_PARAMS}
                        "${PYINST_FULLPATH}"
                      COMMENT "Installing the python package at ${PYINST_FULLPATH} (location ${USER_INSTALL_OPTION})"
                      )

    # during package installation we don't want the package to be installed
    # since skbuild takes care of that.
    if(NOT SKBUILD)
      add_dependencies(install_python ${targetname})
    endif()
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
    if(NOT DUNE_PYTHON_DEPENDENCIES_FAILED)
      if(SKBUILD)
        # this is the only version of the metadata we need for the package insallation
        add_custom_target(
          metadata_${envtargetname}
          COMMAND ${CMAKE_COMMAND}
            -Dmetadatafile=${metadatafile}
            -DDEPS="${PROJECT_NAME};${ALL_DEPENDENCIES}"
            -DMODULENAME=${PROJECT_NAME}
            -DCMAKE_FLAGS="${_cmake_flags}"
            -P ${scriptdir}/WritePythonCMakeMetadata.cmake
          COMMENT "Generating the CMake metadata file at ${PYINST_CMAKE_METADATA_FILE}"
          DEPENDS ${PYINST_DEPENDS}
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
          metadata_${envtargetname}
          COMMAND ${CMAKE_COMMAND}
            -Dmetadatafile=${metadatafile}
            -DDEPBUILDDIRS="${_export_builddirs}"
            -DDEPS="${PROJECT_NAME};${ALL_DEPENDENCIES}"
            -DMODULENAME=${PROJECT_NAME}
            -DCMAKE_FLAGS="${_cmake_flags}"
            -DINSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -P ${scriptdir}/WritePythonCMakeMetadata.cmake
          COMMENT "Generating the CMake metadata file at ${PYINST_CMAKE_METADATA_FILE}"
          DEPENDS ${PYINST_DEPENDS}
        )
        if(NOT "${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "none")
          add_custom_target(
            metadata_${targetname}
            COMMAND ${CMAKE_COMMAND}
              -Dmetadatafile=${metadatafile}
              -DDEPS="${PROJECT_NAME};${ALL_DEPENDENCIES}"
              -DMODULENAME=${PROJECT_NAME}
              -DCMAKE_FLAGS="${_cmake_flags}"
              -DINSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
              -P ${scriptdir}/WritePythonCMakeMetadata.cmake
            COMMENT "Generating the CMake metadata file at ${PYINST_CMAKE_METADATA_FILE}"
          )
          add_dependencies(${targetname} metadata_${targetname})
        endif()
      endif()
      add_dependencies(${envtargetname} metadata_${envtargetname})

      # check consistency of the builddir when using an external venv
      if(DUNE_PYTHON_SYSTEM_IS_VIRTUALENV)
        add_custom_command(TARGET metadata_${envtargetname} PRE_BUILD
                          COMMAND "${CMAKE_COMMAND}" -E echo "configured for interpreter ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE}"
                          COMMAND "${DUNE_PYTHON_VIRTUALENV_EXECUTABLE}" "${scriptdir}/checkvenvconf.py"
                                   checkbuilddirs \"${PROJECT_NAME};${ALL_DEPENDENCIES}\" "${_export_builddirs}"
                          COMMENT checking if the modules used to confiugre this module match those from any installed dune packages
                          )
      endif()

      # Add a custom command that triggers the configuration of dune-py
      add_custom_command(TARGET ${envtargetname} POST_BUILD
                         COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m dune configure
                        )
    endif() # NOT DUNE_PYTHON_DEPENDENCIES_FAILED)


    # Add a custom command that triggers the configuration of dune-py when installing package
    if(NOT "${DUNE_PYTHON_INSTALL_LOCATION}" STREQUAL "none")
      add_custom_command(TARGET ${targetname} POST_BUILD
                         COMMAND ${DUNE_PYTHON_VIRTUALENV_EXECUTABLE} -m dune configure
                        )
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

endfunction()
