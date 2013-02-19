# general stuff
cmake_minimum_required(VERSION 2.8)

# make sure our own modules are found
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")


set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-funroll-loops -O3")
set(CMAKE_C_FLAGS_RELEASE "-funroll-loops -O3")

include(DuneMacros)
dune_module_information(${CMAKE_SOURCE_DIR})

set(ProjectName            "${DUNE_MOD_NAME}")
set(ProjectVersion         "${DUNE_MOD_VERSION}")
set(ProjectMaintainerEmail "${DUNE_MAINTAINER}")
project(${ProjectName} C CXX)# Fortran)
set(DUNE_COMMON_VERSION          "${DUNE_MOD_VERSION}")
set(DUNE_COMMON_VERSION_MAJOR    "${DUNE_VERSION_MAJOR}")
set(DUNE_COMMON_VERSION_MINOR    "${DUNE_VERSION_MINOR}")
set(DUNE_COMMON_VERSION_REVISION "${DUNE_VERSION_REVISION}")


option(DUNE_USE_ONLY_STATIC_LIBS "If set to ON, we will force static linkage everywhere" OFF)
if(DUNE_USE_ONLY_STATIC_LIBS)
  set(_default_enable_shared OFF)
else(DUNE_USE_ONLY_STATIC_LIBS)
  set(_default_enable_shared ON)
endif(DUNE_USE_ONLY_STATIC_LIBS)
option(BUILD_SHARED_LIBS "If set to ON, shared libs will be built" ${_default_enable_shared})

if(DUNE_USE_ONLY_STATIC_LIBS)
  # Use only static libraries.
  # We do this by overriding the library suffixes.
  set( BLA_STATIC 1)
  set( _dune_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
  if (WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .lib ${CMAKE_FIND_LIBRARY_SUFFIXES})
  endif (WIN32)
  if (APPLE)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .lib ${CMAKE_FIND_LIBRARY_SUFFIXES})
  else (APPLE)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a )
  endif (APPLE)
endif()

include(DuneStreams)
dune_set_minimal_debug_level()

# include dune-common and current directory to include pathtest
include_directories("${CMAKE_SOURCE_DIR}")
link_directories("${CMAKE_SOURCE_DIR}/lib")

# set required compiler flags for C++11 (former C++0x)
find_package(CXX11Features)

# search for headers
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
check_include_file("malloc.h" HAVE_MALLOC_H)
check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file_cxx("memory" HAVE_MEMORY)
set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -DHAVE_MEMORY=${HAVE_MEMORY}")

# search for packages
find_package(PkgConfig)

include(DUNEMPI)

find_package(Boost)
set(HAVE_BOOST BOOST_FOUND)
find_package(SharedPtr)
include(LanguageSupport)
workaround_9220(Fortran Fortran_Works)
if(Fortran_Works)
  enable_language(Fortran OPTIONAL)
  # search for lapack
  find_package(LAPACK)
  set(HAVE_LAPACK LAPACK_FOUND)
  set(HAVE_BLAS BLAS_FOUND)
  # make calling fortran routines from C/C++ possible
  include(FortranCInterface)
  FortranCInterface_VERIFY(CXX)
  FortranCInterface_HEADER(FC.h MACRO_NAMESPACE "FC_")
else(Fortran_Works)
  set(HAVE_LAPACK Off)
  set(HAVE_BLAS Off)
  file(WRITE ${CMAKE_BINARY_DIR}/FC.h "")
endif(Fortran_Works)


# actually write the config.h file to disk
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DHAVE_CONFIG_H)

include(DUNEDoxygen)

# Create custom target for building the documentation
# and provide macros for installing the docs and force
# building them before.
include(DUNEDoc)

# Create a custom target for building the tests.
# Thus they will not be built by make all any more.
# Actually I wanted this target to be a dependency
# of make test but that is currently not possible.
# See http://public.kitware.com/Bug/view.php?id=8438
# This really sucks!
# Therefore currently make build_tests has to be called
# before make test.
add_custom_target(build_tests DEPENDS ${TESTPROGS})

# mimic make check of the autotools
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS build_tests)

# enable testing before adding the subdirectories.
# Thus we can add the tests in the subdirectories
# where they actually are.
enable_testing()
#add_custom_target(build_test)
set_property(TEST APPEND PROPERTY DEPENDS build_tests)

# add subdirectories to execute CMakeLists.txt there
add_subdirectory("dune")
add_subdirectory("bin")
add_subdirectory("m4")
add_subdirectory("am")
add_subdirectory("doc")
add_subdirectory("cmake/modules")
add_subdirectory("cmake/scripts")

# set some variables that are used in the pkg-config file
set( CMAKE_FIND_LIBRARY_SUFFIXES ${_dune_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})
set( prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/lib")
set(includedir "\${prefix}/include")
set(PACKAGE_NAME ${DUNE_MOD_NAME})
set(VERSION ${DUNE_MOD_VERSION})
set(CC ${CMAKE_C_COMPILER})
set(CXX ${CMAKE_CXX_COMPILER})

#create pkg-config file
configure_file(
  ${PROJECT_SOURCE_DIR}/${DUNE_MOD_NAME}.pc.in
  ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}.pc
  @ONLY
)

#create cmake-config files
configure_file(
  ${PROJECT_SOURCE_DIR}/${DUNE_MOD_NAME}-config.cmake.in
  ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}-config.cmake @ONLY)

configure_file(
  ${PROJECT_SOURCE_DIR}/${DUNE_MOD_NAME}-version.cmake.in
  ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}-version.cmake @ONLY)

#install dune.module file
install(FILES dune.module DESTINATION lib/dunecontrol/${DUNE_MOD_NAME})
# install pkgconfig file
if(PKG_CONFIG_FOUND )
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DUNE_MOD_NAME}.pc
    DESTINATION lib/pkgconfig)
endif(PKG_CONFIG_FOUND)

#install cmake-config files
install(FILES ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}-config.cmake
  ${PROJECT_BINARY_DIR}/${DUNE_MOD_NAME}-version.cmake
  DESTINATION lib/cmake)

include(CTest)

export(PACKAGE dune-common)
