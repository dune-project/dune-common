# setting like in dune-web
CURDIR=doc/buildsystem
BASEDIR=../..

# install the html pages
docdir=$(datadir)/doc/dune-common/buildsystem
if BUILD_DOCS
  DOCFILES=buildsystem.pdf cmakefaq.pdf
  EXTRA_DIST= CMakeLists.txt $(DOCFILES)
  EXTRAINSTALL=$(DOCFILES)
endif

# include predefined rules
include $(top_srcdir)/am/webstuff
include $(top_srcdir)/am/latex
include $(top_srcdir)/am/global-rules
