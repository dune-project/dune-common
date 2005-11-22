// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:


namespace Dune {

  //- class StandardBaseFunctionSet
  template <class FunctionSpaceImp>
  int StandardBaseFunctionSet<FunctionSpaceImp>::numBaseFunctions() const
  {
    return baseFunctions_.size();
  }

  template <class FunctionSpaceImp>
  template <int diffOrd>
  void StandardBaseFunctionSet<FunctionSpaceImp>::
  evaluate(int baseFunct,
           const FieldVector<deriType, diffOrd>& diffVar,
           const DomainType& xLocal,
           RangeType& phi) const {
    baseFunctions_[baseFunct]->evaluate(diffVar, xLocal, phi);
  }

  template <class FunctionSpaceImp>
  template <int diffOrd, class QuadratureType>
  void StandardBaseFunctionSet<FunctionSpaceImp>::
  evaluate(int baseFunct,
           const FieldVector<int, diffOrd>& diffVar,
           QuadratureType& quad, int quadPoint,
           RangeType& phi) const
  {
    evaluate(baseFunct, diffVar, quad.point(quadPoint), phi);
  }

  //- class VectorialBaseFunctionSet
  template <class FunctionSpaceImp>
  int VectorialBaseFunctionSet<FunctionSpaceImp>::numBaseFunctions() const
  {
    return baseFunctions_.size()*FunctionSpaceImp::DimRange;
  }


  template <class FunctionSpaceImp>
  template <int diffOrd>
  void VectorialBaseFunctionSet<FunctionSpaceImp>::
  evaluate(int baseFunct,
           const FieldVector<int, diffOrd>& diffVar,
           const DomainType& xLocal,
           RangeType& phi) const
  {
    ScalarRangeType tmp;
    baseFunctions_[util_.containedDof(baseFunct)]->evaluate(diffVar, xLocal, tmp);

    phi *= 0.0;
    phi[util_.component(baseFunct)] = tmp[0];
  }

  template <class FunctionSpaceImp>
  template <int diffOrd, class QuadratureType>
  void VectorialBaseFunctionSet<FunctionSpaceImp>::
  evaluate(int baseFunct,
           const FieldVector<int, diffOrd>& diffVar,
           QuadratureType& quad, int quadPoint,
           RangeType& phi) const
  {
    evaluate(baseFunct, diffVar, quad.point(quadPoint), phi);
  }

  template <class FunctionSpaceImp>
  typename VectorialBaseFunctionSet<FunctionSpaceImp>::DofType
  VectorialBaseFunctionSet<FunctionSpaceImp>::
  evaluateSingle(int baseFunct,
                 const DomainType& xLocal,
                 const RangeType& factor) const
  {
    baseFunctions_[util_.containedDof(baseFunct)]->
    evaluate(diffVar0_, xLocal, tmp_);
    return factor[util_.component(baseFunct)]*tmp_[0];
  }

  template <class FunctionSpaceImp>
  template <class Entity>
  typename VectorialBaseFunctionSet<FunctionSpaceImp>::DofType
  VectorialBaseFunctionSet<FunctionSpaceImp>::
  evaluateGradientSingle(int baseFunct,
                         Entity& en,
                         const DomainType& xLocal,
                         const JacobianRangeType& factor) const
  {
    DomainType gradScaled(0.);

    for (int i = 0; i < FunctionSpaceImp::DimDomain; ++i) {
      diffVar1_[0] = i;
      baseFunctions_[util_.containedDof(baseFunct)]->
      evaluate(diffVar1_, xLocal, tmp_);
      jTmp_[i] = tmp_[0];
    }

    en.geometry().jacobianInverseTransposed(xLocal).
    umv(jTmp_, gradScaled);
    return gradScaled*factor[util_.component(baseFunct)];
  }

} // end namespace Dune
