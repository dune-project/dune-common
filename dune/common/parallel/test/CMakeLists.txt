dune_add_test(SOURCES indexsettest.cc
              LABELS quick)

dune_add_test(SOURCES remoteindicestest.cc
              MPI_RANKS 1 2 4
              TIMEOUT 300
              CMAKE_GUARD MPI_FOUND
              LABELS quick)

dune_add_test(SOURCES selectiontest.cc
              LABELS quick)

dune_add_test(SOURCES syncertest.cc
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
              MPI_RANKS 2
              TIMEOUT 300
              LABELS quick)

dune_add_test(SOURCES mpifuturetest.cc
              MPI_RANKS 1 2 4
              TIMEOUT 300
              LABELS quick)

dune_add_test(SOURCES mpipacktest.cc
              MPI_RANKS 2
              TIMEOUT 300
              CMAKE_GUARD MPI_FOUND
              LABELS quick)
