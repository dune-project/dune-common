// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_FEOPERATOR_HH__
#define __DUNE_FEOPERATOR_HH__

#include <dune/common/operator.hh>
#include <dune/fem/basefunctions.hh>
#include <dune/fem/dofiterator.hh>
#include <dune/fem/discretefunction.hh>

namespace Dune {

  template <class DiscFunctionType, class MatrixType, class FEOpImp>
  class FiniteElementOperatorInterface : public Operator<DiscFunctionType::Traits<0>::RangeField,DiscFunctionType,DiscFunctionType> {
  public:
    typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
    typedef typename FunctionSpaceType::GridType GridType;
    typedef typename DiscFunctionType::LocalFunctionType LocalFunctionType;
    typedef typename FunctionSpaceType::Range Range;
    typedef typename FunctionSpaceType::GradientRange GradientRange;
    typedef typename FunctionSpaceType::Domain Domain;
    typedef typename GridType::Traits<0>::Entity EntityType;

    void prepare( EntityType &entity ) const {
      asImp().prepare( entity );
    }

    double getLocalMatrixEntry( EntityType &entity, const int i, const int j ) const {
      return asImp().getLocalMatrixEntry( entity, i, j );
    }

    MatrixType *newEmptyMatrix( ) const {
      return asImp().newEmptyMatrix( );
    }

  protected:
    // Barton-Nackman
    FEOpImp &asImp( ) { return static_cast<FEOpImp&>( *this ); }

    const FEOpImp &asImp( ) const { return static_cast<const FEOpImp&>( *this ); }
  };

  template <class DiscFunctionType, class MatrixType, class FEOpImp>
  class FiniteElementOperator : public FiniteElementOperatorInterface<DiscFunctionType,MatrixType,FEOpImp> {
  protected:
    mutable MatrixType *matrix_;
    const DiscFunctionType::FunctionSpaceType & functionSpace_;
    mutable bool matrix_assembled;

    void assemble ( ) const {
      typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::Traits<0>::LevelIterator LevelIterator;
      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      GridType &grid = const_cast<GridType &> (functionSpace_.getGrid());

      typedef typename DiscFunctionType::LocalFunctionType LocalFunctionType;
      typedef typename FunctionSpaceType::Range Range;
      typedef typename FunctionSpaceType::GradientRange GradientRange;
      typedef typename FunctionSpaceType::Domain Domain;

      LevelIterator it = grid.lbegin<0>( grid.maxlevel() );
      LevelIterator endit = grid.lend<0> ( grid.maxlevel() );
      for( ; it != endit; ++it ) {
        prepare( *it );

        const BaseFunctionSetType & baseSet = functionSpace_.getBaseFunctionSet( *it );
        int numOfBaseFct = baseSet.getNumberOfBaseFunctions();

        // setup matrix
        for(int i=0; i<numOfBaseFct; i++) {
          int row = functionSpace_.mapToGlobal( *it , i );
          for (int j=0; j<numOfBaseFct; j++ ) {
            int col = functionSpace_.mapToGlobal( *it , j );

            double val = getLocalMatrixEntry( *it, i, j );

            matrix_->add( row , col , val);
          }
        }
      }
      matrix_assembled = true;
    }

    void multiplyOnTheFly( const DiscFunctionType &arg, DiscFunctionType &dest ) const {
      typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::Traits<0>::LevelIterator LevelIterator;
      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      GridType &grid = const_cast<GridType &> (functionSpace_.getGrid());

      typedef typename DiscFunctionType::LocalFunctionType LocalFunctionType;
      typedef typename FunctionSpaceType::Range Range;
      typedef typename FunctionSpaceType::GradientRange GradientRange;
      typedef typename FunctionSpaceType::Domain Domain;

      typedef typename DiscFunctionType::GlobalDofIteratorType DofIteratorType;
      int level = arg.getFunctionSpace().getGrid().maxlevel();

      DofIteratorType dest_it = dest.dbegin( level );
      DofIteratorType arg_it = const_cast<DiscFunctionType&>(arg).dbegin( level );

      dest.clear( );

      LevelIterator it = grid.lbegin<0>( grid.maxlevel() );
      LevelIterator endit = grid.lend<0> ( grid.maxlevel() );
      for( ; it != endit; ++it ) {
        prepare( *it );

        const BaseFunctionSetType & baseSet = functionSpace_.getBaseFunctionSet( *it );
        int numOfBaseFct = baseSet.getNumberOfBaseFunctions();

        for(int i=0; i<numOfBaseFct; i++) {
          int row = functionSpace_.mapToGlobal( *it , i );
          for (int j=0; j<numOfBaseFct; j++ ) {
            int col = functionSpace_.mapToGlobal( *it , j );

            double val = getLocalMatrixEntry( *it, i, j );

            arg_it.reset();
            arg_it.operator++( col );
            dest_it.reset();
            dest_it.operator++( row );
            (*dest_it) += (*arg_it) * val;
          }
        }
      }
    }
  public:

    enum OpMode { ON_THE_FLY, ASSEMBLED };

    FiniteElementOperator( const DiscFunctionType::FunctionSpaceType &fuspace,
                           OpMode opMode = ASSEMBLED ) :
      functionSpace_( fuspace ),
      matrix_ (NULL),
      matrix_assembled( false ),
      opMode_(opMode) {}

    ~FiniteElementOperator( ) {
      if ( matrix_ ) delete matrix_;
    }

    void apply( const DiscFunctionType &arg, DiscFunctionType &dest) const {
      if ( opMode_ == ASSEMBLED ) {
        if ( !matrix_assembled ) {
          matrix_ = newEmptyMatrix( );
          assemble();
        }
        matrix_->apply( arg, dest );
      } else {
        multiplyOnTheFly( arg, dest );
      }
    }
  private:
    OpMode opMode_;
  };

} // end namespace

#endif
