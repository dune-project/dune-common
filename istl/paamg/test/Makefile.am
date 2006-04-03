# $Id$

if MPI
  MPITESTS = graphtest
  TESTPROGS = galerkintest hierarchytest amgtest pamgtest transfertest
  TESTRUNS = run-galerkin run-hierarchy run-amg
endif

# link against libcommon to utilize dune stdstreams
AM_LDFLAGS = $(top_builddir)/common/libcommon.la

NORMALTESTS = $(MPITESTS)

# which tests to run
TESTS = $(NORMALTESTS) $(TESTRUNS)

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS) $(NORMALTESTS)

# define the programs

graphtest_SOURCES = graphtest.cc

transfertest_SOURCES = transfertest.cc
transfertest_CXXFLAGS = $(MPI_CPPFLAGS)
transfertest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)
transfertest_DEPENDENCIES = $(LIBDUNE)

galerkintest_SOURCES = galerkintest.cc anisotropic.hh
galerkintest_CXXFLAGS = $(MPI_CPPFLAGS)
galerkintest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)
galerkintest_DEPENDENCIES = $(LIBDUNE)

hierarchytest_SOURCES = hierarchytest.cc anisotropic.hh
hierarchytest_CXXFLAGS = $(MPI_CPPFLAGS)
hierarchytest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)
hierarchytest_DEPENDENCIES = $(LIBDUNE)

amgtest_SOURCES = amgtest.cc
amgtest_CXXFLAGS = $(MPI_CPPFLAGS)
amgtest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)
amgtest_DEPENDENCIES = $(LIBDUNE)

pamgtest_SOURCES = parallelamgtest.cc
pamgtest_CXXFLAGS = $(MPI_CPPFLAGS)
pamgtest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)
pamgtest_DEPENDENCIES = $(LIBDUNE)
include $(top_srcdir)/am/global-rules
