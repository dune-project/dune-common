# $Id $

if MPI
  INDEXPROG = indicestest
  SELECTPROG = selectiontest
  INDEXRUN = run-indices
endif

# which tests where program to build and run are equal
NORMALTESTS = $(SELECTPROG)

# list of tests to run (indicestest is special case)
TESTS = $(NORMALTESTS) $(INDEXRUN)

# programs just to build when "make check" is used
check_PROGRAMS = $(NORMALTESTS) $(INDEXPROG)

# define the programs

indicestest_SOURCES = indicestest.cc

indicestest_CXXFLAGS = $(MPI_CPPFLAGS)
indicestest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)

selectiontest_SOURCES = selectiontest.cc

selectiontest_CXXFLAGS = $(MPI_CPPFLAGS)
selectiontest_LDFLAGS = $(MPI_LDFLAGS) $(MPI_LIBS)
