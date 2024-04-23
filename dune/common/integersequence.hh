// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_INTEGERSEQUENCE_HH
#define DUNE_COMMON_INTEGERSEQUENCE_HH

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <type_traits>
#include <utility>

#include <dune/common/indices.hh>

namespace Dune {

//! Return the entry at position `pos` of the given sequence.
template <std::size_t pos, class T, T... II,
  std::enable_if_t<(!(pos >= sizeof...(II))), int> = 0>
constexpr auto get (std::integer_sequence<T,II...>, std::integral_constant<std::size_t,pos> = {})
{
  constexpr T entry = std::array<T,sizeof...(II)>{II...}[pos];
  return std::integral_constant<T,entry>{};
}

//! Return the entry at position `pos` of the given sequence.
template <class T, T... II>
constexpr T get (std::integer_sequence<T,II...>, std::size_t pos)
{
  assert(pos < sizeof...(II));
  return std::array<T,sizeof...(II)>{II...}[pos];
}

//! Return the first entry of the sequence.
template <class T, T I0, T... II>
constexpr std::integral_constant<T,I0>
front (std::integer_sequence<T,I0,II...>) { return {}; }

//! Return the last entry of the sequence.
template <class T, T... II,
  std::enable_if_t<(sizeof...(II) > 0), int> = 0>
constexpr auto back (std::integer_sequence<T,II...> seq)
{
  return get<sizeof...(II)-1>(seq);
}


//! For a sequence [head,tail...) return the single head element.
template <class T, T I0, T... II>
constexpr std::integral_constant<T,I0>
head (std::integer_sequence<T,I0,II...>) { return {}; }

//! For a sequence [head,tail...) return the tail sequence.
template <class T, T I0, T... II>
constexpr std::integer_sequence<T,II...>
tail (std::integer_sequence<T,I0,II...>) { return {}; }


//! Append an index `I0` to the front of the sequence.
template <auto I0, class T, T... II>
constexpr std::integer_sequence<T,T(I0),II...>
push_front (std::integer_sequence<T,II...>, std::integral_constant<T,I0> = {}) { return {}; }

//! Append an index `IN` to the back of the sequence.
template <auto IN, class T, T... II>
constexpr std::integer_sequence<T,II...,T(IN)>
push_back (std::integer_sequence<T,II...>, std::integral_constant<T,IN> = {}) { return {}; }


//! Return the size of the sequence.
template <class T, T... II>
constexpr std::integral_constant<std::size_t,sizeof...(II)>
size (std::integer_sequence<T,II...>) { return {}; }

//! Checks whether the sequence is empty.
template <class T, T... II>
constexpr std::bool_constant<(sizeof...(II) == 0)>
empty (std::integer_sequence<T,II...>) { return {}; }

namespace Impl {

// constexpr version of swap for older compilers
template <class T>
constexpr void swapImpl (T& a, T& b)
{
  T c = a; a = b; b = c;
}

// constexpr quick sort implementation
template <class T, std::size_t N, class Compare>
constexpr void sortImpl (std::array<T, N>& array, Compare comp, std::size_t left = 0, std::size_t right = N)
{
  // based on https://stackoverflow.com/a/40030044
  if (left < right) {
    std::size_t m = left;
    for (std::size_t i = left + 1; i < right; ++i)
      if (comp(array[i], array[left]))
        swapImpl(array[++m], array[i]);
    swapImpl(array[left], array[m]);
    sortImpl(array, comp, left, m);
    sortImpl(array, comp, m + 1, right);
  }
}

// return the sorted array
template <class T, std::size_t N, class Compare>
constexpr std::array<T, N> sortedImpl (std::array<T, N> array, Compare comp)
{
  sortImpl(array, comp);
  return array;
}

} // end namespace Impl

//! Sort a given sequence by the comparator `comp`.
template <class T, T... II, class Compare>
constexpr auto sorted (std::integer_sequence<T,II...> seq, Compare comp)
{
  constexpr auto sorted = Impl::sortedImpl(std::array<T,sizeof...(II)>{II...}, comp);
  return unpackIntegerSequence([&](auto... i) {
    return std::integer_sequence<T,sorted[i]...>{};
  }, std::make_index_sequence<sizeof...(II)>{});
}

//! Sort a given sequence by less-than comparison.
template <class T, T... II>
constexpr auto sorted (std::integer_sequence<T,II...> seq)
{
  return sorted(seq, std::less<T>{});
}

//! Checks whether or not a given sequence contains a value.
template <class T, T... II, T value>
constexpr std::bool_constant<((II == value) || ...)>
contains (std::integer_sequence<T,II...>, std::integral_constant<T,value>) { return {}; }


//! Return the elements from the sequence [II...) which are not found in the sequence [JJ...).
template <class T, T... II, T... JJ>
constexpr auto difference (std::integer_sequence<T,II...> iSeq, std::integer_sequence<T,JJ...> jSeq)
{
  if constexpr(iSeq.size() == 0 || jSeq.size() == 0)
    return iSeq;
  else {
    constexpr auto I0 = head(iSeq);
    if constexpr(!contains(jSeq,I0))
      return push_front<I0.value>(difference(tail(iSeq),jSeq));
    else
      return difference(tail(iSeq),jSeq);
  }
}

//! Return the elements from the sequence [0,1,...N) which are not found in the sequence [JJ...).
template <std::size_t N, class T, T... JJ,
  std::enable_if_t<(N >= sizeof...(JJ)), int> = 0>
constexpr auto difference (std::integer_sequence<T,JJ...> jSeq)
{
  return difference(std::make_integer_sequence<T,N>{}, jSeq);
}


//! Checks whether two sequences are identical.
template <class S, S... II, class T, T... JJ,
  std::enable_if_t<(sizeof...(II) == sizeof...(JJ)), int> = 0,
  class ST = std::common_type_t<S,T>>
constexpr std::is_same<std::integer_sequence<bool,true,(ST(II) == ST(JJ))...>,
                       std::integer_sequence<bool,(ST(II) == ST(JJ))...,true>>
equal (std::integer_sequence<S,II...>, std::integer_sequence<T,JJ...>) { return {}; }

//! Sequences are unequal if not of the same length.
template <class S, S... II, class T, T... JJ,
  std::enable_if_t<(sizeof...(II) != sizeof...(JJ)), int> = 0>
constexpr std::bool_constant<false>
equal (std::integer_sequence<S,II...>, std::integer_sequence<T,JJ...>) { return {}; }


template <template <auto> class Filter, class T>
constexpr auto filter (std::integer_sequence<T> jSeq) { return jSeq; }

//! Return the elements from the sequence [JJ...) which are accepted by the Filter,
//! i.e., for which `Filter<JJ>::value == true`
template <template <auto> class Filter, class T, T J0, T... JJ>
constexpr auto filter (std::integer_sequence<T,J0,JJ...> jSeq)
{
  if constexpr(Filter<J0>::value)
    return push_front(filter<Filter>(tail(jSeq)), head(jSeq));
  else
    return filter<Filter>(tail(jSeq));
}

template <class Filter, class T>
constexpr auto filter (Filter, std::integer_sequence<T> jSeq) { return jSeq; }

//! Return the elements from the sequence [JJ...) which are accepted by the Filter,
//! i.e., for which `f(integral_constant<JJ>) == true`
template <class Filter, class T, T J0, T... JJ>
constexpr auto filter (Filter f, std::integer_sequence<T,J0,JJ...> jSeq)
{
  constexpr auto jHead = head(jSeq);
  if constexpr(f(jHead))
    return push_front(filter(f, tail(jSeq)), jHead);
  else
    return filter(f, tail(jSeq));
}

} // end namespace Dune

#endif // DUNE_COMMON_INTEGERSEQUENCE_HH
