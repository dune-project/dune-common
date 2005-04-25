// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_L2NORM_HH
#define DUNE_L2NORM_HH

#include <dune/fem/norms/norm.hh>
#include <dune/quadrature/fixedorder.hh>

namespace Dune {

  //! Class calculating the \f$ L_2 \f$ norm of a discrete function
  template <class DiscreteFunctionType>
  class L2Norm : public Norm<DiscreteFunctionType>
  {
    typedef typename DiscreteFunctionType::FunctionSpaceType FunctionSpaceType;

  public:

    //! \todo Please doc me!
    double compute (const DiscreteFunctionType &discFunc)
    {

      /** \todo Automatically choose the correct quadrature order */
      const int polOrd = 2;

      const typename DiscreteFunctionType::FunctionSpace
      & functionSpace_= discFunc.getFunctionSpace();

      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::template codim<0>::LevelIterator LevelIterator;
      typedef typename DiscreteFunctionType::LocalFunctionType LocalFuncType;


      const GridType & grid = functionSpace_.getGrid();

      typename FunctionSpaceType::Range phi (0.0);

      double sum = 0.0;
      LocalFuncType lf = (const_cast<DiscreteFunctionType*>(&discFunc))->newLocalFunction();
      int level = functionSpace_.level();
      LevelIterator endit = grid.template lend<0> ( level );
      LevelIterator it = grid.template lbegin<0> ( level );

      FixedOrderQuad < typename FunctionSpaceType::RangeField,
          typename FunctionSpaceType::Domain , polOrd > quad ( *it );

      for(; it != endit ; ++it) {

        double det = (*it).geometry().integrationElement(quad.point(0));
        /** \todo Do we really need the following const_cast?? */
        (const_cast<DiscreteFunctionType*>(&discFunc))->localFunction(*it,lf);
        for(int qP = 0; qP < quad.nop(); qP++) {

          lf.evaluate((*it),quad,qP,phi);
          sum += det * quad.weight(qP) * SQR(phi[0]);
        }

      }

      return sqrt(sum);
    }
  };

} // namespace Dune


#endif
