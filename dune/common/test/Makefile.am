# -*- tab-width: 4; indent-tabs-mode: nil -*-
# $Id$

TESTPROGS = \
    arraylisttest \
    arraytest \
    bigunsignedinttest \
    bitsetvectortest \
    check_fvector_size \
    conversiontest \
    diagonalmatrixtest \
    dynmatrixtest \
    dynvectortest \
    eigenvaluestest \
    enumsettest \
    fassigntest \
    fmatrixtest \
    fvectortest \
    gcdlcmtest \
    iteratorfacadetest \
    iteratorfacadetest2 \
    lrutest \
    mpicollectivecommunication \
    mpiguardtest \
    mpihelpertest \
    mpihelpertest2 \
    nullptr-test \
    pathtest \
    parametertreetest \
    poolallocatortest \
    shared_ptrtest_config \
    shared_ptrtest_dune \
    singletontest \
    static_assert_test \
    streamtest \
    testdebugallocator \
    testdebugallocator_fail1 \
    testdebugallocator_fail2 \
    testdebugallocator_fail3 \
    testdebugallocator_fail4 \
    testdebugallocator_fail5 \
    testfassign1 \
    testfassign2 \
    testfassign3 \
    testfassign4 \
    testfassign_fail1 \
    testfassign_fail2 \
    testfassign_fail3 \
    testfassign_fail4 \
    testfassign_fail5 \
    testfassign_fail6 \
    testfconstruct \
    testfloatcmp \
    tuplestest_dune \
    tuplestest_std \
    tuplestest_tr1 \
    tupleutilitytest \
    utilitytest

# which tests to run
COMPILE_XFAIL=$(DUNE_COMMON_BIN)/xfail-compile-tests

COMPILE_XFAIL_TESTS = \
    check_fvector_size_fail1 \
    check_fvector_size_fail2 \
    genericiterator_compile_fail \
    nullptr-test-fail \
    static_assert_test_fail \
    testfconstruct_fail1 \
    testfconstruct_fail2

compile_XFAIL:
	for i in $(COMPILE_XFAIL_TESTS); do \
	  if $(MAKE) $$i; then \
	    echo "ERROR: target $$i did compile, although it is listed in the COMPILE_XFAIL_TESTS"; \
	    exit 1; \
	  else true; \
	  fi; \
	done

EXTRA_PROGRAMS = $(COMPILE_XFAIL_TESTS) sllisttest

TESTS = $(TESTPROGS) $(COMPILE_XFAIL)

XFAIL_TESTS = \
    testdebugallocator_fail1 \
    testdebugallocator_fail2 \
    testdebugallocator_fail3 \
    testdebugallocator_fail4 \
    testdebugallocator_fail5 \
    testfassign_fail1 \
    testfassign_fail2 \
    testfassign_fail3 \
    testfassign_fail4 \
    testfassign_fail5 \
    testfassign_fail6

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS)

noinst_HEADERS = dummyiterator.hh iteratorfacadetest.hh

# Install some test headers, because they get used by tests in other modules
testincludedir = $(includedir)/dune/common/test
testinclude_HEADERS = checkmatrixinterface.hh iteratortest.hh

# define the programs
pathtest_SOURCES = pathtest.cc

parametertreetest_SOURCES = parametertreetest.cc

bitsetvectortest_SOURCES = bitsetvectortest.cc

diagonalmatrixtest_SOURCES = diagonalmatrixtest.cc

nullptr_test_SOURCES = nullptr-test.cc nullptr-test2.cc
nullptr_test_fail_SOURCES = nullptr-test.cc
nullptr_test_fail_CPPFLAGS = $(AM_CPPFLAGS) -DFAIL

static_assert_test_SOURCES = static_assert_test.cc
static_assert_test_fail_SOURCES = static_assert_test_fail.cc

fassigntest_SOURCES = fassigntest.cc

bigunsignedinttest_SOURCES=bigunsignedinttest.cc
bigunsignedinttest_CPPFLAGS = $(AM_CPPFLAGS) $(BOOST_CPPFLAGS)

