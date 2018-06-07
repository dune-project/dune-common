# Searches for MPI and thread support and sets the following
# DUNE specific flags:
#
# MPI_DUNE_COMPILE_FLAGS Compiler flags for MPI applications.
# MPI_DUNE_INCLUDE_PATH Include path for MPI applications.
# MPI_DUNE_LINK_FLAGS Linker flags for MPI applications.
# MPI_DUNE_LIBRARIES Libraries for MPI applications.
#
# DISABLE_ULFM_REVOKE Disables the ULFM revoke functionalities.
# USE_BLACKCHANNEL Uses the Blackchannel-ULFM library (if found) instead of the MPI build-in
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
  dune_register_package_flags(COMPILE_DEFINITIONS "ENABLE_MPI=1;MPICH_SKIP_MPICXX;MPIPP_H;MPI_NO_CPPBIND"
                              INCLUDE_DIRS "${MPI_DUNE_INCLUDE_PATH}"
                              LIBRARIES "${MPI_DUNE_LIBRARIES}")

  set(DISABLE_ULFM_REVOKE 0 CACHE BOOL
    "Disable ULFM revoke functionalities")
  if(NOT DISABLE_ULFM_REVOKE)
    list(APPEND CMAKE_REQUIRED_LIBRARIES ${MPI_DUNE_LIBRARIES})
    list(APPEND CMAKE_REQUIRED_FLAGS ${MPI_DUNE_LINK_FLAGS})
    list(APPEND MPI_HEADER "${MPI_DUNE_INCLUDE_PATH}/mpi.h")
    if(EXISTS "${MPI_DUNE_INCLUDE_PATH}/mpi-ext.h") # OpenMPI
      list(APPEND MPI_HEADER "${MPI_DUNE_INCLUDE_PATH}/mpi-ext.h")
    endif()
    # check for MPIX_Comm_revoke
    check_symbol_exists("MPIX_Comm_revoke"
      "${MPI_HEADER}"
      HAVE_ULFM_REVOKE)

    # check for Blackchannel-ULFM, if MPI does not support MPIX_Comm_revoke
    if(NOT HAVE_ULFM_REVOKE OR USE_BLACKCHANNEL)
      find_package(BlackChannel)
      if(BLACKCHANNEL_FOUND)
        set(HAVE_ULFM_REVOKE 1)
      endif()
    endif()
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
    set_property(${_prefix} ${ADD_MPI_UNPARSED_ARGUMENTS} APPEND PROPERTY COMPILE_OPTIONS ${MPI_DUNE_COMPILE_FLAGS})
    set_property(${_prefix} ${ADD_MPI_UNPARSED_ARGUMENTS} APPEND PROPERTY COMPILE_DEFINITIONS ENABLE_MPI=1
      MPICH_SKIP_MPICXX MPIPP_H)
    if(NOT (ADD_MPI_SOURCE_ONLY OR ADD_MPI_OBJECT))
      set_property(${_prefix} ${ADD_MPI_UNPARSED_ARGUMENTS} APPEND_STRING PROPERTY LINK_FLAGS " ${MPI_DUNE_LINK_FLAGS} ")
      foreach(target ${ADD_MPI_UNPARSED_ARGUMENTS})
        target_link_libraries(${target} ${MPI_DUNE_LIBRARIES})
        if(BLACKCHANNEL_FOUND)
          target_link_libraries(${target} ${BLACKCHANNEL_LIBRARIES})
        endif()
      endforeach(target ${ADD_MPI_UNPARSED_ARGUMENTS})
    endif(NOT (ADD_MPI_SOURCE_ONLY OR ADD_MPI_OBJECT))
  endif(MPI_C_FOUND)
endfunction(add_dune_mpi_flags)
