# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

include(DuneInstance)

# Link all test targets in this directory against Dune::Common
link_libraries(Dune::Common)

dune_add_test(SOURCES arithmetictestsuitetest.cc
              LABELS quick)

dune_add_test(SOURCES arraylisttest.cc
              LABELS quick)

dune_add_test(SOURCES assertandreturntest.cc
              LABELS quick)
dune_add_test(NAME assertandreturntest_compiletime_fail
              SOURCES assertandreturntest.cc
              COMPILE_DEFINITIONS "TEST_COMPILETIME_FAIL"
              EXPECT_COMPILE_FAIL
              LABELS quick)
dune_add_test(NAME assertandreturntest_ndebug
              SOURCES assertandreturntest.cc
              COMPILE_DEFINITIONS "TEST_NDEBUG"
              LABELS quick)

dune_add_test(SOURCES autocopytest.cc
              LABELS quick)

dune_add_test(SOURCES bigunsignedinttest.cc
              LABELS quick)

dune_add_test(SOURCES bitsetvectortest.cc
              LABELS quick)

dune_add_test(SOURCES boundscheckingtest.cc
              COMPILE_DEFINITIONS DUNE_CHECK_BOUNDS=1
              LABELS quick)
dune_add_test(SOURCES boundscheckingmvtest.cc
              COMPILE_DEFINITIONS DUNE_CHECK_BOUNDS=1
              LABELS quick)
dune_add_test(SOURCES boundscheckingoptest.cc
              COMPILE_DEFINITIONS DUNE_CHECK_BOUNDS=1
              LABELS quick)

dune_add_test(SOURCES calloncetest.cc
              LABELS quick)

dune_add_test(SOURCES check_fvector_size.cc
              LABELS quick)

dune_add_test(NAME check_fvector_size_fail1
              SOURCES check_fvector_size_fail.cc
              COMPILE_DEFINITIONS DIM=1
              EXPECT_COMPILE_FAIL
              LABELS quick)

dune_add_test(NAME check_fvector_size_fail2
              SOURCES check_fvector_size_fail.cc
              COMPILE_DEFINITIONS DIM=3
              EXPECT_COMPILE_FAIL
              LABELS quick)

dune_add_test(NAME classnametest-demangled
              SOURCES classnametest.cc
              LABELS quick)

dune_add_test(NAME classnametest-fallback
              SOURCES classnametest.cc
              COMPILE_DEFINITIONS DISABLE_CXA_DEMANGLE
              LABELS quick)

dune_add_test(SOURCES concept.cc
              LABELS quick)

dune_add_test(SOURCES concepts.cc
              LABELS quick)

dune_add_test(SOURCES constexprifelsetest.cc
              LABELS quick)

dune_add_test(SOURCES copyableoptionaltest.cc
              LABELS quick)

dune_add_test(SOURCES debugaligntest.cc
              LABELS quick)

# Generate files with instantiations, external declarations, and also the
# invocations in the test for each instance.
dune_instance_begin(FILES debugalignsimdtest.hh debugalignsimdtest.cc)
foreach(SCALAR IN ITEMS double bool)
  dune_instance_add(ID "${SCALAR}")
  foreach(POINT IN ITEMS Type BinaryOpsScalarVector BinaryOpsVectorScalar)
    dune_instance_add(TEMPLATE POINT
                      ID "${POINT}_${SCALAR}"
                      FILES debugalignsimd.cc)
  endforeach()
endforeach()
dune_instance_end()
list(FILTER DUNE_INSTANCE_GENERATED INCLUDE REGEX [[\.cc$]])
dune_add_test(NAME debugalignsimdtest
              SOURCES ${DUNE_INSTANCE_GENERATED}
              LABELS quick)

dune_add_test(SOURCES densematrixassignmenttest.cc
              LABELS quick)
add_dune_gmp_flags(densematrixassignmenttest)