lrutest_SOURCES = lrutest.cc

sllisttest_SOURCES = sllisttest.cc

arraylisttest_SOURCES = arraylisttest.cc

arraytest_SOURCES = arraytest.cc

shared_ptrtest_config_SOURCES = shared_ptrtest.cc

shared_ptrtest_dune_SOURCES = shared_ptrtest.cc
shared_ptrtest_dune_CPPFLAGS = $(AM_CPPFLAGS)   \
    -DDISABLE_CONFIGURED_SHARED_PTR

tuplestest_dune_SOURCES = tuplestest.cc
tuplestest_dune_CPPFLAGS = $(AM_CPPFLAGS)       \
    -DDISABLE_TR1_TUPLE -DDISABLE_STD_TUPLE

tuplestest_std_SOURCES = tuplestest.cc

tuplestest_tr1_SOURCES = tuplestest.cc
tuplestest_tr1_CPPFLAGS = $(AM_CPPFLAGS)        \
    -DDISABLE_STD_TUPLE

tupleutilitytest_SOURCES = tupleutilitytest.cc

streamtest_SOURCES = streamtest.cc

# mention headers so that they are distributed too
iteratorfacadetest_SOURCES = iteratorfacadetest.cc iteratorfacadetest.hh \
   iteratortest.hh

iteratorfacadetest2_SOURCES = iteratorfacadetest2.cc

dynmatrixtest_SOURCES = dynmatrixtest.cc

dynvectortest_SOURCES = dynvectortest.cc

eigenvaluestest_SOURCES = eigenvaluestest.cc
eigenvaluestest_LDADD = $(LAPACK_LIBS) $(LDADD) $(BLAS_LIBS) $(LIBS) $(FLIBS)

fmatrixtest_SOURCES = fmatrixtest.cc
fmatrixtest_LDADD = $(LAPACK_LIBS) $(LDADD) $(BLAS_LIBS) $(LIBS) $(FLIBS)

fvectortest_SOURCES = fvectortest.cc

check_fvector_size_fail1_SOURCES = check_fvector_size_fail.cc
check_fvector_size_fail1_CPPFLAGS = $(AM_CPPFLAGS) -DDIM=1

check_fvector_size_fail2_SOURCES = check_fvector_size_fail.cc
check_fvector_size_fail2_CPPFLAGS = $(AM_CPPFLAGS) -DDIM=3

check_fvector_size_SOURCES = check_fvector_size.cc

poolallocatortest_SOURCES = poolallocatortest.cc

enumsettest_SOURCES=enumsettest.cc

gcdlcmtest_SOURCES = gcdlcmtest.cc

mpihelpertest_SOURCES = mpihelpertest.cc
mpihelpertest_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
mpihelpertest_LDADD = $(DUNEMPILIBS) $(LDADD)
mpihelpertest_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

mpihelpertest2_SOURCES = mpihelpertest.cc
mpihelpertest2_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) -DMPIHELPER_PREINITIALIZE
mpihelpertest2_LDADD = $(DUNEMPILIBS) $(LDADD)
mpihelpertest2_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

mpicollectivecommunication_SOURCES = mpicollectivecommunication.cc
mpicollectivecommunication_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
mpicollectivecommunication_LDADD = $(DUNEMPILIBS) $(LDADD)
mpicollectivecommunication_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

mpiguardtest_SOURCES = mpiguardtest.cc
mpiguardtest_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
mpiguardtest_LDADD = $(DUNEMPILIBS) $(LDADD)
mpiguardtest_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

singletontest_SOURCES = singletontest.cc

utilitytest_SOURCES = utilitytest.cc

testdebugallocator_SOURCES = testdebugallocator.cc
testdebugallocator_CPPFLAGS = $(AM_CPPFLAGS)

testdebugallocator_fail1_SOURCES = testdebugallocator.cc
testdebugallocator_fail1_CPPFLAGS = $(AM_CPPFLAGS) -DFAILURE1

