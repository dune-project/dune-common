// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_QUADRATURE_HH__
#define __DUNE_QUADRATURE_HH__

#include <vector>

#include <dune/common/dynamictype.hh>
#include "basefunctions.hh"

namespace Dune {



  //************************************************************************
  //
  //  --QuadraturePoints
  //
  // Every specialization of this class implements a diffrent quadrature
  // for diffrent elements and polynomial order. This implementation in left
  // to the user.
  //
  //************************************************************************
  // calculates the number of points on on segment, i.e. a line
  template <int order>
  struct PointsOnLine
  {
    // from Peters GaussQuadrature, see Constructor
    enum { points = ( order > 17 ) ? 10 : 1 };
  };

  // specialization for the given order
  template <> struct PointsOnLine <3>  { enum { points = 2 }; };
  template <> struct PointsOnLine <4>  { enum { points = 3 }; };
  template <> struct PointsOnLine <5>  { enum { points = 3 }; };
  template <> struct PointsOnLine <6>  { enum { points = 4 }; };
  template <> struct PointsOnLine <7>  { enum { points = 4 }; };
  template <> struct PointsOnLine <8>  { enum { points = 5 }; };
  template <> struct PointsOnLine <9>  { enum { points = 5 }; };
  template <> struct PointsOnLine <10> { enum { points = 6 }; };
  template <> struct PointsOnLine <11> { enum { points = 6 }; };
  template <> struct PointsOnLine <12> { enum { points = 7 }; };
  template <> struct PointsOnLine <13> { enum { points = 7 }; };
  template <> struct PointsOnLine <14> { enum { points = 8 }; };
  template <> struct PointsOnLine <15> { enum { points = 8 }; };
  template <> struct PointsOnLine <16> { enum { points = 9 }; };
  template <> struct PointsOnLine <17> { enum { points = 9 }; };
  // other specialization possible

  // calculates m^p
  template <int m, int p>
  struct power_M_P
  {
    // power stores m^p
    enum { power = (m * power_M_P<m,p-1>::power ) };
  };


  // end of recursion via specialization
  template <int m>
  struct power_M_P< m , 0>
  {
    // m^0 = 1
    enum { power = 1 };
  };


  /** @defgroup Quadrature Quadrature
     @ingroup DiscreteFunction
     This is the interface for qaudratures. It contains three methods,
     namely for geting the number of quadrature points, the quadrature points
     and the quadrature weights.

     @{
   */

  //***********************************************************************
  //!
  //! Note: The sum over all weigths for the quadrature is the volume of the
  //! reference element, for example the sum over three quadrature point for
  //! a triangle is 0.5 which is the volume on reference triangle
  //!
  //***********************************************************************
  template <class Domain, class RangeField, ElementType ElType, int polOrd>
  struct QuadraturePoints
  {
    enum { identifier = 0 };
    static int numberOfQuadPoints ();
    static int order ();
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  template <class Domain, class RangeField, ElementType ElType, int polOrd>
  int QuadraturePoints<Domain,RangeField, ElType,polOrd>::
  numberOfQuadPoints()
  {
    return -1;
  }
  template <class Domain, class RangeField, ElementType ElType, int polOrd>
  int QuadraturePoints<Domain,RangeField, ElType,polOrd>::order()
  {
    return -1;
  }
  //! default implementation of getPoint throws error because no
  //! default implementation can be done in this matter
  template <class Domain, class RangeField, ElementType ElType, int polOrd>
  Domain QuadraturePoints<Domain,RangeField, ElType,polOrd>::getPoint ( int i )
  {
    Domain tmp;
    std::cerr << "No default implementation of getPoint (i) available for this set of template parameters!\n";
    abort();
    return tmp;
  }


  //! default implementation of getWeight throws error because no
  //! default implementation can be done in this matter
  template <class Domain, class RangeField, ElementType ElType, int polOrd>
  RangeField QuadraturePoints<Domain,RangeField, ElType,polOrd>::
  getWeight ( int i )
  {
    RangeField tmp;
    std::cerr << "No default implementation of getWeight (i) available for this set of template parameters!\n";
    abort();
    return tmp;
  }

  template <class Domain, class RangeField, ElementType ElType, int polOrd>
  struct DualQuadraturePoints
  {
    enum { identifier = 0 };
    enum { numberOfQuadPoints = 0 };
    static Domain getPoint (int i);
    static RangeField getWeight (int i);
  };

  //! default implementation of getPoint throws error because no
  //! default implementation can be done in this matter
  template <class Domain, class RangeField, ElementType ElType, int polOrd>
  Domain DualQuadraturePoints<Domain,RangeField, ElType,polOrd>::getPoint ( int i )
  {
    Domain tmp;
    std::cerr << "No default implementation of getPoint (i) available for this set of template parameters!\n";
    abort();
    return tmp;
  }


