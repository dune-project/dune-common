# $Id$

if BUILD_DOCS
 PAGES = refelements.html
endif

SUBDIRS = refelements

all: $(PAGES)

# place in website
BASEDIR=../..
CURDIR=doc/appl

include $(top_srcdir)/am/webstuff

CLEANFILES = $(PAGES)

include $(top_srcdir)/am/global-rules
