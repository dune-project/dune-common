# $Id$

# distribute these files:
EXTRA_DIST = Buildsystem

SUBDIRS = devel appl doxygen layout istl

if BUILD_DOCS
 PAGES = index.html
endif

all: $(PAGES)

# setting like in dune-web
BASEDIR=..
CURDIR=doc

include $(top_srcdir)/am/webstuff

CLEANFILES = $(PAGES)

include $(top_srcdir)/am/global-rules
