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


find_package(MPI 3.0 COMPONENTS C)

# text for feature summary
set_package_properties("MPI" PROPERTIES
  DESCRIPTION "Message Passing Interface library"
  PURPOSE "Parallel programming on multiple processors")

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

  # TODO check on where to position this exactly, doesn't look completely thought through
  dune_register_package_flags(COMPILE_DEFINITIONS "ENABLE_MPI=1;MPICH_SKIP_MPICXX;OMPI_SKIP_MPICXX;MPIPP_H;MPI_NO_CPPBIND"
                              INCLUDE_DIRS "${MPI_DUNE_INCLUDE_PATH}"
                              LIBRARIES "${MPI_DUNE_LIBRARIES}")
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
    separate_arguments(MPI_DUNE_COMPILE_FLAGS_LIST UNIX_COMMAND ${MPI_DUNE_COMPILE_FLAGS})
    set_property(${_prefix} ${ADD_MPI_UNPARSED_ARGUMENTS} APPEND PROPERTY COMPILE_OPTIONS ${MPI_DUNE_COMPILE_FLAGS_LIST})
    set_property(${_prefix} ${ADD_MPI_UNPARSED_ARGUMENTS} APPEND PROPERTY COMPILE_DEFINITIONS ENABLE_MPI=1
      MPICH_SKIP_MPICXX OMPI_SKIP_MPICXX MPIPP_H)
    if(NOT (ADD_MPI_SOURCE_ONLY OR ADD_MPI_OBJECT))
    set_property(${_prefix} ${ADD_MPI_UNPARSED_ARGUMENTS} APPEND_STRING PROPERTY LINK_FLAGS " ${MPI_DUNE_LINK_FLAGS} ")
    foreach(target ${ADD_MPI_UNPARSED_ARGUMENTS})
      target_link_libraries(${target} PUBLIC ${MPI_DUNE_LIBRARIES})
    endforeach(target ${ADD_MPI_UNPARSED_ARGUMENTS})
    endif(NOT (ADD_MPI_SOURCE_ONLY OR ADD_MPI_OBJECT))
  endif(MPI_C_FOUND)
endfunction(add_dune_mpi_flags)
