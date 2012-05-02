set(MPITESTPROGS indicestest indexsettest syncertest selectiontest)

add_directory_test_target(_test_target)
# We do not want want to build the tests during make all,
# but just build them on demand
add_dependencies(${_test_target} ${MPITESTPROGS})

add_executable("indexsettest" indexsettest.cc)
target_link_libraries("indexsettest" "dunecommon" ${CMAKE_THREAD_LIBS_INIT} ${})

include(DuneMPI)
add_executable("indicestest" indicestest.cc)
target_link_libraries("indicestest" "dunecommon")
add_dune_mpi_flags(indicestest)

add_executable("selectiontest" selectiontest.cc)
target_link_libraries("selectiontest" "dunecommon")
add_dune_mpi_flags(selectiontest)

add_executable("syncertest" syncertest.cc)
target_link_libraries("syncertest" "dunecommon")
add_dune_mpi_flags(syncertest)

add_test(indexsettest			indexsettest)
add_test(selectiontest			selectiontest)
add_test(indicestest			indicestest)
add_test(syncertest			syncertest)
