// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <string>

#include <dune/fem/dgspace/monomialbase.hh>

template <class FunctionSpaceType>
MonomialBaseFunctionSet<FunctionSpaceType >::
MonomialBaseFunctionSet( FunctionSpaceType & fuspace, int polOrder )
  :
    BaseFunctionSetDefault
    <FunctionSpaceType,MonomialBaseFunctionSet <FunctionSpaceType > >
      ( fuspace ),
    polOrder_(polOrder),
    numOfBaseFct_((polOrder + 2) * (polOrder + 1) / 2),
    Phi_(numOfBaseFct_)
{
  assert(DimRange == 1);
  if (DimDomain == 2)
  {
    int i = 0;
    for (int a=0; a<=polOrder_; a++)
    {
      for (int b=0; b<=a; b++)
      {
        // phi_i = x^(a-b) * y^b
        Phi_[i][0] = a-b; // x^(a-b)
        Phi_[i][1] = b;   // y^b
        i++;
      }
    }
  }
  else
  {
    throw std::string("MonomialBaseFunctionSet only supports 2D Domain");
  }
}

template <class FunctionSpaceType>
void MonomialBaseFunctionSet<FunctionSpaceType >::
real_evaluate( int baseFunct, const Vec<0, deriType> &diffVariable,
               const Domain & x,  Range & phi ) const
{
  phi = power(x[0], Phi_[baseFunct][0]) * power(x[1], Phi_[baseFunct][1]);
}

template <class FunctionSpaceType>
void MonomialBaseFunctionSet<FunctionSpaceType >::
real_evaluate( int baseFunct, const Vec<1, deriType> &diffVariable,
               const Domain & x,  Range & phi ) const
{
  if (diffVariable(0) == 0) {
    phi = Phi_[baseFunct][0] * power(x[0], Phi_[baseFunct][0]-1)
          * power(x[1], Phi_[baseFunct][1]);;
  }
  else {
    phi = power(x[0], Phi_[baseFunct][0])
          * Phi_[baseFunct][1] * power(x[1], Phi_[baseFunct][1]-1);
  }
}

template <class FunctionSpaceType>
void MonomialBaseFunctionSet<FunctionSpaceType >::
real_evaluate( int baseFunct, const Vec<2, deriType> &diffVariable,
               const Domain & x,  Range & phi ) const
{
  if (diffVariable(0) == 0) {
    if (diffVariable(1) == 0) {
      // d/(dx^2)
      phi = Phi_[baseFunct][0] * ( Phi_[baseFunct][0] - 1 )
            * power(x[0], Phi_[baseFunct][0]-2)
            * power(x[1], Phi_[baseFunct][1]);
    }
    else {
      // d/(dxdy)
      phi = Phi_[baseFunct][0] * power(x[0], Phi_[baseFunct][0]-1)
            * Phi_[baseFunct][1] * power(x[1], Phi_[baseFunct][1]-1);
    }
  }
  else {
    if (diffVariable(1) == 0) {
      // d/(dydx)
      phi = Phi_[baseFunct][0] * power(x[0], Phi_[baseFunct][0]-1)
            * Phi_[baseFunct][1] * power(x[1], Phi_[baseFunct][1]-1);
    }
    else {
      // d/(dy^2)
      phi = Phi_[baseFunct][1] * ( Phi_[baseFunct][1] - 1 )
            * power(x[1], Phi_[baseFunct][1]-2)
            * power(x[0], Phi_[baseFunct][0]);
    }
  }
}

template <class FunctionSpaceType> template <int diffOrd>
void MonomialBaseFunctionSet<FunctionSpaceType >::
evaluate( int baseFunct, const Vec<diffOrd, deriType> &diffVariable,
          const Domain & x,  Range & phi ) const
{
  assert(baseFunct < numOfBaseFct_);
  real_evaluate(baseFunct, diffVariable, x, phi);
}

template <class FunctionSpaceType> template <int diffOrd, class QuadratureType>
void MonomialBaseFunctionSet<FunctionSpaceType >::
evaluate( int baseFunct, const Vec<diffOrd, deriType> &diffVariable,
          QuadratureType & quad, int quadPoint, Range & phi ) const
{
  Domain x = quad.point(quadPoint);
  real_evaluate(baseFunct, diffVariable, x, phi);
}

template <class FunctionSpaceType>
void MonomialBaseFunctionSet<FunctionSpaceType >::
print (std::ostream& s, const Dune::Vec<2,int> & pol) const
{
  if (pol[0] > 0 && pol[1] > 0) {
    s << "x^" << pol[0]
      << "y^" << pol[1];
  }
  if (pol[0] == 0 && pol[1] > 0) {
    s << "y^" << pol[1];
  }
  if (pol[0] > 0 && pol[1] == 0) {
    s << "x^" << pol[0];
  }
  if (pol[0] == 0 && pol[1] == 0) {
    s << 1;
  }
};
