// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_FEOPERATOR_HH__
#define __DUNE_FEOPERATOR_HH__

#include <dune/fem/common/localoperator.hh>
#include <dune/fem/feop/spmatrix.hh>

namespace Dune {

  static const int edge[4][2] = { {0,2}, {1,3} , {0,1} , {2,3}};

  /** @defgroup FEOpInterface FEOpInterface
      @ingroup DiscreteOperator

      FEopInterface is the interface for the definition of a finite element operator.
     @{
   */
  template <class DiscFunctionType, class FEOpImp>
  class FEOpInterface
    : public Operator<typename DiscFunctionType::DomainFieldType,
          typename DiscFunctionType::RangeFieldType,DiscFunctionType,DiscFunctionType>
  {
  public:

    //! Interface method that returns the local matrix of the finite element operator on an entity
    template <class EntityType, class MatrixImp>
    void getLocalMatrix( EntityType &entity, const int matSize, MatrixImp& mat) const {
      return asImp().getLocalMatrix( entity, matSize, mat);
    }

  protected:
    // Barton-Nackman
    FEOpImp &asImp( ) { return static_cast<FEOpImp&>( *this ); }

    const FEOpImp &asImp( ) const { return static_cast<const FEOpImp&>( *this ); }
  };


  /** @} end documentation group */


