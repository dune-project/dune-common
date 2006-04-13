# $Id$

TESTPROGS = parsetest test-stack arraylisttest smartpointertest \
	sllisttest iteratorfacadetest tuplestest fmatrixtest \
	poolallocatortest settest gcdlcdtest streamtest
# exprtmpl

# which tests to run
TESTS = $(TESTPROGS)

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS)

# output code coverage
#AM_CXXFLAGS = -fprofile-arcs -ftest-coverage
AM_LDFLAGS = $(LIBDUNE)

# define the programs

parsetest_SOURCES = parsetest.cc
parsetest_DEPENDENCIES = $(LIBDUNE)

sllisttest_SOURCES = sllisttest.cc
sllisttest_DEPENDENCIES = $(LIBDUNE)

test_stack_SOURCES = test-stack.cc
test_stack_DEPENDENCIES = $(LIBDUNE)

arraylisttest_SOURCES = arraylisttest.cc
arraylisttest_DEPENDENCIES = $(LIBDUNE)

smartpointertest_SOURCES = smartpointertest.cc
smartpointertest_DEPENDENCIES = $(LIBDUNE)

tuplestest_SOURCES = tuplestest.cc
tuplestest_DEPENDENCIES = $(LIBDUNE)

streamtest_SOURCES = streamtest.cc
streamtest_DEPENDENCIES = $(LIBDUNE)

# mention headers so that they are distributed too
iteratorfacadetest_SOURCES = iteratorfacadetest.cc iteratorfacadetest.hh \
   iteratortest.hh
iteratorfacadetest_DEPENDENCIES = $(LIBDUNE)

fmatrixtest_SOURCES = fmatrixtest.cc
fmatrixtest_DEPENDENCIES = $(LIBDUNE)

poolallocatortest_SOURCES = poolallocatortest.cc
poolallocatortest_DEPENDENCIES = $(LIBDUNE)

settest_SOURCES=settest.cc
settest_DEPENDENCIES = $(LIBDUNE)

gcdlcdtest_SOURCES = gcdlcdtest.cc
gcdlcdtest_DEPENDENCIES = $(LIBDUNE)

# bin_PROGRAMS = exprtmpl timing_old timing_xpr timing_flt
#
exprtmpl_SOURCES = exprtmpl.cc
exprtmpl_CXXFLAGS = -DDUNE_EXPRESSIONTEMPLATES -DDUNE_ISTL_WITH_CHECKING -g
# -DNOPRINT
exprtmpl_DEPENDENCIES = $(LIBDUNE)

dist_noinst_DATA = timing.cc
timing_old_SOURCES = timing.cc
timing_old_DEPENDENCIES = $(LIBDUNE)
timing_xpr_SOURCES = timing.cc
timing_xpr_CXXFLAGS = -DDUNE_EXPRESSIONTEMPLATES -g
timing_xpr_DEPENDENCIES = $(LIBDUNE)
timing_flt_SOURCES = timing.cc
timing_flt_CXXFLAGS = -DDUNE_EXPRESSIONTEMPLATES -DDUNE_FLATIT -g
timing_flt_DEPENDENCIES = $(LIBDUNE)

include $(top_srcdir)/am/global-rules
