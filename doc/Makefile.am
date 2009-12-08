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

if ! BUILD_DOCS
# add tag to notify that dist has been build without documentation
dist-hook:
	echo "# No documentation included in distribution! " > $(distdir)/$(DOCUMENTATION_TAG_FILE)
endif


# include rules for wml -> html transformation
include $(top_srcdir)/am/webstuff

# remove html pages on ``make  clean''
SVNCLEANFILES = $(PAGES)
clean-local:
	if test -e $(top_srcdir)/doc/doxygen/Doxydep; then rm -rf $(SVNCLEANFILES); fi

# include further rules needed by Dune
include $(top_srcdir)/am/global-rules
