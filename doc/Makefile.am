# $Id$

if INKSCAPE
INKSCAPE_SUBDIRS = refinement
endif INKSCAPE
SUBDIRS = devel appl $(INKSCAPE_SUBDIRS) doxygen layout istl

if BUILD_DOCS
 PAGES = index.html buildsystem.html view-concept.html installation-notes.html
endif

all: $(PAGES)

# setting like in dune-web
BASEDIR=..
CURDIR=doc

include $(top_srcdir)/am/webstuff

CLEANFILES = $(PAGES)

include $(top_srcdir)/am/global-rules