testdebugallocator_fail2_SOURCES = testdebugallocator.cc
testdebugallocator_fail2_CPPFLAGS = $(AM_CPPFLAGS) -DFAILURE2

testdebugallocator_fail3_SOURCES = testdebugallocator.cc
testdebugallocator_fail3_CPPFLAGS = $(AM_CPPFLAGS) -DFAILURE3

testdebugallocator_fail4_SOURCES = testdebugallocator.cc
testdebugallocator_fail4_CPPFLAGS = $(AM_CPPFLAGS) -DFAILURE4 -DDEBUG_ALLOCATOR_KEEP=1

testdebugallocator_fail5_SOURCES = testdebugallocator.cc
testdebugallocator_fail5_CPPFLAGS = $(AM_CPPFLAGS) -DFAILURE5

testfassign1_SOURCES = testfassign.cc testfassign2.cc
testfassign1_CPPFLAGS = $(AM_CPPFLAGS) -D_DUNE_TEST_N=3 -D_VALUES="1,2,3"

testfassign2_SOURCES = testfassign.cc
testfassign2_CPPFLAGS = $(AM_CPPFLAGS) -D_DUNE_TEST_N=3 -D_VALUES="1,zero"

testfassign3_SOURCES = testfassign.cc
testfassign3_CPPFLAGS = $(AM_CPPFLAGS) -D_DUNE_TEST_N=3 -D_VALUES="zero"

testfassign4_SOURCES = testfassign.cc
testfassign4_CPPFLAGS = $(AM_CPPFLAGS) -D_DUNE_TEST_N=2 -D_DUNE_TEST_M=3 -D_VALUES="1,zero,nextRow,2,3,4"

testfassign_fail1_SOURCES = testfassign.cc
testfassign_fail1_CPPFLAGS = $(AM_CPPFLAGS) -D_DUNE_TEST_N=3 -D_VALUES="1,2"

testfassign_fail2_SOURCES = testfassign.cc
testfassign_fail2_CPPFLAGS = $(AM_CPPFLAGS) -D_DUNE_TEST_N=3 -D_VALUES="1,2,3,4"

testfassign_fail3_SOURCES = testfassign.cc
testfassign_fail3_CPPFLAGS = $(AM_CPPFLAGS) -D_DUNE_TEST_N=2 -D_DUNE_TEST_M=2 -D_VALUES="1,nextRow,2,3"

testfassign_fail4_SOURCES = testfassign.cc
testfassign_fail4_CPPFLAGS = $(AM_CPPFLAGS) -D_DUNE_TEST_N=2 -D_DUNE_TEST_M=2 -D_VALUES="1,2,3,nextRow,2,3"

testfassign_fail5_SOURCES = testfassign.cc
testfassign_fail5_CPPFLAGS = $(AM_CPPFLAGS) -D_DUNE_TEST_N=2 -D_DUNE_TEST_M=2 -D_VALUES="1,2"

testfassign_fail6_SOURCES = testfassign.cc
testfassign_fail6_CPPFLAGS = $(AM_CPPFLAGS) -D_DUNE_TEST_N=2 -D_DUNE_TEST_M=2 -D_VALUES="1,2,nextRow,2,3,nextRow,4,5"

testfconstruct_SOURCES = testfconstruct.cc
testfconstruct_CPPFLAGS = $(AM_CPPFLAGS) -DFVSIZE=3

testfconstruct_fail1_SOURCES = testfconstruct.cc
testfconstruct_fail1_CPPFLAGS = $(AM_CPPFLAGS) -DFVSIZE=2

testfconstruct_fail2_SOURCES = testfconstruct.cc
testfconstruct_fail2_CPPFLAGS = $(AM_CPPFLAGS) -DFVSIZE=5

conversiontest_SOURCES = conversiontest.cc

sourcescheck_NOSOURCES = timing.cc

testfloatcmp_SOURCES = testfloatcmp.cc

genericiterator_compile_fail_SOURCES = genericiterator_compile_fail.cc

include $(top_srcdir)/am/global-rules

EXTRA_DIST = CMakeLists.txt dummy.f
