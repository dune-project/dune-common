# general stuff
cmake_minimum_required(VERSION 2.8)
set(ProjectName            "dune-common")
set(ProjectVersion         "2.2-svn")
set(ProjectMaintainer      "DUNE Project")
set(ProjectMaintainerEmail "dune@dune-project.org")
project(${ProjectName} C CXX)

# make sure our own modules are found
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

# include dune-common to include pathtest
include_directories("${CMAKE_SOURCE_DIR}")
link_directories("${CMAKE_SOURCE_DIR}")
# set(RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
# set(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
# set(LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")

# set required compiler flags for C++11 (former C++0x)
find_package(CXX11Features)

# search for headers
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
check_include_file("malloc.h" HAVE_MALLOC_H)
check_include_file_cxx("memory" HAVE_MEMORY)
set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -DHAVE_MEMORY=${HAVE_MEMORY}")

# search for packages
find_package(MPI)
set(HAVE_MPI MPI_CXX_FOUND)
find_package(Boost)
set(HAVE_BOOST BOOST_FOUND)
find_package(SharedPtr)

# actually write the config.h file to disk
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DHAVE_CONFIG_H)

# add subdirectories to execute CMakeLIsts.txt there
add_subdirectory("dune")

# set up CTest
enable_testing()
include(CTest)
# add tests from dune/common/test
add_test(arraylisttest			dune/common/test/arraylisttest)
add_test(arraytest			dune/common/test/arraytest)
add_test(bigunsignedinttest		dune/common/test/bigunsignedinttest)
add_test(bitsetvectortest		dune/common/test/bitsetvectortest)
add_test(conversiontest			dune/common/test/conversiontest)
add_test(deprtuplestest			dune/common/test/deprtuplestest)
add_test(dynmatrixtest			dune/common/test/dynmatrixtest)
add_test(dynvectortest			dune/common/test/dynvectortest)
add_test(enumsettest			dune/common/test/enumsettest)
add_test(fassigntest			dune/common/test/fassigntest)
add_test(fmatrixtest			dune/common/test/fmatrixtest)
add_test(fvectortest			dune/common/test/fvectortest)
add_test(gcdlcmtest			dune/common/test/gcdlcmtest)
add_test(iteratorfacadetest2		dune/common/test/iteratorfacadetest2)
add_test(iteratorfacadetest		dune/common/test/iteratorfacadetest)
add_test(lrutest			dune/common/test/lrutest)
add_test(mpiguardtest			dune/common/test/mpiguardtest)
add_test(mpihelpertest			dune/common/test/mpihelpertest)
add_test(nullptr-test2			dune/common/test/nullptr-test2)
add_test(nullptr-test			dune/common/test/nullptr-test)
add_test(parametertreetest		dune/common/test/parametertreetest)
add_test(pathtest			dune/common/test/pathtest)
add_test(poolallocatortest		dune/common/test/poolallocatortest)
add_test(shared_ptrtest			dune/common/test/shared_ptrtest)
add_test(singletontest			dune/common/test/singletontest)
add_test(sllisttest			dune/common/test/sllisttest)
add_test(NAME static_assert_test
  COMMAND ${CMAKE_COMMAND} --build . --target static_assert_test --config $<CONFIGURATION>)
add_test(NAME static_assert_test_fail
  COMMAND ${CMAKE_COMMAND} --build . --target static_assert_test_fail --config $<CONFIGURATION>)
set_tests_properties(static_assert_test_fail PROPERTIES WILL_FAIL true)
add_test(streamtest			dune/common/test/streamtest)
add_test(testfassign2			dune/common/test/testfassign2)
add_test(testfassign			dune/common/test/testfassign)
add_test(testfconstruct			dune/common/test/testfconstruct)
add_test(test-stack			dune/common/test/test-stack)
add_test(tuplestest			dune/common/test/tuplestest)
add_test(tupleutilitytest		dune/common/test/tupleutilitytest)
add_test(utilitytest			dune/common/test/utilitytest)
# add tests from dune/common/parallel/test
add_test(indexsettest			dune/common/parallel/test/indexsettest)
add_test(indicestest			dune/common/parallel/test/indicestest)
add_test(selectiontest			dune/common/parallel/test/selectiontest)
add_test(syncertest			dune/common/parallel/test/syncertest)
