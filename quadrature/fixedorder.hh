// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_FIXED_ORDER_QUAD_HH
#define DUNE_FIXED_ORDER_QUAD_HH

#include "common/quadrature.hh"

// the specialisations
#include "fixedorder/quadlqh.hh"
#include "fixedorder/quadtetratri.hh"

namespace Dune {

  //**************************************************************************
  //
  //  Fast Quadrature for unspecified Element type
  //
  //**************************************************************************

  /** \brief %Quadrature class using the order as template-parameter
   * \ingroup Quadrature

   * Class to create a quadrature-object returning a number of
   * Quadrature points. Needs a RangeType (type of the quadrature
   * weight), a DomainType (type of local coordinates) and a polynomial
   * order as template parameter. The constructor expects an
   * Codim-0-Entity (Element) as parameter or an GeometryType such as triangle
   * or hexahedron.

   * With this class the quadratures are created as templates, whereas
   * with Quadrature the constructor decides on the rule. Thus, this
   * quadrature is instantiated quicker.
   */
  template< class RangeFieldType , class DomainType , int poly_order >
  class FixedOrderQuad
    : public QuadratureDefault  < RangeFieldType , DomainType ,
          FixedOrderQuad < RangeFieldType, DomainType, poly_order > >
  {
  private:
    enum { dim = DomainType::size };

    //! my Type
    typedef FixedOrderQuad <  RangeFieldType , DomainType , poly_order > FixedOrderQuadType;

    // number of quadrature points on segment line
    // upper bound
    enum { m = PointsOnLine<poly_order>::points+2 };

    //! the number of quadrature points is stored as enum
    enum { maxQuadPoints = power_M_P < m , dim >::power };

  public:
    //! Constructor build the vec with the points and weights
    template <class EntityType>
    FixedOrderQuad ( EntityType &en )
    {
      switch(en.geometry().type())
      {
      case line :          { makeQuadrature<line> (); break; }
      case quadrilateral : { makeQuadrature<quadrilateral> (); break; }
      case hexahedron :    { makeQuadrature<hexahedron> (); break; }
      case triangle :      { makeQuadrature<triangle> (); break; }
      case tetrahedron :   { makeQuadrature<tetrahedron> (); break; }
      default :       { std::cerr << "Unkown GeometryType in FixedOrderQuad::makeQuadrature()\n"; abort();  break; }
      }
    };

    //! Constructor build the vec with the points and weights
    FixedOrderQuad ( GeometryType eltype )
    {
      switch(eltype)
      {
      case line :          { makeQuadrature<line> (); break; }
      case quadrilateral : { makeQuadrature<quadrilateral> (); break; }
      case hexahedron :    { makeQuadrature<hexahedron> (); break; }
      case triangle :      { makeQuadrature<triangle> (); break; }
      case tetrahedron :   { makeQuadrature<tetrahedron> (); break; }
      default :       { std::cerr << "Unkown GeometryType in FixedOrderQuad::makeQuadrature()\n"; abort();  break; }
      }
    };

    virtual ~FixedOrderQuad() {}

    //! return number of quadrature points
    int nop() const { return numberOfQuadPoints_; }

    //! return order of quadrature
    int order () const { return order_; }

    //! return weight for point i
    const RangeFieldType& weight ( int i) const
    {
      return weights_[i];
    }

    //! return point i local coordinates
    const DomainType& point (int i) const
    {
      return points_[i];
    }

  private:
    template <GeometryType ElType>
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
        points_[i]  = QuadInitializer::getPoint(i);
        weights_[i] = QuadInitializer::getWeight(i);
      }

      int myType = (int) ElType;
      myType *= 100;
      this->setIdentifier( myType + order_);

