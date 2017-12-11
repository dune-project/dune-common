dune_add_test(SOURCES indexsettest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES remoteindicestest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4
              TIMEOUT 300
              CMAKE_GUARD MPI_FOUND)

dune_add_test(SOURCES selectiontest.cc
              LINK_LIBRARIES dunecommon)

dune_add_test(SOURCES syncertest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4
              TIMEOUT 300
              CMAKE_GUARD MPI_FOUND)

dune_add_test(SOURCES variablesizecommunicatortest.cc
              MPI_RANKS 1 2 4
              TIMEOUT 300
              CMAKE_GUARD MPI_FOUND)
