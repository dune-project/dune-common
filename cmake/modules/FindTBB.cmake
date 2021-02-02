#[=======================================================================[.rst:
FindTBB
-------

Finds the Threading Building Blocks (TBB) library.

This is a fallback implementation in case the TBB library does not provide
itself a corresponding TBBConfig.cmake file.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``TBB::tbb``
  Imported library to link against if TBB should be used.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``TBB_FOUND``
  True if the TBB library was found.

Finding the TBB library
^^^^^^^^^^^^^^^^^^^^^^^

Two strategies are implemented for finding the TBB library:

1. Searching for the TBB cmake config file, typically named
   ``TBBConfig.cmake``. In recent TBB versions, this file can be
   created using a script provided by TBB itself. Simply set the
   variable ``TBB_DIR`` to the directory containing the config file
   in order to find TBB.

2. Using pkg-config to configure TBB. Therefore it is necessary
   to find the ``tbb.pc`` file. Several distributions provide this file
   directly. In order to point pkg-config to the location of that file,
   simply set the environmental variable ``PKG_CONFIG_PATH`` to include
   the directory containing the .pc file, or add this path to the
   ``CMAKE_PREFIX_PATH``.

#]=======================================================================]


# text for feature summary
include(FeatureSummary)
set_package_properties("TBB" PROPERTIES
  DESCRIPTION "Intel's Threading Building Blocks"
)

# first, try to find TBBs cmake configuration
find_package(TBB ${TBB_FIND_VERSION} QUIET CONFIG)
if(TBB_FOUND)
  message(STATUS "Found TBB: using configuration from TBB_DIR=${TBB_DIR} (found version \"${TBB_VERSION}\")")
  return()
endif()

# second, try to find TBBs pkg-config file
find_package(PkgConfig)
if(PkgConfig_FOUND)
  if(TBB_FIND_VERSION)
    pkg_check_modules(_TBB tbb>=${TBB_FIND_VERSION} QUIET IMPORTED_TARGET)
  else()
    pkg_check_modules(_TBB tbb QUIET IMPORTED_TARGET)
  endif()
endif()

# check whether the static library was found
if(_TBB_STATIC_FOUND)
  set(_static _STATIC)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("TBB"
  REQUIRED_VARS
    _TBB${_static}_FOUND PkgConfig_FOUND
  VERSION_VAR
    _TBB${_static}_VERSION
  FAIL_MESSAGE "Could NOT find TBB (set TBB_DIR to path containing TBBConfig.cmake or set PKG_CONFIG_PATH to include the location of the tbb.pc file)"
)

# create an alias for the imported target constructed by pkg-config
if(TBB_FOUND AND NOT TARGET TBB::tbb)
  message(STATUS "Found TBB: ${_TBB${_static}_LINK_LIBRARIES} (found version \"${_TBB${_static}_VERSION}\")")
  add_library(TBB::tbb ALIAS PkgConfig::_TBB)
endif()