      //std::cout << (*this) << "\n";
    }

    //! number of quadrature points
    int numberOfQuadPoints_;

    //! real quadrature order
    int order_;

    //! Vecs with constant length holding the weights and points
    FieldVector<RangeFieldType, maxQuadPoints> weights_;
    FieldVector<DomainType, maxQuadPoints>     points_;

  }; // end class FixedOrderQuadrature


  //********************************************************************
  //
  //  --Quadrature
  /**
   * \brief %Quadrature class where the quadrature order can be chosen
   * dynamically.
   * \ingroup Quadrature

   * %Quadrature is different for different Domain and RangeField types and of
   * course for different element types but for dynamical polynomial order.
   * The element type comes from a given entity or the grid
   * and polOrd is given at runtime. For fast quadratures use the
   * FixedOrderQuad class.
   */
  //********************************************************************
  template< class RangeFieldType , class DomainType >
  class Quadrature : public QuadratureDefault  < RangeFieldType ,
                         DomainType , Quadrature < RangeFieldType , DomainType > >  {

    // my Type
    typedef Quadrature < RangeFieldType , DomainType > QuadratureType;

  public:
    //! Constructor building the quadrature
    Quadrature ( int id , GeometryType eltype, int polOrd ) :
      order_ ( polOrd )  , eltype_ ( eltype )
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
      default : { std::cerr << "Element type is unkown in Constructor of Quadrature! \n"; abort(); }
      }
    };

    virtual ~Quadrature() {}

    //! return number of quadrature points
    int nop () const { return numQuadPoints_; };

    //! return order of quadrature
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
    template <int polynomialOrder, GeometryType ElType >
    void makeQuadrature(int id)
    {
      // is called by the constructor
      typedef QuadraturePoints< DomainType,
          RangeFieldType,ElType,polynomialOrder>  QuadInitializer;

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

      int myType = (int) ElType;
      myType *= 100;
      this->setIdentifier( myType + polynomialOrder );

    };

    // order of quadrature
    int order_;

    // remember which element type the quadrature was made for
    const GeometryType eltype_;

    //! number of quadrature points
    int numQuadPoints_;

    //! Vectors holding the weights and points
    std::vector < RangeFieldType > weights_;
    std::vector < DomainType >     points_;

    //! anoying but what can we do
    //! can be expanded up to inf
    template <GeometryType ElType>
    void buildQuadrature ( int id , int polOrd )
    {
      switch (polOrd)
      {
      case 0 :  { makeQuadrature<0,ElType> (id); break; };
      case 1 :  { makeQuadrature<1,ElType> (id); break; };
      case 2 :  { makeQuadrature<2,ElType> (id); break; };
      case 3 :  { makeQuadrature<3,ElType> (id); break; };
      case 4 :  { makeQuadrature<4,ElType> (id); break; };
      case 5 :  { makeQuadrature<5,ElType> (id); break; };
      case 6 :  { makeQuadrature<6,ElType> (id); break; };
      case 7 :  { makeQuadrature<7,ElType> (id); break; };
      case 8 :  { makeQuadrature<8,ElType> (id); break; };
      case 9 :  { makeQuadrature<9,ElType> (id); break; };
      case 10 : { makeQuadrature<10,ElType> (id); break; };
      case 11 : { makeQuadrature<11,ElType> (id); break; };
      case 12 : { makeQuadrature<12,ElType> (id); break; };
      case 13 : { makeQuadrature<13,ElType> (id); break; };
      case 14 : { makeQuadrature<14,ElType> (id); break; };
      case 15 : { makeQuadrature<15,ElType> (id); break; };
      case 16 : { makeQuadrature<16,ElType> (id); break; };
      case 17 : { makeQuadrature<17,ElType> (id); break; };
      case 18 : { makeQuadrature<18,ElType> (id); break; };
      case 19 : { makeQuadrature<19,ElType> (id); break; };
      case 20 : { makeQuadrature<20,ElType> (id); break; };
      default : {
        std::cerr << "No Rule to make Quadrature with polOrd ";
        std::cerr << polOrd << " in Quadrature ( id , polOrd ) !\n";
        abort();
      };
      }
    };
  }; // end class Quadrature

} // end namespace Dune

#endif
