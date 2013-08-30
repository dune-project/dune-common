add_subdirectory("vec")

dune_add_library(kernel
  _DUNE_TARGET_OBJECTS:vec_
  )

#install headers
install(FILES
  instantiation.hh
  utility.hh
  vec.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/kernel)
