// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

namespace Dune {
  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  const int CombinedSpace<DiscreteFunctionSpaceImp, N, policy>::spaceId_ = 13;

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  CombinedSpace<DiscreteFunctionSpaceImp, N, policy>::
  CombinedSpace(ContainedDiscreteFunctionSpaceType& spc) :
    BaseType(spaceId_),
    spc_(spc),
    mapper_(spc, spc.mapper()),
    baseSetVec_(GeometryIdentifier::numTypes, 0)
  {
    // initialise your basefunction set with all Geometry types found in mesh
    IteratorType endit = spc.end();
    for (IteratorType it = spc.begin(); it != endit; ++it) {
      GeometryType geo = it->geometry().type();
      const int dimension =
        static_cast<int>(IteratorType::Entity::mydimension);
      GeometryIdentifier::IdentifierType id =
        GeometryIdentifier::fromGeo(dimension, geo);

      assert(id >= 0 && id < static_cast<int>(GeometryIdentifier::numTypes));
      if (baseSetVec_[id] == 0) {
        baseSetVec_[id] = new BaseFunctionSetType(spc.getBaseFunctionSet(*it));
      }
    } // end for

  }

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  CombinedSpace<DiscreteFunctionSpaceImp, N, policy>::
  ~CombinedSpace()
  {
    for (unsigned int i = 0; i < baseSetVec_.size(); ++i) {
      delete baseSetVec_[i];
      baseSetVec_[i] = 0;
    }
  }

  //- CombinedBaseFunctionSet
  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  template <int diffOrd>
  void CombinedBaseFunctionSet<DiscreteFunctionSpaceImp, N, policy>::
  evaluate(int baseFunct,
           const FieldVector<deriType, diffOrd> &diffVariable,
           const DomainType & x, RangeType & phi) const
  {
    baseFunctionSet_.evaluate(util_.containedDof(baseFunct),
                              diffVariable, x, containedResult_);
    expand(baseFunct, containedResult_, phi);
  }

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  template <int diffOrd, class QuadratureType>
  void CombinedBaseFunctionSet<DiscreteFunctionSpaceImp, N, policy>::
  evaluate(int baseFunct,
           const FieldVector<deriType, diffOrd> &diffVariable,
           QuadratureType & quad,
           int quadPoint, RangeType & phi) const
  {
    baseFunctionSet_.evaluate(util_.containedDof(baseFunct), diffVariable,
                              quad, quadPoint, containedResult_);
    expand(baseFunct, containedResult_, phi);
  }

  /*
     template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
     inline int CombinedBaseFunctionSet<DiscreteFunctionSpaceImp, N, policy>::
     containedDof(int combinedBaseNumber) const
     {
     return combinedBaseNumber/N;
     }

     template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
     inline int CombinedBaseFunctionSet<DiscreteFunctionSpaceImp, N, policy>::
     component(int combinedBaseNumber) const
     {
     return combinedBaseNumber%N;
     }
   */

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  void CombinedBaseFunctionSet<DiscreteFunctionSpaceImp, N, policy>::
  expand(int baseFunct, const ContainedRangeType& arg, RangeType& dest) const
  {
    dest = 0.0;
    assert(arg.dim() == 1); // only DimRange == 1 allowed
    dest[util_.component(baseFunct)] = arg[0];
  }

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  void CombinedBaseFunctionSet<DiscreteFunctionSpaceImp, N, policy>::
  evaluateScalar(int baseFunct,
                 const DomainType& x,
                 ContainedRangeType& phi) const
  {
    assert(baseFunct >= 0 &&
           baseFunct < baseFunctionSet_.numBaseFunctions());
    baseFunctionSet_.eval(baseFunct, x, phi);
  }

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  void CombinedBaseFunctionSet<DiscreteFunctionSpaceImp, N, policy>::
  jacobianScalar(int baseFunct,
                 const DomainType& x,
                 ContainedJacobianRangeType& phi) const
  {
    assert(baseFunct >= 0 &&
           baseFunct < baseFunctionSet_.numBaseFunctions());
    baseFunctionSet_.jacobian(baseFunct, x, phi);
  }

  //- CombinedMapper
  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::size() const
  {
    return spc_.size()*N;
  }

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  template <class EntityType>
  int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::
  mapToGlobal(EntityType& en, int localNum) const
  {
    const int component = utilLocal_.component(localNum);
    const int containedLocal = utilLocal_.containedDof(localNum);

    const int containedGlobal = spc_.mapToGlobal(en, containedLocal);

    return utilGlobal_.combinedDof(containedGlobal, component);
  }

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::
  newIndex(int num) const
  {
    DofConversionUtility<policy>
    tmpUtilGlobal(chooseSize(N, mapper_.newSize(), Int2Type<policy>()));

    const int component = tmpUtilGlobal.component(num);
    const int contained = tmpUtilGlobal.containedDof(num);

    const int containedNew = mapper_.newIndex(contained);

    return tmpUtilGlobal.combinedDof(containedNew, component);
  }

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::
  oldIndex(int num) const
  {
    DofConversionUtility<policy>
    tmpUtilGlobal(chooseSize(N, mapper_.oldSize(), Int2Type<policy>()));

    const int component = tmpUtilGlobal.component(num);
    const int contained = tmpUtilGlobal.containedDof(num);

    const int containedNew = mapper_.oldIndex(contained);

    return tmpUtilGlobal.combinedDof(containedNew, component);
  }

} // end namespace Dune
