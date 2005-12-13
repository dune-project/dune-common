# $Id$

#if MPI
  MPITESTS = graphtest
  TESTPROGS = galerkintest hierarchytest amgtest
  TESTRUNS = run-galerkin run-hierarchy run-amg
#endif

# link against libcommon to utilize dune stdstreams
AM_LDFLAGS = $(top_builddir)/common/libcommon.la

NORMALTESTS = transfertest $(MPITESTS)

# which tests to run
TESTS = $(NORMALTESTS) $(TESTRUNS)

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS) $(NORMALTESTS)

# define the programs

graphtest_SOURCES = graphtest.cc

transfertest_SOURCES = transfertest.cc
transfertest_CXXFLAGS = $(MPI_CPPFLAGS)
transfertest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)

galerkintest_SOURCES = galerkintest.cc anisotropic.hh
galerkintest_CXXFLAGS = $(MPI_CPPFLAGS)
galerkintest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)


hierarchytest_SOURCES = hierarchytest.cc anisotropic.hh
hierarchytest_CXXFLAGS = $(MPI_CPPFLAGS)
hierarchytest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)

amgtest_SOURCES = amgtest.cc
amgtest_CXXFLAGS = $(MPI_CPPFLAGS)
amgtest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)

include $(top_srcdir)/am/global-rules
