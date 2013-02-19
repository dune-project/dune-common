# general stuff
cmake_minimum_required(VERSION 2.8)

# make sure our own modules are found
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

include(DuneMacros)
dune_module_information(${CMAKE_SOURCE_DIR})

set(ProjectName            "${DUNE_MODULE_NAME}")
set(ProjectVersion         "${DUNE_VERSION_STRING}")
set(ProjectMaintainerEmail "${DUNE_MAINTAINER}")
project(${ProjectName} C CXX)
set(DUNE_COMMON_VERSION          "${DUNE_VERSION_STRING}")
set(DUNE_COMMON_VERSION_MAJOR    "${DUNE_VERSION_MAJOR}")
set(DUNE_COMMON_VERSION_MINOR    "${DUNE_VERSION_MINOR}")
set(DUNE_COMMON_VERSION_REVISION "${DUNE_VERSION_REVISION}")

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
if(MPI_FOUND)
set(HAVE_MPI MPI_CXX_FOUND)
add_definitions("${MPI_CXX_COMPILE_FLAGS}")
 #add_definitions("-DENABLE_MPI=1")
include_directories(${MPI_CXX_INCLUDE_PATH})
endif(MPI_FOUND)

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
add_test(check_fvector_size			dune/common/test/check_fvector_size)
add_test(conversiontest			dune/common/test/conversiontest)
add_test(deprtuplestest			dune/common/test/deprtuplestest)
add_test(dynmatrixtest			dune/common/test/dynmatrixtest)
add_test(dynvectortest			dune/common/test/dynvectortest)
add_test(enumsettest			dune/common/test/enumsettest)
add_test(fmatrixtest			dune/common/test/fmatrixtest)
add_test(fvectortest			dune/common/test/fvectortest)
add_test(gcdlcmtest			dune/common/test/gcdlcmtest)
add_test(iteratorfacadetest		dune/common/test/iteratorfacadetest)
add_test(iteratorfacadetest2		dune/common/test/iteratorfacadetest2)
add_test(lrutest			dune/common/test/lrutest)
if(MPI_CXX_FOUND)
  add_test(mpicollectivecommunication			dune/common/test/mpicollectivecommunication)
endif(MPI_CXX_FOUND)
add_test(mpiguardtest			dune/common/test/mpiguardtest)
add_test(mpihelpertest			dune/common/test/mpihelpertest)
add_test(mpihelpertest2			dune/common/test/mpihelpertest2)
add_test(nullptr_test			dune/common/test/nullptr_test)
add_test(parametertreetest		dune/common/test/parametertreetest)
add_test(pathtest			dune/common/test/pathtest)
add_test(poolallocatortest		dune/common/test/poolallocatortest)
add_test(shared_ptrtest_config			dune/common/test/shared_ptrtest_config)
add_test(shared_ptrtest_dune			dune/common/test/shared_ptrtest_dune)
add_test(singletontest			dune/common/test/singletontest)
add_test(streamtest			dune/common/test/streamtest)
add_test(testfassign1			dune/common/test/testfassign1)
add_test(testfassign2			dune/common/test/testfassign2)
add_test(testfassign3			dune/common/test/testfassign3)
add_test(testfassign4			dune/common/test/testfassign4)
add_test(test_stack			dune/common/test/test_stack)
add_test(tuplestest_config			dune/common/test/tuplestest_config)
add_test(tuplestest_dune			dune/common/test/tuplestest_dune)
add_test(tuplestest_tr1			dune/common/test/tuplestest_tr1)
add_test(tupleutilitytest		dune/common/test/tupleutilitytest)
add_test(utilitytest			dune/common/test/utilitytest)
# failing tests
add_test(testfassign_fail1			dune/common/test/testfassign_fail1)
add_test(testfassign_fail2			dune/common/test/testfassign_fail2)
add_test(testfassign_fail3			dune/common/test/testfassign_fail3)
add_test(testfassign_fail4			dune/common/test/testfassign_fail4)
add_test(testfassign_fail5			dune/common/test/testfassign_fail5)
add_test(testfassign_fail6			dune/common/test/testfassign_fail6)
set_tests_properties(
  testfassign_fail1
  testfassign_fail2
  testfassign_fail3
  testfassign_fail4
  testfassign_fail5
  testfassign_fail6
  PROPERTIES WILL_FAIL true)
# compile test
add_test(NAME static_assert_test
  COMMAND ${CMAKE_COMMAND} --build . --target static_assert_test --config $<CONFIGURATION>)
add_test(NAME testfconstruct
  COMMAND ${CMAKE_COMMAND} --build . --target testfconstruct --config $<CONFIGURATION>)
# failing compile tests
add_test(NAME genericiterator_compile_fail
  COMMAND ${CMAKE_COMMAND} --build . --target genericiterator_compile_fail --config $<CONFIGURATION>)
add_test(NAME check_fvector_size_fail1
  COMMAND ${CMAKE_COMMAND} --build . --target check_fvector_size_fail1 --config $<CONFIGURATION>)
add_test(NAME check_fvector_size_fail2
  COMMAND ${CMAKE_COMMAND} --build . --target check_fvector_size_fail2 --config $<CONFIGURATION>)
add_test(NAME nullptr_test_fail
  COMMAND ${CMAKE_COMMAND} --build . --target nullptr_test_fail --config $<CONFIGURATION>)
add_test(NAME static_assert_test_fail
  COMMAND ${CMAKE_COMMAND} --build . --target static_assert_test_fail --config $<CONFIGURATION>)
add_test(NAME testfconstruct_fail1
  COMMAND ${CMAKE_COMMAND} --build . --target testfconstruct_fail1 --config $<CONFIGURATION>)
add_test(NAME testfconstruct_fail2
  COMMAND ${CMAKE_COMMAND} --build . --target testfconstruct_fail2 --config $<CONFIGURATION>)
set_tests_properties(
  genericiterator_compile_fail
  check_fvector_size_fail1
  check_fvector_size_fail2
  nullptr_test_fail
  static_assert_test_fail
  testfconstruct_fail1
  testfconstruct_fail2
  PROPERTIES WILL_FAIL true)

# add tests from dune/common/parallel/test
add_test(indexsettest			dune/common/parallel/test/indexsettest)
add_test(selectiontest			dune/common/parallel/test/selectiontest)
if(MPI_CXX_FOUND)
  add_test(indicestest			dune/common/parallel/test/indicestest)
  #add_test(syncertest			dune/common/parallel/test/syncertest)
endif(MPI_CXX_FOUND)
