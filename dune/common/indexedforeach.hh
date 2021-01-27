// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_INDEXEDFOREACH_HH
#define DUNE_COMMON_INDEXEDFOREACH_HH

#include<type_traits>

#include<dune/common/fvector.hh>
#include<dune/common/hybridutilities.hh>
#include<dune/common/indices.hh>
#include<dune/common/typetraits.hh>
#include<dune/common/std/type_traits.hh>
#include<dune/common/flatvectorview.hh>

namespace Dune {

  namespace Detail {

    // stolen from dune-functions
    template<class C>
    using staticIndexAccess_t = decltype(std::declval<C>()[Dune::Indices::_0]);

    template<class C>
    using isScalar = Dune::Std::bool_constant<not Dune::Std::is_detected_v<staticIndexAccess_t, std::remove_reference_t<C>>>;

    /**
    * \brief Flat index forEach loop over a container
    *
    * \tparam V Type of given container
    * \tparam F Type of given predicate
    *
    * \param v The container to loop over
    * \param f A predicate that will be called with the flat index and entry at each entry
    * \param index The current index offset this container will be indexed
    *
    *  This routine unfolds the block structure of the container and
    *  loops over each scalar entry.
    *
    *  The functor f is called at each scalar entry taking the index and the value.
    *
    *  This is currently supported for all containers that are
    *  supported by Hybrid::forEach.
    */

    template<class V, class F>
    void flatIndexedForEach(V&& v, F&& f, std::size_t& index)
    {
      Hybrid::forEach(v, [&](auto&& vi) {
        if constexpr( isScalar<std::decay_t<decltype(vi)>>::value )
          f(index++, vi);
        else
          flatIndexedForEach(vi, f, index);
      });
    }
  } // namespace Detail


/**
* \brief Indexed forEach loop over a FlatVectorView container
*
* \tparam V Type of given container
* \tparam F Type of given predicate
*
* \param v The container to loop over
* \param f A predicate that will be called with an index at each entry
*
*  This specialized version for FlatVectorView forwards the call to flatIndexedForEach.
*/
template<class V, class F>
void indexedForEach(FlatVectorView<V>& fvv, F&& f) {
  std::size_t index = 0;
  Detail::flatIndexedForEach(fvv.rawVector(), f, index);
}

} // namespace Dune

#endif
