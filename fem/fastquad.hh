// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_FASTQUAD_HH__
#define __DUNE_FASTQUAD_HH__

#include <dune/common/matvec.hh>
#include "common/quadrature.hh"

#include "fastquad/quadlqh.hh"
#include "fastquad/quadtetratri.hh"
#include "fastquad/facecenterpoints.hh"

namespace Dune {

  //**************************************************************************
  //
  //  Fast Quadrature for unspecified Element type
  //
  //**************************************************************************

  template< class RangeFieldType , class DomainType ,int poly_order >
  class FastQuad
    : public QuadratureDefault  < RangeFieldType , DomainType ,
          FastQuad < RangeFieldType, DomainType, poly_order > >
  {
  private:
    enum { dim = DomainType::dimension };

    //! my Type
    typedef FastQuad <  RangeFieldType , DomainType , poly_order > FastQuadType;

    // number of quadrature points on segment line
    // upper bound
    enum { m = PointsOnLine<poly_order>::points+2 };

    //! the number of quadrature points is stored as enum
    enum { maxQuadPoints = power_M_P < m , dim >::power };

  public:
    //! Constructor build the vec with the points and weights
    template <class EntityType>
    FastQuad ( EntityType &en ) :
      QuadratureDefault < RangeFieldType, DomainType , FastQuadType > (6)
    {
      switch(en.geometry().type())
      {
      case line :          { makeQuadrature<line> (); break; }
      case quadrilateral : { makeQuadrature<quadrilateral> (); break; }
      case hexahedron :    { makeQuadrature<hexahedron> (); break; }
      case triangle :      { makeQuadrature<triangle> (); break; }
      case tetrahedron :   { makeQuadrature<tetrahedron> (); break; }
      default :       { std::cerr << "Unkown ElementType in FastQuad::makeQuadrature()\n"; abort();  break; }
      }
    };

    //! Constructor build the vec with the points and weights
    FastQuad ( ElementType eltype ) :
      QuadratureDefault < RangeFieldType, DomainType , FastQuadType > (6)
    {
      switch(eltype)
      {
      case line :          { makeQuadrature<line> (); break; }
      case quadrilateral : { makeQuadrature<quadrilateral> (); break; }
      case hexahedron :    { makeQuadrature<hexahedron> (); break; }
      case triangle :      { makeQuadrature<triangle> (); break; }
      case tetrahedron :   { makeQuadrature<tetrahedron> (); break; }
      default :       { std::cerr << "Unkown ElementType in FastQuad::makeQuadrature()\n"; abort();  break; }
      }
    };

    //! return number of quadrature points
    int nop() const { return numberOfQuadPoints_; }

    //! return number of quadrature points
    int getNumberOfQuadPoints () const { return numberOfQuadPoints_; }

    //! return number of quadrature points
    int order () const { return order_; }

    //! return weight for point i
    const RangeFieldType& weight ( int i) const
    {
      return (weights_(i));
    }

    //! return weight for point i
    const RangeFieldType& getQuadratureWeight ( int i) const
    {
      return (weights_(i));
    }

    //! return point i
    const DomainType& point (int i) const
    {
      return (points_(i));
    }

    //! return point i
    const DomainType& getQuadraturePoint (int i) const
    {
      return (points_(i));
    }

  private:
    template <ElementType ElType>
    void makeQuadrature ()
    {
      // is called by the constructor
      typedef QuadraturePoints< DomainType,
          RangeFieldType,ElType, poly_order>  QuadInitializer;

      // same story as above
      numberOfQuadPoints_ = QuadInitializer::numberOfQuadPoints();
      order_              = QuadInitializer::order();

      for(int i=0; i<numberOfQuadPoints_; i++)
      {
        points_(i)  = QuadInitializer::getPoint(i);
        weights_(i) = QuadInitializer::getWeight(i);
      }
    }

    //! number of quadrature points
    int numberOfQuadPoints_;

    //! real quadrature order
    int order_;

    //! Vecs with constant length holding the weights and points
    Vec < maxQuadPoints , RangeFieldType > weights_;
    Vec < maxQuadPoints , DomainType >     points_;

  }; // end class FastQuadrature


