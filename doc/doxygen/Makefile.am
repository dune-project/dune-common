# $Id$

doxydir = $(pkgdatadir)/doc/doxygen

dist_doxy_DATA =					\
	Doxystyle

# setting like in dune-web
BASEDIR=../..
CURDIR=doc/doxygen
PAGES =

# add some more stuff to install and tarball
DOCFILES = $(PAGES)
EXTRA_DIST = CMakeLists.txt $(PAGES) Doxylocal

include $(top_srcdir)/am/doxygen
include $(top_srcdir)/am/global-rules
