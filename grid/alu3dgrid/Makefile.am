AM_CPPFLAGS = $(ALU3DGRID_CPPFLAGS)

if ALU3DGRID
  noinst_LTLIBRARIES = libalu3d.la
  libalu3d_la_SOURCES = alu3dmappings.cc reference.cc
endif

alu3dgriddir = $(includedir)/dune/grid/alu3dgrid/
alu3dgrid_HEADERS = alu3dgrid.cc alu3dinclude.hh alu3dgridcomm.hh \
  datahandle.hh leafwalk.hh alumemory.hh alu3dgeometry.cc \
  alu3dgeometry.hh alu3dmappings.cc alu3dmappings.hh reference.hh

include $(top_srcdir)/am/sourcescheck
