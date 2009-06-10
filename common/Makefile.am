# $Id$

SUBDIRS = . test exprtmpl

# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = stdstreams.cc configparser.cc

AM_CPPFLAGS = @AM_CPPFLAGS@ -I$(top_srcdir)/..

commonincludedir = $(includedir)/dune/common
commoninclude_HEADERS = alignment.hh array.hh \
  arraylist.hh bitsetvector.hh debugstream.hh deprecated.hh \
  enumset.hh exceptions.hh fmatrix.hh fmatrixev.hh \
  fvector.hh genericiterator.hh \
  helpertemplates.hh iteratorfacades.hh \
  misc.hh poolallocator.hh finitestack.hh \
  sllist.hh smartpointer.hh stdstreams.hh timer.hh tuples.hh \
  typetraits.hh precision.hh bigunsignedint.hh \
  gcd.hh lcm.hh configparser.hh propertymap.hh \
  collectivecommunication.hh mpihelper.hh singleton.hh \
  mpicollectivecommunication.hh geometrytype.hh utility.hh \
  bartonnackmanifcheck.hh binaryfunctions.hh lru.hh fassign.hh \
  static_assert.hh smallobject.hh version.hh \
  float_cmp.cc float_cmp.hh \
  interfaces.hh nullptr.hh

if EXPRESSIONTEMPLATES
commoninclude_HEADERS += exprtmpl.hh exprtmpl/scalar.inc exprtmpl/exprexpr.inc
libcommon_la_SOURCES += exprtmpl.cc
else
sourcescheck_NOSOURCES = exprtmpl.hh exprtmpl.cc
endif

include $(top_srcdir)/am/global-rules
