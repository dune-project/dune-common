// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_FACECENTERPOINTS_HH__
#define __DUNE_FACECENTERPOINTS_HH__

#include <dune/grid/common/grid.hh>
#include <dune/fem/common/quadrature.hh>

namespace Dune {

  //******************************************************************
  //
  //!  Memorization of the number of quadrature points
  //
  //******************************************************************

  template <class Domain, class RangeField, ElementType ElType>
  struct FaceCenterPoints
  {
    enum { identifier = 0 };
    enum { numberQuadPoints = 0 };
    enum { polynomOrder = 0 };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  template <class Domain, class RangeField, ElementType ElType>
  RangeField FaceCenterPoints<Domain,RangeField,ElType>::getWeight(int i)
  {
    return 0.0;
  }
  template <class Domain, class RangeField, ElementType ElType>
  Domain FaceCenterPoints<Domain,RangeField,ElType>::getPoint(int i)
  {
    Domain tmp(0.0);
    return tmp;
  }

  //***********************************************************************
  //
  //  specialisations for triangles,quadrilateral,tetrahedrons,hexahedrons
  //
  //***********************************************************************

  //! specialization triangles
  template <class Domain, class RangeField>
  struct FaceCenterPoints<Domain,RangeField,triangle>
  {
    enum { identifier = 1 };
    enum { numberOfQuadPoints = 3 };
    enum { polynomOrder = 2 };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  //! the sum over the weigths is the volume of the reference element
  template <class Domain, class RangeField >
  RangeField FaceCenterPoints<Domain,RangeField,triangle>::getWeight(int i)
  {
    return (1.0/6.0);
  }

  template <class Domain, class RangeField >
  Domain FaceCenterPoints<Domain,RangeField,triangle>::getPoint(int i)
  {
    // check whether dimension is 2 or not
    //CompileTimeChecker < Domain::dimension == 2 > check;
    switch (i)
    {
    case 0 : {
      Domain tmp(0.5);
      return tmp;
    }
    case 1 : {
      Domain tmp(0.0);  tmp(1) = 0.5;
      return tmp;
    }
    case 2 : {
      Domain tmp(0.0);  tmp(0) = 0.5;
      return tmp;
    }
    }
    Domain tmp;
    std::cerr << "TriangleQuadPoints::getPoint: i out of range! \n";
    abort();
    return tmp;
  }

  //*******************************************************************
  //! specialization tetrahedrons
  template <class Domain, class RangeField>
  struct FaceCenterPoints<Domain,RangeField,tetrahedron>
  {
    enum { identifier = 2 };
    enum { numberOfQuadPoints = 4 };
    enum { polynomOrder = 2 };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  //! the sum over the weigths is the volume of the reference element
  template <class Domain, class RangeField >
  RangeField FaceCenterPoints<Domain,RangeField,tetrahedron>::getWeight(int i)
  {
    return (1.0/24.0);
  }

  template <class Domain, class RangeField >
  Domain FaceCenterPoints<Domain,RangeField,tetrahedron>::getPoint(int i)
  {
    Domain tmp = (1.0/3.0);
    assert( (i>=0) && (i<4) );

    if(i==0) return tmp;

    tmp(i-1) = 0.0;
    return tmp;
  }

  //*******************************************************************
  //! specialization quadrilaterals
  template <class Domain, class RangeField>
  struct FaceCenterPoints<Domain,RangeField,quadrilateral>
  {
    enum { identifier = 3 };
    enum { numberOfQuadPoints = 4 };
    enum { polynomOrder = 2 };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  //! the sum over the weigths is the volume of the reference element
  template <class Domain, class RangeField >
  RangeField FaceCenterPoints<Domain,RangeField,quadrilateral>::getWeight(int i)
  {
    return 0.25;
  }

  template <class Domain, class RangeField >
  Domain FaceCenterPoints<Domain,RangeField,quadrilateral>::getPoint(int i)
  {
    assert( (i>=0) && (i<4) );
    Domain tmp;

    if(i < 2 )
    {
      tmp(1) = 0.5;
      tmp(0) = static_cast<RangeField> (i);
    }
    else
    {
      tmp(0) = 0.5;
      tmp(1) = static_cast<RangeField> (i-2);
    }
    return tmp;
  }

  //*******************************************************************
  //! specialization hexahedrons
  template <class Domain, class RangeField>
  struct FaceCenterPoints<Domain,RangeField,hexahedron>
  {
    enum { identifier = 4 };
    enum { numberOfQuadPoints = 6 };
    enum { polynomOrder = 2 };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  //! the sum over the weigths is the volume of the reference element
  template <class Domain, class RangeField >
  RangeField FaceCenterPoints<Domain,RangeField,hexahedron>::getWeight(int i)
  {
    return (1.0/6.0);
  }

  template <class Domain, class RangeField >
  Domain FaceCenterPoints<Domain,RangeField,hexahedron>::getPoint(int i)
  {
    assert( (i>=0) && (i<6) );
    Domain tmp;

    tmp(2) = 0.5;
    if(i < 2 )
    {
      tmp(1) = 0.5;
      tmp(0) = static_cast<RangeField> (i);
      return tmp;
    }
    if( (i >= 2) && (i<4) )
    {
      tmp(0) = 0.5;
      tmp(1) = static_cast<RangeField> (i-2);
      return tmp;
    }

    tmp = 0.5;
    tmp(2) = static_cast<RangeField> (i-4);
    return tmp;
  }



} // end namespace Dune

#endif
