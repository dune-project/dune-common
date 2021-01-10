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

macro(dune_pkg_config_name _name _var)
  string(TOLOWER "${_name}" ${_var})
  string(REPLACE "_" "-" ${_var} "${${_var}}")
endmacro(dune_pkg_config_name)

function(dune_create_and_install_pkg_config _name)
  cmake_parse_arguments(PKG_CFG "" "NAME;DESCRIPTION;URL;VERSION;TARGET" "REQUIRES;CFLAGS;LIBS" ${ARGN})
  if(TARGET ${PKG_CFG_TARGET})
    # try to extract all the information from the given target
    get_target_property(PKG_CFG_INCLUDE_DIRS ${PKG_CFG_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
    if(PKG_CFG_INCLUDE_DIRS)
      list(TRANSFORM PKG_CFG_INCLUDE_DIRS PREPEND "-I")
      list(APPEND PKG_CFG_CFLAGS ${PKG_CFG_INCLUDE_DIRS})
    endif()

    get_target_property(PKG_CFG_COMPILE_DEFS ${PKG_CFG_TARGET} INTERFACE_COMPILE_DEFINITIONS)
    if(PKG_CFG_COMPILE_DEFS)
      list(TRANSFORM PKG_CFG_COMPILE_DEFS PREPEND "-D")
      list(APPEND PKG_CFG_CFLAGS ${PKG_CFG_COMPILE_DEFS})
    endif()

    get_target_property(PKG_CFG_COMPILE_OPTS ${PKG_CFG_TARGET} INTERFACE_COMPILE_OPTIONS)
    if(PKG_CFG_COMPILE_OPTS)
      list(APPEND PKG_CFG_CFLAGS ${PKG_CFG_COMPILE_OPTS})
    endif()

    get_target_property(PKG_CFG_LINK_OPTS ${PKG_CFG_TARGET} INTERFACE_LINK_OPTIONS)
    if(PKG_CFG_LINK_OPTS)
      list(APPEND PKG_CFG_LIBS ${INTERFACE_LINK_OPTIONS})
    endif()

    get_target_property(PKG_CFG_LINK_DIRS ${PKG_CFG_TARGET} INTERFACE_LINK_DIRECTORIES)
    if(PKG_CFG_LINK_DIRS)
      list(TRANSFORM PKG_CFG_LINK_DIRS PREPEND "-L")
      list(APPEND PKG_CFG_LIBS ${PKG_CFG_LINK_DIRS})
    endif()

    get_target_property(PKG_CFG_LINK_LIBS ${PKG_CFG_TARGET} INTERFACE_LINK_LIBRARIES)
    if(NOT PKG_CFG_LINK_LIBS)
      set(PKG_CFG_LINK_LIBS "")
    endif()

    get_target_property(PKG_CFG_IMPORTED_LOCATION ${PKG_CFG_TARGET} IMPORTED_LOCATION)
    if(PKG_CFG_IMPORTED_LOCATION)
      list(APPEND PKG_CFG_LINK_LIBS ${PKG_CFG_IMPORTED_LOCATION})
    endif()

    foreach(_lib ${PKG_CFG_LINK_LIBS})
      if(TARGET ${_lib})
        # if link library is actually a target and add as requirement
        string(FIND "${_lib}" "::" _sep)
        if("${_sep}" EQUAL "-1")
          list(APPEND PKG_CFG_REQUIRES ${_lib})
        else()
          math(EXPR _sep "${_sep} + 2")
          string(SUBSTRING "${_lib}" ${_sep} -1 _target_name)
          dune_pkg_config_name(${_target_name} _target_name)
          list(APPEND PKG_CFG_REQUIRES ${_target_name})
        endif()
      elseif(EXISTS ${_lib})
        # if link library is given as full path
        list(APPEND PKG_CFG_LIBS ${_lib})
      else()
        # if link library is given as lib-name
        list(APPEND PKG_CFG_LIBS "-l${_lib}")
      endif()
    endforeach()
  endif()

  if(NOT PKG_CFG_VERSION AND ${PKG_CFG_NAME}_VERSION)
    set(PKG_CFG_VERSION ${${PKG_CFG_NAME}_VERSION})
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
    ${PROJECT_BINARY_DIR}/${_name}.pc
    @ONLY)

  # install pkgconfig file
  install(FILES ${PROJECT_BINARY_DIR}/${_name}.pc
    DESTINATION ${DUNE_INSTALL_LIBDIR}/pkgconfig)

endfunction(dune_create_and_install_pkg_config)
