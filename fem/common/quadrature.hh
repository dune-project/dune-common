// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_QUADRATURE_HH__
#define __DUNE_QUADRATURE_HH__

#include <vector>

#include <dune/common/dynamictype.hh>
#include "basefunctions.hh"

// For ElementType
#include <dune/grid/common/grid.hh>

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
    //QuadratureInterface ( int ident ) : DynamicType (ident) {};
    //QuadratureInterface () {};

    //! return number of quadrature points
    int nop() const { return asImp().nop(); }


    //! return order of quadrature
    int order () const { return asImp().order(); }

    //! return  quadrature weight for point i
    const RangeFieldType& weight ( int i) const
    {
      return (asImp().weight(i));
    }


    //! return  quadrature point i in local coordinates of corresponding
    //! refence element
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
    //QuadratureDefault ( int ident ) :
    //  QuadratureInterface < RangeFieldType , DomainType, QuadratureImp > (ident) {};

    void print (std::ostream& s, int indent) const
    {
      RangeField sum = 0.0;
      s << "quad (id = " << asImp().getIdentifier() <<") { \n" ;
      s << " nop = " << asImp().nop() << " | order = " << asImp().order() << "\n";
      for(int i=0; i<asImp().nop(); i++)
      {
        sum += asImp().weight(i);
        s << " w = " << asImp().weight(i) << " | p = " << asImp().point(i) << "\n";
      }
      s << " Sum of w = " << sum << " \n";
      s << "}" ;
    }
  private:
    //! Barton - Nackman trick
    QuadratureImp &asImp() { return static_cast<QuadratureImp&>(*this); }
    const QuadratureImp &asImp() const
    { return static_cast<const QuadratureImp&>(*this); }

  }; // end class QuadraturDefault

  // overloading the out stream for printing of quadratures
  template< class RangeFieldType , class DomainType , class QuadratureImp>
  inline std::ostream& operator<< (std::ostream& s,
                                   QuadratureDefault<RangeFieldType,DomainType,QuadratureImp>& quad)
  {
    quad.print(s,0);
    return s;
  }


  /** @} end documentation group */

} // end namespace Dune

#endif
