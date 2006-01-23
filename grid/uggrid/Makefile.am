# $Id$

if UG

noinst_LTLIBRARIES = libuggrid.la

libuggrid_la_SOURCES  = uggrid.cc uggridentity.cc boundaryextractor.cc
libuggrid_la_CXXFLAGS = $(AM_CPPFLAGS) $(UG_CPPFLAGS) #-DDUNE_EXPRESSIONTEMPLATES
libuggrid_la_LIBADD   = $(UG_LDFLAGS) $(UG_LIBS2) $(UG_LIBS3)

endif

uggriddir = $(includedir)/dune/grid/uggrid/
uggrid_HEADERS = uggridgeometry.cc \
  uggridentitypointer.hh uggridhieriterator.cc ugintersectionit.cc \
  ugfunctions.hh uggridentity.hh uggridgeometry.hh \
  uggridhieriterator.hh uggridleveliterator.hh ugincludes.hh \
  ugintersectionit.hh ugtypes.hh ug_undefs.hh uggridindexsets.hh \
  ugfunctions3d.hh uggridleafiterator.hh boundaryextractor.hh

# tricks like undefAllMacros.pl don't have to be shipped, have they?
# Oliver Sander: It depends. undefAllMacros.pl creates the file ug_undefs.hh
# which contains an #undef line for each macro encountered in the UG
# headers which DUNE includes.  This should be redone every time UG
# changes, and currently I do it and then check in the new ug_undefs.hh.
# An alternative would be to have the DUNE-build system call undefAllMacros
# for the UG it is configured with.  Then it needed to be shipped.


include $(top_srcdir)/am/global-rules
