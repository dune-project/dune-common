# .. cmake_module::
#
#    Module to checks for Valgrind.
#    Its possible to check for the executable or the headers.
#
#    Example which tries to find the Valgrind executable:
#
#    :code:`find_package(Valgrind OPTIONAL_COMPONENTS Executable)`
#
#    `OPTIONAL_COMPONENTS`
#       A list of components. Components are:
#       Executable Headers
#
#    :ref:`Valgrind_ROOT`
#       Path list to search for Valgrind
#
#    Sets the following variables:
#
#    :code:`Valgrind_<COMPONENT>_FOUND`
#       Whether <COMPONENT> was found as part of Valgrind.
#
#    :code:`Valgrind_FOUND`
#       True if Valgrind was found.
#
#    :code:`Valgrind_INCLUDE_DIRS`
#       Path to the Valgrind include dirs, if `Valgrind_Headers_FOUND`
#       is true.
#
#    :code:`Valgrind_Executable`
#       Path to the Valgrind executable, if `Valgrind_Executable_FOUND`
#       is true.
#
#
# .. cmake_variable:: Valgrind_ROOT
#
#   Set this variable to have :ref:`FindValgrind` look for Valgrind
#   in the given path before inspecting the system paths.

set_package_info("Valgrind" "Memory debugger, memory leak detector, and profiler" "http://valgrind.org")

set(_Valgrind_REQUIRED_VARS "")
foreach(_component ${Valgrind_FIND_COMPONENTS})
  # look for headers
  if(_component STREQUAL "Headers")
    list(APPEND _Valgrind_REQUIRED_VARS Valgrind_INCLUDE_DIR)
    # look for header files, only at positions given by the user
    find_path(Valgrind_INCLUDE_DIR
      NAMES "valgrind/memcheck.h"
      PATHS ${VALGRIND_ROOT}
      PATH_SUFFIXES "include"
      NO_DEFAULT_PATH)
    # look for header files, including default paths
    find_path(Valgrind_INCLUDE_DIR
      NAMES "valgrind/memcheck.h")
    # report success for this component
    if(Valgrind_INCLUDE_DIR)
      set(Valgrind_Headers_FOUND true)
    endif()
  # look for executable
  elseif(_component STREQUAL "Executable")
    list(APPEND _Valgrind_REQUIRED_VARS Valgrind_EXECUTABLE)
    # look for executable, only at positions given by the user
    find_program(Valgrind_EXECUTABLE
      NAMES valgrind
      PATHS ${VALGRIND_ROOT}
      PATH_SUFFIXES "bin"
      NO_DEFAULT_PATH)
    # look for executable, including default paths
    find_program(Valgrind_EXECUTABLE
      NAMES valgrind)
    # report success for this component
    if(Valgrind_EXECUTABLE)
      set(Valgrind_Executable_FOUND true)
    endif()
  endif()
endforeach()

# handle package arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  "Valgrind"
  FOUND_VAR Valgrind_FOUND
  REQUIRED_VARS ${_Valgrind_REQUIRED_VARS}
  HANDLE_COMPONENTS)

mark_as_advanced(
  _Valgrind_REQUIRED_VARS
  Valgrind_INCLUDE_DIR)

# set HAVE_VALGRIND for config.h
set(HAVE_VALGRIND ${Valgrind_Headers_FOUND})

# register include dirs for Valgrind
if(Valgrind_Headers_FOUND)
  set(VALGRIND_INCLUDE_DIR "${VALGRIND_INCLUDE_DIRS}")
  dune_register_package_flags(
    COMPILE_DEFINITIONS "ENABLE_VALGRIND=1"
    INCLUDE_DIRS "${VALGRIND_INCLUDE_DIRS}")
endif()
