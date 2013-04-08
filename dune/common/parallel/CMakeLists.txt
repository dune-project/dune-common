add_subdirectory("test" EXCLUDE_FROM_ALL)

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
        mpihelper.hh
        mpitraits.hh
        plocalindex.hh
        remoteindices.hh
        selection.hh
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/parallel)

