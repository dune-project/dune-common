// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_QUADRATURE_HH__
#define __DUNE_QUADRATURE_HH__

#include <vector>

#include "../common/dynamictype.hh"
#include "basefunctions.hh"

namespace Dune {


  //! no default implementation, because alway depends on element type
  template <class Domain, class RangeField, ElementType ElType, int polOrd>
  struct QuadraturePoints;

  template< class FunctionSpaceType , class QuadratureImp>
  class QuadratureInterface : public DynamicType
  {

    typedef typename FunctionSpaceType::Domain DomainType ;
    typedef typename FunctionSpaceType::RangeField RangeFieldType ;

    // use the methode getIdentifier instead
    //friend class BaseFunctionSetInterface < FunctionSpaceType >  ;

  public:
    //! Contructor passing ident to DynamicType for comparison with other
    //! Quadratures
    QuadratureInterface ( int ident ) : DynamicType (ident) {};

    int getNumberOfQuadPoints () const
    {
      return asImp().getNumberOfQuadPoints();
    };

    const RangeFieldType&  getQuadratureWeight ( int number ) const
    {
      return asImp().getQuadratureWeight(number);
    };

    const DomainType&  getQuadraturePoint (int number ) const
    {
      return asImp().getQuadraturePoint(number);
    };

  private:

    QuadratureImp &asImp() { return static_cast<QuadratureImp&>(*this); }
    const QuadratureImp &asImp() const
    { return static_cast<const QuadratureImp&>(*this); }

#if 0
    not used anymore
    //int numberOfPoints_ ;

    //std::vector< RangeFieldType > weights_ ;
    //std::vector< DomainType > points_ ;
#endif

  }; // end class QuadraturInterface

  //********************************************************************
  //
  //! Quadrature class implementation
  //! Quadrature is diffrent for diffrent Domain and RangeField types and of
  //! course for diffrent element types and polynomial order.
  //! The element type comes from a given entity or the grid and polOrd is
  //! user defined but known a compilation time (if not this has to be
  //! changed.
  //
  //********************************************************************
  template< class FunctionSpaceType, ElementType ElType, int polOrd >
  class Quadrature : public QuadratureInterface  < FunctionSpaceType ,
                         Quadrature < FunctionSpaceType, ElType, polOrd > >  {

    typedef typename FunctionSpaceType::Domain DomainType ;
    typedef typename FunctionSpaceType::RangeField RangeFieldType ;

    typedef QuadraturePoints<DomainType,RangeFieldType,ElType,polOrd> QuadInitializer;
    typedef Quadrature < FunctionSpaceType , ElType , polOrd > MyType;

    enum { id = QuadInitializer::identifier };

    // static const IdentifierType id = 4;

  public:
    enum { numQuadPoints = QuadInitializer::numberOfQuadPoints };

    Quadrature ( ) :
      QuadratureInterface < FunctionSpaceType, MyType > (id)
    {
      std::cout <<"Making Quadrature \n";
      for(int i=0; i<numQuadPoints; i++)
      {
        points_(i) = QuadInitializer::getPoint(i);
        weights_(i) = QuadInitializer::getWeight(i);
      }
    };

    int getNumberOfQuadPoints () const { return numQuadPoints; };

    const RangeFieldType&  getQuadratureWeight ( int i) const
    {
      return (weights_.read(i));
    };

    const DomainType&  getQuadraturePoint (int i) const
    {
      return (points_.read(i));
    };

  private:
    //! Vecs with constant length holding the weights and points
    Vec < numQuadPoints, RangeFieldType > weights_;
    Vec < numQuadPoints, DomainType >     points_;

  }; // end class Quadrature

} // end namespace Dune

#endif
