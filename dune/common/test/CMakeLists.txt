# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

include(DuneInstance)

dune_add_test(SOURCES arithmetictestsuitetest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES arraylisttest.cc
              LABELS quick)

dune_add_test(SOURCES arraytest.cc
              LABELS quick)

dune_add_test(SOURCES assertandreturntest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)
dune_add_test(NAME assertandreturntest_compiletime_fail
              SOURCES assertandreturntest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "TEST_COMPILETIME_FAIL"
              EXPECT_COMPILE_FAIL
              LABELS quick)
dune_add_test(NAME assertandreturntest_ndebug
              SOURCES assertandreturntest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "TEST_NDEBUG"
              LABELS quick)

dune_add_test(SOURCES autocopytest.cc
              LABELS quick)

dune_add_test(SOURCES bigunsignedinttest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES bitsetvectortest.cc
              LABELS quick)

dune_add_test(SOURCES boundscheckingtest.cc
              COMPILE_DEFINITIONS DUNE_CHECK_BOUNDS=1
              LINK_LIBRARIES dunecommon
              LABELS quick)
dune_add_test(SOURCES boundscheckingmvtest.cc
              COMPILE_DEFINITIONS DUNE_CHECK_BOUNDS=1
              LINK_LIBRARIES dunecommon
              LABELS quick)
