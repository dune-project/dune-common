// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_L2_PROJECTION_HH
#define DUNE_L2_PROJECTION_HH

#include <dune/quadrature/fixedorder.hh>


namespace Dune
{

  /** \brief Projects an analytical function orthogonally (in the L2-sense)
      onto a FE-space
   */
  template <class DiscreteFunctionType>
  class L2Projection
  {
    typedef typename DiscreteFunctionType::FunctionSpaceType FunctionSpaceType;

  public:
    /** \brief Do the projection
     *
     * \tparam polOrd The order of the quadrature scheme used
     * \tparam FunctionType The type of the class implementing the analytical function
     */
    template <int polOrd, class FunctionType>
    void project (FunctionType &f, DiscreteFunctionType &discFunc)
    {
      const typename DiscreteFunctionType::FunctionSpace
      & functionSpace_= discFunc.getFunctionSpace();

      int level = functionSpace_.level();

      discFunc.clear();

      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::template codim<0>::LevelIterator LevelIterator;
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
            double det = (*it).geometry().integrationElement(quad.point(qP));
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
