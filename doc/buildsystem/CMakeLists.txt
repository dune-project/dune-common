include(UseLATEX)
ADD_LATEX_DOCUMENT(buildsystem.tex DEFAULT_PDF INPUTS ../Makefile.am ../../configure.ac ../example.opts)
