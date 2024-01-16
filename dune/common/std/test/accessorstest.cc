// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

#include <array>
#include <type_traits>

#include <dune/common/std/default_accessor.hh>
#include <dune/common/test/testsuite.hh>

int main(int argc, char** argv)
{
  Dune::TestSuite testSuite;
  std::array<double, 10> arr{};

  using A = Dune::Std::default_accessor<double>;
  using B = Dune::Std::default_accessor<const double>;
  static_assert(std::is_same_v<typename A::element_type, double>);
  static_assert(std::is_same_v<typename B::element_type, const double>);

  // check default constructors
  A accessor;
  B const_accessor;

  { // check (converting) copy constructor
    [[maybe_unused]] A accessor2(accessor);
    // A accessor2(const_accessor); // this conversion is not allowed!

    [[maybe_unused]] B const_accessor2(const_accessor);
    [[maybe_unused]] B const_accessor3(accessor);
  }

  auto dh = arr.data();

  // check write access
  accessor.access(dh, 0) = 1;
  // check read access
  testSuite.check(const_accessor.access(dh,0) == 1);

  // check offset
  testSuite.check(accessor.offset(dh,1) != dh);
  testSuite.check(const_accessor.offset(dh,1) != dh);
  // testSuite.check(accessor.access(const_accessor.offset(dh,1),0) == 0); // this conversion of data_handle is not allowed!
  testSuite.check(const_accessor.access(accessor.offset(dh,1),0) == 0);
}
