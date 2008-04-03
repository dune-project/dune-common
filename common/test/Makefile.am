# $Id$

TESTPROGS = parsetest test-stack arraylisttest smartpointertest \
	sllisttest iteratorfacadetest tuplestest fvectortest fmatrixtest \
	poolallocatortest settest gcdlcmtest streamtest \
	bigunsignedinttest mpihelpertest singletontest mpicollcomm \
	utilitytest lrutest \
	testfassign1 testfassign2 testfassign3 \
	testfassign4 \
    testfassign_fail1 testfassign_fail2 testfassign_fail3\
    testfassign_fail4 testfassign_fail5 testfassign_fail6


# which tests to run
TESTS = $(TESTPROGS)

XFAIL_TESTS = testfassign_fail1 testfassign_fail2 testfassign_fail3\
    testfassign_fail4 testfassign_fail5 testfassign_fail6

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS)

AM_LDFLAGS = $(LOCAL_LIBS)

# define the programs
bigunsignedinttest_SOURCES=bigunsignedinttest.cc
bigunsignedinttest_DEPENDENCIES = $(LOCAL_LIBS)

parsetest_SOURCES = parsetest.cc
parsetest_DEPENDENCIES = $(LOCAL_LIBS)

lrutest_SOURCES = lrutest.cc
lrutest_DEPENDENCIES = $(LOCAL_LIBS)

sllisttest_SOURCES = sllisttest.cc
sllisttest_DEPENDENCIES = $(LOCAL_LIBS)

test_stack_SOURCES = test-stack.cc
test_stack_DEPENDENCIES = $(LOCAL_LIBS)

arraylisttest_SOURCES = arraylisttest.cc
arraylisttest_DEPENDENCIES = $(LOCAL_LIBS)

smartpointertest_SOURCES = smartpointertest.cc
smartpointertest_DEPENDENCIES = $(LOCAL_LIBS)

tuplestest_SOURCES = tuplestest.cc
tuplestest_DEPENDENCIES = $(LOCAL_LIBS)

streamtest_SOURCES = streamtest.cc
streamtest_DEPENDENCIES = $(LOCAL_LIBS)

# mention headers so that they are distributed too
iteratorfacadetest_SOURCES = iteratorfacadetest.cc iteratorfacadetest.hh \
   iteratortest.hh
iteratorfacadetest_DEPENDENCIES = $(LOCAL_LIBS)

fvectortest_SOURCES = fvectortest.cc
fvectortest_DEPENDENCIES = $(LOCAL_LIBS)

fmatrixtest_SOURCES = fmatrixtest.cc
fmatrixtest_DEPENDENCIES = $(LOCAL_LIBS)

poolallocatortest_SOURCES = poolallocatortest.cc
poolallocatortest_DEPENDENCIES = $(LOCAL_LIBS)

settest_SOURCES=settest.cc
settest_DEPENDENCIES = $(LOCAL_LIBS)

gcdlcmtest_SOURCES = gcdlcmtest.cc
gcdlcmtest_DEPENDENCIES = $(LOCAL_LIBS)

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

sourcescheck_NOSOURCES = exprtmpl.cc timing.cc

include $(top_srcdir)/am/global-rules
