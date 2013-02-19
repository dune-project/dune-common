# Install some test headers, because they get used by tests in other modules
install(FILES iteratortest.hh DESTINATION include/dune/common/test)

set(TESTPROGS
    arraylisttest
    arraytest
    bigunsignedinttest
    bitsetvectortest
    check_fvector_size
    conversiontest
    deprtuplestest
    dynmatrixtest
    dynvectortest
    enumsettest
    fassigntest
    float_cmp
    fmatrixtest
    fvectortest
    gcdlcmtest
    iteratorfacadetest
    iteratorfacadetest2
    lrutest
    mpicollectivecommunication
    mpiguardtest
    mpihelpertest
    mpihelpertest2
    nullptr_test
    pathtest
    parametertreetest
    poolallocatortest
    shared_ptrtest_config
    shared_ptrtest_dune
    singletontest
    static_assert_test
    streamtest
    test_stack
    testfassign1
    testfassign2
    testfassign3
    testfassign4
    testfassign_fail1
    testfassign_fail2
    testfassign_fail3
    testfassign_fail4
    testfassign_fail5
    testfassign_fail6
    testfconstruct
    tuplestest_config
    tuplestest_dune
    tuplestest_std
    tuplestest_tr1
    tupleutilitytest
    utilitytest)

# We do not want want to build the tests during make all,
# but just build them on demand
add_dependencies(build_tests ${TESTPROGS})

# Add the executables needed for the tests
add_executable("arraylisttest" arraylisttest.cc)
add_executable("arraytest" arraytest.cc)

add_executable("bigunsignedinttest" bigunsignedinttest.cc)
target_link_libraries("bigunsignedinttest" "dunecommon")

add_executable("bitsetvectortest" bitsetvectortest.cc)
add_executable("check_fvector_size" check_fvector_size.cc)
add_executable("check_fvector_size_fail1" EXCLUDE_FROM_ALL check_fvector_size_fail.cc)
set_target_properties(check_fvector_size_fail1 PROPERTIES COMPILE_FLAGS "-DDIM=1")
add_executable("check_fvector_size_fail2" EXCLUDE_FROM_ALL check_fvector_size_fail.cc)
set_target_properties(check_fvector_size_fail2 PROPERTIES COMPILE_FLAGS "-DDIM=3")
add_executable("conversiontest" conversiontest.cc)
add_executable("deprtuplestest" deprtuplestest.cc)

add_executable("dynmatrixtest" dynmatrixtest.cc)
target_link_libraries("dynmatrixtest" "dunecommon")

add_executable("dynvectortest" dynvectortest.cc)
add_executable("enumsettest" enumsettest.cc)

add_executable("fassigntest" fassigntest.cc)
target_link_libraries("fassigntest" "dunecommon")

# we provide an empty fortran file to force the linker
# to link to the fortran libraries (needed for static linking)
add_executable("fmatrixtest" fmatrixtest.cc dummy.f)
target_link_libraries("fmatrixtest" "dunecommon" ${LAPACK_LIBRARIES})

add_executable("fvectortest" fvectortest.cc)
add_executable("gcdlcmtest" gcdlcmtest.cc)
add_executable("genericiterator_compile_fail" EXCLUDE_FROM_ALL genericiterator_compile_fail.cc)
add_executable("iteratorfacadetest2" iteratorfacadetest2.cc)
add_executable("iteratorfacadetest" iteratorfacadetest.cc)
add_executable("lrutest" lrutest.cc)
add_executable("mpiguardtest" mpiguardtest.cc)
target_link_libraries("mpiguardtest" "dunecommon")
if(MPI_FOUND)
  target_link_libraries(mpiguardtest ${MPI_CXX_LIBRARIES})
endif(MPI_FOUND)
add_executable("mpicollectivecommunication" mpicollectivecommunication.cc)
target_link_libraries(mpicollectivecommunication "dunecommon")
if(MPI_FOUND)
  target_link_libraries(mpicollectivecommunication ${MPI_CXX_LIBRARIES})
endif(MPI_FOUND)
add_executable("mpihelpertest" mpihelpertest.cc)
target_link_libraries(mpihelpertest "dunecommon")
if(MPI_FOUND)
  target_link_libraries(mpihelpertest ${MPI_CXX_LIBRARIES})
endif(MPI_FOUND)
add_executable("mpihelpertest2" mpihelpertest.cc)
target_link_libraries(mpihelpertest2 "dunecommon")
if(MPI_FOUND)
  target_link_libraries(mpihelpertest2 ${MPI_CXX_LIBRARIES})
