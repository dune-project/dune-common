# $Id$

if MPI
  INDEXPROG = indicestest syncertest
  SELECTPROG = selectiontest
  INDEXRUN = run-indices run-syncer
endif

# link against libcommon to utilize dune stdstreams
AM_LDFLAGS = $(top_builddir)/common/libcommon.la

# which tests where program to build and run are equal
NORMALTESTS = bvectortest indexsettest $(SELECTPROG)

# list of tests to run (indicestest is special case)
TESTS = $(NORMALTESTS) $(INDEXRUN)

# programs just to build when "make check" is used
check_PROGRAMS = $(NORMALTESTS) $(INDEXPROG)

# define the programs

bvectortest_SOURCES = bvectortest.cc

indicestest_SOURCES = indicestest.cc

indicestest_CXXFLAGS = $(MPI_CPPFLAGS)
indicestest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)

selectiontest_SOURCES = selectiontest.cc

selectiontest_CXXFLAGS = $(MPI_CPPFLAGS)
selectiontest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)

indexsettest_SOURCES = indexsettest.cc

syncertest_SOURCES = syncertest.cc

syncertest_CXXFLAGS = $(MPI_CPPFLAGS)
syncertest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)

include $(top_srcdir)/am/global-rules
