# Searches for MPI and thread support and sets the following
# DUNE specific flags if USE_MPI is set to true:
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
if (NOT USE_MPI)
  set (CMAKE_DISABLE_FIND_PACKAGE_MPI TRUE)
endif (NOT USE_MPI)
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
function(add_dune_mpi_flags)
  cmake_parse_arguments(ADD_MPI "SOURCE_ONLY;OBJECT" "" "" ${ARGN})
  if(ADD_MPI_SOURCE_ONLY)
    set(_prefix SOURCE)
  else()
    set(_prefix TARGET)
  endif()
  if(MPI_FOUND)
    set_property(${_prefix} ${ADD_MPI_UNPARSED_ARGUMENTS} APPEND PROPERTY COMPILE_FLAGS ${MPI_DUNE_COMPILE_FLAGS})
    set_property(${_prefix} ${ADD_MPI_UNPARSED_ARGUMENTS} APPEND PROPERTY COMPILE_DEFINITIONS ENABLE_MPI=1
      MPICH_SKIP_MPICXX MPIPP_H)
    if(NOT (ADD_MPI_SOURCE_ONLY OR ADD_MPI_OBJECT))
    set_property(${_prefix} ${ADD_MPI_UNPARSED_ARGUMENTS} APPEND_STRING PROPERTY LINK_FLAGS ${MPI_DUNE_LINK_FLAGS})
    foreach(target ${ADD_MPI_UNPARSED_ARGUMENTS})
      target_link_libraries(${target} ${MPI_DUNE_LIBRARIES})
    endforeach(target ${ADD_MPI_UNPARSED_ARGUMENTS})
    endif(NOT (ADD_MPI_SOURCE_ONLY OR ADD_MPI_OBJECT))
  endif(MPI_FOUND)
endfunction(add_dune_mpi_flags)
