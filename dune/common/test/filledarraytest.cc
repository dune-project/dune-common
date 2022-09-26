// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <array>
#include <iostream>
#include <type_traits>

#include <dune/common/filledarray.hh>

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
  std::cout << "The result of Dune::filledArray() is constexpr" << std::endl;
  constexpr auto test2 = Dune::filledArray<2>(2);
  (void)test2;
#else // !__cpp_lib_array_constexpr
  std::cout << "Not checking whether Dune::filledArray() is constexpr\n"
            << "since the library does not declare std::array as constexpr\n"
            << "(__cpp_lib_array_constexpr is not defined)." << std::endl;
#endif // !__cpp_lib_array_constexpr

  return status;
}
