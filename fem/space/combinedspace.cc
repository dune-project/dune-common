// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

namespace Dune {
  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  const int CombinedSpace<DiscreteFunctionSpaceImp, N, policy>::spaceId_ = 13;

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  CombinedSpace<DiscreteFunctionSpaceImp, N, policy>::
  CombinedSpace(ContainedDiscreteFunctionSpaceType& spc) :
    BaseType(0),
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
      if (baseSetVec_[id] == 0) {
        baseSetVec_[id] = new BaseFunctionSetType(spc.getBaseFunctionSet(*it));
      }
    } // end for

  }

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  CombinedSpace<DiscreteFunctionSpaceImp, N, policy>::
  ~CombinedSpace()
  {
    for (int i = 0; i < baseSetVec_.size(); ++i) {
      delete baseSetVec_[i];
      baseSetVec_[i] = 0;
    }
  }

  //- CombinedBaseFunctionSet
  template <class BaseFunctionSetImp, int N, DofStoragePolicy policy>
  template <int diffOrd>
  void CombinedBaseFunctionSet<BaseFunctionSetImp, N, policy>::
  evaluate(int baseFunct,
           const FieldVector<deriType, diffOrd> &diffVariable,
           const DomainType & x, RangeType & phi) const
  {
    baseFunctionSet_.evaluate(containedDof(baseFunct),
                              diffVariable, x, containedResult_);
    expand(baseFunct, containedResult_, phi);
  }

  template <class BaseFunctionSetImp, int N, DofStoragePolicy policy>
  template <int diffOrd, class QuadratureType>
  void CombinedBaseFunctionSet<BaseFunctionSetImp, N, policy>::
  evaluate(int baseFunct,
           const FieldVector<deriType, diffOrd> &diffVariable,
           QuadratureType & quad,
           int quadPoint, RangeType & phi) const
  {
    baseFunctionSet_.evaluate(containedDof(baseFunct), diffVariable,
                              quad, quadPoint, containedResult_);
    expand(baseFunct, containedResult_, phi);
  }

  template <class BaseFunctionSetImp, int N, DofStoragePolicy policy>
  inline int CombinedBaseFunctionSet<BaseFunctionSetImp, N, policy>::
  containedDof(int combinedBaseNumber) const
  {
    //return global%N;
    return combinedBaseNumber/N;
  }

  template <class BaseFunctionSetImp, int N, DofStoragePolicy policy>
  inline int CombinedBaseFunctionSet<BaseFunctionSetImp, N, policy>::
  component(int combinedBaseNumber) const
  {
    //return global%N;
    return combinedBaseNumber%N;
  }

  template <class BaseFunctionSetImp, int N, DofStoragePolicy policy>
  void CombinedBaseFunctionSet<BaseFunctionSetImp, N, policy>::
  expand(int baseFunct, const ContainedRangeType& arg, RangeType& dest) const
  {
    dest = 0.0;
    assert(arg.dim() == 1); // only DimRange == 1 allowed
    dest[component(baseFunct)] = arg[0];
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
    return mapToGlobal(en, localNum, Int2Type<policy>());
  }

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  template <class EntityType>
  int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::
  mapToGlobal(EntityType& en, int localNum, Int2Type<PointBased>) const
  {
    const int offset = spc_.mapToGlobal(en, containedDof(localNum))*N;
    const int local = component(localNum);
    return offset + local;
  }

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  template <class EntityType>
  int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::
  mapToGlobal(EntityType& en, int localNum, Int2Type<VariableBased>) const
  {
    const int offset = component(localNum)*spc_.size();
    const int local = spc_.mapToGlobal(en, containedDof(localNum));
    return offset + local;
  }

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  inline int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::
  containedDof(int localNum) const
  {
    return localNum/N;
    //return localNum%numBaseLoc_; old version for variable based base function expansion
  }

  template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
  inline int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::
  component(int localNum) const
  {
    return localNum%N;
    //return localNum/numBaseLoc_; old version for variable based base function expansion
  }

  /*
     template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
     int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::
     newIndex(int num) const {
     return newIndex(num, Int2Type<policy>());
     }

     template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
     int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::
     newIndex(int num, Int2Type<PointBased>) const {
     ...
     }

     template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
     int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::
     newIndex(int num, Int2Type<VariableBased>) const {
     ...
     }

     template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
     int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::
     oldIndex(int num) const {
     return oldIndex(num, Int2Type<policy>());
     }

     template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
     int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::
     oldIndex(int num, Int2Type<PointBased>) const {
     int localNum =
      containedIndex(num, mapper_.oldSize(), Int2Type<VariableBased());
     int component = localNum%N;
     int oldLocalIndex = mapper_oldIndex(localNum);

     return oldLocalIndex*N + component;
     }

     template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
     int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::
     oldIndex(int num, Int2Type<VariableBased>) const {
     int localNum =
      containedIndex(num, mapper_.oldSize(), Int2Type<VariableBased>());
     int component = num/mapper_.oldSize();

     int oldLocalIndex = mapper_.oldIndex(localNum);
     return component*mapper_.oldSize() + oldLocalIndex;
     }

     template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
     inline int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::
     containedIndex(int num, int containedSize, Int2Type<PointBased>) const {
     return num/N;
     }

     template <class DiscreteFunctionSpaceImp, int N, DofStoragePolicy policy>
     inline int CombinedMapper<DiscreteFunctionSpaceImp, N, policy>::
     containedIndex(int num, int containedSize, Int2Type<VariableBased>) const {
     return num%containedSize;
     }
   */
} // end namespace Dune
