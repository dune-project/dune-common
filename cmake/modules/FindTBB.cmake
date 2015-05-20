# Locate Intel Threading Building Blocks include paths and libraries
#
# TBB is a little special because there are three different ways to provide it:
# - It can be installed using a package manager and just be available on the system include
#   and library paths.
# - It can be compiled from source. The package doesn't really provide an installation script
#   for this, but expects you to source an environment file called tbbvars.sh that updates the
#   required variables like CPATH etc.
# - TBB is shipped as part of the Intel compilers. This bundled version can be enabled by adding
#   -tbb to the compiler flags.
#
# This module can find all three types of installations. They are looked for in the following
# order of preference: tbbvars.sh file (for a custom installation), system paths and finally
# the built-in version if an Intel compiler is present.
#
# Note: If you provide a tbbvars.sh script (via the CMake variable TBB_VARS_SH), this module will
#       not find any libraries installed in the system path! This is on purpose to avoid
#       accidental fallbacks.
#
# If the option TBB_DEBUG is set to ON, the module will look for the debug version of TBB. Note that
# this does not work for the built-in library of the Intel Compiler due to linking problems. You can
# however provide the module with the tbbvars.sh from that built-in installation (usually in the
# subdirectory tbb/ of the Intel compiler root path), which will fix that problem.
#
# Variables used by this module which you may want to set:
#
# TBB_VARS_SH       Path to the tbbvars.sh script
# TBB_INCLUDE_DIR   Path to the include directory with the TBB headers
# TBB_LIBRARY_DIR   Path to the library directory with the TBB libraries
# TBB_DEBUG         Option that turns on TBB debugging
#
#
# This module supports additional components of TBB that can be listed in the find_package() call:

# cpf               Use comunity preview edition (links to libtbb_preview instead of libtbb). cpf
#                   is not available for the built-in version of the Intel Compiler, but see the note
#                   on debug mode above for a fix.
# allocator         Use TBB's scalable allocator (links to libtbbmalloc).
#
#
# This module sets the following variables:
#
# TBB_FOUND                       True if TBB was found and is usable
# TBB_cpf_FOUND                   True if community preview edition was found and is usable
# TBB_allocator_FOUND             True if scalable allocator library was found and is usable
# TBB_INCLUDE_DIRS                Path to the TBB include dirs. This variable is empty if the
#                                 internal TBB version of an Intel compiler is in use
# TBB_LIBRARIES                   List of the TBB libraries that a target must be linked to
# TBB_COMPILE_DEFINITIONS         Required compile definitions to use TBB
# TBB_COMPILE_OPTIONS             Required compile options to use TBB
# TBB_INTEL_COMPILER_INTERNAL_TBB True if internal TBB version of Intel compiler is in use
#
#
# In addition, TBB is automatically registered with the dune_enable_all_packages() facility. If you
# don't want to use that feature, the module also provides the following function:
#
# add_dune_tbb_flags(target [target]...)  Adds all flags required to use TBB to the listed targets
#

option(
  TBB_DEBUG
  "Turn on TBB debugging (modifies compiler flags and links against debug version of libraries)"
  )

