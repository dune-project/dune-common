# $Id$

uggriddir = $(includedir)/dune/grid/uggrid/
uggrid_HEADERS = uggrid.cc uggridentity.cc uggridgeometry.cc \
  uggridentitypointer.hh uggridhieriterator.cc ugintersectionit.cc \
  ugfunctions.hh uggridboundent.hh uggridentity.hh uggridgeometry.hh \
  uggridhieriterator.hh uggridleveliterator.hh ugincludes.hh \
  ugintersectionit.hh ugtypes.hh ug_undefs.hh ughierarchicindexset.hh

# tricks like undefAllMacros.pl don't have to be shipped, have they?
# Oliver Sander: It depends. undefAllMacros.pl creates the file ug_undefs.hh
# which depends on the current version of UG and on whether you're compiling
# with ModelP or without.  Distributing a ug_undefs.hh is the easiest way,
# but it actually ignores the difference between sequential and parallel UG.
# This may lead to spurious UG macros bleeding into Dune.


include $(top_srcdir)/am/sourcescheck
