// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <array>
#include <iostream>
#include <type_traits>

#include <dune/common/filled_array.hh>

int main() {

  int status = 0;

  auto test1 = Dune::filledArray<2>(2.0);
  static_assert(std::is_same<decltype(test1), std::array<double, 2> >::value,
                "Wrong result type for Dune::filledArray()");

  if(test1[0] != 2.0 || test1[1] != 2.0)
  {
    std::cerr << "Dune::filledArray() produces wrong value" << std::endl;
    status = 1;
  }

#ifdef __cpp_lib_array_constexpr
  // make sure the result is constexpr when array is constexpr
  constexpr auto test2 = Dune::filledArray<2>(2);
  (void)test2;
#endif // __cpp_lib_array_constexpr

  return status;
}
