# searches for pkg-config, creates the
# file <module-name>.pc from <module-name>.pc.in,
# and adds installation directives.
#

find_package(PkgConfig)

# set some variables that are used in the pkg-config file
set( prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/lib")
set(includedir "\${prefix}/include")
set(PACKAGE_NAME ${DUNE_MOD_NAME})
set(VERSION ${DUNE_MOD_VERSION})
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
  ${PROJECT_SOURCE_DIR}/${DUNE_MOD_NAME}.pc.in
  ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}.pc
  @ONLY
)

# install pkgconfig file
if(PKG_CONFIG_FOUND)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DUNE_MOD_NAME}.pc
    DESTINATION lib/pkgconfig)
endif(PKG_CONFIG_FOUND)
