// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_FEOPERATOR_HH__
#define __DUNE_FEOPERATOR_HH__

#include <dune/common/operator.hh>
#include <dune/common/fmatrix.hh>
#include <dune/fem/common/localoperator.hh>
#include <dune/fem/feop/spmatrix.hh>

#include <dune/grid/common/referenceelements.hh>

namespace Dune {

  /** @defgroup FEOpInterface FEOpInterface
      @ingroup DiscreteOperator


     @{
   */
  //! \brief FEopInterface is the interface for the definition of a finite element operator.
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
    FEOpImp &asImp() { return static_cast<FEOpImp&>( *this ); }

    const FEOpImp &asImp( ) const { return static_cast<const FEOpImp&>( *this ); }
  };


  /** @} end documentation group */

  //! \todo Please doc me!
  template <class DiscFunctionType, class MatrixImp, class FEOpImp>
  class FEOp : public FEOpInterface<DiscFunctionType,FEOpImp> ,
               public LocalOperatorDefault <DiscFunctionType,DiscFunctionType, typename
                   DiscFunctionType::RangeFieldType , FEOpImp  >
  {


  public:
    typedef MatrixImp MatrixType;

    enum OpMode { ON_THE_FLY, ASSEMBLED };

    enum { maxnumOfBaseFct = 100 };

    //! \todo Please doc me! HAHA Constructor, what else
    FEOp( const typename DiscFunctionType::FunctionSpaceType &fuspace,
          OpMode opMode = ASSEMBLED, bool leaf = true ) :
      functionSpace_( fuspace ),  matrix_ (0), matrix_assembled_( false ),
      arg_ ( NULL ), dest_ (NULL) , opMode_(opMode) , leaf_ (leaf) {};

    //! \todo Please doc me!
    ~FEOp( ) {
      if ( matrix_ ) delete matrix_;
    };

  public:
    //! print matrix to standart out
    void print () const
    {
      if(!this->matrix_assembled_) this->assemble();
      this->matrix_->print(std::cout);
    }

    //! return reference to Matrix for oem solvers
    MatrixType & myMatrix() const
    {
      //assert(matrix_assembled_ == true);
      if ( !this->matrix_assembled_ )
      {
        if(!this->matrix_)
          this->matrix_ = this->newEmptyMatrix( );
        this->assemble();
      }
      return (*this->matrix_);
    }


    //! methods for global application of the operator
    void initialize(){
      //std::cout << "Matrix reinitialized!" << std::endl ;

      matrix_assembled_ = false;
      if ( matrix_ ) delete(matrix_);
      matrix_ = 0;
    };


    //! \todo Please doc me!
    virtual void operator()(const DiscFunctionType &arg,
                            DiscFunctionType &dest) const
    {
      assert( this->opMode_ == ASSEMBLED );

      if ( !matrix_assembled_ )
      {
        assemble();
      }

      matrix_->apply( arg, dest );
    };


  public:
    //! isLeaf returns true if Leafiterator should be used, else false is returned
    bool isLeaf (){
      return leaf_;
    };

    //! store argument and destination
    void prepareGlobal(const DiscFunctionType &Arg, DiscFunctionType & Dest )
    {
      arg_  = &Arg.argument();
      dest_ = &Dest.destination();
      assert(arg_ != NULL); assert(dest_ != NULL);
      dest_.clear();
    };

    //! set argument and dest to NULL
    void finalizeGlobal()
    {
      arg_ = NULL; dest_ = NULL;
    };

    //! makes local multiply on the fly
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

      typedef typename FunctionSpaceType::RangeType RangeVecType;
      typedef typename FunctionSpaceType::JacobianRange JacobianRange;
      typedef typename FunctionSpaceType::DomainType DomainVecType;

      typedef typename DiscFunctionType::DofIteratorType DofIteratorType;
      typedef typename DiscFunctionType::ConstDofIteratorType ConstDofIteratorType;

      DofIteratorType dest_it = dest.dbegin();
      ConstDofIteratorType arg_it = arg.dbegin();

      const BaseFunctionSetType & baseSet = functionSpace_.getBaseFunctionSet( en );
      int numOfBaseFct = baseSet.numBaseFunctions();

      assert( numOfBaseFct <= maxnumOfBaseFct );

      FieldMatrix<double, maxnumOfBaseFct, maxnumOfBaseFct> mat;

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
            dest_it[ row ] += arg_it[ col ] * mat[i][j];
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
            double val = (this->scalar_) * mat[i][j];

            dest_it[ row ] += arg_it[ col ] * val;
          }
        }
      }
    }; // end applyLocal


    //! corrects the mapping in order to take into account dirichlet boundary conditions
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

      DofIteratorType dest_it = dest.dbegin();
      ConstDofIteratorType arg_it = arg.dbegin();

      const GeometryType t = en.geometry().type();

      NeighIt endnit = en.iend();
      for(NeighIt nit = en.ibegin(); nit != endnit ; ++nit)
      {
        if(nit.boundary())
        {
          int face = nit.numberInSelf();
          enum { dim = EntityType :: dimension };
          typedef typename EntityType :: ctype coordType;

          const int faceCodim = 1;

          if( (t == simplex) || (t == triangle) || (t == tetrahedron ) )
          {
            const BoundaryEntityType & bEl = nit.boundaryEntity();
            if( bEl.id() != 0 )
            {
              static ReferenceSimplex< coordType, dim > refElem;
              int novx = refElem.size( face, faceCodim , dim );
              assert( novx == dim );
              for(int j=0; j<novx ; j++)
              {
                // get all local numbers located on the face
                int vx  = refElem.subentity(face, faceCodim , j , dim );
                // get global dof numbers of this vertices
                int col = functionSpace_.mapToGlobal( en, vx);
                // set solution on dirichlet bnd
                dest_it[col] = arg_it[col];
              }
            }
          }
          if((t == quadrilateral) || (t == cube) || (t == hexahedron))
          {
            static ReferenceCube< coordType, dim > refElem;
            int novx = refElem.size( face, faceCodim , dim );
            for(int j=0; j<novx ; j++)
            {
              // get all local numbers located on the face
              int vx  = refElem.subentity(face, faceCodim , j , dim );
              // get global dof numbers of this vertices
              int col = functionSpace_.mapToGlobal( en, vx );
              // set solution on dirichlet bnd
              dest_it[col] = arg_it[col];
            }
          }
        }
      }
    } // end finalizeLocal

  protected:
    //! the corresponding function_space
    const typename DiscFunctionType::FunctionSpaceType & functionSpace_;

    //! the representing matrix
    mutable MatrixType *matrix_ ;

    //! is matrix assembled
    mutable bool matrix_assembled_;

    //! storage of argument and destination
    const DiscFunctionType * arg_;
    DiscFunctionType * dest_;

    //! \todo Please doc me!
    MatrixType* newEmptyMatrix( ) const
    {
      typedef typename DiscFunctionType::FunctionSpaceType::GridType GridType;
      enum { dim = GridType::dimension };
      return new MatrixType( this->functionSpace_.size ( ) ,
                             this->functionSpace_.size ( ) ,
                             15 * (dim-1));
    };

    //! \todo Please doc me!
    void assemble ( ) const
    {
      if(!this->matrix_) matrix_ = this->newEmptyMatrix( );

      typedef typename DiscFunctionType::FunctionSpaceType FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename FunctionSpaceType::IteratorType IteratorType;

      {
        FieldMatrix<double, maxnumOfBaseFct, maxnumOfBaseFct> mat;

        IteratorType it    = functionSpace_.begin();
        IteratorType endit = functionSpace_.end();

        assembleOnGrid(it, endit, mat);
      }

      {
        IteratorType it    = functionSpace_.begin();
        IteratorType endit = functionSpace_.end();
        bndCorrectOnGrid(it, endit);
      }

      matrix_assembled_ = true;
    };

    //! \todo Please doc me!
    template <class GridIteratorType, class LocalMatrixImp>
    void assembleOnGrid ( GridIteratorType &it, GridIteratorType &endit, LocalMatrixImp &mat) const
    {
      typedef typename DiscFunctionType::FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      // run through grid and add up local contributions
      for( ; it != endit; ++it )
      {
        const BaseFunctionSetType & baseSet = functionSpace_.getBaseFunctionSet( *it );
        const int numOfBaseFct = baseSet.numBaseFunctions();

        // setup matrix
        getLocalMatrix( *it, numOfBaseFct, mat);

        for(int i=0; i<numOfBaseFct; i++)
        {
          int row = functionSpace_.mapToGlobal( *it , i );
          for (int j=0; j<numOfBaseFct; j++ )
          {
            int col = functionSpace_.mapToGlobal( *it , j );
            matrix_->add( row , col , mat[i][j]);
          }
        }
      }
    }

    //! \todo Please doc me!
    template <class GridIteratorType>
    void bndCorrectOnGrid ( GridIteratorType &it, GridIteratorType &endit) const
    {
      // eliminate the Dirichlet rows and columns
      typedef typename DiscFunctionType::FunctionSpaceType FunctionSpaceType;
      typedef typename FunctionSpaceType::GridType GridType;
      typedef typename GridType::template Codim<0>::Entity EntityType;
      typedef typename EntityType::IntersectionIterator NeighIt;
      typedef typename NeighIt::BoundaryEntity BoundaryEntityType;

      for( ; it != endit; ++it )
      {
        const EntityType & en = *it;

        const GeometryType t = en.geometry().type();
        NeighIt endnit = en.iend();
        for(NeighIt nit = en.ibegin(); nit != endnit ; ++nit)
        {
          if(nit.boundary())
          {
            const int faceCodim = 1;
            int face = nit.numberInSelf();
            enum { dim = EntityType :: dimension };
            typedef typename EntityType :: ctype coordType;

            if( (t == simplex) || (t == triangle) || (t == tetrahedron ) )
            {
              const BoundaryEntityType & bEl = nit.boundaryEntity();
              if( bEl.id() != 0 )
              {
                static ReferenceSimplex< coordType, dim > refElem;
                int novx = refElem.size( face, faceCodim , dim );
                assert( novx == dim );
                for(int j=0; j<novx ; j++)
                {
                  // get all local numbers located on the face
                  int vx  = refElem.subentity(face, faceCodim , j , dim );
                  // get global dof numbers of this vertices
                  int col = functionSpace_.mapToGlobal( en, vx);
                  // set solution on dirichlet bnd

                  // unitRow unitCol for boundary
                  matrix_->kroneckerKill(col,col);
                }
              }
            }
            if((t == quadrilateral) || (t == cube) || (t == hexahedron))
            {
              static ReferenceCube< coordType, dim > refElem;
              int novx = refElem.size( face, faceCodim , dim );
              for(int j=0; j<novx ; j++)
              {
                // get all local numbers located on the face
                int vx  = refElem.subentity(face, faceCodim , j , dim );
                // get global dof numbers of this vertices
                int col = functionSpace_.mapToGlobal( en, vx);

                // unitRow unitCol for boundary
                matrix_->kroneckerKill(col,col);
              }
            }
          }
        }
      }
    };

  private:
    // operator mode
    OpMode opMode_;

    // true if LeafIterator is used, deprecated because the Iterator now
    // comes from the space
    bool leaf_;
  };


} // end namespace


#endif
