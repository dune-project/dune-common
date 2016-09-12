// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_TUPLEVECTOR_HH
#define DUNE_COMMON_TUPLEVECTOR_HH

#include <tuple>
#include <utility>

#include <dune/common/indices.hh>



/**
 * \file
 * \brief Provides the TupleVector class that augments std::tuple by operator[]
 * \author Carsten Gräser
 */

namespace Dune
{



/**
 * \brief A class augmenting std::tuple by element access via operator[]
 *
 * \ingroup Utilities
 */
template<class... T>
class TupleVector : public std::tuple<T...>
{
  using Base = std::tuple<T...>;

public:

  /** \brief Construct from a set of arguments
   */
  template<class... TT>
  constexpr TupleVector(TT&&... tt) :
    Base(std::forward<TT>(tt)...)
  {}

  /** \brief Default constructor
   */
  constexpr TupleVector()
  {}

  /** \brief Const access to the tuple elements
   */
  template<std::size_t i>
  constexpr decltype(auto) operator[](const Dune::index_constant<i>&) const
  {
    return std::get<i>(*this);
  }

  /** \brief Non-const access to the tuple elements
   */
  template<std::size_t i>
  decltype(auto) operator[](const Dune::index_constant<i>&)
  {
    return std::get<i>(*this);
  }

  /** \brief Number of elements of the tuple */
  static constexpr std::size_t size()
  {
    return std::tuple_size<Base>::value;
  }
};



template<class... T>
constexpr auto makeTupleVector(T&&... t)
{
  // The std::decay_t<T> is is a slight simplification,
  // because std::reference_wrapper needs special care.
  return TupleVector<std::decay_t<T>...>(std::forward<T>(t)...);
}



}  // namespace Dune

#endif // DUNE_COMMON_TUPLEVECTOR_HH
