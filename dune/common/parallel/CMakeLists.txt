add_subdirectory("test" EXCLUDE_FROM_ALL)

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
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/parallel)

