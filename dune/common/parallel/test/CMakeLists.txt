set(MPITESTPROGS indicestest indexsettest syncertest selectiontest)

# We do not want want to build the tests during make all,
# but just build them on demand
add_dependencies(build_tests ${MPITESTPROGS})
add_executable("indexsettest" indexsettest.cc)
target_link_libraries("indexsettest" "dunecommon")

add_executable("indicestest" indicestest.cc)
target_link_libraries("indicestest" "dunecommon")
if(MPI_FOUND)
  target_link_libraries(indicestest ${MPI_CXX_LIBRARIES})
endif(MPI_FOUND)

add_executable("selectiontest" selectiontest.cc)
target_link_libraries("selectiontest" "dunecommon")
if(MPI_FOUND)
  target_link_libraries(selectiontest ${MPI_CXX_LIBRARIES})
endif(MPI_FOUND)

#add_executable("syncertest" syncertest.cc)
#if(MPI_FOUND)
#  target_link_libraries(syncertest ${MPI_CXX_LIBRARIES})
#endif(MPI_FOUND)

add_test(indexsettest			indexsettest)
add_test(selectiontest			selectiontest)
if(MPI_CXX_FOUND)
  add_test(indicestest			indicestest)
  #add_test(syncertest			syncertest)
endif(MPI_CXX_FOUND)
