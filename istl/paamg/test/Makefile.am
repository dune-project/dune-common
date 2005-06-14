# $Id$

#if MPI
  MPITESTS = graphtest
  TESTPROGS = galerkintest
  TESTRUNS = run-galerkin
#endif

NORMALTESTS = $(MPITESTS)
# which tests to run
TESTS = $(NORMALTESTS) $(TESTRUNS)

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS) $(NORMALTESTS)

# define the programs

graphtest_SOURCES = graphtest.cc

galerkintest_SOURCES = galerkintest.cc

galerkintest_CXXFLAGS = $(MPI_CPPFLAGS)
galerkintest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)

include $(top_srcdir)/am/global-rules
