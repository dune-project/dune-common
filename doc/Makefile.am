# $Id$

# distribute these files:
EXTRA_DIST = BuildSystem

SUBDIRS = devel appl doxygen layout

PAGES = index.html

all: $(PAGES)

# setting like in dune-web
BASEDIR=..
CURDIR=doc

include $(top_srcdir)/am/webstuff

CLEANFILES = $(PAGES)
