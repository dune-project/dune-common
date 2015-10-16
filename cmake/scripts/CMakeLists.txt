install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/BuildTests.cmake.in
  conf.py.in
  CreateDoxyFile.cmake
  extract_cmake_data.py
  FinalizeHeadercheck.cmake
  FindFiles.cmake
  index.rst.in
  InstallFile.cmake
  RunDoxygen.cmake
  sphinx_cmake_dune.py
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/scripts)
