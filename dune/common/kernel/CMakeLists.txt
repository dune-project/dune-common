add_subdirectory("bell")
add_subdirectory("blockdiagonal")
add_subdirectory("ell")
add_subdirectory("vec")

dune_add_library(kernel
  _DUNE_TARGET_OBJECTS:bell_
  _DUNE_TARGET_OBJECTS:blockdiagonal_
  _DUNE_TARGET_OBJECTS:ell_
  _DUNE_TARGET_OBJECTS:vec_
  )

#install headers
install(FILES
  bell.hh
  blockdiagonal.hh
  ell.hh
  instantiation.hh
  utility.hh
  vec.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/kernel)
