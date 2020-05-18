#[=======================================================================[.rst:
DuneTesting
-----------

Enable the testing facilities and include common macros for dune tests

This file should be included in your root CMakeLists file.

#]=======================================================================]

include_guard(GLOBAL)

# enable the testing suite on the CMake side.
enable_testing()
include(CTest)

include(DuneAddTest)
include(DuneDeclareTestLabel)

# Introduce a target that triggers the building of all tests
add_custom_target(build_tests)

# predefine "quick" test label so build_quick_tests can be built
# unconditionally
dune_declare_test_label(LABELS quick)

# Set the default on the variable DUNE_MAX_TEST_CORES
if (NOT DUNE_MAX_TEST_CORES)
  set(DUNE_MAX_TEST_CORES 2)
endif ()