endif(MPI_FOUND)
set_target_properties(mpihelpertest2 PROPERTIES COMPILE_FLAGS "-DMPIHELPER_PREINITIALIZE")
add_executable("nullptr_test" nullptr-test.cc nullptr-test2.cc)
target_link_libraries(nullptr_test "dunecommon")
add_executable("nullptr_test_fail" EXCLUDE_FROM_ALL nullptr-test.cc)
target_link_libraries(nullptr_test_fail "dunecommon")
set_target_properties(nullptr_test_fail PROPERTIES COMPILE_FLAGS "-DFAIL")

add_executable("parametertreetest" parametertreetest.cc)
target_link_libraries("parametertreetest" "dunecommon")

add_executable("pathtest" pathtest.cc)
target_link_libraries("pathtest" "dunecommon")

add_executable("poolallocatortest" poolallocatortest.cc)
add_executable("shared_ptrtest_config" shared_ptrtest.cc)
add_executable("shared_ptrtest_dune" shared_ptrtest.cc)
set_target_properties(shared_ptrtest_dune PROPERTIES COMPILE_FLAGS "-DDISABLE_CONFIGURED_SHARED_PTR")
add_executable("singletontest" singletontest.cc)
add_executable("sllisttest" EXCLUDE_FROM_ALL sllisttest.cc)
add_executable("static_assert_test" EXCLUDE_FROM_ALL static_assert_test.cc)
add_executable("static_assert_test_fail" EXCLUDE_FROM_ALL static_assert_test_fail.cc)

add_executable("streamtest" streamtest.cc)
target_link_libraries("streamtest" "dunecommon")

add_executable("testfassign1" testfassign.cc testfassign2.cc)
set_target_properties(testfassign1 PROPERTIES COMPILE_FLAGS "-D_DUNE_TEST_N=3 -D_VALUES=\"1,2,3\"")
target_link_libraries(testfassign1 "dunecommon")
add_executable("testfassign2" testfassign.cc)
set_target_properties(testfassign2 PROPERTIES COMPILE_FLAGS "-D_DUNE_TEST_N=3 -D_VALUES=\"1,zero\"")
target_link_libraries(testfassign2 "dunecommon")
add_executable("testfassign3" testfassign.cc)
set_target_properties(testfassign3 PROPERTIES COMPILE_FLAGS "-D_DUNE_TEST_N=3 -D_VALUES=\"zero\"")
target_link_libraries(testfassign3 "dunecommon")
add_executable("testfassign4" testfassign.cc)
set_target_properties(testfassign4 PROPERTIES COMPILE_FLAGS "-D_DUNE_TEST_N=2 -D_DUNE_TEST_M=3 -D_VALUES=\"1, zero, nextRow, 2, 3, 4\"")
target_link_libraries(testfassign4 "dunecommon")
add_executable("testfassign_fail1" testfassign.cc)
set_target_properties(testfassign_fail1 PROPERTIES COMPILE_FLAGS "-D_DUNE_TEST_N=3 -D_VALUES=\"1,2\"")
target_link_libraries(testfassign_fail1 "dunecommon")
add_executable("testfassign_fail2" testfassign.cc)
set_target_properties(testfassign_fail2 PROPERTIES COMPILE_FLAGS "-D_DUNE_TEST_N=3 -D_VALUES=\"1,2,3,4\"")
target_link_libraries(testfassign_fail2 "dunecommon")
add_executable("testfassign_fail3" testfassign.cc)
set_target_properties(testfassign_fail3 PROPERTIES COMPILE_FLAGS "-D_DUNE_TEST_N=2 -D_DUNE_TEST_M=2 -D_VALUES=\"1, nextRow, 2, 3\"")
target_link_libraries(testfassign_fail3 "dunecommon")
add_executable("testfassign_fail4" testfassign.cc)
set_target_properties(testfassign_fail4 PROPERTIES COMPILE_FLAGS "-D_DUNE_TEST_N=2 -D_DUNE_TEST_M=2 -D_VALUES=\"1, 2, 3, nextRow, 2, 3\"")
target_link_libraries(testfassign_fail4 "dunecommon")
add_executable("testfassign_fail5" testfassign.cc)
set_target_properties(testfassign_fail5 PROPERTIES COMPILE_FLAGS "-D_DUNE_TEST_N=2 -D_DUNE_TEST_M=2 -D_VALUES=\"1, 2\"")
target_link_libraries(testfassign_fail5 "dunecommon")
add_executable("testfassign_fail6" testfassign.cc)
set_target_properties(testfassign_fail6 PROPERTIES COMPILE_FLAGS "-D_DUNE_TEST_N=2 -D_DUNE_TEST_M=2 -D_VALUES=\"1, 2, nextRow, 2, 3, nextRow, 4, 5\"")
target_link_libraries(testfassign_fail6 "dunecommon")