# source for our little test program. We have to compile this multiple times, so
# store it in a variable for DRY and better readability
set(tbb_compile_source "
#include <tbb/tbb.h>
#include <numeric>

int main()
{
  int x[10] = {0};
  tbb::parallel_for(0,10,[&](int i){ x[i] = i; });
  return !(std::accumulate(x,x+10,0) == (9*10)/2);
}
")


# Function to parse a tbbvars.sh file and extract include and library paths.
# This function relies on the bash shell to source the tbbvars.sh file
function(parse_tbb_vars_sh)
  message(STATUS "Taking TBB location from ${TBB_VARS_SH}")
  find_package(UnixCommands)
  set(tbb_vars_works FALSE)
  execute_process(
    COMMAND ${BASH} -c ". ${TBB_VARS_SH} > /dev/null"
    RESULT_VARIABLE shell_result
    OUTPUT_VARIABLE shell_out
    )
  if (${shell_result} EQUAL 0)
    set(tbb_vars_opt "")
    set(tbb_vars_works TRUE)
  else()
    # try script from binary Linux installs that requires an 'intel64' argument
    execute_process(
      COMMAND ${BASH} -c ". ${TBB_VARS_SH} intel64 >/dev/null"
      RESULT_VARIABLE shell_result
      OUTPUT_VARIABLE shell_out
      )
    if (${shell_result} EQUAL 0)
      set(tbb_vars_opt "intel64")
      set(tbb_vars_works TRUE)
    endif()
  endif()
  if(tbb_vars_works)
    execute_process(
      COMMAND ${BASH} -c "unset CPATH ; . ${TBB_VARS_SH} ${tbb_vars_opt} >/dev/null && echo -n $CPATH"
      RESULT_VARIABLE shell_result
      OUTPUT_VARIABLE shell_out
      )
    find_path(
      TBB_INCLUDE_DIR
      NAMES tbb/task_scheduler_init.h
      PATHS ${shell_out}
      DOC "Path to TBB include directory"
      NO_DEFAULT_PATH
      )
    execute_process(
      COMMAND ${BASH} -c "unset LIBRARY_PATH ; . ${TBB_VARS_SH} ${tbb_vars_opt} >/dev/null && echo -n $LIBRARY_PATH"
      RESULT_VARIABLE shell_result
      OUTPUT_VARIABLE shell_out
      )
    set(
      TBB_LIBRARY_DIR ${shell_out}
      CACHE PATH "Path to TBB library directory"
      )
  else()
    message(WARNING "Could not parse tbbvars.sh file at {TBB_VARS_SH}")
  endif()
endfunction()


# Check whether the user gave us an existing tbbvars.sh file
find_file(
  TBB_VARS_SH
  tbbvars.sh
  DOC "Path to tbbvars.sh script"
  NO_DEFAULT_PATH
  )


if (TBB_VARS_SH)
  parse_tbb_vars_sh()
else()
  # Try to find TBB in standard include paths
  find_path(
    TBB_INCLUDE_DIR
    tbb/task_scheduler_init.h
    PATHS ENV CPATH
    DOC "Path to TBB include directory"
    )
  # Try to find some version of the TBB library in standard library paths
  find_path(
    TBB_LIBRARY_DIR
    "${CMAKE_SHARED_LIBRARY_PREFIX}tbb_preview${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "${CMAKE_SHARED_LIBRARY_PREFIX}tbb${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "${CMAKE_SHARED_LIBRARY_PREFIX}tbb_preview_debug${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "${CMAKE_SHARED_LIBRARY_PREFIX}tbb_debug${CMAKE_SHARED_LIBRARY_SUFFIX}"
    PATHS ENV LIBRARY_PATH
    DOC "Path to TBB library directory"
    )
  message(STATUS "Library dir: ${TBB_LIBRARY_DIR}")
endif()


# helper function to invoke find_library() correctly
# If we are using tbbvars.sh, we exclude system-default library search paths,
# otherwise we leave them in
function(find_tbb_library)
  include(CMakeParseArguments)
  set(OPTIONS)
  set(SINGLEARGS VAR NAME DOC)
  set(MULTIARGS)
  cmake_parse_arguments(LIB "${OPTIONS}" "${SINGLEARGS}" "${MULTIARGS}" ${ARGN})

  if(TBB_VARS_SH)
    find_library(
      ${LIB_VAR}
      ${LIB_NAME}
      PATHS ${TBB_LIBRARY_DIR}
      DOC "${LIB_DOC}"
      NO_DEFAULT_PATH
      )
  else()
    find_library(
      ${LIB_VAR}
      ${LIB_NAME}
      PATHS ${TBB_LIBRARY_DIR}
      DOC "${LIB_DOC}"
      )
  endif()
endfunction()


# we always want to use TBB in C++11 mode
set(TBB_COMPILE_DEFINITIONS _TBB_CPP0X)


if(TBB_DEBUG)
  message(STATUS "Linking against debug version of TBB")
  set(tbb_debug_suffix "_debug")
  # TBB requires this additional compile definition when used in debug mode
  list(APPEND TBB_COMPILE_DEFINITIONS TBB_USE_DEBUG)
else()
  set(tbb_debug_suffix "")
endif()

# start looking for components
# We first look for component libraries, because the "special component" cpf
# actually replaces the standard libtbb

set(TBB_cpf_FOUND FALSE)
set(TBB_allocator_FOUND FALSE)

foreach(component ${TBB_FIND_COMPONENTS})

  if(component STREQUAL "cpf")
    find_tbb_library(
      VAR TBB_LIBTBB_PREVIEW
      NAME "tbb_preview${tbb_debug_suffix}"
      DOC "Path to TBB community preview library"
      )
    if(TBB_LIBTBB_PREVIEW)
      list(APPEND TBB_LIBRARIES ${TBB_LIBTBB_PREVIEW})
      list(APPEND TBB_COMPILE_DEFINITIONS TBB_PREVIEW_LOCAL_OBSERVER=1)
      set(TBB_cpf_FOUND TRUE)
    endif()

  elseif(component STREQUAL "allocator")
    find_tbb_library(
      VAR TBB_LIBTBBMALLOC
      NAME "tbbmalloc${tbb_debug_suffix}"
      DOC "Path to TBB malloc library"
      )
    if(TBB_LIBTBBMALLOC)
      list(APPEND TBB_LIBRARIES ${TBB_LIBTBBMALLOC})
      set(TBB_allocator_FOUND TRUE)
    endif()

  else()
    message(FATAL_ERROR "Unknown TBB component: ${component}")
  endif()
endforeach()


# If we could not find libtbb_preview, look for plain libtbb instead
if(NOT TBB_cpf_FOUND)
  find_tbb_library(
    VAR TBB_LIBTBB
    NAME "tbb${tbb_debug_suffix}"
    DOC "Path to TBB library"
    )
  if(TBB_LIBTBB)
    list(APPEND TBB_LIBRARIES ${TBB_LIBTBB})
  endif()
else()
  # This avoids special-casing later on
  set(
    TBB_LIBTBB ${TBB_LIBTBB_PREVIEW}
    CACHE FILEPATH "Path to TBB library"
    )
endif()

# Don't show these to the user, they are just confusing
mark_as_advanced(
  TBB_LIBTBB_PREVIEW
  TBB_LIBTBB
  TBB_LIBTBBMALLOC
  )

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

# make sure this variable always exists; it is only used if we pick the internal
# TBB implementation from an Intel Compiler
set(TBB_COMPILE_OPTIONS "")
set(TBB_INTEL_COMPILER_INTERNAL_TBB OFF)

# We didn't manage to find TBB yet, so try if we can fall back to the one shipped
# as part of Intel's compiler
if ((NOT TBB_LIBTBB) AND("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))

  # This while doesn't work in debug mode because -tbb always injects -ltbb into the linker flags, and that clashes
  # with -ltbb_debug
  if(NOT TBB_DEBUG)
    message(STATUS "Could not find TBB in normal places, trying to fall back to internal version of Intel Compiler")

    # We'll just compile a program with -tbb and see whether that works
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_FLAGS -tbb)
    foreach(_definition ${TBB_COMPILE_DEFINITIONS})
      list(APPEND CMAKE_REQUIRED_DEFINITIONS "-D${_definition}")
    endforeach()
    set(CMAKE_REQUIRED_QUIET ON)
    check_cxx_source_compiles(
      "${tbb_compile_source}"
      TBB_INTEL_COMPILER_INTERNAL_TBB
      )

    if(TBB_INTEL_COMPILER_INTERNAL_TBB)
      # yeah, success
      set(TBB_COMPILE_OPTIONS -tbb)

      # now check components
      foreach(component ${TBB_FIND_COMPONENTS})

        # again, this doesn't work because of the default -ltbb injected by -tbb
        if(component STREQUAL "cpf")
          message(STATUS "Cannot link to community preview version when using compiler-internal version of TBB. Please specify the tbbvars.sh script in TBB_VARS_SH")

        elseif(component STREQUAL "allocator")
          # we'll check for this by trying to link against the library
          set(CMAKE_REQUIRED_LIBRARIES tbbmalloc)
          check_cxx_source_compiles(
            "${tbb_compile_source}"
            TBB_INTEL_COMPILER_INTERNAL_TBBMALLOC
            )
          if(TBB_INTEL_COMPILER_INTERNAL_TBBMALLOC)
            list(APPEND TBB_LIBRARIES tbbmalloc)
            set(TBB_allocator_FOUND TRUE)
          endif()
        else()
          message(FATAL_ERROR "Unknown TBB component: ${component}")
        endif()
      endforeach()

      # clean up check state
      cmake_pop_check_state()
    endif()

  else()
    message(STATUS "Could not find TBB in normal places, and don't know how to fall back to internal version of Intel Compiler for debug version.
You can either turn of the TBB_DEBUG option or point the TBB_VARS_SH variable at the tbbvars.sh script shipped with your compiler.")
  endif()
endif()

# make sure everything works
cmake_push_check_state(RESET)
set(CMAKE_REQUIRED_INCLUDES ${TBB_INCLUDE_DIR})
foreach(_definition ${TBB_COMPILE_DEFINITIONS})
  list(APPEND CMAKE_REQUIRED_DEFINITIONS "-D${_definition}")
endforeach()
set(CMAKE_REQUIRED_FLAGS ${TBB_COMPILE_OPTIONS})
set(CMAKE_REQUIRED_LIBRARIES ${TBB_LIBRARIES})
check_cxx_source_compiles(
  "${tbb_compile_source}"
  TBB_COMPILE_TEST
  )
cmake_pop_check_state()

# we don't want to leak any helper variables
unset(tbb_compile_source)

# provide standard find_package() interface
include(FindPackageHandleStandardArgs)

if(TBB_INTEL_COMPILER_INTERNAL_TBB)
  set(TBB_INCLUDE_DIRS "")
  set(TBB_LIBRARIES "")

  find_package_handle_standard_args(
    TBB
    REQUIRED_VARS TBB_COMPILE_OPTIONS TBB_COMPILE_TEST
    HANDLE_COMPONENTS
    )
else()

  set(TBB_INCLUDE_DIRS ${TBB_INCLUDE_DIR})

  find_package_handle_standard_args(
    TBB
    REQUIRED_VARS TBB_INCLUDE_DIRS TBB_LIBRARIES TBB_COMPILE_TEST
    HANDLE_COMPONENTS
    )

endif()

# set variable for config.h
set(HAVE_TBB ${TBB_FOUND})

# perform DUNE-specific setup tasks
if (TBB_FOUND)
  set(TBB_CACHE_ALIGNED_ALLOCATOR_ALIGNMENT 128)
  message(STATUS "defaulting TBB_CACHE_ALIGNED_ALLOCATOR_ALIGNMENT to 128")
  dune_register_package_flags(
    COMPILE_DEFINITIONS ENABLE_TBB=1 ${TBB_COMPILE_DEFINITIONS}
    COMPILE_OPTIONS ${TBB_COMPILE_OPTIONS}
    INCLUDE_DIRS ${TBB_INCLUDE_DIRS}
    LIBRARIES ${TBB_LIBRARIES}
    )
endif()


# function for adding TBB flags to a list of targets
function(add_dune_tbb_flags _targets)
  foreach(_target ${_targets})
    target_compile_definitions(${_target} PUBLIC ENABLE_TBB=1)
    if(TBB_COMPILE_DEFINITIONS)
      target_compile_definitions(${_target} PUBLIC ${TBB_COMPILE_DEFINITIONS})
    endif()
    if(TBB_COMPILE_OPTIONS)
      target_compile_options(${_target} PUBLIC ${TBB_COMPILE_OPTIONS})
    endif()
    if(TBB_INCLUDE_DIRS)
      target_include_directories(${_target} PUBLIC ${TBB_INCLUDE_DIRS})
    endif()
    if(TBB_LIBRARIES)
      target_link_libraries(${_target} ${TBB_LIBRARIES})
    endif()
  endforeach(_target)
endfunction(add_dune_tbb_flags)
