// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

namespace Dune {

  //- AdaptiveDiscreteFunction
  // nothing here, everything in adaptivefunction.hh


  //- AdaptiveLocalFunction
  template <class DiscreteFunctionSpaceImp>
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp >::
  AdaptiveLocalFunction(const DiscreteFunctionSpaceType& spc,
                        DofStorageType& dofVec) :
    spc_(spc),
    dofVec_(dofVec),
    values_(),
    tmp_(0.0),
    tmpGrad_(0.0),
    init_(false)
  {}

  template <class DiscreteFunctionSpaceImp>
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp >::
  AdaptiveLocalFunction(const ThisType& other) :
    spc_(other.spc_),
    dofVec_(other.dofVec_),
    values_(),
    tmp_(0.0),
    tmpGrad_(0.0),
    init_(false)
  {}

  template <class DiscreteFunctionSpaceImp>
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp >::
  ~AdaptiveLocalFunction() {}

  template <class DiscreteFunctionSpaceImp>
  typename AdaptiveLocalFunction<DiscreteFunctionSpaceImp >::DofType&
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp >::
  operator[] (int num)
  {
    assert(init_);
    assert(num >= 0 && num < numDofs());
    return (* (values_[num]));
  }

  template <class DiscreteFunctionSpaceImp>
  const typename AdaptiveLocalFunction<DiscreteFunctionSpaceImp >::DofType&
  AdaptiveLocalFunction<DiscreteFunctionSpaceImp >::
  operator[] (int num) const
  {
    assert(init_);
    assert(num >= 0 && num < numDofs());
    return (* (values_[num]));
  }

  template <class DiscreteFunctionSpaceImp>
  int AdaptiveLocalFunction<DiscreteFunctionSpaceImp >::
  numberOfDofs() const
  {
    return values_.size();
  }

  template <class DiscreteFunctionSpaceImp>
  int AdaptiveLocalFunction<DiscreteFunctionSpaceImp >::
  numDofs() const
  {
    return values_.size();
  }

  template <class DiscreteFunctionSpaceImp>
  template <class EntityType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp >::
  evaluate(EntityType& en, const DomainType& x, RangeType& ret) const
  {
    evaluateLocal( en, en.geometry().local(x), ret);
  }

  template <class DiscreteFunctionSpaceImp>
  template <class EntityType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp >::
  evaluateLocal(EntityType& en, const DomainType& x, RangeType& ret) const
  {
    assert(init_);
    assert(en.geometry().checkInside(x));
    ret *= 0.0;
    const BaseFunctionSetType& bSet = spc_.getBaseFunctionSet(en);

    for (int i = 0; i < bSet.numBaseFunctions(); ++i) {
      bSet.eval(i, x, tmp_);
      for (int l = 0; l < dimRange; ++l) {
        ret[l] += (*values_[i]) * tmp_[l];
      }
    }
  }

  template <class DiscreteFunctionSpaceImp>
  template <class EntityType, class QuadratureType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp >::
  evaluate(EntityType& en,
           QuadratureType& quad,
           int quadPoint,
           RangeType& ret) const
  {
    evaluateLocal(en, quad.point(quadPoint), ret);
  }

  template <class DiscreteFunctionSpaceImp>
  template <class EntityType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp >::
  jacobianLocal(EntityType& en,
                const DomainType& x,
                JacobianRangeType& ret) const
  {
    assert(init_);
    enum { dim = EntityType::dimension };

    ret *= 0.0;
    const BaseFunctionSetType& bSet = spc_.getBaseFunctionSet(en);

    for (int i = 0; i < bSet.numBaseFunctions(); ++i) {
      tmpGrad_ *= 0.0;
      bSet.jacobian(i, x, tmpGrad_);

      for (int l = 0; l < dimRange; ++l) {
        tmpGrad_[l] *= *values_[i];
        // * umtv or umv?
        en.geometry().jacobianInverseTransposed(x).umv(tmpGrad_[l], ret[l]);
      }
    }
  }

  template <class DiscreteFunctionSpaceImp>
  template <class EntityType, class QuadratureType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp >::
  jacobian(EntityType& en,
           QuadratureType& quad,
           int quadPoint,
           JacobianRangeType& ret) const
  {
    jacobianLocal(en, quad.point(quadPoint), ret);
  }

