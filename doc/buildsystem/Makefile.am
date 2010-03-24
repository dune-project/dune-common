# $Id: Makefile.am 4499 2006-04-13 14:56:35Z christi $

# setting like in dune-web
CURDIR=doc/buildsystem
BASEDIR=../..

# install the html pages
docdir=$(datadir)/doc/dune-common/buildsystem
if BUILD_DOCS
  DOCFILES=buildsystem.pdf
  EXTRA_DIST=buildsystem.pdf
  EXTRAINSTALL=buildsystem.pdf
endif

# include predefined rules
include $(top_srcdir)/am/webstuff
include $(top_srcdir)/am/latex
include $(top_srcdir)/am/global-rules
