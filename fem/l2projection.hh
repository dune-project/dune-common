// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_L2_PROJECTION_CC__
#define __DUNE_L2_PROJECTION_CC__

#include <dune/quadrature/fixedorder.hh>


namespace Dune
{

  // projection of the rhs
  template <class DiscreteFunctionType>
  class L2Projection
  {
    typedef typename DiscreteFunctionType::FunctionSpaceType FunctionSpaceType;

  public:
    template <int polOrd, class FunctionType>
    void project (FunctionType &f, DiscreteFunctionType &discFunc, int level)
    {
      const typename DiscreteFunctionType::FunctionSpace
      & functionSpace_= discFunc.getFunctionSpace();

      discFunc.clear();

      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::template Traits<0>::LevelIterator LevelIterator;
      typedef typename DiscreteFunctionType::LocalFunctionType LocalFuncType;


      GridType & grid = functionSpace_.getGrid();

      typename FunctionSpaceType::Range ret (0.0);
      typename FunctionSpaceType::Range phi (0.0);

      LevelIterator it = grid.template lbegin<0> ( level );
      LevelIterator endit = grid.template lend<0> ( level );
      FixedOrderQuad <typename FunctionSpaceType::RangeField,
          typename FunctionSpaceType::Domain , polOrd > quad ( *it );

      LocalFuncType lf = discFunc.newLocalFunction();

      for( ; it != endit ; ++it)
      {
        discFunc.localFunction( *it , lf );

        const typename FunctionSpaceType::BaseFunctionSetType & set =
          functionSpace_.getBaseFunctionSet(*it);

        for(int i=0; i<lf.numberOfDofs(); i++)
        {
          for(int qP = 0; qP < quad.nop(); qP++)
          {
            double det = (*it).geometry().integration_element(quad.point(qP));
            f.evaluate((*it).geometry().global( quad.point(qP) ), ret);
            set.eval(i,quad,qP,phi);
            lf[i] += det * quad.weight(qP) * (ret * phi);
          }
        }
      }
    }

  };


} // end namespace

#endif
