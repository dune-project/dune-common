# $Id$

#
## estimate which tests can be built
#

if MPI
  YPROG = test-yaspgrid
  YRUN = run-yaspgrid
endif

if ALBERTA
  APROG = test-alberta
endif

if ALUGRID
  ALUPROG = test-alu3dgrid
endif

# UGGrid needs AmiraMeshReader for now...
# !!! trick around to beat automake
if AMIRAMESH
  DO_UGTEST = test-ug
else
  DO_UGTEST =
endif

if UG
  UPROG = $(DO_UGTEST)
endif

#
## define the lists of tests to build and run
#

# tests where program to build and program to run are equal
NORMALTESTS = test-sgrid test-oned $(APROG) $(UPROG) $(ALUPROG)

# list of tests to run (yaspgrid is special case)
TESTS = $(NORMALTESTS) $(YRUN)

# programs just to build when "make check" is used
check_PROGRAMS = $(NORMALTESTS) $(YPROG)

#
## common flags
#

AM_LDFLAGS = $(top_builddir)/lib/libdune.la

# paranoia
DUNE_EXTRA_CHECKS =  -DDUNE_ISTL_WITH_CHECKING -DDUNE_DEVEL_MODE
# output coverage
#COVERAGE = -fprofile-arcs -ftest-coverage
AM_CPPFLAGS = $(COVERAGE) $(DUNE_EXTRA_CHECKS)

#
## define the programs
#

test_sgrid_SOURCES = test-sgrid.cc
test_sgrid_CXXFLAGS = -DWITH_INDEX_SETS

test_oned_SOURCES = test-oned.cc
test_oned_CXXFLAGS = -DWITH_INDEX_SETS

test_yaspgrid_SOURCES = test-yaspgrid.cc
test_yaspgrid_CXXFLAGS = $(AM_CPPFLAGS) $(MPI_CPPFLAGS) -DWITH_INDEX_SETS
test_yaspgrid_LDADD = $(MPI_LDFLAGS) $(MPI_LIBS)

# this implicitly checks the autoconf-test as well...
test_alberta_SOURCES = test-alberta.cc
test_alberta_CXXFLAGS = $(ALBERTA_CPPFLAGS) $(AM_CPPFLAGS) -DWITH_INDEX_SETS
test_alberta_LDADD = $(ALBERTA_LDFLAGS) $(ALBERTA_LIBS)

# files for alu3dgrid
test_alu3dgrid_SOURCES = test-alu3dgrid.cc
test_alu3dgrid_CXXFLAGS = $(ALU3DGRID_CPPFLAGS) $(AM_CPPFLAGS)
test_alu3dgrid_LDADD = $(ALU3DGRID_LDFLAGS) $(ALU3DGRID_LIBS)

# libdune contains both libug2 and libug3, always test both dimensions
test_ug_SOURCES = test-ug.cc
test_ug_CXXFLAGS = $(AM_CPPFLAGS) $(UG_CPPFLAGS) $(AMIRAMESH_CPPFLAGS) -DWITH_INDEX_SETS
test_ug_LDADD = $(AMIRAMESH_LDFLAGS) $(AMIRAMESH_LIBS) $(UG_LDFLAGS) $(UG_LIBS)

## distribution tarball

testdir = $(includedir)/dune/grid/test/
test_HEADERS = gridcheck.cc

GRIDFILES = alberta-testgrid-2-2.al alberta-testgrid-3-3.al \
  ug-testgrid-2.am ug-testgrid-3.am
# gridcheck not used explicitly, we should still ship it :)
EXTRA_DIST = gridcheck.cc $(GRIDFILES)

CLEANFILES = *.gcda *.gcno semantic.cache

include $(top_srcdir)/am/global-rules
