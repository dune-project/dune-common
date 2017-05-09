# create Doxyfile.in and Doxyfile
add_doxygen_target()

install(FILES Doxystyle DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dune-common/doc/doxygen)
