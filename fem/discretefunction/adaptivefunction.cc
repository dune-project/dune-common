// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

namespace Dune {

  //- AdaptiveDiscreteFunction
  // nothing here, everything in adaptivefunction.hh

  //- AdaptiveDiscreteFunction
  // comes later

  //- AdaptiveLocalFunction
  template <class DiscreteFunctionSpaceImp>
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::
  AdaptiveLocalFunction(const DiscreteFunctionSpaceType& spc,
                        DofStorageType& dofVec) :
    spc_(spc),
    dofVec_(dofVec),
    values_(),
    init_(false)
  {}

  template <class DiscreteFunctionSpaceImp>
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::
  ~AdaptiveLocalFunction() {}

  template <class DiscreteFunctionSpaceImp>
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::DofType&
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::operator[] (int num) {
    return (* (values_[num]));
  }

  template <class DiscreteFunctionSpaceImp>
  const AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::DofType&
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::operator[] (int num) const {
    return (* (values_[num]));
  }

  template <class DiscreteFunctionSpaceImp>
  int AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::
  numberOfDofs() const {
    return values_.size();
  }

  template <class DiscreteFunctionSpaceImp>
  template <class EntityType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::
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

  template <class DiscreteFunctionSpaceImp>
  template <class EntityType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::
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

  /* probably not needed
     template <class DiscreteFunctionSpaceImp>
     AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::DofType&
     AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::dofAt(int num) {

     }

     template <class DiscreteFunctionSpaceImp>
     const AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::DofType&
     AdaptiveLocalFunction<DiscreteFunctionSpaceImp>::dofAt(int num) const {

     }
   */

  template <class DiscreteFunctionSpaceImp>
  template <class EntityType>
  void AdaptiveLocalFuntion<DiscreteFunctionSpaceImp>::init(Entity& en) {
    int numOfDof =
      spc_.getBaseFunctionSet(en).getNumberOfBasefunctions();

    if (!init_) {
      if (numOfDof > this->values_.size()) {
        this->values_.resize(numOfDof);
      }
      init_ = true;
    }

    for (int i = 0; i < numOfDof; ++i) {
      values_[i] = &(this->dofVec_[spc_.mapToGlobal(en, i)]);
    }
  }

  //- AdaptiveLocalFunction (Specialisation for CombinedSpace)
  // comes later

} // end namespace Dune
