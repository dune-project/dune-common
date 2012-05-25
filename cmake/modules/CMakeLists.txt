file(GLOB modules *.cmake)
install(FILES ${modules} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cmake/modules)
