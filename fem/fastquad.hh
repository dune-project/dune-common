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

  /** \brief Quadrature class using the order as template-parameter

   * Class to create a quadrature-object returning a number of
   * Quadrature points. Needs a RangeType (type of the quadrature
   * weight), a DomainType (type of local coordinates) and a polynomial
   * order as template parameter. The constructor expects an
   * Codim-0-Entity (Element) as parameter.

   * With this class the quadratures are created as templates, whereas
   * with QuadratureImp the constructor decides on the rule. Thus, this
   * quadrature is instantiated quicker.
   */
  template< class RangeFieldType , class DomainType ,int poly_order >
  class FastQuad
    : public QuadratureDefault  < RangeFieldType , DomainType ,
          FastQuad < RangeFieldType, DomainType, poly_order > >
  {
  private:
    enum { dim = DomainType::size };

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
    FastQuad ( EntityType &en )
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
    FastQuad ( ElementType eltype )
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

    virtual ~FastQuad() {}

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

  }; // end class FastQuadrature


  /*********************************************************************/
  /** \brief  Quadrature for barycenter of the faces of one element
   *
   *  Supported element types
   * triangles, quadrilaterals, tetrahedrons, hexahedrons
   */
  /*********************************************************************/

  template< class RangeFieldType , class DomainType, int codim >
  class BaryCenterQuad
    : public QuadratureDefault  < RangeFieldType , DomainType ,
          BaryCenterQuad < RangeFieldType, DomainType, codim > >
  {
  private:
    enum { dim = DomainType::dimension };

    //! my Type
    typedef BaryCenterQuad <  RangeFieldType , DomainType, codim > BaryCenterQuadType;

    // number of quadrature points on segment line
    // upper bound
    enum { m = PointsOnLine<2>::points+2 };

    //! the number of quadrature points is stored as enum
    enum { maxQuadPoints = power_M_P < m , dim >::power };

  public:
    //! Constructor build the vec with the points and weights
    template <class EntityType>
    BaryCenterQuad ( EntityType &en )
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
    BaryCenterQuad ( ElementType eltype )
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

    virtual ~BaryCenterQuad () {}

    //! return number of quadrature points
    int nop() const { return numberOfQuadPoints_; }

    //! return order of quadrature
    int order () const { return order_; }

    //! return weight for point i
    const RangeFieldType& weight ( int i) const
    {
      return (weights_[i]);
    }

    //! return point i in local coordinates
    const DomainType& point (int i) const
    {
      return (points_[i]);
    }

  private:
    template <ElementType ElType>
    void makeQuadrature ()
    {
      // is called by the constructor
      typedef BaryCenterPoints< DomainType,
          RangeFieldType,ElType,codim>  QuadInitializerType;

      // same story as above
      numberOfQuadPoints_ = QuadInitializerType::numberOfQuadPoints;
      order_              = QuadInitializerType::polynomOrder;

      for(int i=0; i<numberOfQuadPoints_; i++)
      {
        points_[i]  = QuadInitializerType::getPoint(i);
        weights_[i] = QuadInitializerType::getWeight(i);
      }
      int myType = (int) ElType;
      myType *= 10 * codim;
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

  }; // end class FastQuadrature

  //********************************************************************
  //
  //  --QuadratureImp
  /**
   * \brief Quadrature class implementation

   * Quadrature is different for different Domain and RangeField types and of
   * course for different element types but for dynamical polynomial order.
   * The element type comes from a given entity or the grid
   * and polOrd is given at runtime. For fast quadratures use the
   * FastQuad class.
   */
  //
  //********************************************************************
  template< class RangeFieldType , class DomainType >
  class QuadratureImp : public QuadratureDefault  < RangeFieldType ,
                            DomainType , QuadratureImp < RangeFieldType , DomainType > >  {

    // my Type
    typedef QuadratureImp < RangeFieldType , DomainType > QuadratureType;

  public:
    //! Constructor building the quadrature
    QuadratureImp ( int id , ElementType eltype, int polOrd ) :
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
      default : { std::cerr << "Element type is unkown in Constructor of QuadratureImp! \n"; abort(); }
      }
    };

    virtual ~QuadratureImp() {}

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
    template <int polynomialOrder, ElementType ElType >
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
    const ElementType eltype_;

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
  }; // end class QuadratureImp

} // end namespace Dune

#endif
