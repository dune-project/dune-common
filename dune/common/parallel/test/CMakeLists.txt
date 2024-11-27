# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Link all test targets in this directory against Dune::Common
link_libraries(Dune::Common)

dune_add_test(SOURCES communicationtest.cc
              MPI_RANKS 1 2 4
              TIMEOUT 300
              CMAKE_GUARD MPI_FOUND
              LABELS quick)

dune_add_test(SOURCES indexsettest.cc
              LABELS quick)

dune_add_test(SOURCES parmetistest.cc
              MPI_RANKS 3
              TIMEOUT 300
              CMAKE_GUARD "HAVE_MPI AND ParMETIS_FOUND"
              LABELS quick)
add_dune_mpi_flags(parmetistest)
add_dune_parmetis_flags(parmetistest)

dune_add_test(SOURCES remoteindicestest.cc
              MPI_RANKS 1 2 4
              TIMEOUT 300
              CMAKE_GUARD HAVE_MPI
              LABELS quick)

dune_add_test(SOURCES selectiontest.cc
              LABELS quick)

dune_add_test(SOURCES syncertest.cc
              MPI_RANKS 1 2 4
              TIMEOUT 300
              CMAKE_GUARD HAVE_MPI
              LABELS quick)

dune_add_test(SOURCES variablesizecommunicatortest.cc
              MPI_RANKS 1 2 4
              TIMEOUT 300
              CMAKE_GUARD HAVE_MPI
              LABELS quick)

dune_add_test(SOURCES mpidatatest.cc
              MPI_RANKS 2
              TIMEOUT 300
              LABELS quick)

dune_add_test(SOURCES mpifuturetest.cc
              CMAKE_GUARD MPI_FOUND
              MPI_RANKS 1 2 4
              TIMEOUT 300
              LABELS quick)

dune_add_test(SOURCES mpipacktest.cc
              MPI_RANKS 2
              TIMEOUT 300
              CMAKE_GUARD HAVE_MPI
              LABELS quick)

dune_add_test(SOURCES mpigatherscattertest.cc
              MPI_RANKS 2
              TIMEOUT 300
              CMAKE_GUARD MPI_FOUND
              LABELS quick)
