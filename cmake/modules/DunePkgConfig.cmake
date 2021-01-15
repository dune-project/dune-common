# searches for pkg-config, creates the
# file <module-name>.pc from <module-name>.pc.in,
# and adds installation directives.
#

find_package(PkgConfig)
# text for feature summary
set_package_properties("PkgConfig" PROPERTIES
  DESCRIPTION "Unified interface for querying installed libraries"
  PURPOSE "To find Dune module dependencies")

function(create_and_install_pkconfig installlibdir)
  # set some variables that are used in the pkg-config file
  include(GNUInstallDirs)

  if(SKBUILD)
    # we are using scikit-build to build a python wheel. The install prefix
    # set by scikit is within a tmp directory (isolated build) and
    # therefore not suitable for the prefix in the pc file. At least when
    # installed into a virtual env the correct prefix path is two below the
    # location of the pc file, i.e.,
    # location of pc files: dune-env/lib/pkgconfig
    # location of dune.module files: dune-env/lib/dunecontrol
    # and from the documentation
    #     installed module: ${path}/lib/dunecontrol/${name}/dune.module
    #     and there is a file ${path}/lib/pkgconfig/${name}.pc
    set( prefix "\${pcfiledir}/../..")
  else()
    set( prefix ${CMAKE_INSTALL_PREFIX})
  endif()

  set(exec_prefix "\${prefix}")
  set(libdir "\${exec_prefix}/${DUNE_INSTALL_LIBDIR}")
  set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
  set(PACKAGE_NAME ${ProjectName})
  set(VERSION ${ProjectVersion})
  set(CC ${CMAKE_C_COMPILER})
  set(CXX ${CMAKE_CXX_COMPILER})

  set(PKG_CFG_REQUIRES_LIST ${${ProjectName}_PKG_CFG_REQUIRES})
  list(REMOVE_DUPLICATES PKG_CFG_REQUIRES_LIST)
  list(REVERSE PKG_CFG_REQUIRES_LIST)

  # Add some flags from the config.h file
  dune_module_to_uppercase(PROJECT_NAME ${ProjectName})
  list(APPEND ${ProjectName}_PKG_CFG_CFLAGS
    "-D${PROJECT_NAME}_VERSION=${${PROJECT_NAME}_VERSION}"
    "-D${PROJECT_NAME}_VERSION_MAJOR=${${PROJECT_NAME}_VERSION_MAJOR}"
    "-D${PROJECT_NAME}_VERSION_MINOR=${${PROJECT_NAME}_VERSION_MINOR}"
    "-D${PROJECT_NAME}_VERSION_REVISION=${${PROJECT_NAME}_VERSION_REVISION}")

  string(JOIN "," PKG_CFG_REQUIRES ${PKG_CFG_REQUIRES_LIST})
  string(JOIN " " PKG_CFG_CFLAGS   ${${ProjectName}_PKG_CFG_CFLAGS})
  string(JOIN " " PKG_CFG_LIBS     ${${ProjectName}_PKG_CFG_LIBS})

  #create pkg-config file
  configure_file(
    ${PROJECT_SOURCE_DIR}/${ProjectName}.pc.in
    ${PROJECT_BINARY_DIR}/${ProjectName}.pc
    @ONLY
    )

  # install pkgconfig file
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${ProjectName}.pc
    DESTINATION ${installlibdir}/pkgconfig)

endfunction(create_and_install_pkconfig)


macro(dune_pkg_config_name _target _name _ns)
  set(target_ns "")
  set(target_name "${_target}")

  string(REPLACE "::" ";" _components "${_target}")
  list(LENGTH _components _num_components)

  # extract namespace and target name from <target>
  if(_num_components EQUAL 2)
    list(GET _components 0 target_ns)
    list(GET _components 1 target_name)
  endif()

  # transform name into consistent naming scheme
  string(TOLOWER "${target_name}" target_name)
  string(REPLACE "_" "-" target_name "${target_name}")

  set(${_ns} "${target_ns}")
  set(${_name} "${target_name}")
endmacro(dune_pkg_config_name)


define_property(GLOBAL PROPERTY PKG_CONFIGS
  BRIEF_DOCS "List of created pkg-config files"
  FULL_DOCS "List of created pkg-config files")


