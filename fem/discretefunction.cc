// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

template<typename Field, class DiscreteFunctionSpaceType>
inline void DiscreteFunction< Field, DiscreteFunctionSpaceType >::evaluate
  ( const Domain & , Range &) const
{}


template<typename Field, class DiscreteFunctionSpaceType>
template <class EvalEntityType>
inline void DiscreteFunction< Field, DiscreteFunctionSpaceType >::evaluate
  ( const EvalEntityType & en, const Domain & dom, Range & rang) {

  LocalFunctionType & localFunction ( *this  ) ;

  localFunction.init ( en );

  rang = localFunction.evaluate( dom );


}


template<typename Field, class DiscreteFunctionSpaceType>
inline void DiscreteFunction< Field, DiscreteFunctionSpaceType >::gradient
  ( const Domain &, GradientRange &) const
{}

template<typename Field, class DiscreteFunctionSpaceType>
inline void DiscreteFunction< Field, DiscreteFunctionSpaceType >::hessian
  ( const Domain &, HessianRange &) const
{}
