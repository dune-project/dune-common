# $Id$

if BUILD_DOCS
PAGES = codingstyle.html const-refactoring.html portability.html \
  ugreader.html
endif

all: $(PAGES)

# place in website
BASEDIR=../..
CURDIR=doc/devel

# include this into website
EXTRAINSTALL=replacements

include $(top_srcdir)/am/webstuff

CLEANFILES = $(PAGES)

include $(top_srcdir)/am/global-rules
