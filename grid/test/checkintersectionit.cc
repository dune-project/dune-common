// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_CHECK_INTERSECTIONITERATOR_CC
#define DUNE_CHECK_INTERSECTIONITERATOR_CC

#include <set>

/** \file
    \brief Tests for the IntersectionIterator
 */

template<class K, int N>
struct ltfv
{
  bool operator()(const Dune::FieldVector<K,N> & v1, const Dune::FieldVector<K,N> & v2) const
  {
    for (int i=0; i<N; i++)
      if (v1[i] != v2[i]) return (v1[i] < v2[i]);
    return false;
  }
};

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
        if (iIt.levelNeighbor()) {
          typedef typename EntityType::EntityPointer EntityPointer;
          EntityPointer outside = iIt.outside();
          int numberInSelf     = iIt.numberInSelf();
          int numberInNeighbor = iIt.numberInNeighbor();

          assert(grid.levelIndexSet(i).template subIndex<1>(*eIt, numberInSelf)
                 == grid.levelIndexSet(i).template subIndex<1>(*outside, numberInNeighbor));

#ifndef DUNE_UGGRID_HH
          assert(grid.localIdSet().template subId<1>(*eIt, numberInSelf)
                 == grid.localIdSet().template subId<1>(*outside, numberInNeighbor));

          assert(grid.globalIdSet().template subId<1>(*eIt, numberInSelf)
                 == grid.globalIdSet().template subId<1>(*outside, numberInNeighbor));
#else
#warning Test disabled, as UG does not have Face IDs
#endif
        }
        if (iIt.leafNeighbor()) {
          typedef typename EntityType::EntityPointer EntityPointer;
          EntityPointer outside = iIt.outside();
          int numberInSelf     = iIt.numberInSelf();
          int numberInNeighbor = iIt.numberInNeighbor();

          // if entity is leaf entity then check leaf index set
          if((eIt->isLeaf()) && (outside->isLeaf()))
            assert(grid.leafIndexSet().template subIndex<1>(*eIt, numberInSelf)
                   == grid.leafIndexSet().template subIndex<1>(*outside, numberInNeighbor));

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

#ifndef DUNE_UGGRID_HH
        // The geometry center in local coordinates
        FieldVector<ctype, Geometry::mydimension> localCenter = intersectionGlobal.local(center);

        // Check whether center is within the intersection
        // This implicitly assumes convex intersections
        if (!intersectionGlobal.checkInside(localCenter))
          DUNE_THROW(GridError, "Center of intersectionGlobal is not within intersectionGlobal!");

        // Back to global coordinates to check for correctness
        FieldVector<ctype, Geometry::coorddimension> worldCenter = intersectionGlobal.global(localCenter);
        if ((center-worldCenter).infinity_norm() > 1e-6)
          DUNE_THROW(GridError, "local() and global() are not inverse to each other!");


        // The integration element at the element center
        ctype intElement = intersectionGlobal.integrationElement(localCenter);
        if (intElement <=0)
          DUNE_THROW(GridError, "nonpositive integration element found!");

        const FieldMatrix<ctype, Geometry::mydimension, Geometry::mydimension> jacobi
          = intersectionGlobal.jacobianInverseTransposed(localCenter);
#else
#warning Test disabled, as UG does support local-to-global for faces
#endif

        // //////////////////////////////////////////////////////////
        //   Check the geometry returned by intersectionSelfLocal()
        // //////////////////////////////////////////////////////////

        {
          const typename IntersectionIterator::LocalGeometry& intersectionSelfLocal = iIt.intersectionSelfLocal();
          if (intersectionSelfLocal.corners() <= 0)
            DUNE_THROW(GridError, "Local intersection has nonpositive number of corners!");

          // check points of intersection self
          for(int k=0; k<intersectionSelfLocal.corners(); ++k)
          {
            if (( eIt->geometry().global( intersectionSelfLocal[k] )
                  - intersectionGlobal[k] ).infinity_norm() > 1e-6)
              DUNE_THROW(GridError, "global( intersectionSelfLocal[" << k << "] ) is not the same as intersectionGlobal[" << k <<"]!");
          }
        }

        // ////////////////////////////////////////////////////////////////
        //   Check the geometry returned by intersectionNeighborLocal()
        // ////////////////////////////////////////////////////////////////

        if (iIt.levelNeighbor() || iIt.leafNeighbor()) {

          const typename IntersectionIterator::LocalGeometry& intersectionNeighborLocal = iIt.intersectionNeighborLocal();
          const typename IntersectionIterator::LocalGeometry& intersectionSelfLocal = iIt.intersectionSelfLocal();

          if (intersectionSelfLocal.corners() <= 0)
            DUNE_THROW(GridError, "Local intersection has nonpositive number of corners!");

          if (intersectionSelfLocal.corners() != intersectionNeighborLocal.corners())
            DUNE_THROW(GridError, "Geometry of intersection is incosistent from left and right hand side!");

          if (intersectionSelfLocal.corners() != intersectionGlobal.corners())
            DUNE_THROW(GridError, "Geometry of intersection is incosistent from left hand side and global view!");

          typedef typename EntityType::EntityPointer EntityPointer;
          EntityPointer outside = iIt.outside();
          EntityPointer inside = iIt.inside();

          typedef
          std::set< FieldVector<ctype,GridType::dimensionworld>, ltfv<ctype,GridType::dimensionworld> >
          CornerSet;
          CornerSet corners_self, corners_neighbor, corners_global;

          for(int k=0; k<intersectionSelfLocal.corners(); ++k) {
            corners_self.insert( inside->geometry().global( intersectionSelfLocal[k]) );
            corners_neighbor.insert( outside->geometry().global( intersectionNeighborLocal[k]) );
            corners_global.insert( intersectionGlobal[k] );
          }

          // check points of intersection neighbor local
          typename CornerSet::iterator s_i = corners_self.begin();
          typename CornerSet::iterator n_i = corners_neighbor.begin();
          typename CornerSet::iterator
          g_i = corners_global.begin();
          for(int k=0; k<intersectionNeighborLocal.corners(); ++k)
          {
            if (( *s_i - *g_i ).infinity_norm() > 1e-6)
              DUNE_THROW(GridError,
                         "global( intersectionSelfLocal[" << *s_i << "] ) is not the same as intersectionGlobal[" << *g_i <<
                         "] (delta_max = " << ( *s_i - *g_i ).infinity_norm() << ")!");
            if (( *n_i - *g_i ).infinity_norm() > 1e-6)
              DUNE_THROW(GridError,
                         "global( intersectionNeighborLocal[" << *n_i << "] ) is not the same as intersectionGlobal[" << *g_i <<
                         "] (delta_max = " << ( *n_i - *g_i ).infinity_norm() << ")!");
            if (( *s_i - *n_i ).infinity_norm() > 1e-6)
              DUNE_THROW(GridError,
                         "global( intersectionSelfLocal[" << *s_i <<
                         "] ) is not the same as global( intersectionNeighborLocal[" << *n_i <<
                         "] (delta_max = " << ( *s_i - *n_i ).infinity_norm() << ")!");
            s_i++;
            n_i++;
            g_i++;
          }

        }

      }

    }

  }

}

#endif
