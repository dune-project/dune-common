// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <type_traits>
#include <utility>

#include <dune/common/indices.hh>
#include <dune/common/integersequence.hh>

template <int I>
using IsEven = std::bool_constant<(I % 2 == 0)>;

int main()
{
  using namespace Dune;
  using namespace Dune::Indices;

  auto seq0 = std::integer_sequence<int>{};
  static_assert(size(seq0) == 0);
  static_assert(empty(seq0) == true);

  auto seq1 = std::integer_sequence<int,0>{};
  static_assert(size(seq1) == 1);
  static_assert(empty(seq1) == false);
  static_assert(head(seq1) == std::integral_constant<int,0>{});
  static_assert(front(seq1) == head(seq1));
  static_assert(back(seq1) == head(seq1));
  static_assert(contains(seq1, std::integral_constant<int,0>{}) == true);
  static_assert(contains(seq1, std::integral_constant<int,1>{}) == false);

  auto seq2 = std::integer_sequence<int,0,1>{};
  auto seq2a = std::integer_sequence<int,1,0>{};
  static_assert(size(seq2) == 2);
  static_assert(empty(seq2) == false);
  static_assert(head(seq2) == std::integral_constant<int,0>{});
  static_assert(front(seq2) == head(seq2));
  static_assert(back(seq2) == head(tail(seq2)));
  static_assert(get<1>(seq2) == 1);
  static_assert(get(seq2,1) == 1);
  static_assert(contains(seq2, std::integral_constant<int,0>{}) == true);
  static_assert(contains(seq2, std::integral_constant<int,1>{}) == true);
  static_assert(contains(seq2, std::integral_constant<int,2>{}) == false);
  static_assert(equal(seq2, sorted(seq2a)));
  static_assert(equal(difference<4>(seq2),std::integer_sequence<int,2,3>{}));

  auto seq3 = std::integer_sequence<int,2,7,14>{};
  auto seq3a = std::integer_sequence<int,7,2,14>{};
  auto seq3b = std::integer_sequence<int,14,7,2>{};
  static_assert(size(seq3) == 3);
  static_assert(not equal(seq3, seq3a));
  static_assert(not equal(seq3, seq2));
  static_assert(equal(seq3, sorted(seq3a)));
  static_assert(equal(seq3b, sorted(seq3a,std::greater<>{})));
  static_assert(equal(push_front<2>(tail(seq3)), seq3));
  static_assert(equal(push_back<14>(std::integer_sequence<int,get<0>(seq3),get<1>(seq3)>{}), seq3));
  static_assert(empty(difference(seq3,seq3a)));
  static_assert(empty(difference(seq3a,seq3)));

  auto seq3even = std::integer_sequence<int, 2,14>{};
  static_assert(equal(filter<IsEven>(seq3), seq3even));
  static_assert(equal(filter([](auto i) { return i % 2 == 0; }, seq3), seq3even));

  return 0;
}
