# $Id$

TESTPROGS = parsetest test-stack arraylisttest smartpointertest \
	sllisttest iteratorfacadetest tuplestest fmatrixtest \
	poolallocatortest settest gcdlcdtest streamtest \
	bigunsignedinttest mpihelpertest singletontest mpicollcomm

# which tests to run
TESTS = $(TESTPROGS)

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS)

AM_LDFLAGS = $(LOCAL_LIBS)

# define the programs
bigunsignedinttest_SOURCES=bigunsignedinttest.cc
bigunsignedinttest_DEPENDENCIES = $(LOCAL_LIBS)

parsetest_SOURCES = parsetest.cc
parsetest_DEPENDENCIES = $(LOCAL_LIBS)

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

fmatrixtest_SOURCES = fmatrixtest.cc
fmatrixtest_DEPENDENCIES = $(LOCAL_LIBS)

poolallocatortest_SOURCES = poolallocatortest.cc
poolallocatortest_DEPENDENCIES = $(LOCAL_LIBS)

settest_SOURCES=settest.cc
settest_DEPENDENCIES = $(LOCAL_LIBS)

gcdlcdtest_SOURCES = gcdlcdtest.cc
gcdlcdtest_DEPENDENCIES = $(LOCAL_LIBS)

mpihelpertest_SOURCES = mpihelpertest.cc
mpihelpertest_CXXFLAGS = $(MPI_CPPFLAGS)
mpihelpertest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)

mpicollcomm_SOURCES = mpicollectivecommunication.cc
mpicollcomm_CXXFLAGS = $(MPI_CPPFLAGS)
mpicollcomm_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)

singletontest_SOURCES = singletontest.cc
singletontest_LDFLAGS = $(LOCAL_LIBS)

sourcescheck_NOSOURCES = exprtmpl.cc timing.cc

include $(top_srcdir)/am/global-rules
