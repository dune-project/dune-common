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
	DESTINATION include/dune/common/parallel)

add_test(indexsettest			indexsettest)
add_test(selectiontest			selectiontest)
if(MPI_CXX_FOUND)
  add_test(indicestest			indicestest)
  #add_test(syncertest			syncertest)
endif(MPI_CXX_FOUND)
