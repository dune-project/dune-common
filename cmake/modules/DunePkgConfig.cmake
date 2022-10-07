# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# searches for pkg-config, creates the
# file <module-name>.pc from <module-name>.pc.in,
# and adds installation directives.
#
include_guard(GLOBAL)

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
