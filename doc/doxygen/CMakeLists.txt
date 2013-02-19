# quickhack for creating the Doxyfile.in and Doxyfile
include(DUNEDoxygen)
add_doxygen_target()
install(FILES Doxystyle DESTINATION share/dune-common/doc/doxygen)