dune_add_test(SOURCES boundscheckingoptest.cc
              COMPILE_DEFINITIONS DUNE_CHECK_BOUNDS=1
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES calloncetest.cc
              LINK_LIBRARIES dunecommon
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
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(NAME classnametest-fallback
              SOURCES classnametest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS DISABLE_CXA_DEMANGLE
              LABELS quick)

dune_add_test(SOURCES concept.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES constexprifelsetest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES debugaligntest.cc
              LINK_LIBRARIES dunecommon
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
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)
dune_add_test(NAME densematrixassignmenttest_fail0
              SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE0"
              EXPECT_COMPILE_FAIL
              LABELS quick)
dune_add_test(NAME densematrixassignmenttest_fail1
              SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE1"
              EXPECT_COMPILE_FAIL
              LABELS quick)
dune_add_test(NAME densematrixassignmenttest_fail2
              SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE2"
              EXPECT_COMPILE_FAIL
              LABELS quick)
dune_add_test(NAME densematrixassignmenttest_fail3
              SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE3"
              EXPECT_COMPILE_FAIL
              LABELS quick)
dune_add_test(NAME densematrixassignmenttest_fail4
              SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE4"
              EXPECT_COMPILE_FAIL
              LABELS quick)
dune_add_test(NAME densematrixassignmenttest_fail5
              SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE5"
              EXPECT_COMPILE_FAIL
              LABELS quick)
dune_add_test(NAME densematrixassignmenttest_fail6
              SOURCES densematrixassignmenttest.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE6"
              EXPECT_COMPILE_FAIL
              LABELS quick)

dune_add_test(SOURCES densevectorassignmenttest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES diagonalmatrixtest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES dynmatrixtest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES dynvectortest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES densevectortest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES enumsettest.cc
              LABELS quick)

dune_add_test(SOURCES filledarraytest.cc
              LABELS quick)

dune_add_test(SOURCES fmatrixtest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)
add_dune_vc_flags(fmatrixtest)

dune_add_test(SOURCES functiontest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES fvectortest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES fvectorconversion1d.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES genericiterator_compile_fail.cc
              EXPECT_COMPILE_FAIL
              LABELS quick)

dune_add_test(SOURCES hybridutilitiestest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES indicestest.cc
              LABELS quick)

dune_add_test(SOURCES iscallabletest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES iteratorfacadetest2.cc
              LABELS quick)

dune_add_test(SOURCES iteratorfacadetest.cc
              LABELS quick)

dune_add_test(SOURCES lrutest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES mathclassifierstest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES metistest.cc
              CMAKE_GUARD METIS_FOUND)
add_dune_metis_flags(metistest)

dune_add_test(SOURCES mpicommunicationtest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4 8
              TIMEOUT 300
              LABELS quick)

dune_add_test(SOURCES mpiguardtest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4 8
              TIMEOUT 300
              LABELS quick)

dune_add_test(SOURCES mpihelpertest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4 8
              TIMEOUT 300
              LABELS quick)

dune_add_test(NAME mpihelpertest2
              SOURCES mpihelpertest.cc
              COMPILE_DEFINITIONS MPIHELPER_PREINITIALIZE
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4 8
              TIMEOUT 300
              LABELS quick)

dune_add_test(SOURCES overloadsettest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(NAME parameterizedobjecttest
              SOURCES parameterizedobjecttest.cc parameterizedobjectfactorysingleton.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES parametertreelocaletest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES parametertreetest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES parmetistest.cc
              MPI_RANKS 3
              TIMEOUT 300
              CMAKE_GUARD ParMETIS_FOUND
              LABELS quick)
add_dune_parmetis_flags(parmetistest)

dune_add_test(SOURCES pathtest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES poolallocatortest.cc
              LABELS quick)

dune_add_test(SOURCES powertest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES quadmathtest.cc
              LINK_LIBRARIES dunecommon
              CMAKE_GUARD HAVE_QUADMATH)

dune_add_test(SOURCES rangeutilitiestest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES referencehelpertest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES reservedvectortest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES scotchtest.cc
              LINK_LIBRARIES dunecommon
              CMAKE_GUARD PTScotch_FOUND
              LABELS quick)

dune_add_test(SOURCES shared_ptrtest.cc
              LABELS quick)

dune_add_test(SOURCES singletontest.cc
              LABELS quick)

dune_add_test(SOURCES sllisttest.cc
              LABELS quick)

dune_add_test(SOURCES stdidentity.cc
              LABELS quick)

dune_add_test(SOURCES stdapplytest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES stdchecktypes.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES streamoperatorstest.cc
              LABELS quick)

dune_add_test(SOURCES streamtest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES stringutilitytest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(NAME testdebugallocator_fail1
              SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE1;EXPECTED_SIGNAL=SIGSEGV;EXPECTED_ALT_SIGNAL=SIGBUS"
              EXPECT_FAIL
              LABELS quick)

dune_add_test(NAME testdebugallocator_fail2
              SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE2;EXPECTED_SIGNAL=SIGABRT"
              EXPECT_FAIL
              LABELS quick)

dune_add_test(NAME testdebugallocator_fail3
              SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE3;EXPECTED_SIGNAL=SIGABRT"
              EXPECT_FAIL
              LABELS quick)

dune_add_test(NAME testdebugallocator_fail4
              SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE4;DEBUG_ALLOCATOR_KEEP=1;EXPECTED_SIGNAL=SIGSEGV;EXPECTED_ALT_SIGNAL=SIGBUS"
              EXPECT_FAIL
              LABELS quick)

dune_add_test(NAME testdebugallocator_fail5
              SOURCES testdebugallocator.cc
              LINK_LIBRARIES dunecommon
              COMPILE_DEFINITIONS "FAILURE5;EXPECTED_SIGNAL=SIGSEGV;EXPECTED_ALT_SIGNAL=SIGBUS"
              EXPECT_FAIL
              LABELS quick)

dune_add_test(SOURCES testfloatcmp.cc
              LABELS quick)

dune_add_test(SOURCES transposetest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES tupleutilitytest.cc
              LABELS quick)

dune_add_test(SOURCES typeutilitytest.cc
              LABELS quick)

dune_add_test(SOURCES typelisttest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES utilitytest.cc
              LABELS quick)

dune_add_test(SOURCES eigenvaluestest.cc
              LINK_LIBRARIES dunecommon
              CMAKE_GUARD LAPACK_FOUND
              LABELS quick)

dune_add_test(SOURCES versiontest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES mathtest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES vcexpectedimpltest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick
              CMAKE_GUARD Vc_FOUND)
add_dune_vc_flags(vcexpectedimpltest)

dune_add_test(SOURCES alignedallocatortest.cc
              LINK_LIBRARIES dunecommon)

install(
  FILES
  arithmetictestsuite.hh
  collectorstream.hh
  testsuite.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/test)
