# tests that should build and run successfully
set(TESTS
    arraylisttest
    arraytest
    bigunsignedinttest
    bitsetvectortest
    calloncetest
    check_fvector_size
    conversiontest
    diagonalmatrixtest
    dynmatrixtest
    dynvectortest
    eigenvaluestest
    enumsettest
    fmatrixtest
    fvectortest
    gcdlcmtest
    integersequence
    iteratorfacadetest
    iteratorfacadetest2
    lrutest
    mpicollectivecommunication
    mpiguardtest
    mpihelpertest
    mpihelpertest2
    nullptr_test
    pathtest
    parametertreelocaletest
    parametertreetest
    poolallocatortest
    shared_ptrtest_config
    singletontest
    streamtest
    stringutilitytest
    testfloatcmp
    tuplestest
    tupleutilitytest
    utilitytest)

#test that should build but fail to run successfully
set(FAILTESTS
    testdebugallocator_fail1
    testdebugallocator_fail2
    testdebugallocator_fail3
    testdebugallocator_fail4
    testdebugallocator_fail5
   )

set(COMPILEFAILTESTS
    check_fvector_size_fail1
    check_fvector_size_fail2
    genericiterator_compile_fail
    nullptr-test-fail)

set(TESTPROGS ${TESTS} ${FAILTESTS})

# We do not want want to build the tests during make all,
# but just build them on demand
add_directory_test_target(_test_target)
add_dependencies(${_test_target} ${TESTPROGS})

# Add the executables needed for the tests
add_executable("arraylisttest" arraylisttest.cc)
add_executable("arraytest" arraytest.cc)

add_executable("bigunsignedinttest" bigunsignedinttest.cc)
target_link_libraries("bigunsignedinttest" "dunecommon")

add_executable("bitsetvectortest" bitsetvectortest.cc)
add_executable("calloncetest" calloncetest.cc)
target_link_libraries("calloncetest" "dunecommon")
add_executable("check_fvector_size" check_fvector_size.cc)
add_executable("check_fvector_size_fail1" EXCLUDE_FROM_ALL check_fvector_size_fail.cc)
set_target_properties(check_fvector_size_fail1 PROPERTIES COMPILE_FLAGS "-DDIM=1")
add_executable("check_fvector_size_fail2" EXCLUDE_FROM_ALL check_fvector_size_fail.cc)
set_target_properties(check_fvector_size_fail2 PROPERTIES COMPILE_FLAGS "-DDIM=3")
add_executable("conversiontest" conversiontest.cc)

add_executable("dynmatrixtest" dynmatrixtest.cc)
target_link_libraries("dynmatrixtest" "dunecommon")

add_executable("dynvectortest" dynvectortest.cc)

add_executable("eigenvaluestest" eigenvaluestest.cc)
target_link_libraries(eigenvaluestest dunecommon)
target_link_libraries(eigenvaluestest ${LAPACK_LIBRARIES})

add_executable("diagonalmatrixtest" diagonalmatrixtest.cc)
target_link_libraries("diagonalmatrixtest" "dunecommon")

add_executable("enumsettest" enumsettest.cc)

add_executable("testfloatcmp" testfloatcmp.cc)
target_link_libraries("testfloatcmp" "dunecommon")

# we provide an empty fortran file to force the linker
# to link to the fortran libraries (needed for static linking)
add_executable("fmatrixtest" fmatrixtest.cc dummy.f)
target_link_libraries("fmatrixtest" "dunecommon")
add_executable("fvectortest" fvectortest.cc)
add_dune_gmp_flags("fvectortest")
add_executable("gcdlcmtest" gcdlcmtest.cc)
add_executable("genericiterator_compile_fail" EXCLUDE_FROM_ALL genericiterator_compile_fail.cc)
add_executable("integersequence" integersequence.cc)
add_executable("iteratorfacadetest2" iteratorfacadetest2.cc)
add_executable("iteratorfacadetest" iteratorfacadetest.cc)
add_executable("lrutest" lrutest.cc)
target_link_libraries("lrutest" "dunecommon")
add_executable("mpiguardtest" mpiguardtest.cc)
target_link_libraries("mpiguardtest" "dunecommon")
add_DUNE_MPI_flags(mpiguardtest)

add_executable("mpicollectivecommunication" mpicollectivecommunication.cc)
target_link_libraries(mpicollectivecommunication "dunecommon")
add_DUNE_MPI_flags(mpicollectivecommunication)

