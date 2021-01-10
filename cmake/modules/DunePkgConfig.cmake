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
  set(libdir "\${exec_prefix}/${installlibdir}")
  set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
  set(PACKAGE_NAME ${ProjectName})
  set(VERSION ${ProjectVersion})
  set(CC ${CMAKE_C_COMPILER})
  set(CXX "${CMAKE_CXX_COMPILER} ${CXX_STD11_FLAGS}")

  if(DUNE_DEPENDS)
    foreach(_DUNE_DEPEND ${DUNE_DEPENDS})
      string(REGEX REPLACE "\\(" "" REQF1 ${_DUNE_DEPEND})
      string(REGEX REPLACE "\\)" "" LR ${REQF1})
      if(REQUIRES)
        set(REQUIRES "${REQUIRES} ${LR}")
      else()
        set(REQUIRES ${LR})
      endif(REQUIRES)
    endforeach(_DUNE_DEPEND ${DUNE_DEPENDS})
  endif(DUNE_DEPENDS)

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


function(dune_create_and_install_pkg_config PKG_CFG_NAME)
  cmake_parse_arguments(PKG_CFG "" "DESCRIPTION;URL;VERSION;TARGET" "REQUIRES;CFLAGS;LIBS" ${ARGN})
  if(TARGET ${PKG_CFG_TARGET})
    get_target_property(PKG_CFG_INCLUDE_DIRS ${PKG_CFG_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(PKG_CFG_COMPILE_DEFS ${PKG_CFG_TARGET} INTERFACE_COMPILE_DEFINITIONS)
    get_target_property(PKG_CFG_COMPILE_OPTS ${PKG_CFG_TARGET} INTERFACE_COMPILE_OPTIONS)
    get_target_property(PKG_CFG_LINK_DIRS ${PKG_CFG_TARGET} INTERFACE_LINK_DIRECTORIES)
    get_target_property(PKG_CFG_LINK_LIBS ${PKG_CFG_TARGET} INTERFACE_LINK_LIBRARIES)
    get_target_property(PKG_CFG_LINK_OPTS ${PKG_CFG_TARGET} INTERFACE_LINK_OPTIONS)
    # TODO: transform target properties
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
    ${PROJECT_BINARY_DIR}/${PKG_CFG_NAME}.pc
    @ONLY)

  # install pkgconfig file
  install(FILES ${PROJECT_BINARY_DIR}/${PKG_CFG_NAME}.pc
    DESTINATION ${DUNE_INSTALL_LIBDIR}/pkgconfig)

endfunction(dune_create_and_install_pkg_config)
