add_executable("indexsettest" indexsettest.cc)
target_link_libraries("indexsettest" "dunecommon")

add_executable("indicestest" indicestest.cc)
target_link_libraries("indicestest" "dunecommon")
target_link_libraries(indicestest ${MPI_CXX_LIBRARIES})

add_executable("selectiontest" selectiontest.cc)
target_link_libraries("selectiontest" "dunecommon")
target_link_libraries(selectiontest ${MPI_CXX_LIBRARIES})

#add_executable("syncertest" syncertest.cc)
#target_link_libraries(syncertest ${MPI_CXX_LIBRARIES})
