dune_add_latex_document(buildsystem.tex FATHER_TARGET doc DEFAULT_PDF INPUTS ../Makefile.am ../../configure.ac ../example.opts)
create_doc_install(${CMAKE_CURRENT_BINARY_DIR}/buildsystem.pdf ${CMAKE_INSTALL_DOCDIR}/buildsystem)
