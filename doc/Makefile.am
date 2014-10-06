# also build these sub directories
SUBDIRS = doxygen buildsystem comm

# setting like in dune-web
CURDIR=doc
# position of the web base directory,
# relative to $(CURDIR)
BASEDIR=..
EXTRAINSTALL=example.opts

# install the html pages
DOCFILES = $(PAGES)
DOCFILES_EXTRA = example.opts

EXTRA_DIST = CMakeLists.txt $(PAGES) example.opts

dist_man_MANS = dunecontrol.1

# include rules for wml -> html transformation
include $(top_srcdir)/am/webstuff

# include further rules needed by Dune
include $(top_srcdir)/am/global-rules
