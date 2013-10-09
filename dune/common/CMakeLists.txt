add_subdirectory("parallel")
add_subdirectory("std")
add_subdirectory("test" EXCLUDE_FROM_ALL)

#build the library dunecommon
if(LAPACK_FOUND)
  set(_additional_libs ${LAPACK_LIBRARIES})
elseif(BLAS_FOUND)
  set(_additional_libs ${BLAS_LIBRARIES})
endif(LAPACK_FOUND)

dune_add_library("dunecommon"
  debugallocator.cc
  dynmatrixev.cc
  exceptions.cc
  fmatrixev.cc
  ios_state.cc
  parametertree.cc
  parametertreeparser.cc
  path.cc
  stdstreams.cc
  ADD_LIBS "${_additional_libs}")

#install headers
install(FILES
        alignment.hh
        array.hh
        arraylist.hh
        bartonnackmanifcheck.hh
        bigunsignedint.hh
        binaryfunctions.hh
        bitsetvector.hh
        classname.hh
        collectivecommunication.hh
        debugallocator.hh
        debugstream.hh
        deprecated.hh
        densematrix.hh
        densevector.hh
	diagonalmatrix.hh
        documentation.hh
	dotproduct.hh
        dynmatrix.hh
	dynmatrixev.hh
        dynvector.hh
        enumset.hh
        exceptions.hh
        fassign.hh
        float_cmp.cc
        float_cmp.hh
        fmatrix.hh
        fmatrixev.hh
        forloop.hh
        ftraits.hh
        function.hh
        fvector.hh
        gcd.hh
        genericiterator.hh
        gmpfield.hh
        hash.hh
        identitymatrix.hh
        indent.hh
        interfaces.hh
        ios_state.hh
        iteratorfacades.hh
        lcm.hh
        lru.hh
        mallocallocator.hh
        math.hh
        matvectraits.hh
        misc.hh
        mpicollectivecommunication.hh
        mpiguard.hh
        mpihelper.hh
        mpitraits.hh
        nullptr.hh
        parametertree.hh
        parametertreeparser.hh
        path.hh
        poolallocator.hh
        power.hh
        precision.hh
        propertymap.hh
	promotiontraits.hh
        reservedvector.hh
        shared_ptr.hh
        singleton.hh
        sllist.hh
        static_assert.hh
        stdstreams.hh
        stringutility.hh
        timer.hh
        tuples.hh
        tupleutility.hh
        typetraits.hh
        unused.hh
        version.hh
        visibility.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common)

# Install some test headers, because they get used by tests in other modules
# We do this here as test will not be considered for make install
install(FILES test/iteratortest.hh DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/test)
