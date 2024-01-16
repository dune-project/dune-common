// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

#include <array>
#include <set>
#include <type_traits>

#include <dune/common/filledarray.hh>
#include <dune/common/std/extents.hh>
#include <dune/common/std/layout_left.hh>
#include <dune/common/std/layout_right.hh>
#include <dune/common/std/layout_stride.hh>
#include <dune/common/test/testsuite.hh>

template <class E, class L, class L0>
void test_strided_layout (Dune::TestSuite& testSuite, std::string name, const E& e)
{
  Dune::TestSuite subTestSuite(name);
  using Mapping0 = typename L0::template mapping<E>;
  Mapping0 mapping0(e);

  // extract the strides from the passed second layout
  std::array<int, E::rank()> strides{};
  if constexpr(E::rank() > 0) {
    for (std::size_t i = 0; i < E::rank(); ++i)
      strides[i] = mapping0.stride(i);
  }

  // construct a strided layout with the given strides
  using Mapping = typename L::template mapping<E>;
  Mapping mapping(e, strides);

  // compare the two mappings
  if constexpr(E::rank() == 0)
    subTestSuite.check(mapping() == mapping0(), "mapping() == mapping0()");
  else if constexpr(E::rank() == 1) {
    for (int i = 0; i < e.extent(0); ++i)
      subTestSuite.check(mapping(i) == mapping0(i), "mapping(i) == mapping0(i)");
  }
  else if constexpr(E::rank() == 2) {
    for (int i = 0; i < e.extent(0); ++i) {
      for (int j = 0; j < e.extent(1); ++j) {
        subTestSuite.check(mapping(i,j) == mapping0(i,j), "mapping(i,j) == mapping0(i,j)");
      }
    }
  }
  else if constexpr(E::rank() == 3) {
    for (int i = 0; i < e.extent(0); ++i)
      for (int j = 0; j < e.extent(1); ++j)
        for (int k = 0; k < e.extent(2); ++k)
          subTestSuite.check(mapping(i,j,k) == mapping0(i,j,k), "mapping(i,j,k) == mapping0(i,j,k)");
  }

  testSuite.subTest(subTestSuite);
}

template <class E, class L, class... Args>
void test_layout (Dune::TestSuite& testSuite, std::string name, Args&&... args)
{
  Dune::TestSuite subTestSuite(name);
  using Mapping = typename L::template mapping<E>;
  Mapping mapping(std::forward<Args>(args)...);

  // get the extents from the mapping
  const E& e = mapping.extents();

  // compare the layout type the mapping was created from
  using layout_type = typename Mapping::layout_type;
  static_assert(std::is_same_v<L,layout_type>);

  // call some static property methods
  Mapping::is_always_exhaustive();
  Mapping::is_always_strided();
  Mapping::is_always_unique();

  // call some property methods
  mapping.is_exhaustive();
  mapping.is_strided();
  mapping.is_unique();

  if constexpr(E::rank() == 0)
    subTestSuite.check(mapping() == 0, "mapping() == 0");
  else if constexpr(E::rank() == 1) {
    for (int i = 0; i < e.extent(0); ++i)
      subTestSuite.check(mapping(i) == i, "mapping(i) == i");
  }
  else if constexpr(E::rank() == 2) {
    for (int i = 0; i < e.extent(0); ++i) {
      for (int j = 0; j < e.extent(1); ++j) {
        if constexpr(std::is_same_v<L,Dune::Std::layout_right>)
          subTestSuite.check(mapping(i,j) == i*e.extent(1) + j, "mapping(i,j) == i*e.extent(1) + j");
        if constexpr(std::is_same_v<L,Dune::Std::layout_left>)
          subTestSuite.check(mapping(i,j) == j*e.extent(0) + i, "mapping(i,j) == j*e.extent(0) + i");
      }
    }
  }
  else if constexpr(E::rank() == 3) {
    if (mapping.is_exhaustive()) {
      std::set<int> indices;
      for (int i = 0; i < e.extent(0); ++i)
        for (int j = 0; j < e.extent(1); ++j)
          for (int k = 0; k < e.extent(2); ++k)
            indices.insert(mapping(i,j,k));
      subTestSuite.check(indices.size() == std::size_t(e.extent(0)*e.extent(1)*e.extent(2)), "indices.size() == n*n*n");
    }
  }

  testSuite.subTest(subTestSuite);
}

template <class E>
void test_extents (Dune::TestSuite& testSuite, std::string name)
{
  Dune::TestSuite subTestSuite(name);
  test_layout<E, Dune::Std::layout_left>(subTestSuite, "layout_left()");
  test_layout<E, Dune::Std::layout_left>(subTestSuite, "layout_left(extents)", E{});
  test_layout<E, Dune::Std::layout_right>(subTestSuite, "layout_right()");
  test_layout<E, Dune::Std::layout_right>(subTestSuite, "layout_right(extents)", E{});

  test_strided_layout<E, Dune::Std::layout_stride, Dune::Std::layout_left>(subTestSuite, "layout_stride(layout_left)", E{});
  test_strided_layout<E, Dune::Std::layout_stride, Dune::Std::layout_right>(subTestSuite, "layout_stride(layout_right)", E{});

  testSuite.subTest(subTestSuite);
}


int main (int argc, char** argv)
{
  Dune::TestSuite testSuite;

  // definition of some extents
  test_extents<Dune::Std::extents<int>>(testSuite, "rank=0");
  test_extents<Dune::Std::extents<int,7>>(testSuite, "rank=1");
  test_extents<Dune::Std::extents<int,7,7>>(testSuite, "rank=2");
  test_extents<Dune::Std::extents<int,7,7,7>>(testSuite, "rank=3");

  return testSuite.exit();
}
