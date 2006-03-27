# $Id$

SUBDIRS = common barycenter fixedorder gaussquadrature test

# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libquadrature.la

libquadrature_la_SOURCES = quadraturerules.cc

quadratureincludedir = $(includedir)/dune/quadrature
quadratureinclude_HEADERS = barycenter.hh fixedorder.hh gaussquadrature.hh \
  quadraturerules.hh

include $(top_srcdir)/am/global-rules