  template <class DiscFunctionType, class FEOpImp>
  class FEOp : public FEOpInterface<DiscFunctionType,FEOpImp> ,
               public LocalOperatorDefault <DiscFunctionType,DiscFunctionType, typename
                   DiscFunctionType::RangeFieldType , FEOpImp  >
  {
    typedef typename SparseRowMatrix<double> MatrixType;

  public:
    enum OpMode { ON_THE_FLY, ASSEMBLED };

    FEOp( const typename DiscFunctionType::FunctionSpaceType &fuspace,
          OpMode opMode = ASSEMBLED, bool leaf = true ) :
      functionSpace_( fuspace ),  matrix_ (0), matrix_assembled_( false ),
      arg_ ( NULL ), dest_ (NULL) , opMode_(opMode), leaf_(leaf)  {};

    ~FEOp( ) {
      if ( matrix_ ) delete matrix_;
    };

  public:
    //! methods for global application of the operator

    void initialize(){
      //std::cout << "Matrix reinitialized!" << std::endl ;

      matrix_assembled_ = false;
      if ( matrix_ ) delete(matrix_);
      matrix_ = 0;
    };

    virtual void initLevel ( int level ) const
    {};

    void apply( const DiscFunctionType &arg, DiscFunctionType &dest) const
    {
      assert( opMode_ == ASSEMBLED );

      if ( !matrix_assembled_ )
      {
        matrix_ = this->newEmptyMatrix( );
        assemble();
      }

      matrix_->apply( arg, dest );
    };


  public:
    //! methods for local application of the operator

    //! isLeaf returns true if Leafiterator should be used, else false is returned
    bool isLeaf (){
      return (leaf_);
    };

    // store argument and destination
    void prepareGlobal(const DiscFunctionType &Arg, DiscFunctionType & Dest )
    {
      arg_  = &Arg.argument();
      dest_ = &Dest.destination();
      assert(arg_ != NULL); assert(dest_ != NULL);
      dest.clear();
    };

    // set argument and dest to NULL
    void finalizeGlobal()
    {
      arg_  = NULL; dest_ = NULL;
    };

    // makes local multiply on the fly
    template <class EntityType>
    void applyLocal ( EntityType &en ) const
    {
      DiscFunctionType & arg  = const_cast<DiscFunctionType &> (*arg_);
      DiscFunctionType & dest = (*dest_);

      typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;

      typedef typename EntityType::Traits::IntersectionIterator NeighIt;
      typedef typename NeighIt::Traits::BoundaryEntity BoundaryEntityType;

      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      GridType &grid = functionSpace_.getGrid();

      typedef typename FunctionSpaceType::Range RangeVecType;
      typedef typename FunctionSpaceType::JacobianRange JacobianRange;
      typedef typename FunctionSpaceType::Domain DomainVecType;

      typedef typename DiscFunctionType::DofIteratorType DofIteratorType;
      int level = grid.maxlevel();

      DofIteratorType dest_it = dest.dbegin( level );
      DofIteratorType arg_it = arg.dbegin( level );

      const BaseFunctionSetType & baseSet = functionSpace_.getBaseFunctionSet( en );
      int numOfBaseFct = baseSet.getNumberOfBaseFunctions();
      enum {maxnumOfBaseFct = 10};

      Mat<maxnumOfBaseFct,maxnumOfBaseFct , double> mat;

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
    }; // end applyLocal


    // corrects the mapping in order to take into account dirichlet boundary conditions
    template <class EntityType>
    void finalizeLocal ( EntityType &en ) const
    {
      // eliminate the Dirichlet rows and columns
      typedef typename DiscFunctionType::FunctionSpaceType FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename EntityType::Traits::IntersectionIterator NeighIt;
      typedef typename NeighIt::Traits::BoundaryEntity BoundaryEntityType;

      GridType &grid = functionSpace_.getGrid();

      DiscFunctionType & arg  = const_cast<DiscFunctionType &> (*arg_);
      DiscFunctionType & dest = (*dest_);

      typedef typename DiscFunctionType::DofIteratorType DofIteratorType;
      int level = grid.maxlevel();

      DofIteratorType dest_it = dest.dbegin( level );
      DofIteratorType arg_it = arg.dbegin( level );

      NeighIt nit = en.ibegin();
      NeighIt endnit = en.iend();

      //std::cout << "bin in finalize bei Element "<< en.index() << std::endl;
      for( ; nit != endnit ; ++nit) {

        if(nit.boundary())
        {
          BoundaryEntityType & bEl = nit.boundaryEntity();

          if( bEl.type() == Dirichlet )
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

    }; // end finalizeLocal

  protected:
    // the corresponding function_space
    const typename DiscFunctionType::FunctionSpaceType & functionSpace_;

    // the representing matrix
    mutable MatrixType *matrix_ ;

    // is matrix assembled
    mutable bool matrix_assembled_;

    // storage of argument and destination
    const DiscFunctionType * arg_;
    DiscFunctionType * dest_;

    MatrixType* newEmptyMatrix( ) const
    {
      typedef typename DiscFunctionType::FunctionSpaceType::GridType GridType;
      enum { dim = GridType::dimension };
      return new MatrixType( this->functionSpace_.size ( this->functionSpace_.getGrid().maxlevel() ) ,
                             this->functionSpace_.size ( this->functionSpace_.getGrid().maxlevel() ) ,
                             15 * (dim-1) , 0.0 );
    };

    void assemble ( ) const
    {
      typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::template Traits<0>::LevelIterator LevelIterator;
      typedef typename GridType::LeafIterator LeafIterator;


      GridType &grid = functionSpace_.getGrid();


      std::cout << "Assemble Matrix!" << std::endl ;

      {
        enum {maxnumOfBaseFct = 10};

        Mat<maxnumOfBaseFct,maxnumOfBaseFct , double> mat;

        if (leaf_) {
          LeafIterator it = grid.leafbegin( grid.maxlevel() );
          LeafIterator endit = grid.leafend ( grid.maxlevel() );

          assembleOnGrid(it, endit, mat);


          LeafIterator it2 = grid.leafbegin( grid.maxlevel() );
          LeafIterator endit2 = grid.leafend ( grid.maxlevel() );

          bndCorrectOnGrid(it2, endit2);
        }
        else{
          LevelIterator it = grid.template lbegin<0>( grid.maxlevel() );
          LevelIterator endit = grid.template lend<0> ( grid.maxlevel() );

          assembleOnGrid(it, endit, mat);


          LeafIterator it2 = grid.leafbegin( grid.maxlevel() );
          LeafIterator endit2 = grid.leafend ( grid.maxlevel() );

          bndCorrectOnGrid(it2, endit2);
        }
      }

      matrix_assembled_ = true;
    };


    template <class GridIteratorType, class MatrixImp>
    void assembleOnGrid ( GridIteratorType &it, GridIteratorType &endit, MatrixImp &mat) const
    {
      typedef typename DiscFunctionType::FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      // run through grid and add up local contributions
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
            matrix_->add( row , col , mat(i,j));
          }
        }
      }
    }


    template <class GridIteratorType>
    void bndCorrectOnGrid ( GridIteratorType &it, GridIteratorType &endit) const
    {

      // eliminate the Dirichlet rows and columns
      typedef typename DiscFunctionType::FunctionSpace FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::template Traits<0>::Entity EntityType;
      typedef typename EntityType::Traits::IntersectionIterator NeighIt;
      typedef typename NeighIt::Traits::BoundaryEntity BoundaryEntityType;

      for( ; it != endit; ++it )
      {
        NeighIt nit = it->ibegin();
        NeighIt endnit = it->iend();
        for( ; nit != endnit ; ++nit)
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
    };

  private:

    OpMode opMode_;

    bool leaf_;
  };


} // end namespace


#endif
