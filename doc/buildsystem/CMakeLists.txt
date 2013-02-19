if(LATEX_USABLE)
  ADD_LATEX_DOCUMENT(buildsystem.tex FATHER_TARGET doc DEFAULT_PDF INPUTS ../Makefile.am ../../configure.ac ../example.opts)
  create_doc_install(${CMAKE_CURRENT_BINARY_DIR}/buildsystem.pdf ${CMAKE_INSTALL_DOCDIR}/buildsystem)
endif(LATEX_USABLE)