add_executable("testfconstruct" EXCLUDE_FROM_ALL testfconstruct.cc)
set_target_properties(testfconstruct PROPERTIES COMPILE_FLAGS "-DFVSIZE=3")
target_link_libraries(testfconstruct "dunecommon")
add_executable("testfconstruct_fail1" EXCLUDE_FROM_ALL testfconstruct.cc)
set_target_properties(testfconstruct_fail1 PROPERTIES COMPILE_FLAGS "-DFVSIZE=2")
target_link_libraries(testfconstruct_fail1 "dunecommon")
add_executable("testfconstruct_fail2" EXCLUDE_FROM_ALL testfconstruct.cc)
set_target_properties(testfconstruct_fail2 PROPERTIES COMPILE_FLAGS "-DFVSIZE=5")
target_link_libraries(testfconstruct_fail2 "dunecommon")
add_executable("test_stack" test-stack.cc)
target_link_libraries("test_stack" "dunecommon")
add_executable("tuplestest_config" tuplestest.cc)
add_executable("tuplestest_dune" tuplestest.cc)
set_target_properties(tuplestest_dune PROPERTIES COMPILE_FLAGS "-DDISABLE_TR1_TUPLE -DDISABLE_STD_TUPLE")
add_executable("tuplestest_tr1" tuplestest.cc)
set_target_properties(tuplestest_tr1 PROPERTIES COMPILE_FLAGS "-DDISABLE_STD_TUPLE")
add_executable("tupleutilitytest" tupleutilitytest.cc)
add_executable("utilitytest" utilitytest.cc)

# Add the tests to be executed
add_test(arraylisttest                  arraylisttest)
add_test(arraytest                      arraytest)
add_test(bigunsignedinttest             bigunsignedinttest)
add_test(bitsetvectortest               bitsetvectortest)
add_test(check_fvector_size             check_fvector_size)
add_test(conversiontest                 conversiontest)
add_test(deprtuplestest                 deprtuplestest)
add_test(dynmatrixtest                  dynmatrixtest)
add_test(dynvectortest                  dynvectortest)
add_test(enumsettest                    enumsettest)
add_test(fmatrixtest                    fmatrixtest)
add_test(fvectortest                    fvectortest)
add_test(gcdlcmtest                     gcdlcmtest)
add_test(iteratorfacadetest             iteratorfacadetest)
add_test(iteratorfacadetest2            iteratorfacadetest2)
add_test(lrutest                        lrutest)
if(MPI_CXX_FOUND)
  add_test(mpicollectivecommunication    mpicollectivecommunication)
endif(MPI_CXX_FOUND)
add_test(mpiguardtest                   mpiguardtest)
add_test(mpihelpertest                  mpihelpertest)
add_test(mpihelpertest2                 mpihelpertest2)
add_test(nullptr_test                   nullptr_test)
add_test(parametertreetest              parametertreetest)
add_test(pathtest                       pathtest)
add_test(poolallocatortest              poolallocatortest)
add_test(shared_ptrtest_config          shared_ptrtest_config)
add_test(shared_ptrtest_dune            shared_ptrtest_dune)
add_test(singletontest                  singletontest)
add_test(streamtest                     streamtest)
add_test(testfassign1                   testfassign1)
add_test(testfassign2                   testfassign2)
add_test(testfassign3                   testfassign3)
add_test(testfassign4                   testfassign4)
add_test(test_stack                     test_stack)
add_test(tuplestest_config              tuplestest_config)
add_test(tuplestest_dune                tuplestest_dune)
add_test(tuplestest_tr1                 tuplestest_tr1)
add_test(tupleutilitytest               tupleutilitytest)
add_test(utilitytest                    utilitytest)
# failing tests
add_test(testfassign_fail1              testfassign_fail1)
add_test(testfassign_fail2              testfassign_fail2)
add_test(testfassign_fail3              testfassign_fail3)
add_test(testfassign_fail4              testfassign_fail4)
add_test(testfassign_fail5              testfassign_fail5)
add_test(testfassign_fail6              testfassign_fail6)
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

