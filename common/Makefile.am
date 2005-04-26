# $Id$

SUBDIRS = . test

# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = stdstreams.cc

AM_CPPFLAGS = -I$(top_srcdir)/..

commonincludedir = $(includedir)/dune/common
commoninclude_HEADERS = dlist.cc mapping.cc stdstreams.cc alignment.hh \
  array.hh arraylist.hh bitfield.hh capabilities.hh debugstream.hh dlist.hh \
  dynamictype.hh enumset.hh exceptions.hh fixedarray.hh fmatrix.hh \
  function.hh functionspace.hh fvector.hh genericiterator.hh \
  helpertemplates.hh interfaces.hh iteratorfacades.hh logictraits.hh \
  mapping.hh matrix.hh misc.hh operator.hh poolallocator.hh simplevector.hh \
  sllist.hh smartpointer.hh stack.hh stdstreams.hh timer.hh tuples.hh \
  typeinfo.hh typetraits.hh vectorspace.hh precision.hh bigunsignedint.hh
