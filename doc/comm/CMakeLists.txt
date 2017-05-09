add_executable(poosc08 "poosc08.cc")
target_link_libraries(poosc08 "dunecommon")

add_executable(poosc08_test "poosc08_test.cc")
target_link_libraries(poosc08_test "dunecommon")

add_executable(indexset "indexset.cc")
target_link_libraries(indexset "dunecommon")

add_dune_mpi_flags("poosc08;poosc08_test;indexset")

dune_add_latex_document(communication.tex
  BIBFILES communication.bib
  INPUTS poosc08_test.cc)
create_doc_install(${CMAKE_CURRENT_BINARY_DIR}/communication.pdf ${CMAKE_INSTALL_DOCDIR}/comm communication)
