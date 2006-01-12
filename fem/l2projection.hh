// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_L2_PROJECTION_HH
#define DUNE_L2_PROJECTION_HH

#include <dune/quadrature/quadraturerules.hh>


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
      const typename DiscreteFunctionType::FunctionSpaceType
      & functionSpace_= discFunc.getFunctionSpace();

      discFunc.clear();

      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename FunctionSpaceType::IteratorType IteratorType;
      typedef typename DiscreteFunctionType::LocalFunctionType LocalFuncType;

      const int dim = GridType::dimension;

      typename FunctionSpaceType::RangeType ret (0.0);
      typename FunctionSpaceType::RangeType phi (0.0);

      IteratorType it    = functionSpace_.begin();
      IteratorType endit = functionSpace_.end();

      assert( it != endit );

      // Get quadrature rule
      const QuadratureRule<double, dim>& quad = QuadratureRules<double, dim>::rule(it->geometry().type(), polOrd);

      for( ; it != endit ; ++it)
      {
        LocalFuncType lf = discFunc.localFunction( *it );

        const typename FunctionSpaceType::BaseFunctionSetType & set =
          functionSpace_.getBaseFunctionSet(*it);

        for(int i=0; i<lf.numberOfDofs(); i++)
        {
          for(unsigned int qP = 0; qP < quad.size(); qP++)
          {
            double det = (*it).geometry().integrationElement(quad[qP].position());
            f.evaluate((*it).geometry().global( quad[qP].position() ), ret);
            set.eval(i,quad[qP].position(),phi);
            lf[i] += det * quad[qP].weight() * (ret * phi);
          }
        }
      }
    }
  };

} // end namespace

#endif
