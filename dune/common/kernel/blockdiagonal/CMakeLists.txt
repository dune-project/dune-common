
#install headers
install(FILES
  extractbelldiagonal.hh
  ludecompositionnopivot.hh
  ludecompositionpartialpivot.hh
  lusolvenopivot.hh
  lusolvepartialpivot.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/kernel/blockdiagonal)

set(noinst_LTLIBRARIES libblockdiagonal.la)
dune_add_library(bell OBJECT
  extractbelldiagonal.cc
  ludecompositionnopivot.cc
  ludecompositionpartialpivot.cc
  lusolvenopivot.cc
  lusolvepartialpivot.cc
  )
