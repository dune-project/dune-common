# Defines the functions to use MPI
#
# .. cmake_function:: add_dune_mpi_flags
#
#    .. cmake_param:: targets
#       :positional:
#       :single:
#       :required:
#
#       A list of targets to use MPI with.
#

# register HAVE_MPI for config.h
set(HAVE_MPI ${MPI_FOUND})

# Add function to link targets against MPI library
function(add_dune_mpi_flags _targets)
  if(MPI_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} PUBLIC MPI::MPI_CXX)
      target_compile_definitions(${_target} PUBLIC ENABLE_MPI=1)
    endforeach(_target)
  endif()
endfunction(add_dune_mpi_flags _targets)
