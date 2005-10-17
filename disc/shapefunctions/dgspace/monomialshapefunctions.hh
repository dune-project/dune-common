// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_MONOMIALSHAPEFUNCTIONS_HH__
#define __DUNE_MONOMIALSHAPEFUNCTIONS_HH__

#include <iostream>
#include "common/fvector.hh"
#include "common/exceptions.hh"
#include "common/misc.hh"
#include "dune/grid/common/grid.hh"
#include "dune/grid/common/referenceelements.hh"

#include "shapefunctions.hh"

namespace Dune
{

  //multivariate monomials of the form x^m.y^n
  // monomial order/total degree  == p ; p <= (m+n)

  template<typename C,typename T, int d>
  class MonomialShapeFunction : public ShapeFunction<C,T,d,1>
  {
  public:

    enum { comps=1 };


    typedef C CoordType;
    typedef T ResultType;
  };


  template<typename C,typename T, int d, int polOrder>
  class MonomialShapeFunctionSet : public ShapeFunctionSet<C,T,d,1>
  {
  public:

    enum { comps=1 };
    enum {m=(polOrder+1)*(polOrder+2)/2};

    typedef C CoordType;
    typedef T ResultType;
    typedef MonomialShapeFunction<CoordType,ResultType,polOrder> value_type;
    //access to i'th monomialshapefun
    virtual const value_type& operator[] (int i) const=0;

  };


  template<typename Imp>
  class MonomialShapeFunctionWrapper :
    public MonomialShapeFunction<typename Imp::CoordType, typename Imp::ResultType, Imp::dim>,
    private Imp
  {

  public:
    enum {polOrder=Imp::polOrder};
    enum { dim=Imp::dim };
    enum { comps=1 };

    typedef typename Imp::CoordType CoordType;
    typedef typename Imp::ResultType ResultType;
    typedef Imp ImplementationType;
    //! assignment from implementation type (this class has no data)
    MonomialShapeFunctionWrapper& operator= (const Imp& imp)
    {
      Imp::operator=(imp);
      return *this;
    }
    virtual ResultType evaluateFunction (int i, int comp, const FieldVector<CoordType,dim>& x) const
    {
      return Imp::evaluateFunction(i,comp, x);
    }
    //! evaluate derivative of component comp in direction dir at point x
    virtual ResultType evaluateDerivative (int i, int comp, int dir, const FieldVector<CoordType,dim>& x) const
    {
      return Imp::evaluateDerivative(i,comp,dir,x);
    }

    //! consecutive number of associated dof within element
    virtual int localindex (int comp) const
    {
      //return Imp::localindex(comp);
      return;
    }

    //! codim of associated dof
    virtual int codim () const
    {
      //return Imp::codim();
      return;
    }

    //! entity (of codim) of associated dof
    virtual int entity () const
    {
      //return Imp::entity();
      return;
    }

    //! consecutive number of dof within entity
    virtual int entityindex () const
    {
      //return Imp::entityindex();
      return;
    }

    //! interpolation point associated with shape function
    virtual const FieldVector<CoordType,dim>& position () const
    {
      //return Imp::position();
      return;
    }


  };

  template<typename Imp>
  class MonomialShapeFunctionSetWrapper :
    public MonomialShapeFunctionSet<typename Imp::CoordType,typename Imp::ResultType, Imp::dim, Imp::polOrder>,
    private Imp
  {
  public:
    enum { dim=Imp::dim };
    enum { comps=1 };
    enum {polOrder=Imp::polOrder};
    typedef typename Imp::CoordType CoordType;
    typedef typename Imp::ResultType ResultType;
    typedef Imp ImplementationType;
    typedef MonomialShapeFunction<CoordType,ResultType,polOrder> value_type;
    //! return total number of shape functions
    virtual int size () const
    {
      return Imp::size();
    }

    //! total number of shape functions associated with entity in codim
    virtual int size (int entity, int codim) const
    {
      //return Imp::size(entity,codim);
      return;
    }


    //! random access to i'th ShapeFunction
    virtual const value_type& operator[] (int i) const
    {
      return Imp::operator[](i);
    }

    //! return order
    virtual int order () const
    {
      return Imp::order();
    }

    //! return type of element
    virtual GeometryType type () const
    {
      return Imp::type();
    }

  };


  template<typename C,typename T, int d, int polOrder>
  class MonomialBaseFunction
  {
  public:

    enum {m=(polOrder+1)*(polOrder+2)/2};

