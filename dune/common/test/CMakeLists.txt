dune_add_test(SOURCES arithmetictestsuitetest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES arraylisttest.cc)

dune_add_test(SOURCES arraydeprecationtest.cc)

dune_add_test(SOURCES arraytest.cc)

dune_add_test(SOURCES assertandreturntest.cc
              LINK_LIBRARIES dunecommon)
dune_add_test(NAME assertandreturntest_compiletime_fail
              SOURCES assertandreturntest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "TEST_COMPILETIME_FAIL"
              EXPECT_COMPILE_FAIL)
dune_add_test(NAME assertandreturntest_ndebug
              SOURCES assertandreturntest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "TEST_NDEBUG")

dune_add_test(SOURCES autocopytest.cc)

dune_add_test(SOURCES bigunsignedinttest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES bitsetvectortest.cc)

dune_add_test(SOURCES boundscheckingtest.cc
              COMPILE_DEFINITIONS DUNE_CHECK_BOUNDS=1
              LINK_LIBRARIES dunecommon)
dune_add_test(SOURCES boundscheckingmvtest.cc
              COMPILE_DEFINITIONS DUNE_CHECK_BOUNDS=1
              LINK_LIBRARIES dunecommon)
dune_add_test(SOURCES boundscheckingoptest.cc
              COMPILE_DEFINITIONS DUNE_CHECK_BOUNDS=1
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES calloncetest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES check_fvector_size.cc)

dune_add_test(NAME check_fvector_size_fail1
              SOURCES check_fvector_size_fail.cc
              COMPILE_DEFINITIONS DIM=1
              EXPECT_COMPILE_FAIL)

dune_add_test(NAME check_fvector_size_fail2
              SOURCES check_fvector_size_fail.cc
              COMPILE_DEFINITIONS DIM=3
              EXPECT_COMPILE_FAIL)

dune_add_test(SOURCES classnametest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES concept.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES debugaligntest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon)
dune_add_test(NAME densematrixassignmenttest_fail0
              SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE0"
              EXPECT_COMPILE_FAIL)
dune_add_test(NAME densematrixassignmenttest_fail1
              SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE1"
              EXPECT_COMPILE_FAIL)
dune_add_test(NAME densematrixassignmenttest_fail2
              SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE2"
              EXPECT_COMPILE_FAIL)
dune_add_test(NAME densematrixassignmenttest_fail3
              SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE3"
              EXPECT_COMPILE_FAIL)
dune_add_test(NAME densematrixassignmenttest_fail4
              SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE4"
              EXPECT_COMPILE_FAIL)
dune_add_test(NAME densematrixassignmenttest_fail5
              SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE5"
              EXPECT_COMPILE_FAIL)
dune_add_test(NAME densematrixassignmenttest_fail6
              SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE6"
              EXPECT_COMPILE_FAIL)

dune_add_test(SOURCES diagonalmatrixtest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES dynmatrixtest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES dynvectortest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES densevectortest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES enumsettest.cc)

dune_add_test(SOURCES filledarraytest.cc)

dune_add_test(SOURCES fmatrixtest.cc
              LINK_LIBRARIES dunecommon)
add_dune_vc_flags(fmatrixtest)

dune_add_test(SOURCES functiontest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES fvectortest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES genericiterator_compile_fail.cc
              EXPECT_COMPILE_FAIL)

dune_add_test(SOURCES gcdlcmtest.cc)

dune_add_test(SOURCES hybridutilitiestest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES indicestest.cc)

dune_add_test(SOURCES integersequence.cc)

dune_add_test(SOURCES iteratorfacadetest2.cc)

dune_add_test(SOURCES iteratorfacadetest.cc)

dune_add_test(SOURCES lrutest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES mpicollectivecommunication.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4 8
              TIMEOUT 300)

dune_add_test(SOURCES mpiguardtest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4 8
              TIMEOUT 300)

dune_add_test(SOURCES mpihelpertest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4 8
              TIMEOUT 300)

dune_add_test(NAME mpihelpertest2
              SOURCES mpihelpertest.cc
              COMPILE_DEFINITIONS MPIHELPER_PREINITIALIZE
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4 8
              TIMEOUT 300)

dune_add_test(SOURCES overloadsettest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(NAME parameterizedobjecttest
              SOURCES parameterizedobjecttest.cc parameterizedobjectfactorysingleton.cc
              LINK_LIBRARIES dunecommon
              )

dune_add_test(SOURCES parametertreelocaletest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES parametertreetest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES pathtest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES poolallocatortest.cc)

dune_add_test(SOURCES rangeutilitiestest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES reservedvectortest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES shared_ptrtest.cc)

dune_add_test(SOURCES singletontest.cc)

dune_add_test(SOURCES sllisttest.cc)

dune_add_test(SOURCES stdapplytest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES stdtypetraitstest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES streamoperatorstest.cc)

dune_add_test(SOURCES streamtest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES stringutilitytest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              CMAKE_GUARD HAVE_MPROTECT)

dune_add_test(NAME testdebugallocator_fail1
              SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE1;EXPECTED_SIGNAL=SIGSEGV;EXPECTED_ALT_SIGNAL=SIGBUS"
              EXPECT_FAIL
              CMAKE_GUARD HAVE_MPROTECT)

dune_add_test(NAME testdebugallocator_fail2
              SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE2;EXPECTED_SIGNAL=SIGABRT"
              EXPECT_FAIL
              CMAKE_GUARD HAVE_MPROTECT)

dune_add_test(NAME testdebugallocator_fail3
              SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE3;EXPECTED_SIGNAL=SIGABRT"
              EXPECT_FAIL
              CMAKE_GUARD HAVE_MPROTECT)

dune_add_test(NAME testdebugallocator_fail4
              SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE4;DEBUG_ALLOCATOR_KEEP=1;EXPECTED_SIGNAL=SIGSEGV;EXPECTED_ALT_SIGNAL=SIGBUS"
              EXPECT_FAIL
              CMAKE_GUARD HAVE_MPROTECT)

dune_add_test(NAME testdebugallocator_fail5
              SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE5;EXPECTED_SIGNAL=SIGSEGV;EXPECTED_ALT_SIGNAL=SIGBUS"
              EXPECT_FAIL
              CMAKE_GUARD HAVE_MPROTECT)

dune_add_test(SOURCES testfloatcmp.cc)

dune_add_test(SOURCES to_unique_ptrtest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES tupleutilitytest.cc)

dune_add_test(SOURCES typeutilitytest.cc)

dune_add_test(SOURCES typelisttest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES utilitytest.cc)

dune_add_test(SOURCES eigenvaluestest.cc
              LINK_LIBRARIES dunecommon
              CMAKE_GUARD LAPACK_FOUND)

dune_add_test(SOURCES optionaltest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES versiontest.cc
              LINK_LIBRARIES dunecommon)

install(
  FILES
  arithmetictestsuite.hh
  collectorstream.hh
  testsuite.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/test)
