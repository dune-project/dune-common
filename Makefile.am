# $Id$

# don't follow the full GNU-standard
# need automake >= 1.5
AUTOMAKE_OPTIONS = foreign 1.5

SUBDIRS = common fem disc grid quadrature solver io istl lib doc bin m4 am

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = dune.pc

# use configured compiler for "make distcheck"
# doxygen is difficult to convince to build in a special directory...
DISTCHECK_CONFIGURE_FLAGS = CXX="$(CXX)" CC="$(CC)" DOXYGEN="true"

include $(top_srcdir)/am/global-rules
