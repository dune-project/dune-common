# Defines the functions to use GMP
#
# .. cmake_function:: add_dune_gmp_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use GMP with.
#

# set HAVE_GMP for the config.h file
set(HAVE_GMP ${GMP_FOUND})

# register all GMP related flags
if(GMP_FOUND)
  dune_register_package_flags(
    LIBRARIES GMP::gmpxx
    COMPILE_DEFINITIONS "ENABLE_GMP=1"
  )
  dune_create_and_install_pkg_config("gmp"
    NAME "GMP"
    VERSION "${GMP_VERSION}"
    DESCRIPTION "GNU multi-precision library"
    URL "https://gmplib.org"
    TARGET GMP::gmp)
  dune_create_and_install_pkg_config("gmpxx"
    NAME "GMP"
    VERSION "${GMP_VERSION}"
    DESCRIPTION "GNU multi-precision library"
    URL "https://gmplib.org"
    TARGET GMP::gmpxx)
  dune_add_pkg_config_requirement("gmpxx")
  dune_add_pkg_config_flags("-DHAVE_GMP")
endif()

# add function to link against the GMP library
function(add_dune_gmp_flags _targets)
  if(GMP_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC GMP::gmpxx)
      target_compile_definitions(${_target} PUBLIC ENABLE_GMP=1)
    endforeach(_target ${_targets})
  endif(GMP_FOUND)
endfunction(add_dune_gmp_flags)
