# $Id: Makefile.am 5164 2008-04-21 14:08:28Z christi $

# we need the module file to be able to build via dunecontrol
EXTRA_DIST=dune.module dune-common.m4

# don't follow the full GNU-standard
# need automake >= 1.5
AUTOMAKE_OPTIONS = foreign 1.5

SUBDIRS = common lib doc bin m4 am

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = @DUNE_MOD_NAME@.pc

moduledir = $(libdir)/dunecontrol/@DUNE_MOD_NAME@
module_DATA = dune.module

# use configured compiler for "make distcheck"
DISTCHECK_CONFIGURE_FLAGS = CXX="$(CXX)" CC="$(CC)" --enable-parallel=@ENABLE_PARALLEL@ MPICC="$(MPICC)"

include $(top_srcdir)/am/top-rules
include $(top_srcdir)/am/global-rules