foreach(FAIL RANGE 0 6)
  dune_add_test(NAME densematrixassignmenttest_fail${FAIL}
                SOURCES densematrixassignmenttest.cc
                COMPILE_DEFINITIONS "FAILURE${FAIL}"
                EXPECT_COMPILE_FAIL
                LABELS quick)
  add_dune_gmp_flags(densematrixassignmenttest)
endforeach()

dune_add_test(SOURCES densevectorassignmenttest.cc
              LABELS quick)

dune_add_test(SOURCES diagonalmatrixtest.cc
              LABELS quick)

dune_add_test(SOURCES dunethrowtest.cc
              LABELS quick)

dune_add_test(SOURCES dynmatrixtest.cc
              LABELS quick)

dune_add_test(SOURCES dynvectortest.cc
              LABELS quick)

dune_add_test(SOURCES densevectortest.cc
              LABELS quick)

dune_add_test(SOURCES enumsettest.cc
              LABELS quick)

dune_add_test(SOURCES filledarraytest.cc
              LABELS quick)

dune_add_test(SOURCES fmatrixtest.cc
              LABELS quick)
add_dune_vc_flags(fmatrixtest)

dune_add_test(SOURCES forceinlinetest.cc
              LABELS quick)

dune_add_test(SOURCES fvectortest.cc
              LABELS quick)

dune_add_test(SOURCES fvectorconversion1d.cc
              LABELS quick)

dune_add_test(SOURCES genericiterator_compile_fail.cc
              EXPECT_COMPILE_FAIL
              LABELS quick)

dune_add_test(SOURCES hybridutilitiestest.cc
              LABELS quick)

dune_add_test(SOURCES indexediteratortest.cc
              LABELS quick)

dune_add_test(SOURCES indicestest.cc
              LABELS quick)

dune_add_test(SOURCES integersequencetest.cc
              LABELS quick)

dune_add_test(SOURCES iscallabletest.cc
              LABELS quick)

dune_add_test(SOURCES iteratorfacadetest2.cc
              LABELS quick)

dune_add_test(SOURCES iteratorfacadetest.cc
              LABELS quick)

dune_add_test(SOURCES lrutest.cc
              LABELS quick)

dune_add_test(SOURCES mathclassifierstest.cc)

dune_add_test(SOURCES metistest.cc
              CMAKE_GUARD METIS_FOUND
              LABELS quick)
add_dune_metis_flags(metistest)

dune_add_test(SOURCES mpicommunicationtest.cc
              MPI_RANKS 1 2 4 8
              TIMEOUT 300
              LABELS quick)

dune_add_test(SOURCES mpiguardtest.cc
              MPI_RANKS 1 2 4 8
              TIMEOUT 300
              LABELS quick)

dune_add_test(SOURCES mpihelpertest.cc
              MPI_RANKS 1 2 4 8
              TIMEOUT 300
              LABELS quick)

dune_add_test(NAME mpihelpertest2
              SOURCES mpihelpertest.cc
              COMPILE_DEFINITIONS MPIHELPER_PREINITIALIZE
              MPI_RANKS 1 2 4 8
              TIMEOUT 300
              LABELS quick)

dune_add_test(SOURCES overloadsettest.cc
              LABELS quick)

dune_add_test(NAME parameterizedobjecttest
              SOURCES parameterizedobjecttest.cc parameterizedobjectfactorysingleton.cc
              LABELS quick)

dune_add_test(SOURCES parametertreelocaletest.cc
              LABELS quick)

dune_add_test(SOURCES parametertreetest.cc
              LABELS quick)

dune_add_test(SOURCES pathtest.cc
              LABELS quick)

dune_add_test(SOURCES poolallocatortest.cc
              LABELS quick)

dune_add_test(SOURCES powertest.cc
              LABELS quick)

dune_add_test(SOURCES quadmathtest.cc
              CMAKE_GUARD HAVE_QUADMATH)
add_dune_quadmath_flags(quadmathtest)

dune_add_test(SOURCES rangeutilitiestest.cc
              LABELS quick)

