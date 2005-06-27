# $Id$

if BUILD_DOCS
PAGES = codingstyle.html portability.html \
  ugreader.html
endif

all: $(PAGES)

# place in website
BASEDIR=../..
CURDIR=doc/devel

include $(top_srcdir)/am/webstuff

CLEANFILES = $(PAGES)

include $(top_srcdir)/am/global-rules
