// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRID_RENUMBERER_HH
#define DUNE_UGGRID_RENUMBERER_HH

/** \file
    \brief Contains a helper class that encapsulates the vertex numbering conversions
    between UG and DUNE
 */

namespace Dune {

  /** \brief Empty generic class.  All we need is in the specializations for dim=2 and dim=3
   */
  template <int dim>
  class UGGridRenumberer {};

  /** \brief DUNE and UG use different local numberings for the subentities of elements.
      This class does the conversions for 2d-grids.
   */
  template <>
  class UGGridRenumberer<2> {

  public:

    /** \brief Turn a local vertex number from DUNE numbering to UG numbering */
    static int verticesDUNEtoUG(int i, NewGeometryType type) {

      if (type.isCube()) {
        const int renumbering[4] = {0, 1, 3, 2};
        return renumbering[i];
      }

      return i;
    }

    /** \brief Turn a local face number from DUNE numbering to UG numbering */
    static int facesDUNEtoUG(int i, NewGeometryType type) {

      if (type.isCube()) {

        // Dune numbers the vertices of a hexahedron and quadrilaterals differently than UG.
        // The following two lines do the transformation
        // The renumbering scheme is {0,1,3,2} for quadrilaterals, therefore, the
        // following code works for 2d and 3d.
        const int renumbering[6] = {4, 2, 1, 3, 0, 5};
        return renumbering[i];

      }
      if (type.isSimplex()) {

        /** \todo Check this */
        // Dune numbers the vertices of a hexahedron and quadrilaterals differently than UG.
        // The following two lines do the transformation
        // The renumbering scheme is {0,1,3,2} for quadrilaterals, therefore, the
        // following code works for 2d and 3d.
        const int renumbering[4] = {1, 2, 0, 3};
        return renumbering[i];
      }
      return i;
    }

  };

  /** \brief DUNE and UG use different local numberings for the subentities of elements.
      This class does the conversions for 3d-grids.
   */
  template <>
  class UGGridRenumberer<3> {

  public:

    /** \brief Turn a local vertex number from DUNE numbering to UG numbering */
    static int verticesDUNEtoUG(int i, NewGeometryType type) {

      if (type.isCube()) {
        const int renumbering[8] = {0, 1, 3, 2, 4, 5, 7, 6};
        return renumbering[i];
      }

      return i;
    }

    /** \brief Turn a local face number from DUNE numbering to UG numbering */
    static int facesDUNEtoUG(int i, NewGeometryType type) {

      if (type.isCube()) {

        // Dune numbers the vertices of a hexahedron and quadrilaterals differently than UG.
        // The following two lines do the transformation
        // The renumbering scheme is {0,1,3,2} for quadrilaterals, therefore, the
        // following code works for 2d and 3d.
        const int renumbering[6] = {4, 2, 1, 3, 0, 5};
        return renumbering[i];

      }
      if (type.isSimplex()) {

        /** \todo Check this */
        // Dune numbers the vertices of a hexahedron and quadrilaterals differently than UG.
        // The following two lines do the transformation
        // The renumbering scheme is {0,1,3,2} for quadrilaterals, therefore, the
        // following code works for 2d and 3d.
        const int renumbering[4] = {1, 2, 0, 3};
        return renumbering[i];
      }
      return i;
    }

  };
}

#endif
