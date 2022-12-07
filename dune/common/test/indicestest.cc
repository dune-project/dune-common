// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <type_traits>
#include <tuple>

#include <dune/common/indices.hh>
#include <dune/common/tuplevector.hh>

using namespace Dune;



int main()
{
  using namespace Dune::Indices;

  // Test whether indices can be used to index a data structure
  Dune::TupleVector<int,double,float> v;
  v[_0] = 42;
  v[_1] = 3.14;
  v[_2] = 2.7;

  // Test whether the indices can be used as numbers
  std::get<_0>(v) = 43;
  std::get<_1>(v) = 4.14;
  std::get<_2>(v) = 3.7;

  { // test user-defined literal _xc
    using namespace Dune::Indices::Literals;

    static_assert(_0 == 0_ic);
    static_assert(_1 == 1_ic);
    static_assert(_2 == 2_ic);
    static_assert(_3 == 3_ic);
    static_assert(_4 == 4_ic);
    static_assert(_5 == 5_ic);
    static_assert(_6 == 6_ic);
    static_assert(_7 == 7_ic);
    static_assert(_8 == 8_ic);
    static_assert(_9 == 9_ic);

    static_assert(123_uc == 123u);
    static_assert(-123_sc == -123);
    static_assert(std::is_same_v<decltype(-123_sc), std::integral_constant<int,-123>>);
  }

  return 0;
}
