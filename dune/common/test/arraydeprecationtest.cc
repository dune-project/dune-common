// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// This should be defined for regular unit test use; undefine to check that
// the warnings are indeed displayed.
#define NO_DEPRECATION_WARNINGS

#include <array>
#include <type_traits>

#ifdef NO_DEPRECATION_WARNINGS
#define DUNE_COMMON_ARRAY_HH_DISABLE_DEPRECATION_WARNING
#endif // NO_DEPRECATION_WARNINGS
#include <dune/common/array.hh>
#include <dune/common/deprecated.hh>

int main()
{
  // just a check to make sure we can still use array.hh's functionality while
  // it is deprecated.
  Dune::array<int, 2> test1;
  (void)test1;

  auto test2 = Dune::make_array(1, 2, 3);
  static_assert(std::is_same<decltype(test2), std::array<int, 3> >::value,
                "Unexpected result type for Dune::make_array()");

#ifdef NO_DEPRECATION_WARNINGS
  DUNE_NO_DEPRECATED_BEGIN;
#endif // NO_DEPRECATION_WARNINGS
  auto test3 = Dune::fill_array<int, 2>(0);
#ifdef NO_DEPRECATION_WARNINGS
  DUNE_NO_DEPRECATED_END;
#endif // NO_DEPRECATION_WARNINGS
  (void)test3;
}
