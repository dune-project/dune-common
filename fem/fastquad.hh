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

    //! return order of quadrature
    int order () const { return order_; }

    //! return weight for point i
    const RangeFieldType& weight ( int i) const
    {
      return (weights_(i));
    }

    //! return point i local coordinates
    const DomainType& point (int i) const
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

    //! return order of quadrature
    int order () const { return order_; }

    //! return weight for point i
    const RangeFieldType& weight ( int i) const
    {
      return (weights_(i));
    }

    //! return point i in local coordinates
    const DomainType& point (int i) const
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

  //********************************************************************
  //
  //  --QuadratureImp
  //
  //! Quadrature class implementation
  //! Quadrature is diffrent for diffrent Domain and RangeField types and of
  //! course for diffrent element types but for dynamical polynomial order.
  //! The element type comes from a given entity or the grid
  //! and polOrd is given at runtime. For fast quadratures use the above
  //! FastQuadrature class
  //
  //********************************************************************
  template< class RangeFieldType , class DomainType >
  class QuadratureImp : public QuadratureDefault  < RangeFieldType ,
                            DomainType , QuadratureImp < RangeFieldType , DomainType > >  {

    // my Type
    typedef QuadratureImp < RangeFieldType , DomainType > QuadratureType;

  public:
    //! Constructor building the quadrature
    QuadratureImp ( int id , ElementType eltype, int polOrd ) : order_ ( polOrd ) ,
                                                                , eltype_ ( eltype )
                                                                QuadratureDefault < RangeFieldType , DomainType , QuadratureType > (id)
    {
      switch ( eltype_ )
      {
      case vertex       : { buildQuadrature<vertex> ( id , polOrd ); break; }
      case line         : { buildQuadrature<line> ( id , polOrd ); break; }
      case triangle     : { buildQuadrature<triangle> ( id , polOrd ); break; }
      case quadrilateral : { buildQuadrature<quadrilateral> ( id , polOrd ); break; }
      case tetrahedron  : { buildQuadrature<tetrahedron> ( id , polOrd ); break; }
      case pyramid      : { buildQuadrature<pyramid> ( id , polOrd ); break; }
      case prism        : { buildQuadrature<prism> ( id , polOrd ); break; }
      case hexahedron   : { buildQuadrature<hexahedron> ( id , polOrd ); break; }
      case iso_triangle : { buildQuadrature<iso_triangle> ( id , polOrd ); break; }
      case iso_quadrilateral : { buildQuadrature<iso_quadrilateral> ( id , polOrd ); break; }
      case default : { std::cerr << "Element type is unkown in Constructor of QuadratureImp! \n"; abort(); }
      }
    };

    //! return number of quadrature points
    int nop () const { return numQuadPoints_; };

    //! return number of quadrature points
    int order () const { return order_; };

    //! return weight corresponding to point i
    const RangeFieldType& weight( int i) const
    {
      return (weights_[i]);
    };

    //! return point i in local coordinates corresponding to reference element
    const DomainType& point (int i) const
    {
      return (points_[i]);
    };

  private:
    //! get the quadrature points for storage in the vectors
    template <int polynomialOrder, ElementType ElType >
    void makeQuadrature(int id)
    {
      // is called by the constructor
      typedef QuadraturePoints< DomainType,
          RangeFieldType,ElType,polynomialOrder>  QuadInitializer;

      if(id != QuadInitializer::identifier)
      {
        std::cerr << "wrong identifier given to constructor! \n";
        abort();
      }
      std::cout <<"Making Quadrature with dynamic polOrd! \n";

      // same story as above
      numQuadPoints_ = QuadInitializer::numberOfQuadPoints();
      order_ = QuadInitializer::order();

      // get memory
      weights_.resize( numQuadPoints_);
      points_.resize( numQuadPoints_);

      for(int i=0; i<numQuadPoints_; i++)
      {
        points_[i] = QuadInitializer::getPoint(i);
        weights_[i] = QuadInitializer::getWeight(i);
      }
    };

    // remember which element type the quadrature was made for
    ElementType eltyp_;

    // order of quadrature
    int order_;

    //! number of quadrature points
    int numQuadPoints_;

    //! Vectors holding the weights and points
    std::vector < RangeFieldType > weights_;
    std::vector < DomainType >     points_;

    //! anoying but what can we do
    //! can be expanded up to inf
    template <ElementType ElType>
    void buildQuadrature ( int id , int polOrd )
    {
      switch (polOrd)
      {
      case 0 : { makeQuadrature<0,ElType> (id); break; };
      case 1 : { makeQuadrature<1,ElType> (id); break; };
      case 2 : { makeQuadrature<2,ElType> (id); break; };
      case 3 : { makeQuadrature<3,ElType> (id); break; };
      case 4 : { makeQuadrature<4,ElType> (id); break; };
#if 0
      case 5 : { makeQuadrature<5> (id); break; };
      case 6 : { makeQuadrature<6> (id); break; };
      case 7 : { makeQuadrature<7> (id); break; };
      case 8 : { makeQuadrature<8> (id); break; };
      case 9 : { makeQuadrature<9> (id); break; };
      case 10 : { makeQuadrature<10> (id); break; };
      case 11 : { makeQuadrature<11> (id); break; };
      case 12 : { makeQuadrature<12> (id); break; };
      case 13 : { makeQuadrature<13> (id); break; };
      case 14 : { makeQuadrature<14> (id); break; };
      case 15 : { makeQuadrature<15> (id); break; };
      case 16 : { makeQuadrature<16> (id); break; };
      case 17 : { makeQuadrature<17> (id); break; };
      case 18 : { makeQuadrature<18> (id); break; };
      case 19 : { makeQuadrature<19> (id); break; };
      case 20 : { makeQuadrature<20> (id); break; };
#endif
      default : {
        std::cerr << "No Rule to make Quadrature with polOrd ";
        std::cerr << polOrd << " in Quadrature ( id , polOrd ) !\n";
        abort();
      };
      }
    };
  }; // end class QuadratureImp

} // end namespace Dune

#endif
