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

  template <class Domain, class RangeField, ElementType ElType, int codim >
  struct BaryCenterPoints
  {
    enum { identifier = 0 };
    enum { numberQuadPoints = 0 };
    enum { polynomOrder = 0 };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  template <class Domain, class RangeField, ElementType ElType, int codim>
  RangeField BaryCenterPoints<Domain,RangeField,ElType,codim>::getWeight(int i)
  {
    return 0.0;
  }
  template <class Domain, class RangeField, ElementType ElType, int codim>
  Domain BaryCenterPoints<Domain,RangeField,ElType,codim>::getPoint(int i)
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
  struct BaryCenterPoints<Domain,RangeField,triangle,0>
  {
    enum { identifier = 1 };
    enum { numberOfQuadPoints = 1 };
    enum { polynomOrder = 1 };
    static Domain getPoint (int i)
    {
      Domain tmp = (1.0/3.0);
      return tmp;
    };
    static RangeField getWeight (int i) { return 0.5; };
  };

  //! specialization triangles
  template <class Domain, class RangeField>
  struct BaryCenterPoints<Domain,RangeField,triangle,1>
  {
    enum { identifier = 1 };
    enum { numberOfQuadPoints = 3 };
    enum { polynomOrder = 2 };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  //! the sum over the weigths is the volume of the reference element
  template <class Domain, class RangeField>
  RangeField BaryCenterPoints<Domain,RangeField,triangle,1>::getWeight(int i)
  {
    return (1.0/6.0);
  }

  template <class Domain, class RangeField>
  Domain BaryCenterPoints<Domain,RangeField,triangle,1>::getPoint(int i)
  {
    // check whether dimension is 2 or not
    //CompileTimeChecker < Domain::dimension == 2 > check;
    Domain tmp = 0.5;
    switch (i)
    {
    case 0 : {
      return tmp;
    }
    case 1 : {
      tmp(0) = 0.0;
      return tmp;
    }
    case 2 : {
      tmp(1) = 0.0;
      return tmp;
    }
    default :
    {
      std::cerr << "BaryCenterPoints<triangle>::getPoint: i out of range! \n";
      abort();
    }
    }
    return tmp;
  }

  //*******************************************************************
  //! specialization tetrahedrons
  template <class Domain, class RangeField>
  struct BaryCenterPoints<Domain,RangeField,tetrahedron,0>
  {
    enum { identifier = 2 };
    enum { numberOfQuadPoints = 1 };
    enum { polynomOrder = 1 };
    static Domain getPoint (int i)
    {
      Domain tmp = (0.25);
      return tmp;
    };
    static RangeField getWeight (int i) { return (1.0/6.0); };
  };

  //! specialization tetrahedrons
  template <class Domain, class RangeField, int codim>
  struct BaryCenterPoints<Domain,RangeField,tetrahedron,codim>
  {
    enum { identifier = 2 };
    enum { numberOfQuadPoints = 4 };
    enum { polynomOrder = 2 };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  //! the sum over the weigths is the volume of the reference element
  template <class Domain, class RangeField, int codim>
  RangeField BaryCenterPoints<Domain,RangeField,tetrahedron,codim>::getWeight(int i)
  {
    return (1.0/24.0);
  }

  template <class Domain, class RangeField , int codim>
  Domain BaryCenterPoints<Domain,RangeField,tetrahedron,codim>::getPoint(int i)
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
  struct BaryCenterPoints<Domain,RangeField,quadrilateral,0>
  {
    enum { identifier = 3 };
    enum { numberOfQuadPoints = 1 };
    enum { polynomOrder = 1 };
    static Domain getPoint (int i)
    {
      Domain tmp = 0.5;
      return tmp;
    };
    static RangeField getWeight (int i) { return 1.0; };
  };


  template <class Domain, class RangeField, int codim>
  struct BaryCenterPoints<Domain,RangeField,quadrilateral,codim>
  {
    enum { identifier = 3 };
    enum { numberOfQuadPoints = 4 };
    enum { polynomOrder = 2 };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  //! the sum over the weigths is the volume of the reference element
  template <class Domain, class RangeField,int codim >
  RangeField BaryCenterPoints<Domain,RangeField,quadrilateral,codim>::getWeight(int i)
  {
    return 0.25;
  }

  template <class Domain, class RangeField,int codim >
  Domain BaryCenterPoints<Domain,RangeField,quadrilateral,codim>::getPoint(int i)
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
  struct BaryCenterPoints<Domain,RangeField,hexahedron,0>
  {
    enum { identifier = 4 };
    enum { numberOfQuadPoints = 1 };
    enum { polynomOrder = 1 };
    static Domain getPoint (int i)
    {
      Domain tmp = 0.5;
      return tmp;
    };
    static RangeField getWeight (int i) { return 1.0; };
  };

  template <class Domain, class RangeField,int codim>
  struct BaryCenterPoints<Domain,RangeField,hexahedron,codim>
  {
    enum { identifier = 4 };
    enum { numberOfQuadPoints = 6 };
    enum { polynomOrder = 2 };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  //! the sum over the weigths is the volume of the reference element
  template <class Domain, class RangeField, int codim >
  RangeField BaryCenterPoints<Domain,RangeField,hexahedron,codim>::getWeight(int i)
  {
    return (1.0/6.0);
  }

  template <class Domain, class RangeField,int codim >
  Domain BaryCenterPoints<Domain,RangeField,hexahedron,codim>::getPoint(int i)
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
