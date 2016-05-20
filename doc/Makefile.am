# also build these sub directories
SUBDIRS = doxygen comm

# setting like in dune-web
CURDIR=doc
# position of the web base directory,
# relative to $(CURDIR)
BASEDIR=..

# install the html pages
DOCFILES = $(PAGES)

EXTRA_DIST = CMakeLists.txt $(PAGES)

dist_man_MANS = dunecontrol.1

# include rules for wml -> html transformation
include $(top_srcdir)/am/webstuff

# include further rules needed by Dune
include $(top_srcdir)/am/global-rules
