// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_CHECK_GEOMETRYINFATHER_CC
#define DUNE_CHECK_GEOMETRYINFATHER_CC

#include <dune/common/typetraits.hh>

/** \file
    \brief A test for the Method Geometry::geometryInFather()
 */

/** \brief Test the Method Geometry::geometryInFather()

   This test works by comparing the output of geometryInFather with the vertex positions
   obtained by directly expressing the son vertex positions in local coordinates of the
   father.  That should work for all grid implementations that realize truly nested
   grids.  One exception is UGGrid with parametrized boundaries.
 */
template <class GridType>
void checkGeometryInFather(const GridType& grid) {

  using namespace Dune;

  typedef typename GridType::ctype ctype;
  const int dimworld = GridType::dimensionworld;

  // We need at least two levels to do any checking
  if (grid.maxLevel()==0)
    DUNE_THROW(GridError, "Cannot check geometryInFather() on a single-level grid!");

  // Loop over all levels except the lowest one
  for (int i=1; i<=grid.maxLevel(); i++) {

    typedef typename GridType::template Codim<0>::LevelIterator ElementIterator;
    ElementIterator eIt    = grid.template lbegin<0>(i);
    ElementIterator eEndIt = grid.template lend<0>(i);

    for (; eIt!=eEndIt; ++eIt) {

      // Get geometry in father
      typedef typename GridType::template Codim<0>::Entity::Geometry Geometry;

      const Geometry& geometryInFather = eIt->geometryInFather();

      // //////////////////////////////////////////////////////
      //   Check for types and constants
      // //////////////////////////////////////////////////////


      /** \todo How do I check whether Geometry::ctype == GridType::ctype ? */
      IsTrue< SameType<
              typename Geometry::ctype,
              typename GridType::ctype>::value == true >::yes();

      IsTrue<static_cast<int>(Geometry::dimension)
          == static_cast<int>(GridType::dimension)>::yes();

      IsTrue<static_cast<int>(Geometry::mydimension)
          == static_cast<int>(GridType::dimension)>::yes();

      IsTrue<static_cast<int>(Geometry::coorddimension)
          == static_cast<int>(GridType::dimensionworld)>::yes();

      IsTrue<static_cast<int>(Geometry::dimensionworld)
          == static_cast<int>(GridType::dimensionworld)>::yes();

      // ///////////////////////////////////////////////////////
      //   Check the different methods
      // ///////////////////////////////////////////////////////
      if (geometryInFather.type() != eIt->geometry().type())
        DUNE_THROW(GridError, "Type of geometry and geometryInFather differ!");

      if (geometryInFather.corners() != eIt->geometry().corners())
        DUNE_THROW(GridError, "entity and geometryInFather have different number of corners!");

      // Compute the element center just to have an argument for the following methods
      FieldVector<ctype, Geometry::coorddimension> center(0);
      for (int j=0; j<geometryInFather.corners(); j++)
        center += geometryInFather[j];

      if (geometryInFather.integrationElement(center) <=0)
        DUNE_THROW(GridError, "nonpositive integration element found!");

      /** \todo Missing local() */
      /** \todo Missing global() */
      /** \todo Missing jacobianInverse() */
      /** \todo Missing checkInside() */

      // /////////////////////////////////////////////////////////////////////////////////////
      // Check whether the positions of the vertices of geometryInFather coincide
      // with the ones computed 'by hand'.  This only works if the grids really are nested!
      // /////////////////////////////////////////////////////////////////////////////////////
      for (int j=0; j<geometryInFather.corners(); j++) {

        FieldVector<ctype, dimworld> localPos = eIt->father()->geometry().local(eIt->geometry()[j]);

        if ( (localPos-geometryInFather[j]).infinity_norm() > 1e-7)
          DUNE_THROW(GridError, "geometryInFather yields wrong vertex position!");

      }

    }

  }

}

#endif
