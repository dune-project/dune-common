// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __INVERSE_OPERATORS_HH__
#define __INVERSE_OPERATORS_HH__

#include <dune/fem/common/discretefunction.hh>
#include <dune/common/operator.hh>

namespace Dune {

  /** \brief Inversion operator using CG algorithm
   */
  template <class DiscreteFunctionType>
  class CGInverseOperator : public Operator<
                                typename DiscreteFunctionType::DomainFieldType,
                                typename DiscreteFunctionType::RangeFieldType,
                                DiscreteFunctionType,DiscreteFunctionType>
  {

    typedef Mapping<typename DiscreteFunctionType::DomainFieldType ,
        typename DiscreteFunctionType::RangeFieldType ,
        DiscreteFunctionType,DiscreteFunctionType> MappingType;

  public:
    /** \todo Please doc me! */
    CGInverseOperator( const MappingType & op,
                       double redEps,
                       double absLimit,
                       int maxIter,
                       int verbose )
      : op_(op), _redEps ( redEps ), epsilon_ ( absLimit*absLimit ) ,
        maxIter_ (maxIter ) , _verbose ( verbose ) {}

    /** \todo Please doc me! */
    virtual void operator()(const DiscreteFunctionType& arg,
                            DiscreteFunctionType& dest ) const
    {
      typedef typename DiscreteFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::RangeField Field;

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

      while((spn > epsilon_) && (count++ < maxIter_))
      {
        // fall ab der zweiten iteration *************

        if(count > 1)
        {
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
        if(_verbose > 0)
          std::cerr << count << " cg-Iterationen  " << count << " Residuum:" << spn << "        \r";
      }
      if(_verbose > 0)
        std::cerr << "\n";
    }

  private:
    // reference to operator which should be inverted
    const MappingType &op_;

    // reduce error each step by
    double _redEps;

    // minial error to reach
    typename DiscreteFunctionType::RangeFieldType epsilon_;

    // number of maximal iterations
    int maxIter_;

    // level of output
    int _verbose ;
  };

  /** \todo Please doc me! */
  template <class DiscreteFunctionType, class OperatorType>
  class CGInverseOp : public Operator<
                          typename DiscreteFunctionType::DomainFieldType,
                          typename DiscreteFunctionType::RangeFieldType,
                          DiscreteFunctionType,DiscreteFunctionType>
  {
  public:
    /** \todo Please doc me! */
    CGInverseOp( OperatorType & op , double redEps , double absLimit , int maxIter , int verbose ) :
      op_(op),
      _redEps ( redEps ),
      epsilon_ ( absLimit*absLimit ) ,
      maxIter_ (maxIter ) ,
      _verbose ( verbose ) ,
      r_(0),
      p_(0),
      h_(0) {}

    /** \todo Please doc me! */
    ~CGInverseOp()
    {
      if(p_) delete p_;
      if(r_) delete r_;
      if(h_) delete h_;
    }

    /** \todo Please doc me! */
    virtual void operator() (const DiscreteFunctionType& arg,
                             DiscreteFunctionType& dest ) const
    {
      typedef typename DiscreteFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::RangeField Field;

      int count = 0;
      Field spa=0, spn, q, quad;

      if(!p_) p_ = new DiscreteFunctionType ( arg );
      if(!r_) r_ = new DiscreteFunctionType ( arg );
      if(!h_) h_ = new DiscreteFunctionType ( arg );

      DiscreteFunctionType & r = *r_;
      DiscreteFunctionType & p = *p_;
      DiscreteFunctionType & h = *h_;

      op_.prepareGlobal(arg,dest);

      op_( dest, h );

      r.assign( h );
      r -= arg;

      p.assign( arg );
      p -= h;

      spn = r.scalarProductDofs( r );

      while((spn > epsilon_) && (count++ < maxIter_))
      {
        // fall ab der zweiten iteration *************

        if(count > 1)
        {
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
        if(_verbose > 0)
          std::cerr << count << " cg-Iterationen  " << count << " Residuum:" << spn << "        \r";
      }
      if(_verbose > 0)
        std::cerr << "\n";
      op_.finalizeGlobal();
    }

  private:
    // no const reference, we make const later
    OperatorType &op_;

    // reduce error each step by
    double _redEps;

    // minial error to reach
    typename DiscreteFunctionType::RangeFieldType epsilon_;

    // number of maximal iterations
    int maxIter_;

    // level of output
    int _verbose ;

    // tmp variables
    mutable DiscreteFunctionType* r_;
    mutable DiscreteFunctionType* p_;
    mutable DiscreteFunctionType* h_;
  };


} // end namespace Dune

#endif
