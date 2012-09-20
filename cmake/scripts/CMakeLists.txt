file(GLOB modules *.cmake)
install(FILES ${modules} ${CMAKE_CURRENT_SOURCE_DIR}/BuildTests.cmake.in
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/scripts)
