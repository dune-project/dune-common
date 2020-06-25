# Install non-executable scripts
install(FILES
  conf.py.in
  CreateDoxyFile.cmake
  envdetect.py
  FinalizeHeadercheck.cmake
  index.rst.in
  InstallFile.cmake
  main77.cc.in
  module_library.cc.in
  pyversion.py
  RunDoxygen.cmake
  sphinx_cmake_dune.py
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/scripts)

# Install executable programs
install(PROGRAMS
  extract_cmake_data.py
  run-in-dune-env.sh.in
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/scripts
)
