include(UseLATEX)
ADD_LATEX_DOCUMENT(buildsystem.tex FATHER_TARGET doc DEFAULT_PDF INPUTS ../Makefile.am ../../configure.ac ../example.opts)
create_doc_install(${CMAKE_CURRENT_BINARY_DIR}/buildsystem.pdf share/doc/dune-common/buildsystem)
#add_custom_target(install_buildsystem)
#install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_BINARY_DIR}\" --target buildsystem_install )")
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/buildsystem.pdf
#  DESTINATION share/doc/dune-common/buildsystem)