    typedef C CoordType;
    typedef T ResultType;
    typedef MonomialBaseFunction ImplementationType;
    MonomialBaseFunction(int ordr)
    {
      number = ordr;
      int i = 0;
      for (int alpha=0; alpha<=ordr; alpha++)
      {
        for (int beta=0; beta<=alpha; beta++)
        {
          //  x^(alpha-beta) * y^beta
          exponent[i][0] = alpha-beta; //
          exponent[i][1] = beta; //
          i++;
        }
      }
    }


    //! evaluate shape function in local coordinates
    // sfn is 0 to (noBaseFnt-1)
    ResultType evaluateFunction (int sfn, const FieldVector<CoordType,2>& x) const
    {
      //phi= x^(alpha-beta) * y^beta
      ResultType phi;
      phi = power(x[0],exponent[sfn][0])*(power(x[1],exponent[sfn][1]));
      return phi;
    }

    ResultType evaluateDerivative(int sfn,int dir, const FieldVector<CoordType,2>& x) const

    {
      //dphi/dx= (alpha-beta) * x^(alpha-beta-1) * y^beta
      //dphi/dy=  x^(alpha-beta) * beta * y^(beta-1)
      ResultType dphi;
      if(dir==0)
        dphi=exponent[sfn][0] * power(x[0],exponent[sfn][0]-1) *power(x[1],exponent[sfn][1]);
      else
        dphi=power(x[0],exponent[sfn][0]) *exponent[sfn][1] * power(x[1],exponent[sfn][1]-1);
      return dphi;
    }
    MonomialBaseFunction()
    {}
  private:
    int number, exponent[m][2];

    CoordType power(CoordType xx, int p) const {
      if (p <= 0)
        return 1.0;
      return xx*power(xx, p-1);
    }
  };


  template<typename C, typename T,int d, typename S, int polOrder>
  class MonomialBaseFunctionSet
  {
  public:
    enum {m=(polOrder+1)*(polOrder+2)/2};
    typedef C CoordType;
    typedef T ResultType;
    typedef S value_type;
    typedef typename S::ImplementationType Imp;
    MonomialBaseFunctionSet()

    {
      for(int i=0; i<m; ++i)
        sf[i]=Imp(i);

    }

    //total no of monomial shape functions
    int size() const

    {
      return m;
    }

    //random access to shape functions
    const value_type& operator[](int i) const
    {
      return sf[i];
    }
    int order() const
    {
      return polOrder;
    }

  private:
    S sf;

  };

  template<typename C,typename T, int d,int polOrder>
  class MonomialBaseFunctionSetContainer

  {
  public:
    // now only 2D
    enum {maxsize=(polOrder+1)*(polOrder+2)/2};
    typedef C CoordType;
    typedef T ResultType;

    typedef MonomialBaseFunctionSet<C,T,d,MonomialBaseFunction<C,T,d,polOrder>,polOrder > value_type;
    const value_type& operator()(GeometryType gtype,int order) const
    {
      if ( (gtype==line) || (gtype==triangle) || (gtype==quadrilateral))
        return monombfn;
      DUNE_THROW(NotImplemented,"monomialshapefn only support 2D now");
    }
  private:
    value_type monombfn;
  };

  // general container for monomialshape functions of any order and any element type

  template<typename C,typename T, int d, int polOrder>
  class MonomialShapeFunctionSetContainer;

  template<typename C,typename T,int polOrder>
  class MonomialShapeFunctionSetContainer<C,T,2,polOrder> : public ShapeFunctionSetContainer<C,T,2,1,(polOrder+1)*(polOrder+2)/2 >
  {
  public:
    // only support 2D now
    enum {maxsize=(polOrder+1)*(polOrder+2)/2};
    typedef C CoordType;
    typedef T ResultType;

    typedef MonomialShapeFunctionSet<C,T,2,polOrder> value_type;
    const value_type& operator()(GeometryType gtype,int order) const
    {
      if ( (gtype==line) || (gtype==triangle) || (gtype==quadrilateral))
        return wrappedmonombfn;
      DUNE_THROW(NotImplemented,"monomialshapefn only support 2D Elements now");
    }
  private:
    typedef MonomialShapeFunctionWrapper<MonomialBaseFunction<C,T,2,polOrder> > WrappedMonomialBaseFunction;
    typedef MonomialBaseFunctionSet<C,T,2,WrappedMonomialBaseFunction,polOrder> MonomialWrappedBaseFunctionSet;
    typedef MonomialShapeFunctionSetWrapper<MonomialWrappedBaseFunctionSet> WrappedMonomialBaseFunctionSet;
    WrappedMonomialBaseFunctionSet wrappedmonombfn;
  };


} //end of namespace

#endif
