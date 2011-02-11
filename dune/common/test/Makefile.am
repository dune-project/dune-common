# -*- tab-width: 4; indent-tabs-mode: nil -*-
# $Id$

TESTPROGS = \
    arraylisttest \
    arraytest \
    bigunsignedinttest \
    bitsetvectortest \
    conversiontest \
    deprtuplestest \
    dynmatrixtest \
    dynvectortest \
    enumsettest \
    fassigntest \
    float_cmp \
    fmatrixtest \
    fvectortest \
    gcdlcmtest \
    iteratorfacadetest \
    iteratorfacadetest2 \
    lrutest \
    mpicollcomm \
    mpiguardtest \
    mpihelpertest \
    mpihelpertest2 \
    nullptr-test \
    pathtest \
    paramtreetest \
    poolallocatortest \
    shared_ptrtest_config \
    shared_ptrtest_dune \
    singletontest \
    static_assert_test \
    streamtest \
    test-stack \
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
    tuplestest_dune \
    tuplestest_std \
    tuplestest_tr1 \
    tupleutilitytest \
    utilitytest

# which tests to run
COMPILE_XFAIL=$(DUNE_COMMON_BIN)/xfail-compile-tests

COMPILE_XFAIL_TESTS = genericiterator_compile_fail nullptr-test-fail static_assert_test_fail

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

XFAIL_TESTS = testfassign_fail1 testfassign_fail2 testfassign_fail3 \
    testfassign_fail4 testfassign_fail5 testfassign_fail6

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS)

noinst_HEADERS = dummyiterator.hh

# define the programs
pathtest_SOURCES = pathtest.cc

paramtreetest_SOURCES = paramtreetest.cc

bitsetvectortest_SOURCES = bitsetvectortest.cc

nullptr_test_SOURCES = nullptr-test.cc nullptr-test2.cc
nullptr_test_fail_SOURCES = nullptr-test.cc
nullptr_test_fail_CPPFLAGS = $(AM_CPPFLAGS) -DFAIL

static_assert_test_SOURCES = static_assert_test.cc
static_assert_test_fail_SOURCES = static_assert_test_fail.cc

fassigntest_SOURCES = fassigntest.cc

bigunsignedinttest_SOURCES=bigunsignedinttest.cc

lrutest_SOURCES = lrutest.cc

sllisttest_SOURCES = sllisttest.cc

test_stack_SOURCES = test-stack.cc

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

deprtuplestest_SOURCES = deprtuplestest.cc

streamtest_SOURCES = streamtest.cc

# mention headers so that they are distributed too
iteratorfacadetest_SOURCES = iteratorfacadetest.cc iteratorfacadetest.hh \
   iteratortest.hh

iteratorfacadetest2_SOURCES = iteratorfacadetest2.cc

dynmatrixtest_SOURCES = dynmatrixtest.cc

dynvectortest_SOURCES = dynvectortest.cc

fmatrixtest_SOURCES = fmatrixtest.cc
fmatrixtest_LDADD = $(LAPACK_LIBS) $(LDADD)

fvectortest_SOURCES = fvectortest.cc

poolallocatortest_SOURCES = poolallocatortest.cc

enumsettest_SOURCES=enumsettest.cc

gcdlcmtest_SOURCES = gcdlcmtest.cc

mpihelpertest_SOURCES = mpihelpertest.cc
mpihelpertest_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
mpihelpertest_LDADD = $(DUNEMPILIBS) $(LDADD)
mpihelpertest_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

mpihelpertest2_SOURCES = mpihelpertest.cc
mpihelpertest2_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) -DMPIHELPER_PREINITIALZE
mpihelpertest2_LDADD = $(DUNEMPILIBS) $(LDADD)
mpihelpertest2_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

mpicollcomm_SOURCES = mpicollectivecommunication.cc
mpicollcomm_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
mpicollcomm_LDADD = $(DUNEMPILIBS) $(LDADD)
mpicollcomm_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

mpiguardtest_SOURCES = mpiguardtest.cc
mpiguardtest_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
mpiguardtest_LDADD = $(DUNEMPILIBS) $(LDADD)
mpiguardtest_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

singletontest_SOURCES = singletontest.cc

utilitytest_SOURCES = utilitytest.cc

testfassign1_SOURCES = testfassign.cc testfassign2.cc
testfassign1_CPPFLAGS = $(AM_CPPFLAGS) -D_N=3 -D_VALUES="1,2,3"

testfassign2_SOURCES = testfassign.cc
testfassign2_CPPFLAGS = $(AM_CPPFLAGS) -D_N=3 -D_VALUES="1,zero"

testfassign3_SOURCES = testfassign.cc
testfassign3_CPPFLAGS = $(AM_CPPFLAGS) -D_N=3 -D_VALUES="zero"

testfassign4_SOURCES = testfassign.cc
testfassign4_CPPFLAGS = $(AM_CPPFLAGS) -D_N=2 -D_M=3 -D_VALUES="1, zero, nextRow, 2, 3, 4"

testfassign_fail1_SOURCES = testfassign.cc
testfassign_fail1_CPPFLAGS = $(AM_CPPFLAGS) -D_N=3 -D_VALUES="1,2"

testfassign_fail2_SOURCES = testfassign.cc
testfassign_fail2_CPPFLAGS = $(AM_CPPFLAGS) -D_N=3 -D_VALUES="1,2,3,4"

testfassign_fail3_SOURCES = testfassign.cc
testfassign_fail3_CPPFLAGS = $(AM_CPPFLAGS) -D_N=2 -D_M=2 -D_VALUES="1, nextRow, 2, 3"

testfassign_fail4_SOURCES = testfassign.cc
testfassign_fail4_CPPFLAGS = $(AM_CPPFLAGS) -D_N=2 -D_M=2 -D_VALUES="1, 2, 3, nextRow, 2, 3"

testfassign_fail5_SOURCES = testfassign.cc
testfassign_fail5_CPPFLAGS = $(AM_CPPFLAGS) -D_N=2 -D_M=2 -D_VALUES="1, 2"

testfassign_fail6_SOURCES = testfassign.cc
testfassign_fail6_CPPFLAGS = $(AM_CPPFLAGS) -D_N=2 -D_M=2 -D_VALUES="1, 2, nextRow, 2, 3, nextRow, 4, 5"

conversiontest_SOURCES = conversiontest.cc

sourcescheck_NOSOURCES = exprtmpl.cc timing.cc

float_cmp_SOURCES = float_cmp.cc

genericiterator_compile_fail_SOURCES = genericiterator_compile_fail.cc

include $(top_srcdir)/am/global-rules
