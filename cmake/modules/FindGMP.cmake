# Module that checks for the GNU MP Bignum (GMP) library, include
# dirs and
#
# Variables used by this module which you may want to set:
# GMP_PATH Path list to search for GMP
#
# Sets the following variables:
# GMP_FOUND True if the GMP library was found.
#

include(CheckIncludeFileCXX)
include(CheckCXXSymbolExists)

set(GMP_SEARCH_PATH CACHE FILEPATH "User defined list of directories to search for the GNU GMP library")

# save old variables
set(CMAKE_REQUIRED_INCLUDES_SAVE ${CMAKE_REQUIRED_INCLUDES})
set(CMAKE_REQUIRED_FLAGS_SAVE    ${CMAKE_REQUIRED_FLAGS})
set(CMAKE_REQUIRED_LIBRARIES_SAVE ${CMAKE_REQUIRED_LIBRARIES})


# Search for location of header gmpxx.h"
find_path(GMP_INCLUDE_DIRS "gmpxx.h" PATHS ${GMP_SEARCH_PATH} PATH_SUFFIXES include NO_DEFAULT_PATH)
#If not found fall back to default locations
find_path(GMP_INCLUDE_DIRS "gmpxx.h")

if(GMP_INCLUDE_DIRS)
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES_SAVE} ${GMP_INCLUDE_DIRS})
  check_include_files("gmpxx.h" GMP_FOUND)
  find_library(GMP_LIB gmp  PATHS ${GMP_SEARCH_PATH} PATH_SUFFIXES lib NODEFAULT_PATH DOC "GNU GMP library")
  find_library(GMP_LIB gmp)
  find_library(GMPXX_LIB gmpxx  PATHS ${GMP_SEARCH_PATH} NODEFAULT_PATH DOC DOC "GNU GMPXX library")
  find_library(GMPXX_LIB gmpxx)

  if(GMP_LIB AND GMPXX_LIB)
    set(GMP_FOUND "GMP_FOUND-NOTFOUND")
    check_symbol_exists(__gmpz_abs ${GMP_LIB} GMP_FOUND)
    if(GMP_FOUND)
      set(GMP_LIBRARIES ${GMP_LIB} ${GMPXX_LIB} CACHE INTERNAL)
      set(GMP_COMPILE_FLAGS "-DENABLE_GMP")
    endif(GMP_FOUND)
  else()
    set(GMP_FOUND "GMP_FOUND-NOTFOUND")
  endif()
  set(HAVE_GMP GMP_FOUND)
endif(GMP_INCLUDE_DIRS)

# restore values
set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES_SAVE})
set(CMAKE_REQUIRED_FLAGS    ${CMAKE_REQUIRED_FLAGS_SAVE})
set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES_SAVE})
mark_as_advanced(GMP_LIB GMP_LIBXX)

function(dune_add_gmp_flags _targets)
  if(GMP_FOUND)
    foreach(_target ${_targets})
      target_link_libraries(${_target} ${GMP_LIBRARIES})
      set_property(TARGET ${_target} APPEND_STRING COMPILE_FLAGS "-DENABLE_GMP=1")
      foreach(_path ${GMP_INCLUDE_DIRS})
	set_property(TARGET ${_target} APPEND_STRING COMPILE_FLAGS "-I${_path}")
      endforeach(_path ${GMP_INCLUDE_DIRS})
    endif(GMP_FOUND)
endfunction(dune_add_gmp_flags)

