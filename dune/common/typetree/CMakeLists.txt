install(FILES
  childextraction.hh
  compositenode.hh
  leafnode.hh
  nodebase.hh
  nodeinterface.hh
  nodetags.hh
  pairtraversal.hh
  powernode.hh
  proxynode.hh
  reduction.hh
  traversal.hh
  treecontainer.hh
  treeinfo.hh
  treepath.hh
  typetraits.hh
  visitor.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/typetree)

add_subdirectory(test)