// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_CHECK_INTERSECTIONITERATOR_CC
#define DUNE_CHECK_INTERSECTIONITERATOR_CC

/** \file
    \brief Tests for the IntersectionIterator
 */

/** \brief Test the IntersectionIterator

 */
template <class GridType>
void checkIntersectionIterator(const GridType& grid) {

  using namespace Dune;

  typedef typename GridType::ctype ctype;

  // Loop over all levels
  for (int i=0; i<=grid.maxLevel(); i++) {

    typedef typename GridType::template Codim<0>::LevelIterator ElementIterator;
    ElementIterator eIt    = grid.template lbegin<0>(i);
    ElementIterator eEndIt = grid.template lend<0>(i);

    for (; eIt!=eEndIt; ++eIt) {

      typedef typename GridType::template Codim<0>::Entity EntityType;
      typedef typename EntityType::IntersectionIterator IntersectionIterator;

      IntersectionIterator iIt    = eIt->ibegin();
      IntersectionIterator iEndIt = eIt->iend();

      for (; iIt!=iEndIt; ++iIt) {

        // /////////////////////////////////////////////////////////
        //   Check the types defined by the iterator
        // /////////////////////////////////////////////////////////
        IsTrue< SameType<
                typename IntersectionIterator::ctype,
                typename GridType::ctype>::value == true >::yes();

        IsTrue<static_cast<int>(IntersectionIterator::dimension)
            == static_cast<int>(GridType::dimension)>::yes();

        IsTrue<static_cast<int>(IntersectionIterator::dimensionworld)
            == static_cast<int>(GridType::dimensionworld)>::yes();

        // //////////////////////////////////////////////////////////
        //   Check various methods
        // //////////////////////////////////////////////////////////

        assert(iIt.level() == i);

        assert(eIt == iIt.inside());

        // /////////////////////////////////////////////////////////////
        //   Check the consistency of numberInSelf, numberInNeighbor
        //   and the indices of the subface between.
        // /////////////////////////////////////////////////////////////
        if (iIt.neighbor()) {

          typedef typename EntityType::EntityPointer EntityPointer;
          EntityPointer outside = iIt.outside();
          int numberInSelf     = iIt.numberInSelf();
          int numberInNeighbor = iIt.numberInNeighbor();

          if (outside->level() == eIt->level()) {

            assert(grid.levelIndexSet(i).template subIndex<1>(*eIt, numberInSelf)
                   == grid.levelIndexSet(i).template subIndex<1>(*outside, numberInNeighbor));

            assert(grid.leafIndexSet().template subIndex<1>(*eIt, numberInSelf)
                   == grid.leafIndexSet().template subIndex<1>(*outside, numberInNeighbor));

            assert(grid.localIdSet().template subId<1>(*eIt, numberInSelf)
                   == grid.localIdSet().template subId<1>(*outside, numberInNeighbor));

            assert(grid.globalIdSet().template subId<1>(*eIt, numberInSelf)
                   == grid.globalIdSet().template subId<1>(*outside, numberInNeighbor));

          }

        }

        // //////////////////////////////////////////////////////////
        //   Check the geometry returned by intersectionGlobal()
        // //////////////////////////////////////////////////////////
        typedef typename IntersectionIterator::Geometry Geometry;
        const Geometry& intersectionGlobal = iIt.intersectionGlobal();

        if (intersectionGlobal.corners() <= 0)
          DUNE_THROW(GridError, "Global intersection has nonpositive number of corners!");

        // Compute the element center just to have an argument for the following methods
        FieldVector<ctype, Geometry::coorddimension> center(0);
        for (int j=0; j<intersectionGlobal.corners(); j++)
          center += intersectionGlobal[j];

        center /= intersectionGlobal.corners();

        // The geometry center in local coordinates
        FieldVector<ctype, Geometry::mydimension> localCenter = intersectionGlobal.local(center);

        // Back to global coordinates to check for correctness
        FieldVector<ctype, Geometry::coorddimension> worldCenter = intersectionGlobal.global(localCenter);
        if ((center-worldCenter).infinity_norm() > 1e-6)
          DUNE_THROW(GridError, "local() and global() are not inverse to each other!");


#if 0
        // The integration element at the element center
        ctype intElement = intersectionGlobal.integrationElement(localCenter);
        if (intElement <=0)
          DUNE_THROW(GridError, "nonpositive integration element found!");

        const FieldMatrix<ctype, Geometry::mydimension, Geometry::mydimension> jacobi
          = intersectionGlobal.jacobianInverseTransposed(localCenter);
#endif

        // //////////////////////////////////////////////////////////
        //   Check the geometry returned by intersectionSelfLocal()
        // //////////////////////////////////////////////////////////

        const typename IntersectionIterator::LocalGeometry& intersectionSelfLocal = iIt.intersectionSelfLocal();

        if (intersectionSelfLocal.corners() <= 0)
          DUNE_THROW(GridError, "Local intersection has nonpositive number of corners!");

        // ////////////////////////////////////////////////////////////////
        //   Check the geometry returned by intersectionNeighborLocal()
        // ////////////////////////////////////////////////////////////////

        if (iIt.neighbor()) {

          const typename IntersectionIterator::LocalGeometry& intersectionNeighborLocal = iIt.intersectionNeighborLocal();

          if (intersectionNeighborLocal.corners() <= 0)
            DUNE_THROW(GridError, "Local intersection has nonpositive number of corners!");

        }

      }
    }

  }

}

#endif
