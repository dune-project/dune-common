# quickhack for creating the Doxyfile.in and Doxyfile
include(DUNEDoxygen)
prepare_doxyfile()
add_dependencies(doc Doxyfile)
install(FILES Doxystyle DESTINATION share/dune-common/doc/doxygen)
