add_subdirectory(common)
add_subdirectory(test)

set(HEADERS
  extensions.h
)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/python)
