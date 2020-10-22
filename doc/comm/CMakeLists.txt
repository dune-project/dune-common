add_executable(poosc08 "poosc08.cc")
if(DUNE_USE_DEPRECATED_BUILDSYSTEM)
  target_link_libraries(poosc08 "dunecommon")
else()
  target_link_libraries(poosc08 PRIVATE "dunecommon")
endif()

add_executable(poosc08_test "poosc08_test.cc")
if(DUNE_USE_DEPRECATED_BUILDSYSTEM)
  target_link_libraries(poosc08_test "dunecommon")
else()
  target_link_libraries(poosc08_test PRIVATE "dunecommon")
endif()

add_executable(indexset "indexset.cc")
if(DUNE_USE_DEPRECATED_BUILDSYSTEM)
  target_link_libraries(indexset "dunecommon")
else()
  target_link_libraries(indexset PRIVATE "dunecommon")
endif()

add_dune_mpi_flags("poosc08;poosc08_test;indexset")

dune_add_latex_document(
  SOURCE communication.tex
  FATHER_TARGET doc
  BUILD_ON_INSTALL
  INSTALL ${CMAKE_INSTALL_DOCDIR}/comm)
