// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_QUADTRIANGLE_HH__
#define __DUNE_QUADTRIANGLE_HH__

#include "../grid/common/grid.hh"
#include "quadrature.hh"

namespace Dune {

  //******************************************************************
  //
  //!  Memorization of the number of quadrature points
  //
  //******************************************************************

  //! specialization triangles and polOrd = 1
  template <class Domain, class RangeField>
  struct QuadraturePoints<Domain,RangeField,triangle, 1>
  {
    enum { identifier = 3 };
    enum { numberOfQuadPoints = 1 };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  template <class Domain, class RangeField >
  RangeField QuadraturePoints<Domain,RangeField,triangle,1>::getWeight(int i)
  {
    return 1.0;
  }

  template <class Domain, class RangeField >
  Domain QuadraturePoints<Domain,RangeField,triangle,1>::getPoint(int i)
  {
    // check whether dimension is 2 or not
    CompileTimeChecker < Domain::dimension == 2 > check;
    Domain tmp;  tmp(0) = (1.0/3.0); tmp(1) = (1.0/3.0);
    return tmp;
  }

  //*************************************************************************
  //! specialization triangles and polOrd = 2
  template <class Domain, class RangeField>
  struct QuadraturePoints<Domain,RangeField,triangle, 2>
  {
    enum { identifier = 4 };
    enum { numberOfQuadPoints = 3 };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  template <class Domain, class RangeField >
  RangeField QuadraturePoints<Domain,RangeField,triangle,2>::getWeight(int i)
  {
    return (1.0/3.0);
  }
  template <class Domain, class RangeField >
  Domain QuadraturePoints<Domain,RangeField,triangle,2>::getPoint(int i)
  {
    // check whether dimension is 2 or not
    CompileTimeChecker < Domain::dimension == 2 > check;
    switch (i)
    {
    case 0 : {
      Domain tmp(0.0);  tmp(0) = 0.5;
      //tmp.print(std::cout,1); std::cout << " QuadPoint \n";
      return tmp;
    }
    case 1 : {
      Domain tmp(0.0);  tmp(0) = 0.5; tmp(1) = 0.5;
      //tmp.print(std::cout,1); std::cout << " QuadPoint \n";
      return tmp;
    }
    case 2 : {
      Domain tmp(0.0);  tmp(1) = 0.5;
      //tmp.print(std::cout,1); std::cout << " QuadPoint \n";
      return tmp;
    }
    default : {
      std::cerr << "TriangleQuadPoints::getPoint: i out of range! \n";
      abort();
    }
    }
  }

} // end namespace Dune

#endif
