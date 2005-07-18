# $Id$

commondir = $(includedir)/dune/grid/common
common_HEADERS = virtualrefinement.cc boundary.hh \
  defaultindexsets.hh entity.hh entitypointer.hh geometry.hh grid.hh \
  hierarchiciterator.hh indexstack.hh intersectioniterator.hh \
  leafindexset.hh leafiterator.hh leveliterator.hh refinement.hh \
  virtualrefinement.hh reftopology.hh referenceelements.hh

SUBDIRS = refinement

noinst_LTLIBRARIES = libgridcommon.la
libgridcommon_la_SOURCES = reftopology.cc

include $(top_srcdir)/am/global-rules
