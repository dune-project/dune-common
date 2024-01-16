// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

#include <type_traits>

#include <dune/common/std/extents.hh>
#include <dune/common/std/span.hh>
#include <dune/common/test/testsuite.hh>

int main(int argc, char** argv)
{
  Dune::TestSuite testSuite;

  // a placeholder for dynamic extents
  static constexpr std::size_t dyn = Dune::Std::dynamic_extent;

  // rank 0
  using Extents0 = Dune::Std::extents<int>;
  [[maybe_unused]] Extents0 extents0{};
  static_assert(std::is_empty_v<Extents0>);
  static_assert(std::is_same_v<typename Extents0::index_type, int>);
  static_assert(std::is_same_v<typename Extents0::size_type, unsigned int>);
  static_assert(Extents0::rank() == 0);

  // rank 1
  using Extents1a = Dune::Std::extents<int,2>;
  using Extents1b = Dune::Std::extents<int,dyn>;
  static_assert(std::is_empty_v<Extents1a>);
  static_assert(std::is_same_v<typename Extents1b::index_type, int>);
  static_assert(std::is_same_v<typename Extents1b::size_type, unsigned int>);
  static_assert(Extents1a::rank() == 1);
  static_assert(Extents1b::rank() == 1);
  static_assert(Extents1a::static_extent(0) == 2);
  static_assert(Extents1b::static_extent(0) == dyn);
  Extents1a extents1a{2};
  Extents1b extents1b{2};
  testSuite.check(extents1a.extent(0) == 2, "extents1a.extent(0) == 2");
  testSuite.check(extents1b.extent(0) == 2, "extents1b.extent(0) == 2");

  // rank 2
  using Extents2a = Dune::Std::extents<int,2,3>;
  using Extents2b = Dune::Std::extents<int,2,dyn>;
  using Extents2c = Dune::Std::extents<int,dyn,3>;
  using Extents2d = Dune::Std::extents<int,dyn,dyn>;
  static_assert(std::is_empty_v<Extents2a>);
  static_assert(std::is_same_v<typename Extents2b::index_type, int>);
  static_assert(std::is_same_v<typename Extents2b::size_type, unsigned int>);
  static_assert(Extents2a::rank() == 2);
  static_assert(Extents2b::rank() == 2);
  static_assert(Extents2c::rank() == 2);
  static_assert(Extents2d::rank() == 2);
  static_assert(Extents2a::static_extent(0) == 2);
  static_assert(Extents2a::static_extent(1) == 3);
  static_assert(Extents2b::static_extent(0) == 2);
  static_assert(Extents2b::static_extent(1) == dyn);
  static_assert(Extents2c::static_extent(0) == dyn);
  static_assert(Extents2c::static_extent(1) == 3);
  static_assert(Extents2d::static_extent(0) == dyn);
  static_assert(Extents2d::static_extent(1) == dyn);
  Extents2a extents2a{2,3};
  Extents2b extents2b{2,3};
  Extents2c extents2c{2,3};
  Extents2d extents2d{2,3};
  testSuite.check(extents2a.extent(0) == 2, "extents2a.extent(0) == 2");
  testSuite.check(extents2a.extent(1) == 3, "extents2a.extent(1) == 3");
  testSuite.check(extents2b.extent(0) == 2, "extents2b.extent(0) == 2");
  testSuite.check(extents2b.extent(1) == 3, "extents2b.extent(1) == 3");
  testSuite.check(extents2c.extent(0) == 2, "extents2c.extent(0) == 2");
  testSuite.check(extents2c.extent(1) == 3, "extents2c.extent(1) == 3");
  testSuite.check(extents2d.extent(0) == 2, "extents2d.extent(0) == 2");
  testSuite.check(extents2d.extent(1) == 3, "extents2d.extent(1) == 3");

  // rank 3
  using Extents3a = Dune::Std::extents<int,2,3,4>;
  using Extents3b = Dune::Std::extents<int,2,dyn,4>;
  static_assert(std::is_empty_v<Extents3a>);
  static_assert(std::is_same_v<typename Extents3b::index_type, int>);
  static_assert(std::is_same_v<typename Extents3b::size_type, unsigned int>);
  static_assert(Extents3a::rank() == 3);
  static_assert(Extents3b::rank() == 3);
  static_assert(Extents3a::static_extent(0) == 2);
  static_assert(Extents3a::static_extent(1) == 3);
  static_assert(Extents3a::static_extent(2) == 4);
  static_assert(Extents3b::static_extent(0) == 2);
  static_assert(Extents3b::static_extent(1) == dyn);
  static_assert(Extents3b::static_extent(2) == 4);
  Extents3a extents3a{2,3,4};
  Extents3b extents3b{2,3,4};
  testSuite.check(extents3a.extent(0) == 2, "extents3a.extent(0) == 2");
  testSuite.check(extents3a.extent(1) == 3, "extents3a.extent(1) == 3");
  testSuite.check(extents3a.extent(2) == 4, "extents3a.extent(2) == 4");
  testSuite.check(extents3b.extent(0) == 2, "extents3b.extent(0) == 2");
  testSuite.check(extents3b.extent(1) == 3, "extents3b.extent(1) == 3");
  testSuite.check(extents3b.extent(2) == 4, "extents3b.extent(2) == 4");

  return testSuite.exit();
}
