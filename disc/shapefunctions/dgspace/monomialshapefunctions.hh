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

namespace Dune
{

  //multivariate monomials of the form x^m.y^n
  // monomial order/total degree  == p ; p <= (m+n)

  template<typename C,typename T>
  class MonomialShapeFunction
  {
  public:
    typedef C CoordType;
    typedef T ResultType;
    typedef  MonomialShapeFunction ImplementationType;
    //constructor
    MonomialShapeFunction(int polOrder)
    {
      number = polOrder;
      int i = 0;
      for (int alpha=0; alpha<=polOrder_; alpha++)
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
    ResultType evaluateFunction (int sfn, const FieldVector<CoordType,d>& x) const
    {
      //phi= x^(alpha-beta) * y^beta
      ResultType phi;
      phi = power(x[0],exponent[sfn][0])*(power(x[1],exponent[sfn][1]))
            return phi;
    }

    ResultType evaluateDerivative(int sfn,int dir, const FieldVector<CoordType,d>& x) const

    {
      //dphi/dx= (alpha-beta) * x^(alpha-beta-1) * y^beta
      //dphi/dy=  x^(alpha-beta) * beta * y^(beta-1)
      ResultType dphi;
      if(dir==0)
        dphi=exponent[sfn][0] * power(x[0],exponent[sfn][0]-1) *power(x[1],exponent[sfn][1]);
      else
        dphi=power(x[0],exponent[sfn][0]) *exponent[sfn][1] * power(x[1],exponent[sfn][1]-1);
    }

  };

  template<typename C,typename T, typename S>
  class MonomialShapeFunctionSet
  {
  public:
    enum {dim=2};
    enum {m=(polOrder+1)*(polOrder+2)/2};

    typedef C CoordType;
    typedef T ResultType;
    typedef S value_type;
    typedef typename S::ImplementationType Imp;
  };

  MonomialShapeFunctionSet()

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
} // end of namespace

template<typename C,typename T>
class MonomialShapeFunctionSetContainer

{
public:
  enum {d=2};
  enum {maxsize=(polOrder+1)*(polOrder+2)/2};
  typedef C CoordType;
  typedef T ResultType;

  typedef MonomialShapeFunctionSet<C,T,d,polOrder,MonomialShapeFunction<C,T,d,polOrder> > value_type;
  const value_type& operator()(polOrder) const
  {
    if (d==2) return monombfn;
    DUNE_THROW(NotImplemented,"monomialshapefn only support 2D now");
  }
private:
  value_type monombfn;
};


} //end of namespace
