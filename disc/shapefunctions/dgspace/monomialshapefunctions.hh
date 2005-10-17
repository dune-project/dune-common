// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MONOMIALSHAPEFUNCTIONS_HH
#define DUNE_MONOMIALSHAPEFUNCTIONS_HH

#include <iostream>
#include "common/fvector.hh"
#include "common/simplevector.hh"
#include "common/exceptions.hh"
#include "common/misc.hh"
#include "dune/grid/common/grid.hh"
#include "dune/grid/common/referenceelements.hh"

namespace Dune
{
  enum { MonomialShapeFunctionDefaultMaxOrder = 5 };

  template<typename C,typename T, int dim>
  class MonomialShapeFunction;
  template<typename C,typename T, int dim>
  class MonomialShapeFunctionSet;
  template<typename C,typename T, int dim, int maxOrder=MonomialShapeFunctionDefaultMaxOrder>
  class MonomialShapeFunctionSetContainer;

  /*
     \class MonomialShapeFunction
     \brief multivariate monomials of the form x^a.y^b
     monomial order/total degree == p ; p <= (a+b)
   */
  template<typename C,typename T>
  class MonomialShapeFunction<C,T,2> : public ShapeFunction<C,T,2,1>
  {
  public:
    enum { dim=2 /*< Dimension of the ShapeFunction */ };
    typedef C CoordType;
    typedef T ResultType;

    /*
       Create the n-th MonomialShapeFunction of the ShapeFunctionSet of
       the given order.

       \param n this is the n-th shape functions in the set
       \param a exponent a of x^a
       \param b exponent b of y^b
     */
    MonomialShapeFunction(int n, int a, int b) :
      n_(n), a_(a), b_(b) {}

    //! \copydoc ShapeFunction::evaluateFunction
    virtual ResultType
    evaluateFunction (int, const FieldVector<CoordType,dim>& x) const
    {
      ResultType phi;
      // phi= x^(alpha-beta) * y^beta
      phi = power(x[0],a_)*(power(x[1],b_));
      return phi;
    }

    //! \copydoc ShapeFunction::evaluateDerivative
    virtual ResultType
    evaluateDerivative(int, int dir, const FieldVector<CoordType,dim>& x) const
    {
      ResultType dphi;
      if(dir==0)
        // dphi/dx= (alpha-beta) * x^(alpha-beta-1) * y^beta
        dphi = a_ * power(x[0],a_-1) * power(x[1],b_);
      else
        // dphi/dy=  x^(alpha-beta) * beta * y^(beta-1)
        dphi = power(x[0],a_) * b_ * power(x[1],b_-1);
      return dphi;
    }

    //! \copydoc ShapeFunction::localindex
    virtual int localindex (int comp) const { return n_; };

    //! \copydoc ShapeFunction::codim()
    virtual int codim () const { return 0; };

    //! \copydoc ShapeFunction::entity()
    virtual int entity () const { return 0; };

    //! \copydoc ShapeFunction::entityindex()
    virtual int entityindex () const { return 0; };

    //! pretty print this shape function
    void print (std::ostream& s) const
    {
      if (a_ > 0 && b_ > 0) {
        s << "x^" << a_
          << "y^" << b_;
      }
      if (a_ == 0 && b_ > 0) {
        s << "y^" << b_;
      }
      if (a_ > 0 && b_ == 0) {
        s << "x^" << a_;
      }
      if (a_ == 0 && b_ == 0) {
        s << 1;
      }
    };
  private:
    /** this is the n-th shape functions in the set */
    int n_;
    /** exponent of x^a */
    int a_;
    /** exponent of y^b */
    int b_;
    /** calculate the power x^p */
    ResultType power(CoordType x, int p) const {
      if (p <= 0)
        return 1.0;
      return x*power(x, p-1);
    }
  };

  template<typename C,typename T>
  class MonomialShapeFunctionSet<C, T, 2> : public ShapeFunctionSet<C, T, 2, 1>
  {
  public:
    enum { dim=2 /*< Dimension of the ShapeFunctionSet */ };
    typedef C CoordType;
    typedef T ResultType;
    typedef MonomialShapeFunction<C,T,2> ShapeFunction;

    MonomialShapeFunctionSet(int order) :
      order_(order), n_((order+1)*(order+2)/2), shapeFunctions(n_)
    {
      shapeFunctions = 0;
      int i = 0;
      for (int alpha=0; alpha<=order_; alpha++)
      {
        for (int beta=0; beta<=alpha; beta++)
        {
          //  x^(alpha-beta) * y^beta
          shapeFunctions[i] =
            new ShapeFunction(i, alpha-beta, beta);
          i++;
        }
      }
    }

    virtual ~MonomialShapeFunctionSet()
    {
      for (int i=0; i < n_; i++)
      {
        if (shapeFunctions[i]) delete shapeFunctions[i];
      }
    }

    //! \copydoc ShapeFunctionSet::size()
    virtual int size() const
    {
      return n_;
    }

    //! \copydoc ShapeFunctionSet::size(int, int)
    virtual int size (int entity, int codim) const
    {
      if (codim == 0) return n_;
      return 0;
    };

    //! \copydoc ShapeFunctionSet::operator[](int i)
    virtual const ShapeFunction& operator[](int i) const
    {
      return *shapeFunctions[i];
    }

    //! \copydoc ShapeFunctionSet::order()
    virtual int order() const
    {
      return order_;
    }

    //! \copydoc ShapeFunctionSet::type()
    virtual GeometryType type () const
    {
      //! throw an exception... we dont' have a special geometry
#warning what to do here?
      DUNE_THROW(MathError, "monomial shape function don't have a geometry type");
    };

  private:
    /** order of the ShapeFunctionSet */
    const int order_;
    /** total number of shapefunctions in this set */
    const int n_;
    /** vector which holds the shape functions */
    SimpleVector< ShapeFunction* > shapeFunctions;
  };

  template<typename C,typename T, int maxOrder>
  class MonomialShapeFunctionSetContainer<C,T,2,maxOrder> :
    public ShapeFunctionSetContainer<C,T,2,1,(maxOrder+1)*(maxOrder+2)/2>
  {
  public:
    enum { d=2 };
    enum { maxsize=(maxOrder+1)*(maxOrder+2)/2 };
    typedef C CoordType;
    typedef T ResultType;
    typedef MonomialShapeFunctionSet<C,T,d> ShapeFunctionSet;

    MonomialShapeFunctionSetContainer() :
      shapeFunctionSets(maxsize)
    {
      for (int order=0; order<=maxOrder; order++)
      {
        shapeFunctionSets[order] =
          new ShapeFunctionSet(order);
      }
    }

    virtual ~MonomialShapeFunctionSetContainer()
    {
      for (int i=0; i < maxOrder; i++)
      {
        if (shapeFunctionSets[i]) delete shapeFunctionSets[i];
      }
    }
    //! \copydoc ShapeFunctionSetContainer::operator()
    virtual const ShapeFunctionSet& operator() (GeometryType type, int order) const
    {
      assert(order <= maxOrder);
      return *shapeFunctionSets[order];
    }
  private:
    /** vector which holds the shape functions */
    SimpleVector< ShapeFunctionSet* > shapeFunctionSets;
  };

} //end of namespace

#endif // DUNE_MONOMIALSHAPEFUNCTIONS_HH
