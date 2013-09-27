set(modules
  CreateDoxyFile.cmake
  FinalizeHeadercheck.cmake
  FindFiles.cmake
  InstallFile.cmake
  RunDoxygen.cmake)

install(FILES ${modules} ${CMAKE_CURRENT_SOURCE_DIR}/BuildTests.cmake.in
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/scripts)
