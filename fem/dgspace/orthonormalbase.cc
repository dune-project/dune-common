// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <string>

#include <dune/fem/dgspace/orthonormalbase.hh>

template <class FunctionSpaceType>
OrthonormalBaseFunctionSet<FunctionSpaceType >::
OrthonormalBaseFunctionSet( FunctionSpaceType & fuspace,
                            int polOrder )
  :
    BaseFunctionSetDefault
    <FunctionSpaceType,OrthonormalBaseFunctionSet <FunctionSpaceType > >
      ( fuspace ),
    polOrder_(polOrder),
    type_(Dune::line)
{
  assert(DimRange == 1);
  switch (DimDomain)
  {
  case 2 :
    numOfBaseFct_ = (polOrder + 2) * (polOrder + 1) / 2;
    break;
  case 3 :
    numOfBaseFct_ = ((polOrder+1)*(polOrder+2)*(2*polOrder+3)/6 +
                     (polOrder+1)*(polOrder+2)/2)/2;
    break;
  default :
    DUNE_THROW(NotImplemented, "OrthonormalBaseFunctionSet only supports 2D and 3D Domain");
  }
}

template <class FunctionSpaceType>
void OrthonormalBaseFunctionSet<FunctionSpaceType >::
real_evaluate( int baseFunct, const FieldVector<deriType, 0> &diffVariable,
               const Domain & x,  Range & phi ) const
{
  switch(type_)
  {
  case triangle :
    phi[0] = eval_triangle_2d (baseFunct, x); break;
  case quadrilateral :
    phi[0] = eval_quadrilateral_2d (baseFunct, x); break;
  case tetrahedron :
    phi[0] = eval_tetrahedron_3d (baseFunct, x); break;
  case pyramid :
    phi[0] = eval_pyramid_3d (baseFunct, x); break;
  case prism :
    phi[0] = eval_prism_3d (baseFunct, x); break;
  case hexahedron :
    phi[0] = eval_hexahedron_3d (baseFunct, x); break;
  default :
    DUNE_THROW(NotImplemented, "GeometryType not suppoerted by OrthonormalBaseFunctionSet");
  }
}

template <class FunctionSpaceType>
void OrthonormalBaseFunctionSet<FunctionSpaceType >::
jacobian ( int baseFunct, const Domain & x, JacobianRange & grad ) const
{
  switch(type_)
  {
  case triangle :
    return grad_triangle_2d (baseFunct, x, grad);
  case quadrilateral :
    return grad_quadrilateral_2d (baseFunct, x, grad);
  case tetrahedron :
    return grad_tetrahedron_3d (baseFunct, x, grad);
  case pyramid :
    return grad_pyramid_3d (baseFunct, x, grad);
  case prism :
    return grad_prism_3d (baseFunct, x, grad);
  case hexahedron :
    return grad_hexahedron_3d (baseFunct, x, grad);
  default :
    DUNE_THROW(NotImplemented, "GeometryType not suppoerted by OrthonormalBaseFunctionSet");
  }
}

template <class FunctionSpaceType>
void OrthonormalBaseFunctionSet<FunctionSpaceType >::
real_evaluate( int baseFunct, const FieldVector<deriType, 1> &diffVariable,
               const Domain & x,  Range & phi ) const
{
  JacobianRange grad;
  jacobian(baseFunct, x, grad);
  phi[0] = grad[diffVariable[0]];
}

template <class FunctionSpaceType> template <int diffOrd>
void OrthonormalBaseFunctionSet<FunctionSpaceType >::
evaluate( int baseFunct, const FieldVector<deriType, diffOrd> &diffVariable,
          const Domain & x,  Range & phi ) const
{
  assert(baseFunct < numOfBaseFct_);
  real_evaluate(baseFunct, diffVariable, x, phi);
}

template <class FunctionSpaceType> template <int diffOrd, class QuadratureType>
void OrthonormalBaseFunctionSet<FunctionSpaceType >::
evaluate( int baseFunct, const FieldVector<deriType, diffOrd> &diffVariable,
          QuadratureType & quad, int quadPoint, Range & phi ) const
{
  Domain x = quad.point(quadPoint);
  real_evaluate(baseFunct, diffVariable, x, phi);
}