  //*********************************************************************
  //
  //
  //  Quadrature for bary center of the faces of one element
  //  supported element types
  //  triangles, quadrilaterals, tetrahedrons, hexahedrons
  //
  //*********************************************************************
  template< class RangeFieldType , class DomainType>
  class FaceCenterQuad
    : public QuadratureDefault  < RangeFieldType , DomainType ,
          FaceCenterQuad < RangeFieldType, DomainType > >
  {
  private:
    enum { dim = DomainType::dimension };

    //! my Type
    typedef FaceCenterQuad <  RangeFieldType , DomainType > FaceCenterQuadType;

    // number of quadrature points on segment line
    // upper bound
    enum { m = PointsOnLine<2>::points+2 };

    //! the number of quadrature points is stored as enum
    enum { maxQuadPoints = power_M_P < m , dim >::power };

  public:
    //! Constructor build the vec with the points and weights
    template <class EntityType>
    FaceCenterQuad ( EntityType &en ) :
      QuadratureDefault < RangeFieldType, DomainType , FaceCenterQuadType > (6)
    {
      switch(en.geometry().type())
      {
      case quadrilateral : { makeQuadrature<quadrilateral> (); break; }
      case hexahedron :    { makeQuadrature<hexahedron> (); break; }
      case triangle :      { makeQuadrature<triangle> (); break; }
      case tetrahedron :   { makeQuadrature<tetrahedron> (); break; }
      default :       { std::cerr << "Unkown ElementType in FastQuad::makeQuadrature()\n"; abort();  break; }
      }
    };

    //! Constructor build the vec with the points and weights
    FaceCenterQuad ( ElementType eltype ) :
      QuadratureDefault < RangeFieldType, DomainType , FaceCenterQuadType > (6)
    {
      switch(eltype)
      {
      case quadrilateral : { makeQuadrature<quadrilateral> (); break; }
      case hexahedron :    { makeQuadrature<hexahedron> (); break; }
      case triangle :      { makeQuadrature<triangle> (); break; }
      case tetrahedron :   { makeQuadrature<tetrahedron> (); break; }
      default :       { std::cerr << "Unkown ElementType in FastQuad::makeQuadrature()\n"; abort();  break; }
      }
    };

    //! return number of quadrature points
    int nop() const { return numberOfQuadPoints_; }

    //! return number of quadrature points
    int getNumberOfQuadPoints () const { return numberOfQuadPoints_; }

    //! return number of quadrature points
    int order () const { return order_; }

    //! return weight for point i
    const RangeFieldType& weight ( int i) const
    {
      return (weights_(i));
    }

    //! return point i
    const DomainType& point (int i) const
    {
      return (points_(i));
    }
    //! return weight for point i
    const RangeFieldType& getQuadratureWeight ( int i) const
    {
      return (weights_(i));
    }

    //! return point i
    const DomainType& getQuadraturePoint (int i) const
    {
      return (points_(i));
    }


  private:
    template <ElementType ElType>
    void makeQuadrature ()
    {
      // is called by the constructor
      typedef FaceCenterPoints< DomainType,
          RangeFieldType,ElType>  QuadInitializer;

      // same story as above
      numberOfQuadPoints_ = QuadInitializer::numberOfQuadPoints;
      order_              = QuadInitializer::polynomOrder;

      for(int i=0; i<numberOfQuadPoints_; i++)
      {
        points_(i) = QuadInitializer::getPoint(i);
        weights_(i) = QuadInitializer::getWeight(i);
      }
    }

    //! number of quadrature points
    int numberOfQuadPoints_;

    //! real quadrature order
    int order_;

    //! Vecs with constant length holding the weights and points
    Vec < maxQuadPoints , RangeFieldType > weights_;
    Vec < maxQuadPoints , DomainType >     points_;

  }; // end class FastQuadrature

} // end namespace Dune

#endif
