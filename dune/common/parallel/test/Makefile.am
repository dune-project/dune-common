# $Id$

MPITESTS = mpicollectivecommunication mpiguardtest mpihelpertest mpihelpertest2 \
           indicestest indexsettest syncertest selectiontest

# which tests where program to build and run are equal
NORMALTESTS =

# list of tests to run (indicestest is special case)
TESTS = $(NORMALTESTS) $(MPITESTS)

# programs just to build when "make check" is used
check_PROGRAMS = $(NORMALTESTS) $(MPITESTS)

# define the programs
#
mpihelpertest_SOURCES = mpihelpertest.cc
mpihelpertest_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
mpihelpertest_LDADD = $(DUNEMPILIBS) $(LDADD)
mpihelpertest_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

mpihelpertest2_SOURCES = mpihelpertest.cc
mpihelpertest2_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS) -DMPIHELPER_PREINITIALIZE
mpihelpertest2_LDADD = $(DUNEMPILIBS) $(LDADD)
mpihelpertest2_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

mpicollectivecommunication_SOURCES = mpicollectivecommunication.cc
mpicollectivecommunication_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
mpicollectivecommunication_LDADD = $(DUNEMPILIBS) $(LDADD)
mpicollectivecommunication_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

mpiguardtest_SOURCES = mpiguardtest.cc
mpiguardtest_CPPFLAGS = $(AM_CPPFLAGS) $(DUNEMPICPPFLAGS)
mpiguardtest_LDADD = $(DUNEMPILIBS) $(LDADD)
mpiguardtest_LDFLAGS = $(AM_LDFLAGS) $(DUNEMPILDFLAGS)

indicestest_SOURCES = indicestest.cc
indicestest_CPPFLAGS = $(AM_CPPFLAGS)		\
	$(DUNEMPICPPFLAGS)
indicestest_LDFLAGS = $(AM_LDFLAGS)		\
	$(DUNEMPILDFLAGS)
indicestest_LDADD =				\
	$(DUNEMPILIBS)				\
	$(LDADD)

selectiontest_SOURCES = selectiontest.cc
selectiontest_CPPFLAGS = $(AM_CPPFLAGS)	\
	$(DUNEMPICPPFLAGS)
selectiontest_LDFLAGS = $(AM_LDFLAGS)		\
	$(DUNEMPILDFLAGS)
selectiontest_LDADD =				\
	$(DUNEMPILIBS)				\
	$(LDADD)

indexsettest_SOURCES = indexsettest.cc

syncertest_SOURCES = syncertest.cc
syncertest_CPPFLAGS = $(AM_CPPFLAGS)		\
	$(DUNEMPICPPFLAGS)			\
	$(DUNE_COMMON_CPPFLAGS)
syncertest_LDFLAGS = $(AM_LDFLAGS)		\
	$(DUNEMPILDFLAGS)			\
	$(DUNE_COMMON_LDFLAGS)
syncertest_LDADD =					\
	$(DUNE_COMMON_LDFLAGS) $(DUNE_COMMON_LIBS)	\
	$(DUNEMPILIBS)					\
	$(LDADD)

include $(top_srcdir)/am/global-rules
