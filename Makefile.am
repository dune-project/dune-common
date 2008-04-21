# $Id$

# we need the module file to be able to build via dunecontrol
EXTRA_DIST=dune.module dune-common.m4

# don't follow the full GNU-standard
# need automake >= 1.5
AUTOMAKE_OPTIONS = foreign 1.5

SUBDIRS = lib common doc bin m4 am

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = dune-common.pc

# use configured compiler for "make distcheck"
# doxygen is difficult to convince to build in a special directory...
DISTCHECK_CONFIGURE_FLAGS = CXX="$(CXX)" CC="$(CC)" --enable-parallel=@ENABLE_PARALLEL@ MPICC="$(MPICC)"

include $(top_srcdir)/am/global-rules
