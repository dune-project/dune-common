# $Id$

if UG

noinst_LTLIBRARIES = libuggrid2.la libuggrid3.la

# The two libs are compiled from the same source, but with different options
libuggrid2_la_SOURCES  = uggrid.cc
libuggrid2_la_CXXFLAGS = $(AM_CPPFLAGS) $(UG_NODIM_CPPFLAGS) -D_2
libuggrid2_la_LIBADD   = $(UG_LDFLAGS) $(UG_LIBS2)

libuggrid3_la_SOURCES  = uggrid.cc
libuggrid3_la_CXXFLAGS = $(AM_CPPFLAGS) $(UG_NODIM_CPPFLAGS) -D_3
libuggrid3_la_LIBADD   = $(UG_LDFLAGS) $(UG_LIBS3)

endif

uggriddir = $(includedir)/dune/grid/uggrid/
uggrid_HEADERS = uggridentity.cc uggridgeometry.cc \
  uggridentitypointer.hh uggridhieriterator.cc ugintersectionit.cc \
  ugfunctions.hh uggridboundent.hh uggridentity.hh uggridgeometry.hh \
  uggridhieriterator.hh uggridleveliterator.hh ugincludes.hh \
  ugintersectionit.hh ugtypes.hh ug_undefs.hh uggridindexsets.hh

# tricks like undefAllMacros.pl don't have to be shipped, have they?
# Oliver Sander: It depends. undefAllMacros.pl creates the file ug_undefs.hh
# which depends on the current version of UG and on whether you're compiling
# with ModelP or without.  Distributing a ug_undefs.hh is the easiest way,
# but it actually ignores the difference between sequential and parallel UG.
# This may lead to spurious UG macros bleeding into Dune.


include $(top_srcdir)/am/global-rules
