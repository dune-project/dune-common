// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_TUPLEVECTOR_HH
#define DUNE_COMMON_TUPLEVECTOR_HH

#include <tuple>
#include <utility>

#include <dune/common/indices.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/std/type_traits.hh>



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

  template<class... TT>
  using TupleConstructorDetector = decltype(Base(std::declval<TT&&>()...));

  template<class... TT>
  using hasTupleConstructor = Dune::Std::is_detected<TupleConstructorDetector, TT...>;


public:

  /** \brief Construct from a set of arguments
   *
   * This is only available if you can construct
   * the underlying std::tuple from the same argument
   * list.
   */
  template<class... TT,
    std::enable_if_t<hasTupleConstructor<TT...>::value, int> = 0>
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
