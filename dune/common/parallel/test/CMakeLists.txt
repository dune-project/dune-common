dune_add_test(SOURCES indexsettest.cc
              LINK_LIBRARIES dunecommon
              CMAKE_GUARD MPI_FOUND
              MPI_RANKS 1 2
              TIMEOUT 300
              COMMAND ${CMAKE_CURRENT_BINARY_DIR}/indexsettest)

dune_add_test(SOURCES remoteindicestest.cc
              LINK_LIBRARIES dunecommon
              CMAKE_GUARD MPI_FOUND
              MPI_RANKS 1 2
              TIMEOUT 300
              COMMAND ${CMAKE_CURRENT_BINARY_DIR}/remoteindicestest)

dune_add_test(SOURCES selectiontest.cc
              LINK_LIBRARIES dunecommon
              CMAKE_GUARD MPI_FOUND
              MPI_RANKS 1 2
              TIMEOUT 300
              COMMAND ${CMAKE_CURRENT_BINARY_DIR}/selectiontest)

dune_add_test(SOURCES syncertest.cc
              LINK_LIBRARIES dunecommon
              CMAKE_GUARD MPI_FOUND
              MPI_RANKS 1 2
              TIMEOUT 300
              COMMAND ${CMAKE_CURRENT_BINARY_DIR}/syncertest)

dune_add_test(SOURCES variablesizecommunicatortest.cc
              LINK_LIBRARIES dunecommon
              CMAKE_GUARD MPI_FOUND
              MPI_RANKS 1 2
              TIMEOUT 300
              COMMAND ${CMAKE_CURRENT_BINARY_DIR}/variablesizecommunicatortest)

dune_add_test(SOURCES mpipacktest.cc
              LINK_LIBRARIES dunecommon
              CMAKE_GUARD
              MPI_RANKS 1 2
              TIMEOUT 300
              COMMAND ${CMAKE_CURRENT_BINARY_DIR}/mpipacktest)

dune_add_test(SOURCES pointtopointtest.cc
              LINK_LIBRARIES dunecommon
              CMAKE_GUARD MPI_FOUND
              MPI_RANKS 1 2
              TIMEOUT 300
              COMMAND ${CMAKE_CURRENT_BINARY_DIR}/pointtopointtest)

dune_add_test(SOURCES collectivecommunicationtest.cc
              LINK_LIBRARIES dunecommon
              CMAKE_GUARD MPI_FOUND
              MPI_RANKS 1 2
              TIMEOUT 300
              COMMAND ${CMAKE_CURRENT_BINARY_DIR}/collectivecommunicationtest)
