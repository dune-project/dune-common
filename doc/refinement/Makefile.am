# $Id$

#refinementdir = $(includedir)/dune/grid/common/refinement

EXTRA_DIST = referencetokuhn0.svg referencetokuhn0.png \
	     simplexvertexindex.svg simplexvertexindex.png \
	     kuhntriangulation.svg

# Update png's if possible
noinst_DATA = referencetokuhn0.png simplexvertexindex.png \
	      refinement.pdf refinement.ps

refinement.dvi: refinement.tex simplexvertexindex.eps referencetokuhn0.eps kuhntriangulation.eps

.svg.eps:
	$(INKSCAPE) --export-dpi=180 -e $@.png $< || { rm -f $@.png ; exit 1; }
	$(CONVERT) $@.png EPS:$@.tmp || { rm -f $@.tmp ; exit 1; }
	mv $@.tmp $@
	rm $@.png

MOSTLYCLEANFILES = refinement.aux refinement.log refinement.dvi	\
		   refinement.ps simplexvertexindex.eps		\
		   referencetokuhn0.eps kuhntriangulation.eps

include $(top_srcdir)/am/latex
include $(top_srcdir)/am/inkscape.am
include $(top_srcdir)/am/webstuff
include $(top_srcdir)/am/global-rules
