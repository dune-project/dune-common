# $Id$

# setting like in dune-web
BASEDIR=../..
CURDIR=doc/doxygen
PAGES =

if BUILD_DOCS
if WML
  PAGES += bestpractice.html
endif
endif

# add some more stuff to install and tarball
doxygen_DATA = $(PAGES)
EXTRA_DIST = $(PAGES)

include $(top_srcdir)/am/doxygen
include $(top_srcdir)/am/global-rules

foo:
	echo $(PAGES)
