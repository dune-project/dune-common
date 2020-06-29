// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

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
