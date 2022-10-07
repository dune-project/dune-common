// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#include "config.h"

#include <type_traits>

#include <dune/common/hybridutilities.hh>

int main()
{
  // check that the id argument is a constexpr functor
  Dune::Hybrid::ifElse(std::true_type{}, [](auto id) {
      static_assert(id(true),
        "id() argument of ifElse() branches should be a constexpr functor");
    });
}
