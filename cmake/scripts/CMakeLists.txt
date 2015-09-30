set(modules
  CreateDoxyFile.cmake
  extract_cmake_data.py
  FinalizeHeadercheck.cmake
  FindFiles.cmake
  InstallFile.cmake
  RunDoxygen.cmake
  sphinx_cmake_dune.py)

install(FILES ${modules} ${CMAKE_CURRENT_SOURCE_DIR}/BuildTests.cmake.in
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/scripts)
