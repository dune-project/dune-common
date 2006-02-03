// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_QUADLQH_HH
#define DUNE_QUADLQH_HH

#include <dune/common/misc.hh>
#include "gaussquadimp.hh"

// the UG Quadratures
#include "ugquadratures.hh"

namespace Dune {

  //************************************************************************
  //
  //! Quadratures for lines, quadrilaterals and hexahedrons
  //! see gaussquadimp.hh for detailed description
  //
  //************************************************************************
  //
  //! specialization for lines
  //
  //************************************************************************
  template <class Domain, class RangeField, int polOrd >
  struct QuadraturePoints
  {
    enum { identifier = 5*(polOrd+1) };
    enum { numberOfQuadPoints_ = GaussQuadrature<Domain,RangeField,1,polOrd>::n };
    static int numberOfQuadPoints (GeometryType type);
    static int order (GeometryType type);
    static Domain getPoint (GeometryType type,int i);
    static RangeField getWeight (GeometryType type,int i);
  };

  template <class Domain, class RangeField , int polOrd >
  int QuadraturePoints<Domain,RangeField,polOrd>::
  numberOfQuadPoints(GeometryType type)
  {
    if(type.isLine()) return GaussQuadrature<Domain,RangeField,1,polOrd>::n;
    if(type.isQuadrilateral()) return GaussQuadrature<Domain,RangeField,2,polOrd>::n;
    if(type.isHexahedron()) return GaussQuadrature<Domain,RangeField,3,polOrd>::n;

    if(type.isSimplex())
    {
      int dim = type.dim();
      int numberOfCorners = dim+1;
      typedef UG_Quadratures::QUADRATURE QUADRATURE;
      QUADRATURE * quad = UG_Quadratures::GetQuadratureRule(dim,numberOfCorners,polOrd);
      return quad->nip;
    }

    DUNE_THROW(NotImplemented,"geom type not implemented");
    return -1;
  }

  template <class Domain, class RangeField , int polOrd >
  int QuadraturePoints<Domain,RangeField,polOrd>::
  order (GeometryType type)
  {
    if(type.isCube())
      return polOrd;

    if(type.isSimplex())
    {
      int dim = type.dim();
      int numberOfCorners = dim+1;
      typedef UG_Quadratures::QUADRATURE QUADRATURE;
      QUADRATURE * quad = UG_Quadratures::GetQuadratureRule(dim,numberOfCorners,polOrd);
      return quad->order;
    }

    DUNE_THROW(NotImplemented,"geom type not implemented");
    return polOrd;
  }

  template <class Domain, class RangeField , int polOrd >
  RangeField QuadraturePoints<Domain,RangeField,polOrd>::
  getWeight(GeometryType type, int i)
  {
    if(type.isLine())
    {
      static GaussQuadrature<Domain,RangeField,1,polOrd> gaussquad;
      return gaussquad.w(i);
    }
    if(type.isQuadrilateral())
    {
      static GaussQuadrature<Domain,RangeField,2,polOrd> gaussquad;
      return gaussquad.w(i);
    }
    if(type.isHexahedron())
    {
      static GaussQuadrature<Domain,RangeField,3,polOrd> gaussquad;
      return gaussquad.w(i);
    }

    if(type.isSimplex())
    {
      int dim = type.dim();
      int numberOfCorners = dim+1;
      typedef UG_Quadratures::QUADRATURE QUADRATURE;
      QUADRATURE * quad = UG_Quadratures::GetQuadratureRule(dim,numberOfCorners,polOrd);
      RangeField ref = 1.0;
      if(dim > 1) RangeField ref = static_cast<RangeField> (ref/((dim-1)*dim));
      RangeField w(ref * static_cast<RangeField> (quad->weight[i]));
      return w;
    }

    DUNE_THROW(NotImplemented,"geom type not implemented");
    return -1.0;
  }

  template <class Domain, class RangeField,int polOrd>
  Domain QuadraturePoints<Domain,RangeField,polOrd>::
  getPoint(GeometryType type, int i)
  {
    if(type.isLine())
    {
      static GaussQuadrature<Domain,RangeField,1,polOrd> gaussquad;
      return gaussquad.ip(i);
    }
    if(type.isQuadrilateral())
    {
      static GaussQuadrature<Domain,RangeField,2,polOrd> gaussquad;
      return gaussquad.ip(i);
    }
    if(type.isHexahedron())
    {
      static GaussQuadrature<Domain,RangeField,3,polOrd> gaussquad;
      return gaussquad.ip(i);
    }

    if(type.isSimplex())
    {
      int dim = type.dim();
      int numberOfCorners = dim+1;
      typedef UG_Quadratures::QUADRATURE QUADRATURE;
      QUADRATURE * quad = UG_Quadratures::GetQuadratureRule(dim,numberOfCorners,polOrd);
      Domain tmp;
      for(int j=0; j<dim; j++) tmp[j] = quad->local[i][j];
      return tmp;
    }

    DUNE_THROW(NotImplemented,"geom type not implemented");
    return Domain(-1.0);
  }

} // end namespace Dune
#endif
