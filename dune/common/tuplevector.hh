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

public:

  /** \brief Construct from a set of arguments
   *
   * This is only available if you can construct
   * the underlying std::tuple from the same argument
   * list.
   */
  template<class... TT,
    class = std::void_t<decltype(Base(std::declval<TT&&>()...))>>
  constexpr TupleVector(TT&&... tt) :
    Base(std::forward<TT>(tt)...)
  {}

  /** \brief Default constructor
   */
  constexpr TupleVector()
  {}

  /** \brief Const access to the tuple elements
   */
  template<std::size_t i,
    std::enable_if_t<(i < sizeof...(T)), int> = 0>
  constexpr decltype(auto) operator[](const Dune::index_constant<i>&) const
  {
    return std::get<i>(*this);
  }

  /** \brief Non-const access to the tuple elements
   */
  template<std::size_t i,
    std::enable_if_t<(i < sizeof...(T)), int> = 0>
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

/// \name Deduction guides for TupleVector
/// \relates TupleVector
/// @{

template<class... T>
TupleVector(T...) -> TupleVector<T...>;

template<class T1, class T2>
TupleVector(std::pair<T1, T2>) -> TupleVector<T1, T2>;

template<class... T>
TupleVector(std::tuple<T...>) -> TupleVector<T...>;

template <class Alloc, class... T>
TupleVector(std::allocator_arg_t, Alloc, T...) -> TupleVector<T...>;

template<class Alloc, class T1, class T2>
TupleVector(std::allocator_arg_t, Alloc, std::pair<T1, T2>) -> TupleVector<T1, T2>;

template<class Alloc, class... T>
TupleVector(std::allocator_arg_t, Alloc, std::tuple<T...>) -> TupleVector<T...>;

/// @}


template<class... T>
constexpr auto makeTupleVector(T&&... t)
{
  // The std::decay_t<T> is is a slight simplification,
  // because std::reference_wrapper needs special care.
  return TupleVector<std::decay_t<T>...>(std::forward<T>(t)...);
}



}  // namespace Dune

namespace std
{
  /** \brief Make std::tuple_element work for TupleVector
   *
   * It derives from std::tuple after all.
   */
  template <size_t i, typename... Args>
  struct tuple_element<i,Dune::TupleVector<Args...> >
  {
    using type = typename std::tuple_element<i, std::tuple<Args...> >::type;
  };

  /** \brief Make std::tuple_size work for TupleVector
   *
   * It derives from std::tuple after all.
   */
  template <typename... Args>
  struct tuple_size<Dune::TupleVector<Args...> >
    : std::integral_constant<std::size_t, sizeof...(Args)>
  {};
}

#endif // DUNE_COMMON_TUPLEVECTOR_HH
