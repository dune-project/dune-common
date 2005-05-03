# $Id$

TESTPROGS = test-stream

# which tests to run
TESTS = $(TESTPROGS)

# programs just to build when "make check" is used
check_PROGRAMS = $(TESTPROGS)

# we want to check the library, thus link to it :)
LDADD = ../libdune.la

# define the programs

test_stream_SOURCES = test-stream.cc

include $(top_srcdir)/am/sourcescheck
