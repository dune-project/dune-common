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
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::
  operator[] (int num)
  {
    return (* (values_[num]));
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  const AdaptiveLocalFunction<
      DiscreteFunctionSpaceImp, DofManagerImp>::DofType&
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::
  operator[] (int num) const
  {
    return (* (values_[num]));
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  int AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::
  numberOfDofs() const
  {
    return values_.size();
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  template <class EntityType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::
  evaluateLocal(EntityType& en, const DomainType& x, RangeType& ret) const
  {
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
  template <class EntityType, class QuadratureType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::
  evaluate(EntityType& en,
           QuadratureType& quad,
           int quadPoint,
           RangeType& ret) const
  {
    evaluatueLocal(en, quad.point(quadPoint), ret);
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  template <class EntityType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::
  jacobianLocal(EntityType& en,
                const DomainType& x,
                JacobianRangeType& ret) const
  {
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
  template <class EntityType, class QuadratureType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::
  jacobian(EntityType& en,
           QuadratureType& quad,
           int quadPoint,
           JacobianRangeType& ret) const
  {
    jacobianLocal(en, quad.point(quadPoint), ret);
  }

  template <class DiscreteFunctionSpaceImp, class DofManagerImp>
  template <class EntityType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp, DofManagerImp>::
  init(EntityType& en)
  {
    int numOfDof =
      spc_.getBaseFunctionSet(en).getNumberOfBaseFunctions();
    values_.resize(numOfDof);

    for (int i = 0; i < numOfDof; ++i) {
      values_[i] = &(this->dofVec_[spc_.mapToGlobal(en, i)]);
    }
  }

  //- AdaptiveLocalFunction (Specialisation for CombinedSpace)
  template <class ContainedFunctionSpaceImp, int N,
      DofStoragePolicy p, class DofManagerImp>
  AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>::
  AdaptiveLocalFunction(const DiscreteFunctionSpaceType& spc,
                        DofStorageType& dofVec) :
    spc_(spc),
    dofVec_(dofVec),
    values_(),
    tmp_(0.0),
    tmpGrad_(0.0)
  {}

  // * more to come

  template <class ContainedFunctionSpaceImp, int N,
      DofStoragePolicy p, class DofManagerImp>
  AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>::
  AdaptiveLocalFunction(const ThisType& other) :
    spc_(other.spc_),
    dofVec_(other.dofVec_),
    values_(),
    tmp_(0.0),
    tmpGrad_(0.0)
  {
    assert(false);
    // * check correctness of copy first
  }

  template <class ContainedFunctionSpaceImp, int N,
      DofStoragePolicy p, class DofManagerImp>
  AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>::
  ~AdaptiveLocalFunction() {}

  template <class ContainedFunctionSpaceImp, int N,
      DofStoragePolicy p, class DofManagerImp>
  AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>::DofType&
  AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>::
  operator[] (int num)
  {
    return *values_[num];
  }

  template <class ContainedFunctionSpaceImp, int N,
      DofStoragePolicy p, class DofManagerImp>
  const AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>::DofType&
  AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>::
  operator[] (int num) const
  {
    return *values_[num];
  }

  template <class ContainedFunctionSpaceImp, int N,
      DofStoragePolicy p, class DofManagerImp>
  int AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>::
  numberOfDofs() const {
    return values_.size();
  }

  template <class ContainedFunctionSpaceImp, int N,
      DofStoragePolicy p, class DofManagerImp>
  template <class EntityType>
  void AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>::
  evaluateLocal(EntityType& en,
                const DomainType& x,
                RangeType& ret) const
  {
    assert(false);
  }

  template <class ContainedFunctionSpaceImp, int N,
      DofStoragePolicy p, class DofManagerImp>
  template <class EntityType, class QuadratureType>
  void AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>::
  evaluate(EntityType& en,
           QuadratureType& quad,
           int quadPoint,
           RangeType& ret) const
  {
    evaluateLocal(en, quad.point(quadPoint), ret);
  }

  template <class ContainedFunctionSpaceImp, int N,
      DofStoragePolicy p, class DofManagerImp>
  template <class EntityType>
  void AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>::
  jacobianLocal(EntityType& en,
                const DomainType& x,
                JacobianRangeType& ret) const
  {
    assert(false); // * more to come (optimised version)
  }

  template <class ContainedFunctionSpaceImp, int N,
      DofStoragePolicy p, class DofManagerImp>
  template<class EntityType, class QuadratureType>
  void AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>::
  jacobian(EntityType& en,
           QuadratureType& quad,
           int quadPoint,
           JacobianRangeType& ret) const
  {
    jacobianLocal(en, quad.point(quadPoint), ret);
  }

  template <class ContainedFunctionSpaceImp, int N,
      DofStoragePolicy p, class DofManagerImp>
  void AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>::
  assign(const DofVectorType& dofs) {
    // * more to come
  }

  template <class ContainedFunctionSpaceImp, int N,
      DofStoragePolicy p, class DofManagerImp>
  int AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>::
  numberOfBaseFunctions() const {}

  template <class ContainedFunctionSpaceImp, int N,
      DofStoragePolicy p, class DofManagerImp>
  template <class EntityType>
  void AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p>, DofManagerImp>::
  init(EntityType& en) {
    int numOfDof =
      spc_.getBaseFunctionSet(en).getNumberOfBaseFunctions();
    values_.resize(numOfDof);

    for (int i = 0; i < numOfDof; ++i) {
      values_[i] = &(this->dofVec_[spc_.mapToGlobal(en, i)]);
    }

  }


} // end namespace Dune
