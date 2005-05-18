# $Id$

TESTPROGS = parsetest test-stack arraylisttest smartpointertest \
	sllisttest iteratorfacadetest tuplestest fmatrixtest \
	poolallocatortest settest gcdlcdtest

# which tests to run
TESTS = $(TESTPROGS)

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS)

# output code coverage
AM_CXXFLAGS = -fprofile-arcs -ftest-coverage

# define the programs

parsetest_SOURCES = parsetest.cc

sllisttest_SOURCES = sllisttest.cc

test_stack_SOURCES = test-stack.cc

arraylisttest_SOURCES = arraylisttest.cc

smartpointertest_SOURCES = smartpointertest.cc

tuplestest_SOURCES = tuplestest.cc

# mention headers so that they are distributed too
iteratorfacadetest_SOURCES = iteratorfacadetest.cc iteratorfacadetest.hh \
   iteratortest.hh

fmatrixtest_SOURCES = fmatrixtest.cc

poolallocatortest_SOURCES = poolallocatortest.cc

settest_SOURCES=settest.cc

gcdlcdtest_SOURCES = gcdlcdtest.cc

include $(top_srcdir)/am/sourcescheck
