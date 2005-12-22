# $Id$

#
## estimate which tests can be built
#

if ALBERTA
  ALBERTATEST = test-albertawriter
endif

#
## define the lists of tests to build and run
#

# tests where program to build and program to run are equal
NORMALTESTS = $(ALBERTATEST)

# list of tests to run (yaspgrid is special case)
TESTS = $(NORMALTESTS)

# programs just to build when "make check" is used
check_PROGRAMS = $(NORMALTESTS)

#
## common flags
#

AM_LDFLAGS = $(LIBDUNE)

# paranoia
DUNE_EXTRA_CHECKS =  -DDUNE_ISTL_WITH_CHECKING -DDUNE_DEVEL_MODE

AM_CPPFLAGS = $(DUNE_EXTRA_CHECKS)

#
## define the programs
#

#
test_albertawriter_SOURCES = test-albertawriter.cc
test_albertawriter_CXXFLAGS = $(ALBERTA_CPPFLAGS) $(AM_CPPFLAGS)
test_albertawriter_LDADD = $(ALBERTA_LDFLAGS) $(ALBERTA_LIBS)
test_albertawriter_DEPENDENCIES = $(LIBDUNE)

## distribution tarball

testdir = $(includedir)/dune/io/file/test/
test_HEADERS =

GRIDFILES = alberta-testgrid-2-2.al alberta-testgrid-3-3.al \
  ug-testgrid-2.am ug-testgrid-3.am

# gridcheck not used explicitly, we should still ship it :)
EXTRA_DIST = gridcheck.cc $(GRIDFILES)

CLEANFILES = *.gcda *.gcno semantic.cache

include $(top_srcdir)/am/global-rules
