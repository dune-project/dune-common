add_subdirectory(test)

#install headers
install(FILES
        blackchannelcommunicator.hh
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
        mpiwin.hh
        neighborindexmappingbuilder.hh
        plocalindex.hh
        pointtopointcommunication.hh
        remoteindices.hh
        ringcommunication.hh
        selection.hh
        span.hh
        variablesizecommunicator.hh
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/parallel)

