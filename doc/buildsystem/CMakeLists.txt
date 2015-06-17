dune_add_latex_document(buildsystem.tex FATHER_TARGET doc DEFAULT_PDF INPUTS ../Makefile.am ../../configure.ac ../example.opts)
create_doc_install(${CMAKE_CURRENT_BINARY_DIR}/buildsystem.pdf ${CMAKE_INSTALL_DOCDIR}/buildsystem)

dune_add_latex_document(cmakefaq.tex FATHER_TARGET doc DEFAULT_PDF)
create_doc_install(${CMAKE_CURRENT_BINARY_DIR}/cmakefaq.pdf ${CMAKE_INSTALL_DOCDIR}/buildsystem)
