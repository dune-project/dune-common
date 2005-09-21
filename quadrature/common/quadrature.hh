// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_QUADRATURE_HH
#define DUNE_QUADRATURE_HH

#include <dune/common/dynamictype.hh>

// For GeometryType
#include <dune/grid/common/grid.hh>

#include <dune/common/exceptions.hh>

// For Quadrature::Iterator
#include <dune/common/genericiterator.hh>

namespace Dune {


  //************************************************************************
  //
  //  --QuadraturePoints
  //
  // Every specialization of this class implements a different quadrature
  // for different elements and polynomial order. This implementation is left
  // to the user.
  //
  //************************************************************************

  /** @defgroup Quadrature Quadrature
     This is the interface for quadratures. It contains three methods,
     namely for getting the number of quadrature points, the quadrature points
     and the quadrature weights.

     @{
   */

  //***********************************************************************
  //!
  //! Note: The sum over all weights for the quadrature is the volume of the
  //! reference element, for example the sum over three quadrature point for
  //! a triangle is 0.5 which is the volume of the reference triangle
  //!
  //***********************************************************************
  template <class Domain, class RangeField, GeometryType ElType, int polOrd>
  struct QuadraturePoints
  {
    enum { identifier = 0 };
    static int numberOfQuadPoints () {
      DUNE_THROW(NotImplemented, "");
      return -1;
    };
    static int order () {
      DUNE_THROW(NotImplemented, "");
      return -1;
    };
    static Domain getPoint (int i) {
      DUNE_THROW(NotImplemented, "");
      return Domain();
    };
    static RangeField getWeight (int i) {
      DUNE_THROW(NotImplemented, "");
      return RangeField();
    };
  };

  template <class Domain, class RangeField, GeometryType ElType, int polOrd>
  struct DualQuadraturePoints
  {
    enum { identifier = 0 };
    enum { numberOfQuadPoints = 0 };
    static Domain getPoint (int i) { DUNE_THROW(NotImplemented, ""); };
    static RangeField getWeight (int i) { DUNE_THROW(NotImplemented, ""); };
  };

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
    typedef QuadratureDefault< RangeFieldType , DomainType , QuadratureImp >
    QuadratureClass;

    //! A single quadrature point
    class QuadraturePoint
    {
      DomainType p;
      RangeFieldType w;
      QuadraturePoint() {};
    public:
      friend class QuadratureDefault< RangeFieldType , DomainType , QuadratureImp >;
      //! return the evalution point
      const DomainType & point () const { return p; }
      //! return the weight
      const RangeFieldType & weight () const { return w; }
    };

    //! get quadrature point i
    QuadraturePoint & operator[] (int i) const
    {
      static QuadraturePoint qp;
      qp.p = this->point(i);
      qp.w = this->weight(i);
      return qp;
    };

    //! Iterator for quadrature points
    typedef GenericIterator<QuadratureClass, QuadraturePoint>
    Iterator;
    //! const Iterator for quadrature points
    typedef GenericIterator<const QuadratureClass, const QuadraturePoint>
    constIterator;

    /** Returns an Iterator pointing to the beginning of the
        quadrature point list. */
    Iterator begin(){
      return Iterator(*this, 0);
    }

    /** Returns an constIterator pointing to the beginning of the
        quadrature point list. */
    constIterator begin() const {
      return constIterator(*this, 0);
    }

    /** Returns an Iterator pointing to the end of the
        quadrature point list. */
    Iterator end(){
      return Iterator(*this, asImp().nop());
    }

    /** Returns an constIterator pointing to the end of the
        quadrature point list. */
    constIterator end() const {
      return constIterator(*this, asImp().nop());
    }

    //! pretty print this Quadrature to ostream s
    void print (std::ostream& s) const
    {
      double sum = 0.0;
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
    quad.print(s);
    return s;
  }

  /** @} end documentation group */

} // end namespace Dune

#endif
