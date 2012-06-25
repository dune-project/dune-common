# $Id$

# we need the module file to be able to build via dunecontrol
EXTRA_DIST=dune.module

# don't follow the full GNU-standard
# need automake 1.9 or newer
AUTOMAKE_OPTIONS = foreign 1.9

SUBDIRS = dune lib doc bin m4 am

# use configured compiler for "make distcheck"
DISTCHECK_CONFIGURE_FLAGS = CXX="$(CXX)" CC="$(CC)" --enable-parallel=@ENABLE_PARALLEL@ MPICC="$(MPICC)"

include $(top_srcdir)/am/top-rules
include $(top_srcdir)/am/global-rules
