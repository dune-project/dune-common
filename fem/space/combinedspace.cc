// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

namespace Dune {
  template <class DiscreteFunctionSpaceImp, int N>
  CombinedSpace<DiscreteFunctionSpaceImp, int N>::
  CombinedSpace(DiscreteFunctionSpaceType& spc) :
    spc_(spc)
  {
    // * more to come
    // * initialise your basefunction set

    // * initialise your mapper
  }


  template <class BaseFunctionSetImp, int N>
  template <int diffOrd>
  void CombinedBaseFunctionSet<BaseFunctionSetImp, int N>::
  evaluate (int baseFunct,
            const FieldVector<deriType, diffOrd> &diffVariable,
            const DomainType & x, RangeType & phi ) const {
    expand(containedResult_, phi);
  }

  template <class BaseFunctionSetImp, int N>
  template <int diffOrd, class QuadratureType>
  void CombinedBaseFunctionSet<BaseFunctionSetImp, int N>::
  evaluate (int baseFunct,
            const FieldVector<deriType, diffOrd> &diffVariable,
            QuadratureType & quad,
            int quadPoint, RangeType & phi ) const {

    expand(containedResult_, phi);
  }

  template <class BaseFunctionSetImp, int N>
  void CombinedBaseFunctionSet<BaseFunctionSetImp, int N>::
  expand(const ContainedRangeType& arg, RangeType& dest) const {}


} // end namespace Dune
