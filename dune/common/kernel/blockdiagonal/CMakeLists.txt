
#install headers
install(FILES
  extractbelldiagonal.hh
  ludecompositionnopivot.hh
  ludecompositionpartialpivot.hh
  lusolvenopivot.hh
  lusolvepartialpivot.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/kernel/blockdiagonal)

dune_library_add_sources(
  dunecommon
  SOURCES
  extractbelldiagonal.cc
  ludecompositionnopivot.cc
  ludecompositionpartialpivot.cc
  lusolvenopivot.cc
  lusolvepartialpivot.cc
  )
