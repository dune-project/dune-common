// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __INVERSE_OPERATORS_HH__
#define __INVERSE_OPERATORS_HH__

#include "discretefunction.hh"
#include "../common/operator.hh"

namespace Dune {

  template <class DiscreteFunctionType>
  class CGInverseOperator : public Operator<DiscreteFunctionType::Traits<0>::RangeField,
                                DiscreteFunctionType,DiscreteFunctionType> {

    DiscreteFunctionType::Traits<0>::RangeField epsilon_;
    int maxIter_;
  public:

    CGInverseOperator( const Mapping<DiscreteFunctionType::Traits<0>::RangeField,
                           DiscreteFunctionType,DiscreteFunctionType>& op ) : op_(op),
                                                                              epsilon_( 1e-16 ),
                                                                              maxIter_( 1000 ) {}

    void apply( const DiscreteFunctionType& arg, DiscreteFunctionType& dest ) const {

      typedef typename DiscreteFunctionType::FunctionSpace FunctionSpaceType;
      typedef FunctionSpaceType::RangeField Field;

      int count = 0;
      Field spa=0, spn, q, quad;

      DiscreteFunctionType r( arg );
      DiscreteFunctionType p( arg );
      DiscreteFunctionType h( arg );

      op_( dest, h );

      r.assign( h );
      r -= arg;

      p.assign( arg );
      p -= h;

      spn = r.scalarProductDofs( r );

      while((spn > epsilon_) && (count++ < maxIter_)) {

        // fall ab der zweiten iteration *************

        if(count > 1) {
          const Field e = spn / spa;
          p *= e;
          p -= r;
        }

        // grund - iterations - schritt **************

        op_( p, h );

        quad = p.scalarProductDofs( h );
        q    = spn / quad;

        dest.add( p, q );
        r.add( h, q );

        spa = spn;

        // residuum neu berechnen *********************

        spn = r.scalarProductDofs( r );
        std::cerr << count << " cg-Iterationen  " << count << " Residuum:" << spn << "        \r";
      }
      std::cerr << "\n";
    }

  private:
    const Mapping<DiscreteFunctionType::Traits<0>::RangeField,DiscreteFunctionType,DiscreteFunctionType> &op_;
  };

}

#endif
