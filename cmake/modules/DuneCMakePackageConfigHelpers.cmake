# .. cmake_module::
#
#    .. note::
#       This module should be removed soon, as it provides fallbacks for pre 2.8.8
#
# A poor mans version of CMakePackageConfigHelpers.cmake for as a substitute
# for old Cmake versions.
#
# Provides the function
#  configure_package_config_file infile outfile INSTALL_DESTINATION relative_install_path
#  [PATH_VARS relative_path_varname1 ...])
#
# all paths are assumed to be relative!
#
find_file(CMakePkgConfigHelpersMacroFile CMakePackageConfigHelpers.cmake PATHS ${CMAKE_ROOT}/Modules ${CMAKE_MODULE_PATH})

if(CMakePkgConfigHelpersMacroFile)
  include(CMakePackageConfigHelpers)
else(CMakePkgConfigHelpersMacroFile)
  function(configure_package_config_file infile outfile)
    include(CMakeParseArguments)
    cmake_parse_arguments(PMPKG "" "INSTALL_DESTINATION" "PATH_VARS" "${ARGN}")
    if(NOT PMPKG_INSTALL_DESTINATION)
      message(FATAL_ERROR "configure_package_config_file needs an option INSTALL_DESTINATION with a relative path")
    endif(NOT PMPKG_INSTALL_DESTINATION)
    # asume that PMPKG_INSTALL_DESTINATION is relative
    string(REGEX REPLACE "[^/]+" ".." prefix_path ${PMPKG_INSTALL_DESTINATION})
    set(PACKAGE_INIT "# Set prefix to source dir
get_filename_component(PACKAGE_PREFIX_DIR \"\${CMAKE_CURRENT_LIST_DIR}/${prefix_path}\" ABSOLUTE)
macro(set_and_check _var _file)
  set(\${_var} \"\${_file}\")
  if(NOT EXISTS \"\${_file}\")
    message(FATAL_ERROR \"File or directory \${_file} referenced by variable \${_var} does not exist !\")
  endif()
endmacro()")
    if(PMPKG_PATH_VARS)
    foreach(varname ${PMPKG_PATH_VARS})
      set(PACKAGE_${varname} "\${PACKAGE_PREFIX_DIR}/${${varname}}")
    endforeach(varname "${PMPKG_PATH_VARS}")
    endif(PMPKG_PATH_VARS)
    configure_file(${infile} ${outfile} @ONLY)
  endfunction(configure_package_config_file infile outfile)
endif(CMakePkgConfigHelpersMacroFile)
