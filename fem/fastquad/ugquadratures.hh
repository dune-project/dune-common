// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//************************************************************************
// File:  former UG quadrature.h
//************************************************************************
#ifndef __DUNE_UG_QUADRATURE_HH__
#define __DUNE_UG_QUADRATURE_HH__

namespace Dune {

  namespace UG_Quadratures {

    enum { dimension = 3 };

    typedef double DOUBLE;
    typedef int INT;

    typedef DOUBLE DOUBLE_VECTOR[dimension];
    typedef DOUBLE DOUBLE_VECTOR_3D[3];

    /****************************************************************************/
    /*                                                                          */
    /* data structures exported by the corresponding source file                */
    /*                                                                          */
    /****************************************************************************/

    typedef struct quadrature_struct QUADRATURE;

    struct quadrature_struct {
      INT nip;                            /* number of integration points     */
      INT order;    /* NEW ! */           /* order of quadrature rule         */
      const DOUBLE_VECTOR_3D *local;      /* array[nip] for local coordinates */
      const DOUBLE *weight;                             /* array[nip] for weights           */
    };

    typedef struct gauss_point_struct GAUSS_POINT;

    struct gauss_point_struct {
      DOUBLE_VECTOR local;
      DOUBLE_VECTOR global;
      DOUBLE weight;
      DOUBLE_VECTOR Jinv[dimension];
    };

    /****************************************************************************/
    /*                                                                          */
    /* function declarations                                                    */
    /*                                                                          */
    /****************************************************************************/

    inline QUADRATURE *GetQuadrature(INT dim, INT n, INT order);
    inline QUADRATURE *GetQuadratureRule(INT dim, INT n, INT order);
    inline QUADRATURE *GetSymmetricQuadratureRule(INT dim, INT n, INT order);
    inline
    INT GaussPoints(INT dim, INT n, INT order, DOUBLE_VECTOR *x, GAUSS_POINT *gp);

#include "ugquadratures.cc"

  } // end namespace UGQuadratures

} // end namespace Dune

#endif
