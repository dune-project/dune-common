# $Id$

doxydir = $(pkgdatadir)/doc/doxygen

doxy_DATA =					\
	Doxystyle

# setting like in dune-web
BASEDIR=../..
CURDIR=doc/doxygen
PAGES =

# add some more stuff to install and tarball
DOCFILES = $(PAGES)
EXTRA_DIST = $(PAGES) Doxystyle

include $(top_srcdir)/am/doxygen
include $(top_srcdir)/am/global-rules
