# $Id$

if BUILD_DOCS
# only build html when wml is available
if WML
PAGES = codingstyle.html portability.html \
  tests.html features.html
endif
endif

# place in website
BASEDIR=../..
CURDIR=doc/devel

# install the html pages
develdir=$(datadir)/doc/dune-common/devel
if BUILD_DOCS
DOCFILES = $(PAGES)
EXTRA_DIST = $(PAGES)
endif

include $(top_srcdir)/am/webstuff
include $(top_srcdir)/am/global-rules
