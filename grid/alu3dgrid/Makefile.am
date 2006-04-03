AM_CPPFLAGS = $(ALUGRID_CPPFLAGS)

if ALUGRID
  noinst_LTLIBRARIES = libalu3d.la
  libalu3d_la_SOURCES = mappings.cc topology.cc faceutility.cc
endif

alu3dgriddir = $(includedir)/dune/grid/alu3dgrid/
alu3dgrid_HEADERS = alu3dinclude.hh communications.hh \
  datahandle.hh memory.hh grid.hh grid_imp.cc \
  mappings.hh mappings_imp.cc topology.hh entity.hh entity_imp.cc \
  faceutility.hh faceutility_imp.cc geometry.hh geometry_imp.cc \
  myautoptr.hh indexsets.hh iterator.hh iterator_imp.cc

include $(top_srcdir)/am/global-rules
