# $Id$

TESTPROGS = parsetest test-stack arraylisttest smartpointertest \
	sllisttest iteratorfacadetest tuplestest fmatrixtest \
	poolallocatortest settest gcdlcdtest streamtest exprtmpl

# which tests to run
TESTS = $(TESTPROGS)

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS)

# output code coverage
#AM_CXXFLAGS = -fprofile-arcs -ftest-coverage
#AM_LDFLAGS = $(top_builddir)/lib/libdune.la
AM_LDFLAGS = $(top_builddir)/common/libcommon.la

# define the programs

parsetest_SOURCES = parsetest.cc

sllisttest_SOURCES = sllisttest.cc

test_stack_SOURCES = test-stack.cc

arraylisttest_SOURCES = arraylisttest.cc

smartpointertest_SOURCES = smartpointertest.cc

tuplestest_SOURCES = tuplestest.cc

streamtest_SOURCES = streamtest.cc

# mention headers so that they are distributed too
iteratorfacadetest_SOURCES = iteratorfacadetest.cc iteratorfacadetest.hh \
   iteratortest.hh

fmatrixtest_SOURCES = fmatrixtest.cc

poolallocatortest_SOURCES = poolallocatortest.cc

settest_SOURCES=settest.cc

gcdlcdtest_SOURCES = gcdlcdtest.cc

bin_PROGRAMS = exprtmpl timing_old timing_xpr timing_flt
exprtmpl_SOURCES = exprtmpl.cc
exprtmpl_CXXFLAGS = -DDUNE_EXPRESSIONTEMPLATES -DDUNE_ISTL_WITH_CHECKING -g
# -DNOPRINT

dist_noinst_DATA = timing.cc
timing_old_SOURCES = timing.cc
timing_xpr_SOURCES = timing.cc
timing_xpr_CXXFLAGS = -DDUNE_EXPRESSIONTEMPLATES -g -finline-limit=1200
timing_flt_SOURCES = timing.cc
timing_flt_CXXFLAGS = -DDUNE_FLATIT -g

include $(top_srcdir)/am/global-rules
