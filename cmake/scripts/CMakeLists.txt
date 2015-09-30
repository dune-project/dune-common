install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/BuildTests.cmake.in
  CreateDoxyFile.cmake
  FinalizeHeadercheck.cmake
  FindFiles.cmake
  InstallFile.cmake
  RunDoxygen.cmake
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dune/cmake/scripts)
