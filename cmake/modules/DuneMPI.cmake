# Searches for MPI and thread support and sets the following
# DUNE specific flags:
#
# MPI_DUNE_COMPILE_FLAGS Compiler flags for MPI applications.
# MPI_DUNE_INCLUDE_PATH Include path for MPI applications.
# MPI_DUNE_LINK_FLAGS Linker flags for MPI applications.
# MPI_DUNE_LIBRARIES Libraries for MPI applications.
#
# The DUNE way to compile MPI applications is to use the CXX
# compiler with MPI flags usually used for C. CXX bindings
# are deactivated to prevent ABI problems.
#
# The following function is defined:
#
# add_dune_mpi_flags(targets)
#
# Adds the above flags and libraries to the specified targets.
#

find_package(MPI)
find_package(Threads)

if(MPI_FOUND)
  set(HAVE_MPI MPI_CXX_FOUND)
  # We do not support the CXX bindings of MPI
  set(MPI_DUNE_COMPILE_FLAGS ${MPI_C_COMPILE_FLAGS} CACHE STRING
    "Compile flags used by DUNE when compiling MPI programs")
  set(MPI_DUNE_INCLUDE_PATH ${MPI_C_INCLUDE_PATH} CACHE STRING
    "Include path used by DUNE when compiling MPI programs")
  # There seems to be no target specific include path, use the global one.
  include_directories(${MPI_DUNE_INCLUDE_PATH})
  set(MPI_DUNE_LINK_FLAGS ${MPI_C_LINK_FLAGS} CACHE STRING
    "Linker flags used by DUNE when compiling MPI programs")
  set(MPI_DUNE_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} ${MPI_C_LIBRARIES} CACHE STRING
    "Libraries used by DUNE when linking MPI programs")

  # Check whether the MPI-2 standard is supported
  include(CMakePushCheckState)
  include(CheckFunctionExists)
  cmake_push_check_state()
  set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES};${MPI_DUNE_LIBRARIES})
  set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} "-DENABLE_MPI=1 -DMPICH_SKIP_MPICXX -DMPIPP_H")
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES};${MPI_DUNE_INCLUDE_PATH})
  check_function_exists(MPI_Finalized MPI_2)
  cmake_pop_check_state()
endif(MPI_FOUND)

# adds MPI flags to the targets
function(add_dune_mpi_flags _targets)
  if(MPI_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} ${MPI_DUNE_LIBRARIES})
      # The definitions are a hack as we do not seem to know which MPI implementation was
      # found.
      GET_TARGET_PROPERTY(_props ${_target} COMPILE_FLAGS)
      string(REPLACE "_props-NOTFOUND" "" _props "${_props}")
      SET_TARGET_PROPERTIES(${_target} PROPERTIES COMPILE_FLAGS
        "${_props} ${MPI_DUNE_COMPILE_FLAGS} -DENABLE_MPI=1 -DMPICH_SKIP_MPICXX -DMPIPP_H")
      GET_TARGET_PROPERTY(_props ${_target} LINK_FLAGS)
      string(REPLACE "_props-NOTFOUND" "" _props "${_props}")
      SET_TARGET_PROPERTIES(${_target} PROPERTIES LINK_FLAGS "${_props} ${MPI_DUNE_LINK_FLAGS}")
    endforeach(_target ${_targets})
  endif(MPI_FOUND)
endfunction(add_dune_mpi_flags)
