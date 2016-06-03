install(FILES
  conf.py.in
  CreateDoxyFile.cmake
  extract_cmake_data.py
  FinalizeHeadercheck.cmake
  index.rst.in
  InstallFile.cmake
  main77.cc
  RunDoxygen.cmake
  sphinx_cmake_dune.py
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/scripts)
