// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_LAPLACE_HH
#define DUNE_LAPLACE_HH

#include <dune/fem/feoperator.hh>
#include <dune/fem/feop/spmatrix.hh>

#include <dune/quadrature/quadraturerules.hh>

namespace Dune
{

  /** \brief The Laplace operator
   */
  template <class DiscFunctionType, class TensorType, int polOrd>
  class LaplaceFEOp :
    public FiniteElementOperator<DiscFunctionType,
        SparseRowMatrix<double>,
        LaplaceFEOp<DiscFunctionType,TensorType, polOrd> > {

    //! The corresponding function space type
    typedef typename DiscFunctionType::FunctionSpaceType FunctionSpaceType;

    //! The grid
    typedef typename FunctionSpaceType::GridType GridType;

    //! The grid's dimension
    enum { dim = GridType::dimension };

    //! ???
    typedef typename FunctionSpaceType::JacobianRangeType JacobianRange;

    //! ???
    typedef typename FunctionSpaceType::RangeFieldType RangeFieldType;

    //! ???
    typedef typename FiniteElementOperator<DiscFunctionType,
        SparseRowMatrix<double>,
        LaplaceFEOp<DiscFunctionType, TensorType, polOrd> >::OpMode OpMode;


    mutable JacobianRange grad;
    mutable JacobianRange othGrad;
    mutable JacobianRange mygrad[4];

  public:

    //! ???
    DiscFunctionType *stiffFunktion_;

    //! ???
    TensorType *stiffTensor_;

    //! ???
    LaplaceFEOp( const typename DiscFunctionType::FunctionSpaceType &f, OpMode opMode ) :
      FiniteElementOperator<DiscFunctionType,SparseRowMatrix<double>,LaplaceFEOp<DiscFunctionType,TensorType, polOrd> >( f, opMode ) ,
      stiffFunktion_(NULL), stiffTensor_(NULL)
    {}

    //! ???
    LaplaceFEOp( const DiscFunctionType &stiff, const typename DiscFunctionType::FunctionSpaceType &f, OpMode opMode ) :
      FiniteElementOperator<DiscFunctionType,SparseRowMatrix<double>,LaplaceFEOp<DiscFunctionType,TensorType, polOrd> >( f, opMode ) ,
      stiffFunktion_(&stiff), stiffTensor_(NULL)
    {}

    //! ???
    LaplaceFEOp( TensorType &stiff, const typename DiscFunctionType::FunctionSpaceType &f, OpMode opMode ) :
      FiniteElementOperator<DiscFunctionType,SparseRowMatrix<double>,LaplaceFEOp<DiscFunctionType,TensorType, polOrd> >( f, opMode ) ,
      stiffFunktion_(NULL), stiffTensor_(&stiff)
    {}

    //! Returns the actual matrix if it is assembled
    const SparseRowMatrix<double>* getMatrix() const {
      assert(this->matrix_);
      return this->matrix_;
    }

    //! Creates a new empty matrix
    SparseRowMatrix<double>* newEmptyMatrix( ) const
    {
      return new SparseRowMatrix<double>( this->functionSpace_.size () ,
                                          this->functionSpace_.size () ,
                                          15 * dim);
    }

    //! Prepares the local operator before calling apply()
    void prepareGlobal ( const DiscFunctionType &arg, DiscFunctionType &dest )
    {
      this->arg_  = &arg.argument();
      this->dest_ = &dest.destination();
      assert(this->arg_ != 0); assert(this->dest_ != 0);
      dest.clear();
    }

    //! ???
    template <class EntityType>
    double getLocalMatrixEntry( EntityType &entity, const int i, const int j ) const
    {
      enum { dim = GridType::dimension };
      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      const BaseFunctionSetType & baseSet = this->functionSpace_.getBaseFunctionSet( entity );
      // Get quadrature rule
      const QuadratureRule<double, dim>& quad = QuadratureRules<double, dim>::rule(entity.geometry().type(), polOrd);

      double val = 0.;
      for ( int pt=0; pt < quad.size(); pt++ )
      {
        baseSet.jacobian(i,quad[pt].position(),grad);

        // calc Jacobian inverse before volume is evaluated
        const FieldMatrix<double,dim,dim>& inv = entity.geometry().jacobianInverse(quad[pt].position());
        const double vol = entity.geometry().integrationElement(quad[pt].position());

        // multiply with transpose of jacobian inverse
        JacobianRange tmp(0);
        inv.umv(grad[0], tmp[0]);
        grad[0] = tmp[0];

        if( i != j )
        {
          baseSet.jacobian(j,quad[pt].position(),othGrad);

          // multiply with transpose of jacobian inverse
          JacobianRange tmp(0);
          inv.umv(othGrad[0], tmp[0]);
          othGrad[0] = tmp[0];

          val += ( grad[0] * othGrad[0] ) * quad[pt].weight() * vol;
        }
        else
        {
          val += ( grad[0] * grad[0] ) * quad[pt].weight() * vol;
        }
      }

      return val;
    }

    //! ???
    template < class EntityType, class MatrixType>
    void getLocalMatrix( EntityType &entity, const int matSize, MatrixType& mat) const {
      enum { dim = GridType::dimension };

      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      const BaseFunctionSetType & baseSet = this->functionSpace_.getBaseFunctionSet( entity );

      int i,j;

      for(i=0; i<matSize; i++)
        for (j=0; j<=i; j++ )
          mat[j][i]=0.0;

      // Get quadrature rule
      const QuadratureRule<double, dim>& quad = QuadratureRules<double, dim>::rule(entity.geometry().type(), polOrd);

      for ( int pt=0; pt < quad.size(); pt++ ) {

        // calc Jacobian inverse before volume is evaluated
        const FieldMatrix<double,dim,dim>& inv = entity.geometry().jacobianInverse(quad[pt].position());
        const double vol = entity.geometry().integrationElement(quad[pt].position());

        for(i=0; i<matSize; i++) {

          baseSet.jacobian(i,quad[pt].position(),mygrad[i]);

          // multiply with transpose of jacobian inverse
          JacobianRange tmp(0);
          inv.umv(mygrad[i][0], tmp[0]);
          mygrad[i][0] = tmp[0];
        }

        typename FunctionSpaceType::RangeType ret;

        if(stiffTensor_) {
          stiffTensor_->evaluate(entity.geometry().global(quad[pt].position()),ret);
          ret[0] *= quad[pt].weight();
          for(i=0; i<matSize; i++)
            for (j=0; j<=i; j++ )
              mat[j][i] += ( mygrad[i][0] * mygrad[j][0] ) * ret[0] * vol;
        }
        else{
          for(i=0; i<matSize; i++)
            for (j=0; j<=i; j++ )
              mat[j][i] += ( mygrad[i][0] * mygrad[j][0] ) * quad[pt].weight() * vol;
        }



      }

      for(i=0; i<matSize; i++)
        for (j=matSize-1; j>i; j--)
          mat[j][i] = mat[i][j];

    }

  };   // end class

} // end namespace

#endif
