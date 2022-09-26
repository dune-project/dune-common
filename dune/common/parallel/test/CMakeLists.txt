# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

dune_add_test(SOURCES communicationtest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4
              TIMEOUT 300
              CMAKE_GUARD MPI_FOUND
              LABELS quick)

dune_add_test(SOURCES indexsettest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES remoteindicestest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 1 2 4
              TIMEOUT 300
              CMAKE_GUARD HAVE_MPI
              LABELS quick)

dune_add_test(SOURCES selectiontest.cc
              LINK_LIBRARIES dunecommon
              LABELS quick)

dune_add_test(SOURCES syncertest.cc
              LINK_LIBRARIES dunecommon
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
              LINK_LIBRARIES dunecommon
              MPI_RANKS 2
              TIMEOUT 300
              LABELS quick)

dune_add_test(SOURCES mpifuturetest.cc
              LINK_LIBRARIES dunecommon
              CMAKE_GUARD MPI_FOUND
              MPI_RANKS 1 2 4
              TIMEOUT 300
              LABELS quick)

dune_add_test(SOURCES mpipacktest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 2
              TIMEOUT 300
              CMAKE_GUARD HAVE_MPI
              LABELS quick)

dune_add_test(SOURCES mpigatherscattertest.cc
              LINK_LIBRARIES dunecommon
              MPI_RANKS 2
              TIMEOUT 300
              CMAKE_GUARD MPI_FOUND
              LABELS quick)
