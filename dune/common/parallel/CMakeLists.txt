add_subdirectory(test)

#install headers
install(FILES
        collectivecommunication.hh
        communicator.hh
        future.hh
        indexset.hh
        indicessyncer.hh
        interface.hh
        localindex.hh
        managedmpicomm.hh
        mpicollectivecommunication.hh
        mpiexceptions.hh
        mpifuture.hh
        mpiguard.hh
        mpihelper.hh
        mpipack.hh
        mpistatus.hh
        mpitraits.hh
        plocalindex.hh
        pointtopointcommunication.hh
        remoteindices.hh
        selection.hh
        span.hh
        variablesizecommunicator.hh
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/parallel)

