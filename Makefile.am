# $Id: Makefile.am 5198 2008-05-15 16:13:23Z christi $

# we need the module file to be able to build via dunecontrol
EXTRA_DIST=dune.module dune-common.m4

# don't follow the full GNU-standard
# need automake >= 1.5
AUTOMAKE_OPTIONS = foreign 1.5

SUBDIRS = common lib doc bin m4 am

# use configured compiler for "make distcheck"
DISTCHECK_CONFIGURE_FLAGS = CXX="$(CXX)" CC="$(CC)" --enable-parallel=@ENABLE_PARALLEL@ MPICC="$(MPICC)"

include $(top_srcdir)/am/top-rules
include $(top_srcdir)/am/global-rules
