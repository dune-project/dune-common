# $Id: Makefile.am 4499 2006-04-13 14:56:35Z christi $

if BUILD_DOCS
  DOCS=buildsystem.pdf buildsystem.ps
endif

all: $(DOCS)

# setting like in dune-web
CURDIR=doc
# position of the web base directory,
# relative to $(CURDIR)
BASEDIR=..
EXTRAINSTALL = buildsystem.pdf

# include rules for wml -> html transformation
include $(top_srcdir)/am/webstuff

include $(top_srcdir)/am/latex

# remove html pages on ``make  clean''
CLEANFILES = *.ps *.pdf auto *.aux *.dvi *.log *.out *.toc

# include further rules needed by Dune
include $(top_srcdir)/am/global-rules
