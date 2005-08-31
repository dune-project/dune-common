// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

namespace Dune {

  //- AdaptiveDiscreteFunction
  // nothing here, everything in adaptivefunction.hh

  //- AdaptiveDiscreteFunction
  // comes later

  //- AdaptiveLocalFunction
  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::
  AdaptiveLocalFunction(const DiscreteFunctionSpaceType& spc,
                        DofStorageType& dofVec) :
    spc_(spc),
    dofVec_(dofVec),
    values_(),
    init_(false)
  {}

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::
  ~AdaptiveLocalFunction() {}

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::DofType&
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::operator[] (int num) {
    return (* (values_[num]));
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  const AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::DofType&
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::operator[] (int num) const {
    return (* (values_[num]));
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  int AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::
  numberOfDofs() const {
    return values_.size();
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  template <class EntityType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::
  evaluateLocal(EntityType& en, const DomainType& x, RangeType& ret) {
    ret *= 0.0;
    const BaseFunctionSetType& bSet = spc_.getBaseFunctionSet(en);

    for (int i = 0; i < bSet.getNumberOfBaseFunctions(); ++i) {
      bSet.evaluate(i, x, tmp_);
      for (int l = 0; l < dimRange; ++l) {
        ret[l] += (*values_[i]) * tmp_[l];
      }
    }
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  template <class EntityType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::
  jacobianLocal(EntityType& en, const DomainType& x,
                JacobianRangeType& ret) {
    enum { dim = EntityType::dimension };

    ret *= 0.0;
    const BaseFunctionSetType& bSet = spc_.getBaseFunctionSet(en);

    for (int i = 0; i < bSet.getNumberOfBaseFunctions(); ++i) {
      tmpGrad_ *= 0.0;
      bSet.jacobian(i, x, tmpGrad_);

      for (int l = 0; l < dimRange; ++l) {
        tmpGrad_[l] *= *values_[i];
        en.geometry().jacobianInverse(x).umtv(tmpGrad_[l], ret[l]);
      }
    }

  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  template <class EntityType>
  void AdaptiveLocalFuntion<DiscreteFunctionSpaceImp, DofManagerImp>::init(Entity& en) {
    int numOfDof =
      spc_.getBaseFunctionSet(en).getNumberOfBaseFunctions();
    values_.resize(numOfDof);

    for (int i = 0; i < numOfDof; ++i) {
      values_[i] = &(this->dofVec_[spc_.mapToGlobal(en, i)]);
    }

  }

  //- AdaptiveLocalFunction (Specialisation for CombinedSpace)
  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>::
  AdaptiveLocalFunction(const DiscreteFunctionSpaceType & spc,
                        DofStorageType dofVec) :
    spc_(spc),
    dofVec_(dofVec),
    values_(),
  {}

  // * more to come

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>::
  AdaptiveLocalFunction(const ThisType & other) :
    spc_(other.spc_),
    dofVec_(other.dofVec_),
    values_(),
  {
    assert(false);
    // * check correctness of copy first
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>::
  ~AdaptiveLocalFunction() {}

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>::DofType&
  AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>::
  operator[] (int num) {
    return *values[num];
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  const AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>::DofType&
  AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>::
  operator[] (int num) const {
    return *values[num];
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  int AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>::
  numberOfDofs() const {
    return values_.size();
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  template <class EntityType>
  void AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>::
  evaluateLocal(EntityType& en, const DomainType & x, RangeType& ret) {
    // * more to come (optimised version)
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  template <class EntityType>
  void AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>::
  jacobianLocal(EntityType& en, const DomainType & x, JacobianRangeType& ret) {
    // * more to come (optimised version)
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  void AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>::
  assign(const DofVectorType & dofs) {
    // * more to come
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  int AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>::
  numberOfBaseFunctions() const {}

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  template <class EntityType>
  void AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p>::
  init(EntityType& en) {
    int numOfDof =
      spc_.getBaseFunctionSet(en).getNumberOfBaseFunctions();
    values_.resize(numOfDof);

    for (int i = 0; i < numOfDof; ++i) {
      values_[i] = &(this->dofVec_[spc_.mapToGlobal(en, i)]);
    }

  }

} // end namespace Dune
