// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/** \file
 * \brief Contains quadrature formulas taken form UG
 */
#ifndef DUNE_UG_QUADRATURES_HH
#define DUNE_UG_QUADRATURES_HH

namespace Dune {

  namespace UG_Quadratures {

    enum { dimension = 3 };

    typedef double DOUBLE_VECTOR[dimension];
    typedef double DOUBLE_VECTOR_3D[3];

    /****************************************************************************/
    /*                                                                          */
    /* data structures exported by the corresponding source file                */
    /*                                                                          */
    /****************************************************************************/

    /** \brief %Quadrature formula borrowed from UG */
    struct QUADRATURE {

      /** \brief Number of integration points     */
      int nip;

      /** \brief Order of quadrature rule         */
      int order;

      /** \brief %Array[nip] for local coordinates */
      const DOUBLE_VECTOR_3D *local;

      /** \brief %Array[nip] for weights           */
      const double *weight;
    };

    struct GAUSS_POINT {
      DOUBLE_VECTOR local;
      DOUBLE_VECTOR global;
      double weight;
      DOUBLE_VECTOR Jinv[dimension];
    };

    /****************************************************************************/
    /*                                                                          */
    /* function declarations                                                    */
    /*                                                                          */
    /****************************************************************************/

    /****************************************************************************/
    /** \brief Provide a quadrature formula

       \param dim - dimension of the quadrature domain
       \param n - number of corners of the element
       \param order - order of approximation

       This function returns a pointer to a quadrature formula.

       \section Example
       \verbatim
       QUADRATURE *quadrature;

       if ((quadrature = GetQuadrature(2,3,2)) == NULL)
              return(1);
       sum = 0.0;
       for (l=0; l<Q_NIP(quadrature); l++)
       {
        LOCAL_TO_GLOBAL(n,x,Q_LOCAL(quadrature,l),global);
        (*SourceFunc)(global,val);
        sum += val * Q_WEIGHT(quadrature,l);
       }
       sum = val * AreaOfTriangle;
       \endverbatim

       \return <ul>
       <li> pointer to quadrature </li>
       <li> NULL if the quadrature formula cannot be found </li>
       </ul>
     */
    inline QUADRATURE *GetQuadrature(int dim, int n, int order);


    /****************************************************************************/
    /** \brief Provide a quadrature formula

       \param dim - dimension of the formula
       \param n - number of corners of the element
       \param order - order of approximation

       This function returns a pointer to a quadrature formula.

       It is different from GetQuadrature() in the case that the quadrature
       formula of order 'order' is not exactly available.  'GetQuadrature'
       just returns the highest order formula, GetQuadratureRule() returns
       the formula of the smallest degree that integrates exactly until 'order'.

       \section Example
       \verbatim
       QUADRATURE *quadrature;

       if ((quadrature = GetQuadratureRule(2,3,2)) == NULL)
              return(1);
       sum = 0.0;
       for (l=0; l<Q_NIP(quadrature); l++)
       {
        LOCAL_TO_GLOBAL(n,x,Q_LOCAL(quadrature,l),global);
        (*SourceFunc)(global,val);
        sum += val * Q_WEIGHT(quadrature,l);
       }
       sum = val * AreaOfTriangle;
       \endverbatim

       \return <ul>
       <li> pointer to quadrature </li>
       <li> NULL if the quadrature formula cannot be found </li>
       </ul>
     */
    /****************************************************************************/
    inline QUADRATURE *GetQuadratureRule(int dim, int n, int order);

    /****************************************************************************/
    /** \brief Provide a quadrature formula with axial-symmetric integration points

       \param dim - dimension of the formular
       \param n - number of corners of the element
       \param order - order of approximation

       This function returns a pointer to a quadrature formula.  The
       quadrature rule uses symmetric integration points in 1D and 2D,
       i.e. the integration points are symmetric w.r.t. the point 0.5 in
       the 1D case and symmetric to the x- and y-axis in 2D.

       It is different from GetQuadrature in the case that the quadrature
       formula of order 'order' is not exactly available.  'GetQuadrature'
       just returns the highest order formula, GetQuadratureRule returns
       the formula of the smallest degree that integrates exactly until 'order'.

       \section Example
       \verbatim
       QUADRATURE *quadrature;

       if ((quadrature = GetSymmetricQuadratureRule(2,3,2)) == NULL)
              return(1);
       sum = 0.0;
       for (l=0; l<Q_NIP(quadrature); l++)
       {
        LOCAL_TO_GLOBAL(n,x,Q_LOCAL(quadrature,l),global);
        (*SourceFunc)(global,val);
        sum += val * Q_WEIGHT(quadrature,l);
       }
       sum = val * AreaOfTriangle;
       \endverbatim

       \return <ul>
       <li> pointer to quadrature </li>
       <li> NULL if the quadrature formula cannot be found </li>
       </ul>
     */
    /****************************************************************************/

    inline QUADRATURE *GetSymmetricQuadratureRule(int dim, int n, int order);
    inline int         GaussPoints(int dim, int n, int order, DOUBLE_VECTOR *x, GAUSS_POINT *gp);

#include "ugquadratures.cc"

  } // end namespace UGQuadratures

} // end namespace Dune

#endif
