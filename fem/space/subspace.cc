// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

namespace Dune {
  //- class SubSpace
  template <class CombinedSpaceImp>
  const int SubSpace<CombinedSpaceImp>::spaceId_ = 1984;

  template <class CombinedSpaceImp>
  SubSpace<CombinedSpaceImp>::SubSpace(const CombinedSpaceType& spc,
                                       int component) :
    spc_(spc),
    mapper_(spc, spc.mapper(), component),
    component_(component)
  {
    assert(false);
    // * more to come here
  }

  //- class SubBaseFunctionSet
  template <class CombinedSpaceImp>
  template <int diffOrd>
  void SubMapper<CombinedSpaceImp>::
  evaluate (int baseFunct,
            const FieldVector<deriType, diffOrd> &diffVariable,
            const DomainType & x, RangeType & phi ) const;

  //! evaluate base function at quadrature point
  template <int diffOrd, class QuadratureType >
  void SubMapper<CombinedSpaceImp>::
  evaluate (int baseFunct,
            const FieldVector<deriType, diffOrd> &diffVariable,
            QuadratureType & quad,
            int quadPoint, RangeType & phi ) const;

  //- class SubMapper
  template <class CombinedSpaceImp>
  int SubMapper<CombinedSpaceImp>::size() const
  {
    assert(false);
    // *more to come
  }

  template <class CombinedSpaceImp>
  template <class EntityType>
  SubMapper<CombinedSpaceImp>::mapToGlobal(EntityType& en, int localNum) const
  {
    assert(false);
    // *more to come
  }


} // end namespace Dune
