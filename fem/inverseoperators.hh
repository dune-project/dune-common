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

    double _redEps;

  public:

    CGInverseOperator( const Mapping<DiscreteFunctionType::Traits<0>::RangeField,
                           DiscreteFunctionType,DiscreteFunctionType>& op ,
                       double redEps , double absLimit , int maxIter , int verbose ) : op_(op),
                                                                                       _redEps ( redEps ), epsilon_ ( absLimit*absLimit ) ,
                                                                                       maxIter_ (maxIter ) , _verbose ( verbose ) {}

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

      while((spn > epsilon_) && (count++ < maxIter_))
      {

        std::cout << "CG Step\n";
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
    const Mapping<DiscreteFunctionType::Traits<0>::RangeField,DiscreteFunctionType,DiscreteFunctionType> &op_;
    int _verbose ;
  };


  template <class DiscreteFunctionType, class OperatorType>
  class CGInverseOp : public Operator<DiscreteFunctionType::Traits<0>::RangeField,
                          DiscreteFunctionType,DiscreteFunctionType> {

    DiscreteFunctionType::Traits<0>::RangeField epsilon_;
    int maxIter_;

    double _redEps;

  public:

    //CGInverseOp( const OperatorType & op , double  redEps , double absLimit , int maxIter , int verbose ) : op_(op),
    CGInverseOp( OperatorType & op , double redEps , double absLimit , int maxIter , int verbose ) : op_(op),
                                                                                                     _redEps ( redEps ), epsilon_ ( absLimit ) ,
                                                                                                     maxIter_ (maxIter ) , _verbose ( verbose ) {}

    void apply( const DiscreteFunctionType& arg, DiscreteFunctionType& dest ) //const
    {

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
    //const OperatorType &op_;
    OperatorType &op_;
    int _verbose ;
  };



  //********************************************************
  //
  //  with CGnew
  //
  //********************************************************
  //template <class DiscreteFunctionType>
  template <class DiscreteFunctionType, class OperatorType>
  class CGInvOp : public Operator<DiscreteFunctionType::Traits<0>::RangeField,
                      DiscreteFunctionType,DiscreteFunctionType> {

    DiscreteFunctionType::Traits<0>::RangeField epsilon_;
    int maxIter_;

    double _redEps;

  public:

    //CGInvOp( const Mapping<DiscreteFunctionType::Traits<0>::RangeField,
    //	     DiscreteFunctionType,DiscreteFunctionType>& op ,
    CGInvOp( const OperatorType & op ,
             double redEps , double absLimit , int maxIter , int verbose ) : op_(op),
                                                                             _redEps ( redEps ), epsilon_ ( absLimit ) ,
                                                                             maxIter_ (maxIter ) , _verbose ( verbose ) {}

    void apply( DiscreteFunctionType& arg, DiscreteFunctionType& dest )
    {
      int status = CGnew(op_,arg,dest,_redEps, epsilon_,maxIter_,_verbose);
    }
  private:
    //const Mapping<DiscreteFunctionType::Traits<0>::RangeField,
    //        DiscreteFunctionType,DiscreteFunctionType> &op_;
    const OperatorType & op_;

    int _verbose ;

  };

}

#endif
