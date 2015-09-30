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
# .. cmake_function:: add_dune_mpi_flags
#
#    .. cmake_param:: targets
#       :single:
#       :required:
#       :positional:
#
#       The target list to add the MPI flags to.
#


find_package(MPI)
find_package(Threads)

if(MPI_C_FOUND)
  set(HAVE_MPI ${MPI_C_FOUND})
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

  # TODO check on where to position this exactly, doesnt look completely thought through
  dune_register_package_flags(COMPILE_DEFINITIONS "ENABLE_MPI=1;MPICH_SKIP_MPICXX;MPIPP_H"
                              INCLUDE_DIRS "${MPI_DUNE_INCLUDE_PATH}"
                              LIBRARIES "${MPI_DUNE_LIBRARIES}")

  # Check whether the MPI-2 standard is supported
  include(CMakePushCheckState)
  include(CheckFunctionExists)
  include(CheckCXXSourceCompiles)
  cmake_push_check_state()
  set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES};${MPI_DUNE_LIBRARIES})
  set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} "-DENABLE_MPI=1 -DMPICH_SKIP_MPICXX -DMPIPP_H")
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES};${MPI_DUNE_INCLUDE_PATH})
  check_function_exists(MPI_Finalized MPI_2)

  # proper version check
  check_cxx_source_compiles("
    #include <mpi.h>

    #if !((MPI_VERSION > 2) || (MPI_VERSION == 2 && MPI_SUBVERSION >= 1))
    fail with a horribe compilation error due to old MPI version
    #endif

    int main(int argc, char** argv)
    {
      MPI_Init(&argc,&argv);
      MPI_Finalize();
    }
" MPI_VERSION_SUPPORTED)

  cmake_pop_check_state()

  if(NOT MPI_VERSION_SUPPORTED)
    message(FATAL_ERROR "Your MPI implementation is too old. Please upgrade to an MPI-2.1 compliant version.")
  endif()
endif(MPI_C_FOUND)

# adds MPI flags to the targets
function(add_dune_mpi_flags)
  cmake_parse_arguments(ADD_MPI "SOURCE_ONLY;OBJECT" "" "" ${ARGN})
  if(ADD_MPI_SOURCE_ONLY)
    set(_prefix SOURCE)
  else()
    set(_prefix TARGET)
  endif()
  if(MPI_C_FOUND)
    set_property(${_prefix} ${ADD_MPI_UNPARSED_ARGUMENTS} APPEND PROPERTY COMPILE_FLAGS ${MPI_DUNE_COMPILE_FLAGS})
    set_property(${_prefix} ${ADD_MPI_UNPARSED_ARGUMENTS} APPEND PROPERTY COMPILE_DEFINITIONS ENABLE_MPI=1
      MPICH_SKIP_MPICXX MPIPP_H)
    if(NOT (ADD_MPI_SOURCE_ONLY OR ADD_MPI_OBJECT))
    set_property(${_prefix} ${ADD_MPI_UNPARSED_ARGUMENTS} APPEND_STRING PROPERTY LINK_FLAGS ${MPI_DUNE_LINK_FLAGS})
    foreach(target ${ADD_MPI_UNPARSED_ARGUMENTS})
      target_link_libraries(${target} ${MPI_DUNE_LIBRARIES})
    endforeach(target ${ADD_MPI_UNPARSED_ARGUMENTS})
    endif(NOT (ADD_MPI_SOURCE_ONLY OR ADD_MPI_OBJECT))
  endif(MPI_C_FOUND)
endfunction(add_dune_mpi_flags)
