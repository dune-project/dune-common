// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_QUADLQH_HH__
#define __DUNE_QUADLQH_HH__

#include <dune/grid/common/grid.hh>

#include "common/quadrature.hh"
#include <dune/common/misc.hh>

#include "quadlqh/gaussquadimp.hh"


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
  struct QuadraturePoints<Domain,RangeField,line, polOrd>
  {
    enum { identifier = 5*(polOrd+1) };
    enum { numberOfQuadPoints = GaussQuadrature<Domain,RangeField,1,polOrd>::n };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  template <class Domain, class RangeField , int polOrd >
  RangeField QuadraturePoints<Domain,RangeField,line,polOrd>::getWeight(int i)
  {
    GaussQuadrature<Domain,RangeField,1,polOrd> gaussquad;
    return gaussquad.w(i);
  }

  template <class Domain, class RangeField , int polOrd>
  Domain QuadraturePoints<Domain,RangeField,line,polOrd>::getPoint(int i)
  {
    // check whether dimension is 1 or not
    //CompileTimeChecker < Domain::dimension == 1 > check;
    GaussQuadrature<Domain,RangeField,1,polOrd> gaussquad;
    return gaussquad.ip(i);
  }

  //**************************************************************************
  //
  //! specialization for quadrilaterals
  //
  //**************************************************************************
  template <class Domain, class RangeField, int polOrd >
  struct QuadraturePoints<Domain,RangeField,quadrilateral, polOrd>
  {
    enum { identifier = 6*(polOrd+1) };
    enum { numberOfQuadPoints = GaussQuadrature<Domain,RangeField,2,polOrd>::n };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  template <class Domain, class RangeField , int polOrd >
  RangeField QuadraturePoints<Domain,RangeField,quadrilateral,polOrd>::getWeight(int i)
  {
    GaussQuadrature<Domain,RangeField,2,polOrd> gaussquad;
    return gaussquad.w(i);
  }

  template <class Domain, class RangeField , int polOrd>
  Domain QuadraturePoints<Domain,RangeField,quadrilateral,polOrd>::getPoint(int i)
  {
    // check whether dimension is 2 or not
    //CompileTimeChecker < Domain::dimension == 2 > check;
    GaussQuadrature<Domain,RangeField,2,polOrd> gaussquad;
    return gaussquad.ip(i);
  }

  //**************************************************************************
  //
  //! specialization for hexahedron
  //
  //**************************************************************************
  template <class Domain, class RangeField, int polOrd >
  struct QuadraturePoints<Domain,RangeField,hexahedron, polOrd>
  {
    enum { identifier = 7*(polOrd+1) };
    enum { numberOfQuadPoints = GaussQuadrature<Domain,RangeField,3,polOrd>::n };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  template <class Domain, class RangeField , int polOrd >
  RangeField QuadraturePoints<Domain,RangeField,hexahedron,polOrd>::getWeight(int i)
  {
    GaussQuadrature<Domain,RangeField,3,polOrd> gaussquad;
    return gaussquad.w(i);
  }

  template <class Domain, class RangeField , int polOrd>
  Domain QuadraturePoints<Domain,RangeField,hexahedron,polOrd>::getPoint(int i)
  {
    // check whether dimension is 3 or not
    //CompileTimeChecker < Domain::dimension == 3 > dim_is_not_equal_3;
    GaussQuadrature<Domain,RangeField,3,polOrd> gaussquad;
    return gaussquad.ip(i);
  }


} // end namespace Dune

#endif
