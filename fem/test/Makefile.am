# $Id$

#
## estimate which tests can be built
#

if MPI
  YPROG = test-fem-yaspgrid
  YRUN = run-fem-yaspgrid
endif

if ALBERTA
  APROG = test-fem-alberta
endif

if ALUGRID
  ALUPROG = test-fem-alu3dgrid
endif

# UGGrid needs AmiraMeshReader for now...
# !!! trick around to beat automake
if AMIRAMESH
  DO_UGTEST = test-fem-ug
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
NORMALTESTS = test-fem-sgrid test-fem-oned $(APROG) $(UPROG) $(ALUPROG)

# list of tests to run (yaspgrid is special case)
TESTS = $(NORMALTESTS) $(YRUN)

# programs just to build when "make check" is used
check_PROGRAMS = $(NORMALTESTS) $(YPROG)

#
## common flags
#

AM_LDFLAGS = $(LIBDUNE)

# paranoia
DUNE_EXTRA_CHECKS =  -DDUNE_ISTL_WITH_CHECKING -DDUNE_DEVEL_MODE
# output coverage
#COVERAGE = -fprofile-arcs -ftest-coverage
AM_CXXFLAGS = $(COVERAGE) $(DUNE_EXTRA_CHECKS)

#
## define the programs
#

test_fem_sgrid_SOURCES = test-fem-sgrid.cc
test_fem_sgrid_DEPENDENCIES = $(LIBDUNE)

test_fem_oned_SOURCES = test-fem-oned.cc
test_fem_oned_DEPENDENCIES = $(LIBDUNE)

test_fem_yaspgrid_SOURCES = test-fem-yaspgrid.cc
test_fem_yaspgrid_CXXFLAGS = $(AM_CPPFLAGS) $(MPI_CPPFLAGS)
test_fem_yaspgrid_LDADD = $(MPI_LDFLAGS) $(MPI_LIBS)
test_fem_yaspgrid_DEPENDENCIES = $(LIBDUNE)

# this implicitly checks the autoconf-test as well...
test_fem_alberta_SOURCES = test-fem-alberta.cc
test_fem_alberta_CXXFLAGS = $(ALBERTA_CPPFLAGS) $(AM_CPPFLAGS)
test_fem_alberta_LDADD = $(ALBERTA_LDFLAGS) $(ALBERTA_LIBS)
test_fem_alberta_DEPENDENCIES = $(LIBDUNE)

# files for alu3dgrid
test_fem_alu3dgrid_SOURCES = test-fem-alu3dgrid.cc
test_fem_alu3dgrid_CXXFLAGS = $(ALUGRID_CPPFLAGS) $(AM_CPPFLAGS)
test_fem_alu3dgrid_LDADD = $(ALUGRID_LDFLAGS) $(ALUGRID_LIBS)
test_fem_alu3dgrid_DEPENDENCIES = $(LIBDUNE)

# libdune contains both libug2 and libug3, always test both dimensions
test_fem_ug_SOURCES = test-fem-ug.cc
test_fem_ug_CXXFLAGS = $(AM_CPPFLAGS) $(UG_CPPFLAGS) $(AMIRAMESH_CPPFLAGS)
test_fem_ug_LDADD = $(AMIRAMESH_LDFLAGS) $(AMIRAMESH_LIBS)
test_fem_ug_DEPENDENCIES = $(LIBDUNE)

## distribution tarball

testdir = $(includedir)/dune/fem/test/
test_HEADERS = femcheck.cc

GRIDFILES = alberta-testgrid-2-2.al alberta-testgrid-3-3.al \
  ug-testgrid-2.am ug-testgrid-3.am
# gridcheck not used explicitly, we should still ship it :)
EXTRA_DIST = femcheck.cc $(GRIDFILES)

CLEANFILES = *.gcda *.gcno semantic.cache

include $(top_srcdir)/am/global-rules
