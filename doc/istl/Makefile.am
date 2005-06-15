# $id: $

SUBDIRS = comm

if BUILD_DOCS
  DOCS = istl.pdf istl.ps
endif

dist_pkgdata_DATA = $(DOCS)

# rerun TEX if log-file suggests that
.tex.dvi:
	$(TEX) $*
	while grep Rerun $*.log > /dev/null ; do \
	  $(TEX) $* ; \
	done
# check if Bibtex needs to be called
	if grep '^\\citation{' *.aux > /dev/null ; then \
	  $(BIBTEX) $* ; \
	  $(TEX) $* ; \
	  while grep Rerun $*.log > /dev/null ; do \
	    $(TEX) $* ; \
	  done ; \
	fi

.dvi.pdf:
	$(DVIPDF) $*

.dvi.ps:
	$(DVIPS) $*

CLEANFILES = *.aux *.bbl *.blg *.log *.out *.toc *.dvi *.pdf *.ps

# setting like in dune-web
CURDIR=doc/istl

include $(top_srcdir)/am/webstuff
include $(top_srcdir)/am/global-rules
