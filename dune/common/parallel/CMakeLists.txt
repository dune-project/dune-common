add_subdirectory("test")

#install headers
install(FILES
        communicator.hh
        indexset.hh
        indicessyncer.hh
        interface.hh
        localindex.hh
        plocalindex.hh
        remoteindices.hh
        selection.hh
	DESTINATION include/dune/common/parallel)
