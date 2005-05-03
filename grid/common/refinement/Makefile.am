# $Id$

refinementdir = $(includedir)/dune/grid/common/refinement
refinement_HEADERS = base.cc hcube.cc simplex.cc hcubetriangulation.cc

EXTRA_DIST = referencetokuhn0.svg referencetokuhn0.png \
	     simplexvertexindex.svg simplexvertexindex.png

# Update png's if possible
noinst_DATA = referencetokuhn0.png simplexvertexindex.png

include $(top_srcdir)/am/inkscape.am

include $(top_srcdir)/am/sourcescheck
