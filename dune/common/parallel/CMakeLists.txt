# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

add_subdirectory(test)
add_subdirectory(benchmark)

#install headers
install(FILES
        collectivecommunication.hh
        communication.hh
        communicator.hh
        indexset.hh
        indicessyncer.hh
        interface.hh
        localindex.hh
        mpicollectivecommunication.hh
        mpicommunication.hh
        mpiguard.hh
        future.hh
        mpifuture.hh
        mpidata.hh
        mpipack.hh
        mpihelper.hh
        mpitraits.hh
        plocalindex.hh
        remoteindices.hh
        selection.hh
        variablesizecommunicator.hh
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/parallel)
