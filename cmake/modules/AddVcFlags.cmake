# Defines the functions to use Vc
#
# Vc is a library for high-level Vectorization support in C++
# see https://github.com/VcDevel/Vc
#
# .. cmake_function:: add_dune_vc_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use VC with.
#

function(add_dune_vc_flags _targets)
  if(Vc_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC ${Vc_LIBRARIES})
      target_compile_options(${_target} PUBLIC ${Vc_COMPILE_FLAGS})
      target_compile_definitions(${_target} PUBLIC ENABLE_VC=1)
      target_include_directories(${_target} SYSTEM PUBLIC ${Vc_INCLUDE_DIR})
    endforeach(_target ${_targets})
  endif(Vc_FOUND)
endfunction(add_dune_vc_flags)

if(Vc_FOUND)
  dune_register_package_flags(COMPILE_OPTIONS "${Vc_COMPILE_FLAGS};-DENABLE_VC=1"
                              LIBRARIES "${Vc_LIBRARIES}"
                              INCLUDE_DIRS "${Vc_INCLUDE_DIR}")
  string(JOIN " " Vc_FLAGS ${Vc_COMPILE_FLAGS})
  set(Vc_FLAGS "${Vc_FLAGS} -I${Vc_INCLUDE_DIR}")

  dune_create_and_install_pkg_config("Vc"
    VERSION "${Vc_VERSION}"
    DESCRIPTION "C++ Vectorization library"
    URL "https://github.com/VcDevel/Vc"
    CFLAGS "${Vc_COMPILE_FLAGS};-I${Vc_INCLUDE_DIR};-DHAVE_VC"
    LIBS "${Vc_LIBRARIES}")
  dune_add_pkg_config_requirement("Vc")
endif(Vc_FOUND)
set(HAVE_VC ${Vc_FOUND})