dune_add_test(SOURCES referencehelpertest.cc
              LABELS quick)

dune_add_test(SOURCES reservedvectortest.cc
              LABELS quick)

dune_add_test(SOURCES scotchtest.cc
              CMAKE_GUARD PTScotch_FOUND
              LABELS quick)
add_dune_ptscotch_flags(scotchtest)

dune_add_test(SOURCES shared_ptrtest.cc
              LABELS quick)

dune_add_test(SOURCES singletontest.cc
              LABELS quick)

dune_add_test(SOURCES sllisttest.cc
              LABELS quick)

dune_add_test(SOURCES stdidentity.cc
              LABELS quick)

dune_add_test(SOURCES stdchecktypes.cc
              LABELS quick)

dune_add_test(SOURCES streamoperatorstest.cc
              LABELS quick)

dune_add_test(SOURCES streamtest.cc
              LABELS quick)

dune_add_test(SOURCES stringutilitytest.cc
              LABELS quick)

dune_add_test(SOURCES testdebugallocator.cc
              LABELS quick)

dune_add_test(NAME testdebugallocator_fail1
              SOURCES testdebugallocator.cc
              COMPILE_DEFINITIONS "FAILURE1;EXPECTED_SIGNAL=SIGSEGV;EXPECTED_ALT_SIGNAL=SIGBUS"
              EXPECT_FAIL
              LABELS quick)

dune_add_test(NAME testdebugallocator_fail2
              SOURCES testdebugallocator.cc
              COMPILE_DEFINITIONS "FAILURE2;EXPECTED_SIGNAL=SIGABRT"
              EXPECT_FAIL
              LABELS quick)

dune_add_test(NAME testdebugallocator_fail3
              SOURCES testdebugallocator.cc
              COMPILE_DEFINITIONS "FAILURE3;EXPECTED_SIGNAL=SIGABRT"
              EXPECT_FAIL
              LABELS quick)

dune_add_test(NAME testdebugallocator_fail4
              SOURCES testdebugallocator.cc
              COMPILE_DEFINITIONS "FAILURE4;DEBUG_ALLOCATOR_KEEP=1;EXPECTED_SIGNAL=SIGSEGV;EXPECTED_ALT_SIGNAL=SIGBUS"
              EXPECT_FAIL
              LABELS quick)

dune_add_test(NAME testdebugallocator_fail5
              SOURCES testdebugallocator.cc
              COMPILE_DEFINITIONS "FAILURE5;EXPECTED_SIGNAL=SIGSEGV;EXPECTED_ALT_SIGNAL=SIGBUS"
              EXPECT_FAIL
              LABELS quick)

dune_add_test(SOURCES testfloatcmp.cc
              LABELS quick)

dune_add_test(SOURCES transposetest.cc
              LABELS quick)

dune_add_test(SOURCES tupleutilitytest.cc
              LABELS quick)

dune_add_test(SOURCES typetraitstest.cc
              LABELS quick)

dune_add_test(SOURCES typeutilitytest.cc
              LABELS quick)

dune_add_test(SOURCES typelisttest.cc
              LABELS quick)

dune_add_test(SOURCES timertest.cc
              LABELS quick)

dune_add_test(SOURCES utilitytest.cc
              LABELS quick)

dune_add_test(SOURCES eigenvaluestest.cc
              CMAKE_GUARD LAPACK_FOUND
              LABELS quick)

dune_add_test(SOURCES versiontest.cc
              LABELS quick)

dune_add_test(SOURCES mathtest.cc
              LABELS quick)

dune_add_test(SOURCES vcexpectedimpltest.cc
              LABELS quick
              CMAKE_GUARD Vc_FOUND)
add_dune_vc_flags(vcexpectedimpltest)

dune_add_test(SOURCES alignedallocatortest.cc)

install(
  FILES
  arithmetictestsuite.hh
  collectorstream.hh
  testsuite.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/test)
