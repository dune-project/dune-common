// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_FEOPERATOR_HH
#define DUNE_FEOPERATOR_HH

#include <dune/fem/common/discreteoperator.hh>
#include <dune/fem/common/localoperator.hh>
#include <dune/common/fmatrix.hh>

namespace Dune {

  static const int edge[4][2] = { {0,2}, {1,3} , {0,1} , {2,3}};

  /** \brief Base class for local Finite Element operators
   */
  template <class DiscFunctionType, class MatrixType, class FEOpImp>
  class FiniteElementOperatorInterface
    : public Operator<typename DiscFunctionType::DomainFieldType,
          typename DiscFunctionType::RangeFieldType,DiscFunctionType,DiscFunctionType>
  {
  public:

    //! ???
    typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
    //! ???
    typedef typename FunctionSpaceType::GridType GridType;
    //! ???
    typedef typename DiscFunctionType::LocalFunctionType LocalFunctionType;
    //! ???
    typedef typename FunctionSpaceType::Range RangeVecType;
    //! ???
    typedef typename FunctionSpaceType::JacobianRange JacobianRange;
    //! ???
    typedef typename FunctionSpaceType::Domain DomainVecType;


    //! return entry i,j of the local matrix
    template<class EntityType>
    double getLocalMatrixEntry( EntityType &entity, const int i, const int j ) const {
      return asImp().getLocalMatrixEntry( entity, i, j );
    }

    //! assemble local matrix
    template <class EntityType,class MatrixImp>
    void getLocalMatrix( EntityType &entity, const int matSize, MatrixImp& mat) const
    {
      asImp().getLocalMatrix( entity, matSize, mat);
      return ;
    }

    //! ???
    MatrixType *newEmptyMatrix( ) const {
      return asImp().newEmptyMatrix( );
    }

  protected:
    // Barton-Nackman
    FEOpImp &asImp( ) { return static_cast<FEOpImp&>( *this ); }

    const FEOpImp &asImp( ) const { return static_cast<const FEOpImp&>( *this ); }
  };

