# $Id$

if BUILD_DOCS
PAGES = codingstyle.html portability.html \
  ugreader.html tests.html features.html indexidsets.html
endif

all: $(PAGES)

# place in website
BASEDIR=../..
CURDIR=doc/devel

# install the html pages
docdir=$(datadir)/doc/dune/devel
doc_DATA = $(PAGES)
EXTRA_DIST= $(PAGES)

include $(top_srcdir)/am/webstuff

CLEANFILES = $(PAGES)

include $(top_srcdir)/am/global-rules
