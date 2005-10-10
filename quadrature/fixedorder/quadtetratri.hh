// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_QUADTETRATRI_HH
#define DUNE_QUADTETRATRI_HH

// the UG Quadratures
#include "ugquadratures.hh"

namespace Dune {

  //******************************************************************
  //
  //!  Memorization of the number of quadrature points
  //
  //******************************************************************

  static const double referenceVol_triangle    = 0.5;
  static const double referenceVol_tetrahedron = 1.0/6.0;

  //! specialization triangles
  template <class Domain, class RangeField, int polOrd>
  struct QuadraturePoints<Domain,RangeField,triangle,polOrd>
  {
    enum { dim = 2 };
    enum { numberOfCorners = dim+1 };
    typedef UG_Quadratures::QUADRATURE QUADRATURE;
    static int numberOfQuadPoints ();
    static int order ();
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  //! the weight is the volume of the reference element
  template <class Domain, class RangeField, int polOrd>
  int QuadraturePoints<Domain,RangeField,triangle,polOrd>::
  numberOfQuadPoints()
  {
    QUADRATURE * quad = UG_Quadratures::GetQuadratureRule(dim,numberOfCorners,polOrd);
    return quad->nip;
  }

  //! the weight is the volume of the reference element
  template <class Domain, class RangeField, int polOrd>
  int QuadraturePoints<Domain,RangeField,triangle,polOrd>::
  order()
  {
    QUADRATURE * quad = UG_Quadratures::GetQuadratureRule(dim,numberOfCorners,polOrd);
    return quad->order;
  }
  //! the weight is the volume of the reference element
  template <class Domain, class RangeField,int polOrd>
  RangeField QuadraturePoints<Domain,RangeField,triangle,polOrd>::
  getWeight(int i)
  {
    QUADRATURE * quad = UG_Quadratures::GetQuadratureRule(dim,numberOfCorners,polOrd);
    RangeField ref(referenceVol_triangle);
    RangeField
    w(ref * static_cast<RangeField> (quad->weight[i]));
    return w;
  }

  //! the weight is the volume of the reference element
  template <class Domain, class RangeField,int polOrd>
  Domain QuadraturePoints<Domain,RangeField,triangle,polOrd>::
  getPoint(int i)
  {
    QUADRATURE * quad = UG_Quadratures::GetQuadratureRule(dim,numberOfCorners,polOrd);
    Domain tmp;
    for(int j=0; j<dim; j++)
      tmp[j] = quad->local[i][j];
    return tmp;
  }

  //*************************************************************
  //! specialization tetrahedrons
  template <class Domain, class RangeField, int polOrd>
  struct QuadraturePoints<Domain,RangeField,tetrahedron,polOrd>
  {
    enum { dim = 3 }; // tetrahedrons
    enum { numberOfCorners = dim+1 };
    typedef UG_Quadratures::QUADRATURE QUADRATURE;
    static int numberOfQuadPoints ();
    static int order ();
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  //! the weight is the volume of the reference element
  template <class Domain, class RangeField, int polOrd>
  int QuadraturePoints<Domain,RangeField,tetrahedron,polOrd>::
  numberOfQuadPoints()
  {
    QUADRATURE * quad = UG_Quadratures::GetQuadratureRule(dim,numberOfCorners,polOrd);
    return quad->nip;
  }

  //! the weight is the volume of the reference element
  template <class Domain, class RangeField, int polOrd>
  int QuadraturePoints<Domain,RangeField,tetrahedron,polOrd>::
  order()
  {
    QUADRATURE * quad = UG_Quadratures::GetQuadratureRule(dim,numberOfCorners,polOrd);
    return quad->order;
  }

  //! the weight is the volume of the reference element
  template <class Domain, class RangeField, int polOrd>
  RangeField QuadraturePoints<Domain,RangeField,tetrahedron,polOrd>::
  getWeight(int i)
  {
    QUADRATURE * quad = UG_Quadratures::GetQuadratureRule(dim,numberOfCorners,polOrd);
    RangeField ref(referenceVol_tetrahedron);
    RangeField
    w(ref * static_cast<RangeField> (quad->weight[i]));
    return w;
  }

  //! the weight is the volume of the reference element
  template <class Domain, class RangeField,int polOrd>
  Domain QuadraturePoints<Domain,RangeField,tetrahedron,polOrd>::
  getPoint(int i)
  {
    QUADRATURE * quad = UG_Quadratures::GetQuadratureRule(dim,numberOfCorners,polOrd);
    Domain tmp;
    for(int j=0; j<dim; j++)
      tmp[j] = quad->local[i][j];
    return tmp;
  }

} // end namespace Dune

#endif
