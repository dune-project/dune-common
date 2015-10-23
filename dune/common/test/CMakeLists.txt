dune_add_test(SOURCES arraylisttest.cc)

dune_add_test(SOURCES arraytest.cc)

dune_add_test(SOURCES bigunsignedinttest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES bitsetvectortest.cc)

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

dune_add_test(SOURCES classnametest.cc)

dune_add_test(SOURCES conversiontest.cc)

dune_add_test(SOURCES diagonalmatrixtest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES dynmatrixtest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES dynvectortest.cc)

dune_add_test(SOURCES enumsettest.cc)

dune_add_test(NAME fmatrixtest
              SOURCES fmatrixtest.cc dummy.f
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES fvectortest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES genericiterator_compile_fail.cc
              EXPECT_COMPILE_FAIL)

dune_add_test(SOURCES gcdlcmtest.cc)

dune_add_test(SOURCES integersequence.cc)

dune_add_test(SOURCES iteratorfacadetest2.cc)

dune_add_test(SOURCES iteratorfacadetest.cc)

dune_add_test(SOURCES lrutest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES mpicollectivecommunication.cc
              LINK_LIBRARIES dunecommon
              SKIP_ON_77)

dune_add_test(SOURCES mpiguardtest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES mpihelpertest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(NAME mpihelpertest2
              SOURCES mpihelpertest.cc
              COMPILE_DEFINITIONS MPIHELPER_PREINITIALIZE
              LINK_LIBRARIES dunecommon
              )

dune_add_test(SOURCES parametertreelocaletest.cc
              LINK_LIBRARIES dunecommon
              SKIP_ON_77)

dune_add_test(SOURCES parametertreetest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES pathtest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES poolallocatortest.cc)

dune_add_test(SOURCES shared_ptrtest.cc)

dune_add_test(SOURCES singletontest.cc)

dune_add_test(SOURCES sllisttest.cc)

dune_add_test(SOURCES streamtest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES stringutilitytest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(NAME testdebugallocator_fail1
              SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE1;EXPECTED_SIGNAL=SIGSEGV"
              EXPECT_FAIL)

dune_add_test(NAME testdebugallocator_fail2
              SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE2;EXPECTED_SIGNAL=SIGABRT"
              EXPECT_FAIL)

dune_add_test(NAME testdebugallocator_fail3
              SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE3;EXPECTED_SIGNAL=SIGABRT"
              EXPECT_FAIL)

dune_add_test(NAME testdebugallocator_fail4
              SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE4;DEBUG_ALLOCATOR_KEEP=1;EXPECTED_SIGNAL=SIGSEGV"
              EXPECT_FAIL)

dune_add_test(NAME testdebugallocator_fail5
              SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE5;EXPECTED_SIGNAL=SIGSEGV"
              EXPECT_FAIL)

dune_add_test(SOURCES testfloatcmp.cc)

dune_add_test(SOURCES tuplestest.cc)

dune_add_test(SOURCES tupleutilitytest.cc)

dune_add_test(SOURCES typetraitstest.cc)

dune_add_test(SOURCES typetraitstest_deprecated.cc)

#dune_add_test(SOURCES utilitytest.cc)

if(${LAPACK_FOUND})
  dune_add_test(SOURCES eigenvaluestest.cc
                LINK_LIBRARIES dunecommon
                SKIP_ON_77)
endif()
