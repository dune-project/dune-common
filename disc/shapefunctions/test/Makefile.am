# $Id$

# programs just to build when "make check" is used
check_PROGRAMS = refelemshapefunctest

#
## common flags
#

AM_LDFLAGS = $(top_builddir)/lib/libdune.la

# paranoia
DUNE_EXTRA_CHECKS =  -DDUNE_ISTL_WITH_CHECKING -DDUNE_DEVEL_MODE

# output coverage
#COVERAGE = -fprofile-arcs -ftest-coverage
AM_CPPFLAGS = $(DUNE_EXTRA_CHECKS) -g -Wall

#
## define the programs
#

refelemshapefunctest_SOURCES = refelemshapefunctest.cc

## distribution tarball

testdir = $(includedir)/dune/disc/shapefunctions/test/
test_HEADERS =

CLEANFILES = *.gcda *.gcno semantic.cache

include $(top_srcdir)/am/global-rules
