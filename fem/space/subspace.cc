// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

namespace Dune {
  //- class SubSpace
  template <class CombinedSpaceImp>
  const int SubSpace<CombinedSpaceImp>::spaceId_ = 1984;

  template <class CombinedSpaceImp>
  SubSpace<CombinedSpaceImp>::SubSpace(const CombinedSpaceType& spc,
                                       int component) :
    BaseType(type()),
    spc_(spc),
    mapper_(spc, spc.mapper().containedMapper(), component),
    component_(component),
    baseSetVec_(GeometryIdentifier::numTypes, 0)
  {
    // initialise your basefunction set with all Geometry types found in mesh
    IteratorType endit = spc.end();
    for (IteratorType it = spc.begin(); it != endit; ++it) {
      NewGeometryType geo = it->geometry().type();
      const int dimension =
        static_cast<int>(IteratorType::Entity::mydimension);
      GeometryIdentifier::IdentifierType id =
        GeometryIdentifier::fromGeo(dimension, geo);

      assert(id >= 0 && id < static_cast<int>(GeometryIdentifier::numTypes));
      if (baseSetVec_[id] == 0) {
        baseSetVec_[id] =
          new BaseFunctionSetType(spc.getBaseFunctionSet(*it), component);
      }
    } // end for
  }

  //- class SubBaseFunctionSet
  template <class CombinedSpaceImp>
  template <int diffOrd>
  void SubBaseFunctionSet<CombinedSpaceImp>::
  evaluate (int baseFunct,
            const FieldVector<deriType, diffOrd>& diffVariable,
            const DomainType& x, RangeType& phi ) const
  {
    // Assumption: dimRange == 1
    bSet_.evaluate(baseFunct, diffVariable, x, tmp_);
    phi[0] = tmp_[component_];
  }

  //! evaluate base function at quadrature point
  template <class CombinedSpaceImp>
  template <int diffOrd, class QuadratureType >
  void SubBaseFunctionSet<CombinedSpaceImp>::
  evaluate (int baseFunct,
            const FieldVector<deriType, diffOrd> &diffVariable,
            QuadratureType & quad,
            int quadPoint, RangeType & phi ) const
  {
    // Assumption: dimRange == 1
    bSet_.evaluate(baseFunct, diffVariable, quad, quadPoint, tmp_);
    phi[0] = tmp_[component_];
  }

  //- class SubMapper
  template <class CombinedSpaceImp>
  int SubMapper<CombinedSpaceImp>::size() const
  {
    return mapper_.size();
  }

  template <class CombinedSpaceImp>
  template <class EntityType>
  int SubMapper<CombinedSpaceImp>::
  mapToGlobal(EntityType& en, int localNum) const
  {
    const int containedGlobal = mapper_.mapToGlobal(en, localNum);

    utilGlobal_.newSize(mapper_.size()); // ok, since pointbased specialisation does nothing for newSize
    return utilGlobal_.combinedDof(containedGlobal, component_);
  }


} // end namespace Dune
