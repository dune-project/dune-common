# $Id $

if MPI
TESTPROGS = indicestest
endif

# which tests to run
TESTS = $(TESTPROGS)

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS)

# define the programs

indicestest_SOURCES = indicestest.cc

indicestest_CXXFLAGS = @MPI_CPPFLAGS@

indicestest_LDFLAGS = @MPI_LDFLAGS@

#indicestest_LDFLAGS = $(MPI_LIBS)