  /** \brief Base class for local Finite Element operators
   * \todo Please doc me!
   */
  template <class DiscFunctionType, class MatrixType, class FEOpImp>
  class FiniteElementOperator : public FiniteElementOperatorInterface<DiscFunctionType,MatrixType,FEOpImp> ,
                                public LocalOperatorDefault <DiscFunctionType,DiscFunctionType, typename
                                    DiscFunctionType::RangeFieldType , FEOpImp  >
  {

    typedef FiniteElementOperator <DiscFunctionType,MatrixType,FEOpImp> MyType;

  protected:
    //! The corresponding function_space
    const typename DiscFunctionType::FunctionSpaceType & functionSpace_;

    //! The representing matrix
    mutable MatrixType *matrix_ ;

    //! Is matrix assembled
    mutable bool matrix_assembled_;

    //! Storage of argument
    const DiscFunctionType * arg_;

    //! Stores the destination
    DiscFunctionType * dest_;

  public:
    //! ???
    void assemble ( ) const
    {
      typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::template codim<0>::LevelIterator LevelIterator;
      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      const GridType &grid = functionSpace_.getGrid();

      {
        LevelIterator it = grid.template lbegin<0>( grid.maxlevel() );
        LevelIterator endit = grid.template lend<0> ( grid.maxlevel() );
        enum {maxnumOfBaseFct = 10};

        FieldMatrix<double,maxnumOfBaseFct,maxnumOfBaseFct> mat;

        for( ; it != endit; ++it )
        {
          const BaseFunctionSetType & baseSet = functionSpace_.getBaseFunctionSet( *it );
          const int numOfBaseFct = baseSet.getNumberOfBaseFunctions();

          // setup matrix
          getLocalMatrix( *it, numOfBaseFct, mat);

          for(int i=0; i<numOfBaseFct; i++)
          {
            int row = functionSpace_.mapToGlobal( *it , i );
            for (int j=0; j<numOfBaseFct; j++ )
            {
              int col = functionSpace_.mapToGlobal( *it , j );
              matrix_->add( row , col , mat[j][i]);
            }
          }
        }
      }

#if 0
      {
        // eliminate the Dirichlet rows and columns
        typedef typename GridType::template codim<0>::Entity EntityType;
        typedef typename GridType::template codim<0>::IntersectionIterator NeighIt;
        typedef typename NeighIt::BoundaryEntity BoundaryEntityType;

        LevelIterator it = grid.template lbegin<0>( grid.maxlevel() );
        LevelIterator endit = grid.template lend<0> ( grid.maxlevel() );
        for( ; it != endit; ++it )
        {
          NeighIt endnit = it->iend();
          for(NeighIt nit = it->ibegin() ; nit != endnit ; ++nit)
          {

            if(nit.boundary())
            {
              BoundaryEntityType & bEl = nit.boundaryEntity();

              if( functionSpace_.boundaryType( bEl.id() ) == Dirichlet )
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
                if((*it).geometry().type() == tetrahedron)
                {
                  int numDof = 4;
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
                if((*it).geometry().type() == quadrilateral)
                {
                  for(int i=0; i<2; i++)
                  {
                    // funktioniert nur fuer Dreiecke
                    // hier muss noch gearbeitet werden. Wie kommt man von den
                    // Intersections zu den localen Dof Nummern?
                    int col = functionSpace_.mapToGlobal(*it,edge[neigh][i]);
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

    //! \todo Please doc me!
    void multiplyOnTheFly( const DiscFunctionType &arg, DiscFunctionType &dest ) const
    {
      typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::template codim<0>::LevelIterator LevelIterator;
      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      const GridType &grid = functionSpace_.getGrid();

      typedef typename DiscFunctionType::LocalFunctionType LocalFunctionType;
      typedef typename FunctionSpaceType::Range RangeVecType;
      typedef typename FunctionSpaceType::JacobianRange JacobianRange;
      typedef typename FunctionSpaceType::Domain DomainVecType;

      typedef typename DiscFunctionType::DofIteratorType DofIteratorType;
      typedef typename DiscFunctionType::ConstDofIteratorType ConstDofIteratorType;

      DofIteratorType dest_it = dest.dbegin();
      ConstDofIteratorType arg_it = arg.dbegin();

      dest.clear();

      LevelIterator it = grid.template lbegin<0>( grid.maxlevel() );
      LevelIterator endit = grid.template lend<0> ( grid.maxlevel() );
      for( ; it != endit; ++it )
      {
        //prepare( *it );

        const BaseFunctionSetType & baseSet = functionSpace_.getBaseFunctionSet( *it );
        int numOfBaseFct = baseSet.getNumberOfBaseFunctions();

        for(int i=0; i<numOfBaseFct; i++)
        {
          int row = functionSpace_.mapToGlobal( *it , i );
          for (int j=0; j<numOfBaseFct; j++ )
          {
            int col = functionSpace_.mapToGlobal( *it , j );

            // scalar comes from LocalOperatorDefault, if operator is scaled,
            // i.e. with timestepsize
            double val = (this->scalar_) * getLocalMatrixEntry( *it, i, j );

            dest_it[col] += arg_it[ row ] * val;
          }
        }
      }
    }

  public:
    //! %Operator mode
    enum OpMode { ON_THE_FLY, ASSEMBLED };

    //! ???
    FiniteElementOperator( const typename DiscFunctionType::FunctionSpaceType &fuspace,
                           OpMode opMode = ON_THE_FLY ) :
      functionSpace_( fuspace ),
      matrix_ (NULL),
      matrix_assembled_( false ),
      arg_ ( NULL ), dest_ (NULL) ,
      opMode_(opMode) {}

    //! ???
    ~FiniteElementOperator( ) {
      if ( matrix_ ) delete matrix_;
    }

    //! ???
    void initialize(){
      std::cout << "Matrix reinitialized!" << std::endl ;

      matrix_assembled_ = false;
      delete(matrix_);

      matrix_ = 0;
    }

    //! ???
    void apply( const DiscFunctionType &arg, DiscFunctionType &dest) const
    {
      if ( opMode_ == ASSEMBLED )
      {
        if ( !matrix_assembled_ )
        {
          matrix_ = this->newEmptyMatrix( );
          assemble();
        }
        matrix_->apply( arg, dest );
      }
      else
      {
        multiplyOnTheFly( arg, dest );
      }
    }

    //! ???
    void assembleMatrix() const
    {
      if ( opMode_ == ASSEMBLED )
      {
        if ( !matrix_assembled_ )
        {
          matrix_ = this->newEmptyMatrix( );
          assemble();
        }
        //matrix_->apply( arg, dest );
      }
      else
      {
        //multiplyOnTheFly( arg, dest );
      }
    }

  public:
    //! Store argument and destination
    void prepareGlobal(const DiscFunctionType &Arg, DiscFunctionType & Dest )
    {
      arg_  = &Arg.argument();
      dest_ = &Dest.destination();
      assert(arg_ != NULL); assert(dest_ != NULL);
    }
    //! Set argument and dest to NULL
    void finalizeGlobal()
    {
      arg_  = NULL; dest_ = NULL;
    }

    //! Makes local multiply on the fly
    template <class EntityType>
    void applyLocal ( EntityType &en ) const
    {
      const DiscFunctionType & arg  = (*arg_);
      DiscFunctionType & dest = (*dest_);

      typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;

      typedef typename EntityType::IntersectionIterator NeighIt;
      typedef typename NeighIt::BoundaryEntity BoundaryEntityType;

      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      typedef typename FunctionSpaceType::Range RangeVecType;
      typedef typename FunctionSpaceType::JacobianRange JacobianRange;
      typedef typename FunctionSpaceType::Domain DomainVecType;

      typedef typename DiscFunctionType::DofIteratorType DofIteratorType;
      typedef typename DiscFunctionType::ConstDofIteratorType ConstDofIteratorType;

      DofIteratorType dest_it = dest.dbegin();
      ConstDofIteratorType arg_it = arg.dbegin();

      const BaseFunctionSetType & baseSet = functionSpace_.getBaseFunctionSet( en );
      int numOfBaseFct = baseSet.getNumberOfBaseFunctions();
      enum {maxnumOfBaseFct = 10};

      FieldMatrix<double,maxnumOfBaseFct,maxnumOfBaseFct> mat;

      getLocalMatrix( en, numOfBaseFct, mat);

      if(this->scalar_ == 1.)
      {
        for(int i=0; i<numOfBaseFct; i++)
        {
          int row = functionSpace_.mapToGlobal( en , i );
          for (int j=0; j<numOfBaseFct; j++ )
          {
            int col = functionSpace_.mapToGlobal( en , j );

            // scalar comes from LocalOperatorDefault, if operator is scaled,
            // i.e. with timestepsize
            dest_it[ row ] += arg_it[ col ] * mat(i,j);
          }
        }
      }
      else
      {
        for(int i=0; i<numOfBaseFct; i++)
        {
          int row = functionSpace_.mapToGlobal( en , i );
          for (int j=0; j<numOfBaseFct; j++ )
          {
            int col = functionSpace_.mapToGlobal( en , j );

            // scalar comes from LocalOperatorDefault, if operator is scaled,
            // i.e. with timestepsize
            double val = (this->scalar_) * mat(i, j );

            dest_it[ row ] += arg_it[ col ] * val;
          }
        }
      }
    } // end applyLocal


    //! Eliminates the Dirichlet rows and columns
    template <class EntityType>
    void finalizeLocal ( EntityType &en ) const
    {
      // eliminate the Dirichlet rows and columns
      typedef typename DiscFunctionType::FunctionSpaceType FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename EntityType::IntersectionIterator NeighIt;
      typedef typename NeighIt::BoundaryEntity BoundaryEntityType;

      const DiscFunctionType & arg  = (*arg_);
      DiscFunctionType & dest = (*dest_);

      typedef typename DiscFunctionType::DofIteratorType DofIteratorType;
      typedef typename DiscFunctionType::ConstDofIteratorType ConstDofIteratorType;

      DofIteratorType dest_it = dest.dbegin( );
      ConstDofIteratorType arg_it = arg.dbegin( );

      NeighIt nit = en.ibegin();
      NeighIt endnit = en.iend();

      //std::cout << "bin in finalize bei Element "<< en.index() << std::endl;
      for( ; nit != endnit ; ++nit) {

        if(nit.boundary())
        {

          BoundaryEntityType & bEl = nit.boundaryEntity();

          if( functionSpace_.boundaryType( bEl.id() ) == Dirichlet )
          {
            int neigh = nit.number_in_self();

            if(en.geometry().type() == triangle)
            {
              int numDof = 3;
              //std::cout << "Dreieck erkannt "<< en.index() << std::endl;

              for(int i=1; i<numDof; i++)
              {
                // funktioniert nur fuer Dreiecke
                // hier muss noch gearbeitet werden. Wie kommt man von den
                // Intersections zu den localen Dof Nummern?
                int col = functionSpace_.mapToGlobal(en,(neigh+i)%numDof);
                // unitRow unitCol for boundary
                //matrix_->kroneckerKill(col,col);
                dest_it[col] = arg_it[col];
              }
            }
            if(en.geometry().type() == tetrahedron)
            {
              int numDof = 4;
              for(int i=1; i<numDof; i++)
              {
                // funktioniert nur fuer Dreiecke
                // hier muss noch gearbeitet werden. Wie kommt man von den
                // Intersections zu den localen Dof Nummern?
                int col = functionSpace_.mapToGlobal(en,(neigh+i)%numDof);
                // unitRow unitCol for boundary
                //matrix_->kroneckerKill(col,col);
                dest_it[col] = arg_it[col];

              }
            }
            if(en.geometry().type() == quadrilateral)
            {
              for(int i=0; i<2; i++)
              {
                // funktioniert nur fuer Dreiecke
                // hier muss noch gearbeitet werden. Wie kommt man von den
                // Intersections zu den localen Dof Nummern?
                int col = functionSpace_.mapToGlobal(en,edge[neigh][i]);
                // unitRow unitCol for boundary
                //matrix_->kroneckerKill(col,col);
                dest_it[col] = arg_it[col];
              }
            }
          }
        }

      }

    } // end finalizeLocal


  private:
    OpMode opMode_;
  };

} // end namespace

#endif