  //! default implementation of getWeight throws error because no
  //! default implementation can be done in this matter
  template <class Domain, class RangeField, ElementType ElType, int polOrd>
  RangeField DualQuadraturePoints<Domain,RangeField, ElType,polOrd>::
  getWeight ( int i )
  {
    RangeField tmp;
    std::cerr << "No default implementation of getWeight (i) available for this set of template parameters!\n";
    abort();
    return tmp;
  }

  //**************************************************************************
  //
  //  --QuadratureInterface
  //
  //! Interface for Quadratures. The Interface used the good old
  //! Barton-Nackmen technique. There are two diffrent implementations of the
  //! Interface. One has to be parametrized by polynomial order as template
  //! parameter and the other uses dynamic polynomial order but the two have
  //! to be parametrized by the element type.
  //!
  //**************************************************************************
  template< class RangeFieldType , class DomainType , class QuadratureImp>
  class QuadratureInterface : public DynamicType
  {
  public:
    //! Contructor passing ident to DynamicType for comparison with other
    //! Quadratures
    QuadratureInterface ( int ident ) : DynamicType (ident) {};

    //! return number of quadrature points
    int nop() const { return asImp().nop(); }


    //! return order of quadrature
    int order () const { return asImp().order(); }

    //! return  quadrature weight for point i
    const RangeFieldType& weight ( int i) const
    {
      return (asImp().weight(i));
    }


    //! return  quadrature point i in local coordinates
    const DomainType& point (int i) const
    {
      return (asImp().point(i));
    }


  private:
    //! Barton - Nackman trick
    QuadratureImp &asImp() { return static_cast<QuadratureImp&>(*this); }
    const QuadratureImp &asImp() const
    { return static_cast<const QuadratureImp&>(*this); }

  }; // end class QuadraturInterface

  template< class RangeFieldType , class DomainType , class QuadratureImp>
  class QuadratureDefault
    : public QuadratureInterface  < RangeFieldType , DomainType , QuadratureImp >
  {
  public:
    QuadratureDefault ( int ident ) :
      QuadratureInterface < RangeFieldType , DomainType, QuadratureImp > (ident) {};

    void printQuad () const
    {
      for(int i=0; i<asImp().nop(); i++)
      {
        std::cout << asImp().weight(i) << asImp().point(i) << "\n";
      }
    }
  private:
    //! Barton - Nackman trick
    QuadratureImp &asImp() { return static_cast<QuadratureImp&>(*this); }
    const QuadratureImp &asImp() const
    { return static_cast<const QuadratureImp&>(*this); }

  }; // end class QuadraturDefault

  /** @} end documentation group */



  /** @defgroup QuadratureImp QuadratureImp
     @ingroup Quadrature

     @{
   */

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
  template< class FunctionSpaceType, ElementType ElType >
  class QuadratureImp : public QuadratureDefault  <
                            typename FunctionSpaceType::RangeField , typename
                            FunctionSpaceType::Domain, Quadrature < FunctionSpaceType, ElType > >  {

    // just for readability
    typedef typename FunctionSpaceType::Domain DomainType ;
    typedef typename FunctionSpaceType::RangeField RangeFieldType ;

    // my Type
    typedef Quadrature < FunctionSpaceType , ElType > QuadratureType;

  public:
    //! Constructor building the quadrature
    Quadrature ( int id , int polOrd ) :
      QuadratureDefault < FunctionSpaceType, QuadratureType > (id)
    {
      buildQuadrature ( id , polOrd );
    };

    //! return number of quadrature points
    int getNumberOfQuadPoints () const { return numQuadPoints_; };

    //! return weight corresponding to point i
    const RangeFieldType&  getQuadratureWeight ( int i) const
    {
      return (weights_[i]);
    };

    //! return point i
    const DomainType&  getQuadraturePoint (int i) const
    {
      return (points_[i]);
    };

  private:
    //! get the quadrature points for storage in the vectors
    template <int polynomialOrder>
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

      // get memory
      weights_.resize( numQuadPoints_);
      points_.resize( numQuadPoints_);

      for(int i=0; i<numQuadPoints_; i++)
      {
        points_[i] = QuadInitializer::getPoint(i);
        weights_[i] = QuadInitializer::getWeight(i);
      }
    };

    //! number of quadrature points
    int numQuadPoints_;

    //! Vectors holding the weights and points
    std::vector < RangeFieldType > weights_;
    std::vector < DomainType >     points_;

    //! anoying but what can we do
    //! can be expanded up to inf
    void buildQuadrature ( int id , int polOrd )
    {
      switch (polOrd)
      {
      case 0 : { makeQuadrature<0> (id); break; };
      case 1 : { makeQuadrature<1> (id); break; };
      case 2 : { makeQuadrature<2> (id); break; };
      case 3 : { makeQuadrature<3> (id); break; };
      case 4 : { makeQuadrature<4> (id); break; };
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
      default : {
        std::cerr << "No Rule to make Quadrature with polOrd ";
        std::cerr << polOrd << " in Quadrature ( id , polOrd ) !\n";
        abort();
      };
      }
    };
  }; // end class QuadratureImp

  /** @} end documentation group */


} // end namespace Dune

#endif
