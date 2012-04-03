include(UseLATEX)
ADD_LATEX_DOCUMENT(buildsystem.tex DEFAULT_PDF INPUTS ../Makefile.am ../../configure.ac ../example.opts)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/buildsystem.pdf
  DESTINATION share/doc/dune-common/buildsystem)
