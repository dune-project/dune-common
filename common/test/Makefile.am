# $Id$

TESTPROGS = parsetest test-stack arraylisttest smartpointertest \
	sllisttest iteratorfacadetest tuplestest fvectortest fmatrixtest \
	poolallocatortest settest gcdlcmtest streamtest \
	bigunsignedinttest mpihelpertest singletontest mpicollcomm \
	utilitytest lrutest \
	smallobject nullptr-test \
	testfassign1 testfassign2 testfassign3 \
	testfassign4 \
    testfassign_fail1 testfassign_fail2 testfassign_fail3 \
    testfassign_fail4 testfassign_fail5 testfassign_fail6 \
    conversiontest blockbitfieldtest

# which tests to run
COMPILE_XFAIL=$(DUNE_COMMON_ROOT)/bin/xfail-compile-tests

COMPILE_XFAIL_TESTS = nullptr-test-fail static_assert_test

compile_XFAIL:
	for i in $(COMPILE_XFAIL_TESTS); do \
	  if $(MAKE) $$i; then \
	    echo "ERROR: target $$i did compile, although it is listed in the COMPILE_XFAIL_TESTS"; \
	    exit 1; \
	  else true; \
	  fi; \
	done

EXTRA_PROGRAMS = $(COMPILE_XFAIL_TESTS)

TESTS = $(TESTPROGS) $(COMPILE_XFAIL)

XFAIL_TESTS = testfassign_fail1 testfassign_fail2 testfassign_fail3 \
    testfassign_fail4 testfassign_fail5 testfassign_fail6

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS)

AM_LDFLAGS = $(LOCAL_LIBS)

# define the programs
smallobject_SOURCES = smallobject.cc

blockbitfieldtest_SOURCES = blockbitfieldtest.cc

nullptr_test_SOURCES = nullptr-test.cc nullptr-test2.cc
nullptr_test_fail_SOURCES = nullptr-test.cc
nullptr_test_fail_CPPFLAGS = -DFAIL

static_assert_test_SOURCES = static_assert_test.cc

bigunsignedinttest_SOURCES=bigunsignedinttest.cc

parsetest_SOURCES = parsetest.cc

lrutest_SOURCES = lrutest.cc

sllisttest_SOURCES = sllisttest.cc

test_stack_SOURCES = test-stack.cc

arraylisttest_SOURCES = arraylisttest.cc

smartpointertest_SOURCES = smartpointertest.cc

tuplestest_SOURCES = tuplestest.cc

streamtest_SOURCES = streamtest.cc

# mention headers so that they are distributed too
iteratorfacadetest_SOURCES = iteratorfacadetest.cc iteratorfacadetest.hh \
   iteratortest.hh

fvectortest_SOURCES = fvectortest.cc

fmatrixtest_SOURCES = fmatrixtest.cc

poolallocatortest_SOURCES = poolallocatortest.cc

settest_SOURCES=settest.cc

gcdlcmtest_SOURCES = gcdlcmtest.cc

mpihelpertest_SOURCES = mpihelpertest.cc
mpihelpertest_CXXFLAGS = $(MPI_CPPFLAGS)
mpihelpertest_LDFLAGS = $(LOCAL_LIBS) $(MPI_LDFLAGS) $(MPI_LIBS)

mpicollcomm_SOURCES = mpicollectivecommunication.cc
mpicollcomm_CXXFLAGS = $(MPI_CPPFLAGS)
mpicollcomm_LDFLAGS = $(LOCAL_LIBS) $(MPI_LDFLAGS) $(MPI_LIBS)

singletontest_SOURCES = singletontest.cc
singletontest_LDFLAGS = $(LOCAL_LIBS)

utilitytest_SOURCES = utilitytest.cc
utilitytest_LDFLAGS = $(LOCAL_LIBS)

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

include $(top_srcdir)/am/global-rules
