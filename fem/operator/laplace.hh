// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_LAPLACE_HH__
#define __DUNE_LAPLACE_HH__

#include <dune/fem/feoperator.hh>
#include <dune/fem/feop/spmatrix.hh>

#include <dune/quadrature/fixedorder.hh>

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

    //! The coordinate type
    typedef typename GridType::template Traits<0>::CoordType CoordType;

    //! ???
    typedef typename FunctionSpaceType::JacobianRange JacobianRange;

    //! ???
    typedef typename FunctionSpaceType::RangeField RangeFieldType;

    //! ???
    typedef typename FiniteElementOperator<DiscFunctionType,
        SparseRowMatrix<double>,
        LaplaceFEOp<DiscFunctionType, TensorType, polOrd> >::OpMode OpMode;


    mutable JacobianRange grad;
    mutable JacobianRange othGrad;
    mutable JacobianRange mygrad[4];

  public:

    //! ???
    FixedOrderQuad < typename FunctionSpaceType::RangeField, typename
        FunctionSpaceType::Domain , polOrd > quad;

    //! ???
    DiscFunctionType *stiffFunktion_;

    //! ???
    TensorType *stiffTensor_;

    //! ???
    LaplaceFEOp( const typename DiscFunctionType::FunctionSpace &f, OpMode opMode ) :
      FiniteElementOperator<DiscFunctionType,SparseRowMatrix<double>,LaplaceFEOp<DiscFunctionType,TensorType, polOrd> >( f, opMode ) ,
      quad ( *(f.getGrid().template lbegin<0> (0))), stiffFunktion_(NULL), stiffTensor_(NULL)
    {}

    //! ???
    LaplaceFEOp( const DiscFunctionType &stiff, const typename DiscFunctionType::FunctionSpace &f, OpMode opMode ) :
      FiniteElementOperator<DiscFunctionType,SparseRowMatrix<double>,LaplaceFEOp<DiscFunctionType,TensorType, polOrd> >( f, opMode ) ,
      quad ( *(f.getGrid().template lbegin<0> (0))), stiffFunktion_(&stiff), stiffTensor_(NULL)
    {}

    //! ???
    LaplaceFEOp( TensorType &stiff, const typename DiscFunctionType::FunctionSpace &f, OpMode opMode ) :
      FiniteElementOperator<DiscFunctionType,SparseRowMatrix<double>,LaplaceFEOp<DiscFunctionType,TensorType, polOrd> >( f, opMode ) ,
      quad ( *(f.getGrid().template lbegin<0> (0))), stiffFunktion_(NULL), stiffTensor_(&stiff)
    {}

    //! Returns the actual matrix if it is assembled
    const SparseRowMatrix<double>* getMatrix() const {
      assert(this->matrix_);
      return this->matrix_;
    }

    //! Creates a new empty matrix
    SparseRowMatrix<double>* newEmptyMatrix( ) const
    {
      return new SparseRowMatrix<double>( this->functionSpace_.size ( this->functionSpace_.getGrid().maxlevel() ) ,
                                          this->functionSpace_.size ( this->functionSpace_.getGrid().maxlevel() ) ,
                                          15 * dim);
    }

    //! ???
    void prepareGlobal ( const DiscFunctionType &arg, DiscFunctionType &dest )
    {
      this->arg_  = &arg.argument();
      this->dest_ = &dest.destination();
      assert(this->arg_ != NULL); assert(this->dest_ != NULL);
      dest.clear();
    }

    //! ???
    template <class EntityType>
    double getLocalMatrixEntry( EntityType &entity, const int i, const int j ) const
    {
      enum { dim = GridType::dimension };
      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      const BaseFunctionSetType & baseSet = this->functionSpace_.getBaseFunctionSet( entity );

      // calc Jacobian inverse before volume is evaluated
      const Mat<dim,dim,double>& inv = entity.geometry().Jacobian_inverse(quad.point(0));

      const double vol = entity.geometry().integration_element(quad.point(0));

      double val = 0.;
      for ( int pt=0; pt < quad.nop(); pt++ )
      {
        baseSet.jacobian(i,quad,pt,grad);

        // multiply with transpose of jacobian inverse
        grad(0) = inv.mult_t (grad(0));

        if( i != j )
        {
          baseSet.jacobian(j,quad,pt,othGrad);

          // multiply with transpose of jacobian inverse
          othGrad(0) = inv.mult_t(othGrad(0));

          val += ( grad(0) * othGrad(0) ) * quad.weight( pt );
        }
        else
        {
          val += ( grad(0) * grad(0) ) * quad.weight( pt );
        }
      }
      val *= vol;
      return val;
    }

    //! ???
    template < class EntityType, class MatrixType>
    void getLocalMatrix( EntityType &entity, const int matSize, MatrixType& mat) const {
      enum { dim = GridType::dimension };
      typedef typename FunctionSpaceType::BaseFunctionSetType BaseFunctionSetType;

      const BaseFunctionSetType & baseSet = this->functionSpace_.getBaseFunctionSet( entity );

      // calc Jacobian inverse before volume is evaluated
      const Mat<dim,dim,double>& inv = entity.geometry().Jacobian_inverse(quad.point(0));

      const double vol = entity.geometry().integration_element(quad.point(0));
      int i,j;

      for(i=0; i<matSize; i++)
        for (j=0; j<=i; j++ )
          mat(i,j)=0.0;

      for ( int pt=0; pt < quad.nop(); pt++ )
      {
        for(i=0; i<matSize; i++)
        {
          baseSet.jacobian(i,quad,pt,mygrad[i]);

          // multiply with transpose of jacobian inverse
          mygrad[i](0) = inv.mult_t (mygrad[i](0));
        }

        typename FunctionSpaceType::Range ret;

        if(stiffTensor_) {
          stiffTensor_->evaluate(entity.geometry().global(quad.point(pt)),ret);
          ret[0] *= quad.weight( pt );
          for(i=0; i<matSize; i++)
            for (j=0; j<=i; j++ )
              mat(i,j) += ( mygrad[i][0] * mygrad[j][0] ) * ret[0];
        }
        else{
          for(i=0; i<matSize; i++)
            for (j=0; j<=i; j++ )
              mat(i,j) += ( mygrad[i][0] * mygrad[j][0] ) * quad.weight( pt );
        }



      }

      for(i=0; i<matSize; i++)
        for (j=0; j<=i; j++ )
          mat(i,j) *= vol;

      for(i=0; i<matSize; i++)
        for (j=matSize; j>i; j--)
          mat(i,j) = mat(j,i);

      return;
    }

  };   // end class

} // end namespace

#endif
