add_subdirectory(test)
add_subdirectory(benchmark)

#install headers
install(FILES
        collectivecommunication.hh
        communicator.hh
        indexset.hh
        indicessyncer.hh
        interface.hh
        localindex.hh
        mpicollectivecommunication.hh
        mpiguard.hh
        future.hh
        mpifuture.hh
        mpidata.hh
        mpihelper.hh
        mpitraits.hh
        plocalindex.hh
        remoteindices.hh
        selection.hh
        variablesizecommunicator.hh
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/parallel)

