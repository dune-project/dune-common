# $Id$

if UG
if AMIRAMESH
TESTPROGS = pantypeindexsettest
endif
endif

# which tests to run
TESTS = $(TESTPROGS)

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS)

# output code coverage
#AM_CXXFLAGS = -fprofile-arcs -ftest-coverage
AM_LDFLAGS = $(LIBDUNE)

# define the programs

pantypeindexsettest_SOURCES = pantypeindexsettest.cc
pantypeindexsettest_CXXFLAGS = $(AM_CPPFLAGS) $(UG_CPPFLAGS) $(AMIRAMESH_CPPFLAGS)
pantypeindexsettest_LDADD = $(AMIRAMESH_LDFLAGS) $(AMIRAMESH_LIBS) $(UG_LDFLAGS) $(UG_LIBS)
pantypeindexsettest_DEPENDENCIES = $(LIBDUNE)

include $(top_srcdir)/am/global-rules
