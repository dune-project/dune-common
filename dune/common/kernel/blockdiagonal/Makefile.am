# the standard debug streams are put into the libdune
noinst_LTLIBRARIES = libblockdiagonal.la

libblockdiagonal_la_SOURCES =			\
  extractbelldiagonal.cc			\
  ludecompositionnopivot.cc			\
  ludecompositionpartialpivot.cc		\
  lusolvenopivot.cc				\
  lusolvepartialpivot.cc

blockdiagonalincludedir = $(includedir)/dune/common/kernel/blockdiagonal
blockdiagonalinclude_HEADERS =			\
  extractbelldiagonal.hh			\
  ludecompositionnopivot.hh			\
  ludecompositionpartialpivot.hh		\
  lusolvenopivot.hh				\
  lusolvepartialpivot.hh

include $(top_srcdir)/am/global-rules

EXTRA_DIST = CMakeLists.txt
