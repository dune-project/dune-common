# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

add_subdirectory("parallel")
add_subdirectory("simd")
add_subdirectory("std")
add_subdirectory("test")

# add some sources to the dunecommon library
target_sources(dunecommon PRIVATE
  debugalign.cc
  debugallocator.cc
  exceptions.cc
  fmatrixev.cc
  ios_state.cc
  parametertree.cc
  parametertreeparser.cc
  path.cc
  simd/test.cc
  stdstreams.cc
  stdthread.cc)

#install headers
install(FILES
        alignedallocator.hh
        arraylist.hh
        assertandreturn.hh
        bartonnackmanifcheck.hh
        bigunsignedint.hh
        binaryfunctions.hh
        bitsetvector.hh
        boundschecking.hh
        classname.hh
        concept.hh
        conditional.hh
        debugalign.hh
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
        filledarray.hh
        float_cmp.cc
        float_cmp.hh
        fmatrix.hh
        fmatrixev.hh
        ftraits.hh
        function.hh
        fvector.hh
        genericiterator.hh
        gmpfield.hh
        hash.hh
        hybridutilities.hh
        indent.hh
        indices.hh
        interfaces.hh
        ios_state.hh
        iteratorfacades.hh
        iteratorrange.hh
        keywords.hh
        lru.hh
        mallocallocator.hh
        math.hh
        matrixconcepts.hh
        matvectraits.hh
        overloadset.hh
        parameterizedobject.hh
        parametertree.hh
        parametertreeparser.hh
        path.hh
        poolallocator.hh
        power.hh
        precision.hh
        propertymap.hh
        promotiontraits.hh
        proxymemberaccess.hh
        quadmath.hh
        rangeutilities.hh
        referencehelper.hh
        reservedvector.hh
        scalarvectorview.hh
        scalarmatrixview.hh
        shared_ptr.hh
        simd.hh
        singleton.hh
        sllist.hh
        stdstreams.hh
        stdthread.hh
        streamoperators.hh
        stringutility.hh
        to_unique_ptr.hh
        timer.hh
        transpose.hh
        tupleutility.hh
        tuplevector.hh
        typelist.hh
        typetraits.hh
        typeutilities.hh
        unused.hh
        vc.hh
        version.hh
        visibility.hh
DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common)

# Install some test headers, because they get used by tests in other modules
# We do this here as test will not be considered for make install
install(FILES test/iteratortest.hh
  test/checkmatrixinterface.hh
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dune/common/test)
