# $Id$

TESTPROGS = parsetest test-stack arraylisttest smartpointertest \
	sllisttest iteratorfacadetest

# which tests to run
TESTS = $(TESTPROGS)

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS)

# define the programs

parsetest_SOURCES = parsetest.cc

sllisttest_SOURCES = sllisttest.cc

test_stack_SOURCES = test-stack.cc

arraylisttest_SOURCES = arraylisttest.cc

smartpointertest_SOURCES = smartpointertest.cc

# mention headers so that they are distributed too
iteratorfacadetest_SOURCES = iteratorfacadetest.cc iteratorfacadetest.hh \
   iteratortest.hh
