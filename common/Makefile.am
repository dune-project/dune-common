# $Id$

SUBDIRS = . test

# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = stdstreams.cc configparser.cc exprtmpl.cc

AM_CPPFLAGS = @AM_CPPFLAGS@ -I$(top_srcdir)/..

commonincludedir = $(includedir)/dune/common
commoninclude_HEADERS = dlist.cc stdstreams.cc alignment.hh \
  array.hh arraylist.hh bitfield.hh capabilities.hh debugstream.hh dlist.hh \
  enumset.hh exceptions.hh fixedarray.hh fmatrix.hh \
  fvector.hh genericiterator.hh \
  helpertemplates.hh interfaces.hh iteratorfacades.hh logictraits.hh \
  matrix.hh misc.hh poolallocator.hh simplevector.hh \
  sllist.hh smartpointer.hh stack.hh stdstreams.hh timer.hh tuples.hh \
  typeinfo.hh typetraits.hh precision.hh bigunsignedint.hh \
  gcd.hh lcm.hh configparser.hh propertymap.hh \
  exprtmpl.hh exprtmpl/scalar.inc exprtmpl/exprexpr.inc collectivecommunication.hh\
  mpicollectivecommunication.hh geometrytype.hh tripel.hh utility.hh


include $(top_srcdir)/am/global-rules
