// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BARYCENTER_HH__
#define __DUNE_BARYCENTER_HH__

#include <dune/common/matvec.hh>

#include "common/quadrature.hh"

// the barycenters for each codim
#include "barycenter/facecenterpoints.hh"

namespace Dune {

  /*********************************************************************/
  /** \brief  %Quadrature for barycenter of the faces of one element
   * \ingroup Quadrature
   *
   * Supported element types are
   * lines, triangles, quadrilaterals, tetrahedrons, hexahedrons
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

    //! the number of quadrature points is stored as enum
    enum { maxQuadPoints = 9 };

  public:
    //! Constructor build the vec with the points and weights
    template <class EntityType>
    BaryCenterQuad ( EntityType &en )
    {
      switch(en.geometry().type())
      {
      case line :          { makeQuadrature<line> (); break; }
      case quadrilateral : { makeQuadrature<quadrilateral> (); break; }
      case hexahedron :    { makeQuadrature<hexahedron> (); break; }
      case triangle :      { makeQuadrature<triangle> (); break; }
      case tetrahedron :   { makeQuadrature<tetrahedron> (); break; }
      default :       { std::cerr << "Unkown GeometryType in BaryCenterQuad::makeQuadrature()\n"; abort();  break; }
      }
    };

    //! Constructor build the vec with the points and weights
    BaryCenterQuad ( GeometryType eltype )
    {
      switch(eltype)
      {
      case line :          { makeQuadrature<line> (); break; }
      case quadrilateral : { makeQuadrature<quadrilateral> (); break; }
      case hexahedron :    { makeQuadrature<hexahedron> (); break; }
      case triangle :      { makeQuadrature<triangle> (); break; }
      case tetrahedron :   { makeQuadrature<tetrahedron> (); break; }
      default :       { std::cerr << "Unkown GeometryType in BaryCenterQuad::makeQuadrature()\n"; abort();  break; }
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
    template <GeometryType ElType>
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
    }

    //! number of quadrature points
    int numberOfQuadPoints_;

    //! real quadrature order
    int order_;

    //! Vecs with constant length holding the weights and points
    FieldVector<RangeFieldType, maxQuadPoints> weights_;
    FieldVector<DomainType, maxQuadPoints>     points_;

  }; // end class DuneQuadrature

} // end namespace Dune
#endif
