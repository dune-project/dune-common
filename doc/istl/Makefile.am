# $id: $

SUBDIRS = comm

dist_pkgdata_DATA = istl.pdf istl.ps


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

include $(top_srcdir)/am/webstuff
include $(top_srcdir)/am/sourcescheck
