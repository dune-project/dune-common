// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <algorithm>
#include <array>

#include <dune/common/std/algorithm.hh>
#include <dune/common/test/testsuite.hh>

int main(int argc, char** argv)
{
  using namespace Dune;
  TestSuite testSuite;

  std::array<double, 10> arr1{};
  std::array<double, 10> arr2{};

  std::fill(arr1.begin(), arr1.end(), 1.0);
  std::fill(arr2.begin(), arr2.end(), 2.0);

  auto cmp = Std::lexicographical_compare_three_way(arr1.begin(), arr1.end(), arr2.begin(), arr2.end());
  testSuite.check(cmp < 0);

  return testSuite.exit();
}