add_executable("mpihelpertest" mpihelpertest.cc)
target_link_libraries(mpihelpertest "dunecommon")
add_DUNE_MPI_flags(mpihelpertest)

add_executable("mpihelpertest2" mpihelpertest.cc)
target_link_libraries(mpihelpertest2 "dunecommon")
add_DUNE_MPI_flags(mpihelpertest2)
set_target_properties(mpihelpertest2 PROPERTIES COMPILE_FLAGS "-DMPIHELPER_PREINITIALIZE")

add_executable("nullptr_test" nullptr-test.cc nullptr-test2.cc)
target_link_libraries(nullptr_test "dunecommon")
add_executable("nullptr_test_fail" EXCLUDE_FROM_ALL nullptr-test.cc)
target_link_libraries(nullptr_test_fail "dunecommon")
set_target_properties(nullptr_test_fail PROPERTIES COMPILE_FLAGS "-DFAIL")

add_executable("parametertreelocaletest" parametertreelocaletest.cc)
target_link_libraries("parametertreelocaletest" "dunecommon")

add_executable("parametertreetest" parametertreetest.cc)
target_link_libraries("parametertreetest" "dunecommon")

add_executable("pathtest" pathtest.cc)
target_link_libraries("pathtest" "dunecommon")

add_executable("poolallocatortest" poolallocatortest.cc)
add_executable("shared_ptrtest_config" shared_ptrtest.cc)
add_executable("singletontest" singletontest.cc)
add_executable("sllisttest" EXCLUDE_FROM_ALL sllisttest.cc)

add_executable("streamtest" streamtest.cc)
target_link_libraries("streamtest" "dunecommon")

add_executable("stringutilitytest" stringutilitytest.cc)
target_link_libraries("stringutilitytest" "dunecommon")

add_executable("testdebugallocator" testdebugallocator.cc)
target_link_libraries(testdebugallocator dunecommon)
add_executable("testdebugallocator_fail1" testdebugallocator.cc)
target_link_libraries(testdebugallocator_fail1 dunecommon)
set_target_properties(testdebugallocator_fail1 PROPERTIES COMPILE_DEFINITIONS "FAILURE1;EXPECTED_SIGNAL=SIGSEGV")
add_executable("testdebugallocator_fail2" testdebugallocator.cc)
target_link_libraries(testdebugallocator_fail2 dunecommon)
set_target_properties(testdebugallocator_fail2 PROPERTIES COMPILE_DEFINITIONS "FAILURE2;EXPECTED_SIGNAL=SIGABRT")
add_executable("testdebugallocator_fail3" testdebugallocator.cc)
target_link_libraries(testdebugallocator_fail3 dunecommon)
set_target_properties(testdebugallocator_fail3 PROPERTIES COMPILE_DEFINITIONS "FAILURE3;EXPECTED_SIGNAL=SIGABRT")
add_executable("testdebugallocator_fail4" testdebugallocator.cc)
target_link_libraries(testdebugallocator_fail4 dunecommon)
set_target_properties(testdebugallocator_fail4 PROPERTIES COMPILE_DEFINITIONS "FAILURE4;DEBUG_ALLOCATOR_KEEP=1;EXPECTED_SIGNAL=SIGSEGV")
add_executable("testdebugallocator_fail5" testdebugallocator.cc)
target_link_libraries(testdebugallocator_fail5 dunecommon)
set_target_properties(testdebugallocator_fail5 PROPERTIES COMPILE_DEFINITIONS "FAILURE5;EXPECTED_SIGNAL=SIGSEGV")

add_executable("tuplestest" tuplestest.cc)
add_executable("tupleutilitytest" tupleutilitytest.cc)
add_executable("utilitytest" utilitytest.cc)

# Add the tests to be executed
foreach(_TEST ${TESTPROGS})
  add_test(${_TEST} ${_TEST})
endforeach(_TEST)

# treat tests returning code 77 as skipped
set_tests_properties(
  eigenvaluestest
  mpicollectivecommunication
  parametertreelocaletest
  PROPERTIES SKIP_RETURN_CODE 77)

# set properties for failing tests
set_tests_properties(
  ${FAILTESTS}
  PROPERTIES WILL_FAIL true)

# compile tests that should fail
foreach(_TEST ${COMPILEFAILTESTS})
  add_test(NAME ${_TEST}
  COMMAND ${CMAKE_COMMAND} --build . --target ${_TEST} --config $<CONFIGURATION>)
endforeach(_TEST)
set_tests_properties(
  ${COMPILEFAILTESTS}
  PROPERTIES WILL_FAIL true)
