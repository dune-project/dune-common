# $id: $

SUBDIRS = comm

if BUILD_DOCS
  DOCS = istl.pdf istl.ps
endif

dist_pkgdata_DATA = $(DOCS)

CLEANFILES = *.aux *.bbl *.blg *.log *.out *.toc *.dvi *.pdf *.ps

# setting like in dune-web
CURDIR=doc/istl

include $(top_srcdir)/am/latex
include $(top_srcdir)/am/webstuff
include $(top_srcdir)/am/global-rules
