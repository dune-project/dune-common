# Defines the functions to use spdlog
#
# .. cmake_function:: add_dune_spdlog_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use spdlog with.
#

set(HAVE_SPDLOG ${spdlog_FOUND})

if(spdlog_FOUND)
  dune_register_package_flags(
    LIBRARIES spdlog::spdlog
    COMPILE_DEFINITIONS "ENABLE_SPDLOG=1"
  )
endif()

# add function to link against the spdlog library
function(add_dune_spdlog_flags _targets)
  if(spdlog_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} spdlog::spdlog)
      target_compile_definitions(${_target} PUBLIC ENABLE_SPDLOG=1)
    endforeach(_target ${_targets})
  endif(spdlog_FOUND)
endfunction(add_dune_spdlog_flags)
