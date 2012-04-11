set(MPITESTPROGS indicestest indexsettest syncertest selectiontest)

# We do not want want to build the tests during make all,
# but just build them on demand
add_dependencies(build_tests ${MPITESTPROGS})
add_executable("indexsettest" indexsettest.cc)
target_link_libraries("indexsettest" "dunecommon" ${CMAKE_THREAD_LIBS_INIT} ${})

include(DUNEMPI)
add_executable("indicestest" indicestest.cc)
target_link_libraries("indicestest" "dunecommon")
add_DUNE_MPI_flags(indicestest)

add_executable("selectiontest" selectiontest.cc)
target_link_libraries("selectiontest" "dunecommon")
add_DUNE_MPI_flags(selectiontest)

add_executable("syncertest" syncertest.cc)
target_link_libraries("syncertest" "dunecommon")
add_DUNE_MPI_flags(syncertest)

add_test(indexsettest			indexsettest)
add_test(selectiontest			selectiontest)
add_test(indicestest			indicestest)
add_test(syncertest			syncertest)
