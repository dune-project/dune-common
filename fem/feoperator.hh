// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_FEOPERATOR_HH__
#define __DUNE_FEOPERATOR_HH__

#include <dune/fem/common/discreteoperator.hh>

namespace Dune {

  template <class DiscFunctionType, class MatrixType, class FEOpImp>
  class FiniteElementOperatorInterface
    : public Operator<typename DiscFunctionType::RangeFieldType,DiscFunctionType,DiscFunctionType>
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
    const typename DiscFunctionType::FunctionSpaceType & functionSpace_;
    mutable bool matrix_assembled_;

    void assemble ( ) const
    {
      std::cout << "Assemble FiniteElementOperator \n";
      typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::Traits<0>::LevelIterator LevelIterator;
      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      GridType &grid = const_cast<GridType &> (functionSpace_.getGrid());

      {
        LevelIterator it = grid.template lbegin<0>( grid.maxlevel() );
        LevelIterator endit = grid.template lend<0> ( grid.maxlevel() );
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

      {

        // eliminate the Dirichlet rows
        typedef typename GridType::Traits<0>::Entity EntityType;
        typedef typename EntityType::Traits::IntersectionIterator NeighIt;
        typedef typename NeighIt::Traits::BoundaryEntity BoundaryEntityType;

        LevelIterator it = grid.template lbegin<0>( grid.maxlevel() );
        LevelIterator endit = grid.template lend<0> ( grid.maxlevel() );
        for( it ; it != endit; ++it )
        {
          NeighIt nit = it->ibegin();
          NeighIt endnit = it->iend();
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
      matrix_assembled_ = true;
    }

    void multiplyOnTheFly( const DiscFunctionType &arg, DiscFunctionType &dest ) const
    {
      typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::Traits<0>::LevelIterator LevelIterator;
      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      GridType &grid = functionSpace_.getGrid();

      typedef typename DiscFunctionType::LocalFunctionType LocalFunctionType;
      typedef typename FunctionSpaceType::Range RangeType;
      typedef typename FunctionSpaceType::JacobianRange JacobianRange;
      typedef typename FunctionSpaceType::Domain DomainType;

      typedef typename DiscFunctionType::DofIteratorType DofIteratorType;
      int level = arg.getFunctionSpace().getGrid().maxlevel();

      DofIteratorType dest_it = dest.dbegin( level );
      DofIteratorType arg_it = arg.dbegin( level );

      dest.clear();

      LevelIterator it = grid.template lbegin<0>( grid.maxlevel() );
      LevelIterator endit = grid.template lend<0> ( grid.maxlevel() );
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

    FiniteElementOperator( const typename DiscFunctionType::FunctionSpaceType &fuspace,
                           OpMode opMode = ON_THE_FLY ) :
      functionSpace_( fuspace ),
      matrix_ (NULL),
      matrix_assembled_( false ),
      opMode_(opMode) {}

    ~FiniteElementOperator( ) {
      if ( matrix_ ) delete matrix_;
    }

    void apply( const DiscFunctionType &arg, DiscFunctionType &dest) const
    {
      if ( opMode_ == ASSEMBLED )
      {
        if ( !matrix_assembled_ )
        {
          matrix_ = newEmptyMatrix( );
          assemble();
        }
        matrix_->apply( arg, dest );
      }
      else
      {
        multiplyOnTheFly( arg, dest );
      }
    }

  public:
    template <class GridIteratorType>
    void prepareLocal(GridIteratorType &it , const DiscFunctionType &Arg,
                      DiscFunctionType & Dest )  { }

    template <class GridIteratorType>
    void finalizeLocal(GridIteratorType &it , const DiscFunctionType &Arg,
                       DiscFunctionType & Dest ) { }


    // makes local multiply on the fly
    template <class GridIteratorType>
    void applyLocal ( GridIteratorType &it ,
                      const DiscFunctionType &arg , DiscFunctionType &dest ) const
    {
      typedef typename GridType::Traits<0>::Entity EntityType;
      EntityType &en = (*it);

      typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;

      typedef typename EntityType::Traits::IntersectionIterator NeighIt;
      typedef typename NeighIt::Traits::BoundaryEntity BoundaryEntityType;

      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      GridType &grid = const_cast<GridType &> (functionSpace_.getGrid());

      typedef typename FunctionSpaceType::Range RangeType;
      typedef typename FunctionSpaceType::JacobianRange JacobianRange;
      typedef typename FunctionSpaceType::Domain DomainType;

      typedef typename DiscFunctionType::DofIteratorType DofIteratorType;
      int level = grid.maxlevel();

      DofIteratorType dest_it = dest.dbegin( level );
      DofIteratorType arg_it = const_cast<DiscFunctionType&>(arg).dbegin( level );

      const BaseFunctionSetType & baseSet = functionSpace_.getBaseFunctionSet( en );
      int numOfBaseFct = baseSet.getNumberOfBaseFunctions();

      for(int i=0; i<numOfBaseFct; i++)
      {
        int row = functionSpace_.mapToGlobal( en , i );
        for (int j=0; j<numOfBaseFct; j++ )
        {
          int col = functionSpace_.mapToGlobal( en , j );
          double val = getLocalMatrixEntry( en , i, j );

          dest_it[ row ] += arg_it[ col ] * val;
        }
      }
    } // end applyLocal

    // find Dirichlet points and erase them
    void finalizeGlobal (const DiscFunctionType &arg , DiscFunctionType &dest )
    {
#if 0
      typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::Traits<0>::LevelIterator LevelIterator;
      typedef typename GridType::Traits<0>::Entity EntityType;
      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      GridType &grid = const_cast<GridType &> (functionSpace_.getGrid());

      typedef typename DiscFunctionType::DofIteratorType DofIteratorType;
      int level = grid.maxlevel();

      DofIteratorType dest_it = dest.dbegin( level );

      LevelIterator it = grid.lbegin<0>( grid.maxlevel() );
      LevelIterator endit = grid.lend<0> ( grid.maxlevel() );
      for( ; it != endit; ++it )
      {
        typedef typename EntityType::Traits::IntersectionIterator NeighIt;
        typedef typename NeighIt::Traits::BoundaryEntity BoundaryEntityType;

        EntityType &en = (*it);

        const BaseFunctionSetType & baseSet = functionSpace_.getBaseFunctionSet( en );
        int numDof = baseSet.getNumberOfBaseFunctions();

        NeighIt nit = en.ibegin();
        NeighIt endnit = en.iend();
        for(nit; nit != endnit ; ++nit)
        {
          if(nit.boundary())
          {
            BoundaryEntityType & bEl = nit.boundaryEntity();

            if( bEl.type() == Dirichlet )
            {
              int neigh = nit.number_in_self();

              if(en.geometry().type() == triangle)
              {
                for(int i=1; i<numDof; i++)
                {
                  int col = functionSpace_.mapToGlobal(*it,(neigh+i)%numDof);
                  dest_it[col] = 0.0;
                }
              }
            }
          }
        } // end fot(nit..

      } // end for(it ..
#endif
    }

  private:
    OpMode opMode_;
  };

} // end namespace

#endif
