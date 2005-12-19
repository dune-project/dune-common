# $Id$

if MPI
  INDEXPROG = indicestest syncertest vectorcommtest
  SELECTPROG = selectiontest
  INDEXRUN = run-indices run-syncer run-vectorcommtest
endif

# link against libcommon to utilize dune stdstreams
AM_LDFLAGS = $(top_builddir)/common/libcommon.la

# which tests where program to build and run are equal
NORMALTESTS = matrixutilstest bvectortest indexsettest $(SELECTPROG)

# list of tests to run (indicestest is special case)
TESTS = $(NORMALTESTS) $(INDEXRUN)

# programs just to build when "make check" is used
check_PROGRAMS = $(NORMALTESTS) $(INDEXPROG)

# define the programs

bvectortest_SOURCES = bvectortest.cc
bvectortest_DEPENDENCIES = $(LIBDUNE)

indicestest_SOURCES = indicestest.cc
indicestest_DEPENDENCIES = $(LIBDUNE)

matrixutilstest_SOURCES = matrixutilstest.cc laplacian.hh
matrixutilstest_DEPENDENCIES = $(LIBDUNE)

indicestest_CXXFLAGS = $(MPI_CPPFLAGS)
indicestest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)
indicestest_DEPENDENCIES = $(LIBDUNE)

selectiontest_SOURCES = selectiontest.cc
selectiontest_CXXFLAGS = $(MPI_CPPFLAGS)
selectiontest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)
selectiontest_DEPENDENCIES = $(LIBDUNE)

indexsettest_SOURCES = indexsettest.cc
indexsettest_DEPENDENCIES = $(LIBDUNE)

syncertest_SOURCES = syncertest.cc
syncertest_CXXFLAGS = $(MPI_CPPFLAGS)
syncertest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)
syncertest_DEPENDENCIES = $(LIBDUNE)

vectorcommtest_SOURCES = vectorcommtest.cc
vectorcommtest_CXXFLAGS = $(MPI_CPPFLAGS)
vectorcommtest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)
vectorcommtest_DEPENDENCIES = $(LIBDUNE)

include $(top_srcdir)/am/global-rules
