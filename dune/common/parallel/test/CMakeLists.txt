dune_add_test(SOURCES indexsettest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES remoteindicestest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4
              TIMEOUT 300
              CMAKE_GUARD MPI_FOUND
              LABELS quick)

dune_add_test(SOURCES selectiontest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES syncertest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4
              TIMEOUT 300
              CMAKE_GUARD MPI_FOUND
              LABELS quick)

dune_add_test(SOURCES variablesizecommunicatortest.cc
              MPI_RANKS 1 2 4
              TIMEOUT 300
              CMAKE_GUARD MPI_FOUND
              LABELS quick)

dune_add_test(SOURCES mpidatatest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 2
              TIMEOUT 300
              LABELS quick)

dune_add_test(SOURCES mpifuturetest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4
              TIMEOUT 300
              LABELS quick)

dune_add_test(SOURCES futurebatchtest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4
              TIMEOUT 300
              LABELS quick)
