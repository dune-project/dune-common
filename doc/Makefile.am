# $Id$

# also build these sub directories
SUBDIRS = devel doxygen layout buildsystem

# only build html pages, if documentation is enabled
if BUILD_DOCS
# only build html when wml is available
if WML
 PAGES = view-concept.html installation-notes.html installation-notes-bluegenep.html
endif
endif

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

# include further rules needed by Dune
include $(top_srcdir)/am/global-rules