  template <class DiscreteFunctionSpaceImp>
  template <class EntityType>
  void AdaptiveLocalFunction<DiscreteFunctionSpaceImp >::
  init(const EntityType& en)
  {
    int numOfDof =
      spc_.getBaseFunctionSet(en).numBaseFunctions();
    values_.resize(numOfDof);

    for (int i = 0; i < numOfDof; ++i) {
      values_[i] = &(this->dofVec_[spc_.mapToGlobal(en, i)]);
    }

    init_ = true;
  }
  //- AdaptiveDiscreteFunction (specialisation)
  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  AdaptiveDiscreteFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  ~AdaptiveDiscreteFunction()
  {
    for (unsigned int i = 0; i < subSpaces_.size(); ++i) {
      delete subSpaces_[i];
      subSpaces_[i] = 0;
    }
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  typename AdaptiveDiscreteFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p> >::SubDiscreteFunctionType
  AdaptiveDiscreteFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  subFunction(int component)
  {
    SubSpaceType* subSpace = new SubSpaceType(this->space(), component);
    subSpaces_.push_back(subSpace);


    return SubDiscreteFunctionType(std::string("Subfunction of ")+this->name(),
                                   *subSpace,
                                   this->dofStorage());
  }

  //- AdaptiveLocalFunction (Specialisation for CombinedSpace)
  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  AdaptiveLocalFunction(const DiscreteFunctionSpaceType& spc,
                        DofStorageType& dofVec) :
    spc_(spc),
    dofVec_(dofVec),
    values_(),
    cTmp_(0.0),
    cTmpGradRef_(0.0),
    cTmpGradReal_(0.0),
    tmp_(0.0)
  {}

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  AdaptiveLocalFunction(const ThisType& other) :
    spc_(other.spc_),
    dofVec_(other.dofVec_),
    values_(),
    cTmp_(0.0),
    cTmpGradRef_(0.0),
    cTmpGradReal_(0.0),
    tmp_(0.0)
  {}

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  ~AdaptiveLocalFunction() {}

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  typename AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p> >::DofType&
  AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  operator[] (int num)
  {
    assert(num >= 0 && num < numDofs());
    return *values_[num/N][num%N];
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  const typename AdaptiveLocalFunction<
      CombinedSpace<ContainedFunctionSpaceImp, N, p> >::DofType&
  AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  operator[] (int num) const
  {
    assert(num >= 0 && num < numDofs());
    return *values_[num/N][num%N];
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  int AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  numberOfDofs() const
  {
    return values_.size()*N;
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  int AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  numDofs() const
  {
    return values_.size()*N;
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  template <class EntityType>
  void AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  evaluate(EntityType& en, const DomainType& x, RangeType& ret) const
  {
    evaluateLocal( en, en.geometry().local(x), ret);
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  template <class EntityType>
  void AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  evaluateLocal(EntityType& en,
                const DomainType& x,
                RangeType& result) const
  {
    assert(en.geometry().checkInside(x));

    const BaseFunctionSetType& bSet = spc_.getBaseFunctionSet(en);
    result *= 0.0;

    assert(static_cast<int>(values_.size()) == bSet.numContainedFunctions());
    for (unsigned int i = 0; i < values_.size(); ++i) {
      // Assumption: scalar contained base functions
      bSet.evaluateContained(i, x, cTmp_);
      for (int j = 0; j < N; ++j) {
        result[j] += cTmp_[0]*(*values_[i][j]);
      }
    }
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  template <class EntityType, class QuadratureType>
  void AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  evaluate(EntityType& en,
           QuadratureType& quad,
           int quadPoint,
           RangeType& ret) const
  {
    evaluateLocal(en, quad.point(quadPoint), ret);
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  template <class EntityType>
  void AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  jacobianLocal(EntityType& en,
                const DomainType& x,
                JacobianRangeType& result) const
  {
    enum { dim = EntityType::dimension };
    typedef FieldMatrix<DofType, RangeType::size, RangeType::size> JacobianInverseType;
    result *= 0.0;

    const BaseFunctionSetType& bSet = spc_.getBaseFunctionSet(en);
    const JacobianInverseType& jInv =
      en.geometry().jacobianInverseTransposed(x);

    for (int i = 0; i < bSet.numContainedFunctions(); ++i) {
      //cTmpGradRef_ *= 0.0;
      cTmpGradReal_ *= 0.0;
      bSet.jacobianContained(i, x, cTmpGradRef_);
      jInv.umv(cTmpGradRef_[0], cTmpGradReal_[0]);

      for (int j = 0; j < N; ++j) {
        // Assumption: ContainedDimRange == 1
        //cTmpGrad_[0] *= *values_[i][j];
        result[j].axpy(*values_[i][j], cTmpGradReal_[0]);
      }
    }

  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  template<class EntityType, class QuadratureType>
  void AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  jacobian(EntityType& en,
           QuadratureType& quad,
           int quadPoint,
           JacobianRangeType& ret) const
  {
    jacobianLocal(en, quad.point(quadPoint), ret);
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  void AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  assign(int dofNum, const DofVectorType& dofs) {
    for (int i = 0; i < N; ++i) {
      // Assumption: the local ordering is point based
      *values_[dofNum][i] = dofs[i];
    }
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  int AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  numberOfBaseFunctions() const {
    return values_.size();
  }

  template <class ContainedFunctionSpaceImp, int N, DofStoragePolicy p>
  template <class EntityType>
  void AdaptiveLocalFunction<CombinedSpace<ContainedFunctionSpaceImp, N, p> >::
  init(const EntityType& en) {
    int numOfDof =
      spc_.getBaseFunctionSet(en).numContainedFunctions();
    values_.resize(numOfDof);

    for (int i = 0; i < numOfDof; ++i) {
      for (int j = 0; j < N; ++j) {
        values_[i][j] = &(dofVec_[spc_.mapToGlobal(en, i*N+j)]);
      } // end for j
    } // end for i

  }

} // end namespace Dune
