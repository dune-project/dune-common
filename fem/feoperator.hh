// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_FEOPERATOR_HH__
#define __DUNE_FEOPERATOR_HH__

#include "discreteoperator.hh"
#include <dune/fem/basefunctions.hh>
#include <dune/fem/dofiterator.hh>
#include <dune/fem/discretefunction.hh>

namespace Dune {

  template <class DiscFunctionType, class MatrixType, class FEOpImp>
  class FiniteElementOperatorInterface
    : public Operator<DiscFunctionType::Traits<0>::RangeField,DiscFunctionType,DiscFunctionType>
      //: public DiscreteOperator<DiscFunctionType , FiniteElementOperatorInterface
      //<DiscFunctionType , MatrixType ,FEOpImp > >
  {
  public:
    typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
    typedef typename FunctionSpaceType::GridType GridType;
    typedef typename DiscFunctionType::LocalFunctionType LocalFunctionType;
    typedef typename FunctionSpaceType::Range RangeType;
    typedef typename FunctionSpaceType::JacobianRange JacobianRange;
    typedef typename FunctionSpaceType::Domain DomainType;
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
  class FiniteElementOperator : public FiniteElementOperatorInterface<DiscFunctionType,MatrixType,FEOpImp>
  {

    typedef FiniteElementOperator <DiscFunctionType,MatrixType,FEOpImp> MyType;

  protected:
    mutable MatrixType *matrix_;
    const DiscFunctionType::FunctionSpaceType & functionSpace_;
    mutable bool matrix_assembled_;

    void assemble ( ) const
    {
      std::cout << "Assemble FiniteElementOperator \n";
      typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::Traits<0>::LevelIterator LevelIterator;
      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      GridType &grid = const_cast<GridType &> (functionSpace_.getGrid());

      //typedef typename DiscFunctionType::LocalFunctionType LocalFunctionType;
      //typedef typename FunctionSpaceType::JacobianRange JacobianRange;
      {
        LevelIterator it = grid.lbegin<0>( grid.maxlevel() );
        LevelIterator endit = grid.lend<0> ( grid.maxlevel() );
        for( it ; it != endit; ++it )
        {
          prepare( *it );

          const BaseFunctionSetType & baseSet = functionSpace_.getBaseFunctionSet( *it );
          int numOfBaseFct = baseSet.getNumberOfBaseFunctions();

          // setup matrix
          for(int i=0; i<numOfBaseFct; i++)
          {
            int row = functionSpace_.mapToGlobal( *it , i );
            for (int j=0; j<numOfBaseFct; j++ )
            {
              int col = functionSpace_.mapToGlobal( *it , j );
              double val = getLocalMatrixEntry( *it, i, j );

              matrix_->add( row , col , val);
            }
          }
        }
      }

#if 0
      {

        // eliminate the Dirichlet rows
        typedef typename GridType::Traits<0>::Entity EntityType;
        typedef typename EntityType::Traits::NeighborIterator NeighIt;
        typedef typename NeighIt::Traits::BoundaryEntity BoundaryEntityType;

        LevelIterator it = grid.lbegin<0>( grid.maxlevel() );
        LevelIterator endit = grid.lend<0> ( grid.maxlevel() );
        for( it ; it != endit; ++it )
        {
          NeighIt nit = it->nbegin();
          NeighIt endnit = it->nend();
          for(nit; nit != endnit ; ++nit)
          {

            if(nit.boundary())
            {
              BoundaryEntityType & bEl = nit.boundaryEntity();

              if( bEl.type() == Dirichlet )
              {
                int neigh = nit.number_in_self();

                if((*it).geometry().type() == triangle)
                {
                  int numDof = 3;
                  for(int i=1; i<numDof; i++)
                  {
                    // funktioniert nur fuer Dreiecke
                    // hier muss noch gearbeitet werden. Wie kommt man von den
                    // Intersections zu den localen Dof Nummern?
                    int col = functionSpace_.mapToGlobal(*it,(neigh+i)%numDof);
                    // unitRow unitCol for boundary
                    matrix_->kroneckerKill(col,col);
                  }
                }
              }
            }

          }
        }
      }
#endif
      matrix_assembled_ = true;
    }

    template <class EntityType>
    void multiOnTheFly( EntityType &en,
                        const DiscFunctionType &arg, DiscFunctionType &dest ) const
    {
      //std::cout << "Mulitipla on the fla local\n";
      typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::Traits<0>::LevelIterator LevelIterator;
      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      GridType &grid = const_cast<GridType &> (functionSpace_.getGrid());

      typedef typename DiscFunctionType::LocalFunctionType LocalFunctionType;
      typedef typename FunctionSpaceType::Range RangeType;
      typedef typename FunctionSpaceType::JacobianRange JacobianRange;
      typedef typename FunctionSpaceType::Domain DomainType;

      typedef typename DiscFunctionType::DofIteratorType DofIteratorType;
      int level = arg.getFunctionSpace().getGrid().maxlevel();

      DofIteratorType dest_it = dest.dbegin( level );
      DofIteratorType arg_it = const_cast<DiscFunctionType&>(arg).dbegin( level );

      dest.clear();

      //prepare( *it );

      const BaseFunctionSetType & baseSet = functionSpace_.getBaseFunctionSet( en );
      int numOfBaseFct = baseSet.getNumberOfBaseFunctions();

      for(int i=0; i<numOfBaseFct; i++)
      {
        int row = functionSpace_.mapToGlobal( en , i );
        for (int j=0; j<numOfBaseFct; j++ )
        {
          int col = functionSpace_.mapToGlobal( en , j );

          double val = getLocalMatrixEntry( en , i, j );

          dest_it[col] += arg_it[ row ] * val;
        }
      }
    }

    void multiplyOnTheFly( const DiscFunctionType &arg, DiscFunctionType &dest ) const
    {
      //std::cout << "Mulitipla on the fla\n";
      typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::Traits<0>::LevelIterator LevelIterator;
      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      GridType &grid = const_cast<GridType &> (functionSpace_.getGrid());

      typedef typename DiscFunctionType::LocalFunctionType LocalFunctionType;
      typedef typename FunctionSpaceType::Range RangeType;
      typedef typename FunctionSpaceType::JacobianRange JacobianRange;
      typedef typename FunctionSpaceType::Domain DomainType;

      typedef typename DiscFunctionType::DofIteratorType DofIteratorType;
      int level = arg.getFunctionSpace().getGrid().maxlevel();

      DofIteratorType dest_it = dest.dbegin( level );
      DofIteratorType arg_it = const_cast<DiscFunctionType&>(arg).dbegin( level );

      dest.clear();

      LevelIterator it = grid.lbegin<0>( grid.maxlevel() );
      LevelIterator endit = grid.lend<0> ( grid.maxlevel() );
      for( ; it != endit; ++it )
      {
        prepare( *it );

        const BaseFunctionSetType & baseSet = functionSpace_.getBaseFunctionSet( *it );
        int numOfBaseFct = baseSet.getNumberOfBaseFunctions();

        for(int i=0; i<numOfBaseFct; i++)
        {
          int row = functionSpace_.mapToGlobal( *it , i );
          for (int j=0; j<numOfBaseFct; j++ )
          {
            int col = functionSpace_.mapToGlobal( *it , j );

            double val = getLocalMatrixEntry( *it, i, j );

            dest_it[col] += arg_it[ row ] * val;
          }
        }
      }
    }

  public:

    enum OpMode { ON_THE_FLY, ASSEMBLED };

    FiniteElementOperator( const DiscFunctionType::FunctionSpaceType &fuspace,
                           OpMode opMode = ON_THE_FLY ) :
      functionSpace_( fuspace ),
      matrix_ (NULL),
      matrix_assembled_( false ),
      opMode_(opMode) {}

    ~FiniteElementOperator( ) {
      if ( matrix_ ) delete matrix_;
    }

    void apply( const DiscFunctionType &arg, DiscFunctionType &dest) const {
      if ( opMode_ == ASSEMBLED ) {
        if ( !matrix_assembled_ ) {
          matrix_ = newEmptyMatrix( );
          assemble();
        }
        std::cout << "Matrix apply \n";
        matrix_->apply( arg, dest );
      } else {
        multiplyOnTheFly( arg, dest );
      }
    }

  public:
    template <class EntityType>
    void applyLocal ( EntityType &en , const DiscFunctionType &Arg , DiscFunctionType &Dest ) //const
    {
      std::cout << "FEOperator::applyLocal \n";
      //multiOnTheFly(en , Arg, Dest );
    }

  private:
    OpMode opMode_;
  };

} // end namespace

#endif