# try to extract all the information from the given target
macro(dune_pkg_config_from_target _target _requires _cflags _libs)
  get_target_property(PKG_CFG_INCLUDE_DIRS ${_target} INTERFACE_INCLUDE_DIRECTORIES)
  if(PKG_CFG_INCLUDE_DIRS)
    list(TRANSFORM PKG_CFG_INCLUDE_DIRS PREPEND "-I")
    list(APPEND ${_cflags} ${PKG_CFG_INCLUDE_DIRS})
  endif()

  get_target_property(PKG_CFG_COMPILE_DEFS ${_target} INTERFACE_COMPILE_DEFINITIONS)
  if(PKG_CFG_COMPILE_DEFS)
    list(TRANSFORM PKG_CFG_COMPILE_DEFS PREPEND "-D")
    list(APPEND ${_cflags} ${PKG_CFG_COMPILE_DEFS})
  endif()

  get_target_property(PKG_CFG_COMPILE_OPTS ${_target} INTERFACE_COMPILE_OPTIONS)
  if(PKG_CFG_COMPILE_OPTS)
    list(APPEND ${_cflags} ${PKG_CFG_COMPILE_OPTS})
  endif()

  get_target_property(PKG_CFG_LINK_OPTS ${_target} INTERFACE_LINK_OPTIONS)
  if(PKG_CFG_LINK_OPTS)
    list(APPEND ${_libs} ${INTERFACE_LINK_OPTIONS})
  endif()

  get_target_property(PKG_CFG_LINK_DIRS ${_target} INTERFACE_LINK_DIRECTORIES)
  if(PKG_CFG_LINK_DIRS)
    list(TRANSFORM PKG_CFG_LINK_DIRS PREPEND "-L")
    list(APPEND ${_libs} ${PKG_CFG_LINK_DIRS})
  endif()

  get_target_property(PKG_CFG_LINK_LIBS ${_target} INTERFACE_LINK_LIBRARIES)
  if(NOT PKG_CFG_LINK_LIBS)
    set(PKG_CFG_LINK_LIBS "")
  endif()

  get_property(TARGET_TYPE TARGET ${_target} PROPERTY TYPE)
  if(NOT TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
    get_property(TARGET_HAS_IMPORTED_LOCATION TARGET ${_target} PROPERTY IMPORTED_LOCATION SET)
    if(TARGET_HAS_IMPORTED_LOCATION)
      get_target_property(PKG_CFG_IMPORTED_LOCATION ${_target} IMPORTED_LOCATION)
      if(PKG_CFG_IMPORTED_LOCATION)
        list(APPEND PKG_CFG_LINK_LIBS ${PKG_CFG_IMPORTED_LOCATION})
      endif()
    endif()
  endif()

  foreach(_lib ${PKG_CFG_LINK_LIBS})
    if(TARGET ${_lib})
      # if link library is an actual target
      dune_pkg_config_name(${_lib} _lib_pkg _lib_namespace)
      # do not recursively create pkg-config files
      # dune_create_and_install_pkg_config(${_lib_pkg} NAME ${_lib_pkg} TARGET ${_lib})
      list(APPEND ${_requires} "${_lib_pkg}")
    elseif(EXISTS ${_lib})
      # if link library is given as full path
      list(APPEND ${_libs} ${_lib})
    else()
      # if link library is given as lib-name
      list(APPEND ${_libs} "-l${_lib}")
    endif()
  endforeach()
endmacro(dune_pkg_config_from_target)


function(dune_create_and_install_pkg_config _pkg)
  # check whether pkg-config file was already created
  get_property(PKG_CONFIGS GLOBAL PROPERTY PKG_CONFIGS)
  if(${_pkg} IN_LIST PKG_CONFIGS)
    return()
  endif()

  cmake_parse_arguments(PKG_CFG "" "NAME;DESCRIPTION;URL;VERSION;TARGET" "REQUIRES;CFLAGS;LIBS" ${ARGN})

  if(NOT PKG_CFG_VERSION AND ${PKG_CFG_NAME}_VERSION)
    set(PKG_CFG_VERSION ${${PKG_CFG_NAME}_VERSION})
  endif()

  if(TARGET ${PKG_CFG_TARGET})
    dune_pkg_config_from_target(${PKG_CFG_TARGET} PKG_CFG_REQUIRES PKG_CFG_CFLAGS PKG_CFG_LIBS)
  endif()

  if(PKG_CFG_REQUIRES)
    string(JOIN "," PKG_CFG_REQUIRES ${PKG_CFG_REQUIRES})
  endif()

  if(PKG_CFG_CFLAGS)
    string(JOIN " " PKG_CFG_CFLAGS ${PKG_CFG_CFLAGS})
  endif()

  if(PKG_CFG_LIBS)
    string(JOIN " " PKG_CFG_LIBS ${PKG_CFG_LIBS})
  endif()

  if(EXISTS ${PROJECT_SOURCE_DIR}/cmake/pkg/template.pc.in)
    set(TEMPLATE_PC_FILE "${PROJECT_SOURCE_DIR}/cmake/pkg/template.pc.in")
  elseif(EXISTS ${dune-common_PREFIX}/cmake/pkg/template.pc.in)
    set(TEMPLATE_PC_FILE "${dune-common_PREFIX}/cmake/pkg/template.pc.in")
  endif()

  #create pkg-config file
  configure_file(${TEMPLATE_PC_FILE}
    ${PROJECT_BINARY_DIR}/${_pkg}.pc
    @ONLY)

  # install pkgconfig file
  install(FILES ${PROJECT_BINARY_DIR}/${_pkg}.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

  set_property(GLOBAL APPEND PROPERTY PKG_CONFIGS ${_pkg})
endfunction(dune_create_and_install_pkg_config)


macro(dune_add_pkg_config_requirement reqs)
  foreach(req ${reqs})
    string(REPLACE "(" "" req2 ${req})
    string(REPLACE ")" "" req ${req2})
    list(APPEND ${ProjectName}_PKG_CFG_REQUIRES ${req})
  endforeach()
endmacro(dune_add_pkg_config_requirement)


macro(dune_add_pkg_config_flags flags)
  list(APPEND ${ProjectName}_PKG_CFG_CFLAGS ${flags})
endmacro(dune_add_pkg_config_flags)
