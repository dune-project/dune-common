// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_FLATVECTORVIEW_HH
#define DUNE_COMMON_FLATVECTORVIEW_HH

#include <cstddef>

#include<dune/common/hybridutilities.hh>
#include<dune/common/indices.hh>
#include<dune/common/typetraits.hh>
#include<dune/common/std/type_traits.hh>


namespace Dune
{
  namespace Detail
  {
    // stolen from dune-functions
    template<class C>
    using staticIndexAccess_t = decltype(std::declval<C>()[Indices::_0]);

    template<class C>
    using isScalar = Std::bool_constant<not Std::is_detected_v<staticIndexAccess_t, std::remove_reference_t<C>>>;
  }


/** \brief Wrapper for blocked vector types to export a flat vector interface
 *
 * \tparam Vector The original vector type that is wrapped
 */
template<class Vector>
class FlatVectorView
{
public:

  /** \brief Type used for vector sizes */
  using size_type = std::size_t;

  /** \brief Default constructor referencing the original vector */
  FlatVectorView(const Vector& vector)
  : vector_(vector)
  {}

  /** \brief Return the number of entries
    *
    *  Go through the blocks and call `size()` recursively until down to the scalar case
    */
  size_type size() const
  {
    size_type result = 0;
    Hybrid::forEach(vector_, [&](auto&& block){
      using Block = std::decay_t<decltype(block)>;
      if constexpr ( Detail::isScalar<Block>::value )
        result += 1;
      else
        result += FlatVectorView<Block>(block).size();
    });
    return result;
  }

  /** \brief Return reference to the stored original vector */
  const Vector& rawVector() const
  {
    return vector_;
  }


private:
  const Vector& vector_;

};

}

#endif
