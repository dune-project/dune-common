# $Id$

# also build these sub directories
SUBDIRS = devel doxygen layout buildsystem

# only build html pages, if documentation is enabled
if BUILD_DOCS
 PAGES = view-concept.html installation-notes.html
endif

# automatically create these web pages
all: $(PAGES)

# setting like in dune-web
CURDIR=doc
# position of the web base directory,
# relative to $(CURDIR)
BASEDIR=..
EXTRAINSTALL=example.opts

# install the html pages
docdir=$(datadir)/doc/dune-common
doc_DATA = $(PAGES) example.opts
EXTRA_DIST = $(PAGES) example.opts

# include rules for wml -> html transformation
include $(top_srcdir)/am/webstuff

# remove html pages on ``make  clean''
CLEANFILES = $(PAGES)

# include further rules needed by Dune
include $(top_srcdir)/am/global-rules
