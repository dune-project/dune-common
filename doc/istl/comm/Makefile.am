# $Id$

# only build these programs if an MPI-implementation was found
if MPI
  MPIPROGRAMS = indexset
endif

SUFFIXES = .dvi .tex .pdf

dist_pkgdata_DATA = communication.pdf communication.ps

noinst_PROGRAMS = $(MPIPROGRAMS)

indexset_SOURCES = indexset.cc
indexset_CXXFLAGS = $(MPI_CPPFLAGS)
indexset_LDADD = $(MPI_LDFLAGS)

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


communication.dvi : communication.tex buildindexset.hh reverse.hh


CLEANFILES = *.aux *.bbl *.blg *.log *.out *.toc *.dvi *.pdf *.ps

include $(top_srcdir)/am/sourcescheck
