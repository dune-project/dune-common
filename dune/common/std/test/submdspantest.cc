// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

#include <array>
#include <type_traits>
#include <utility>
#include <vector>

#include <dune/common/std/extents.hh>
#include <dune/common/std/layout_left.hh>
#include <dune/common/std/layout_left_padded.hh>
#include <dune/common/std/layout_right.hh>
#include <dune/common/std/layout_right_padded.hh>
#include <dune/common/std/layout_stride.hh>
#include <dune/common/std/mdarray.hh>
#include <dune/common/std/mdspan.hh>
#include <dune/common/std/submdspan.hh>
#include <dune/common/test/testsuite.hh>

template <class Mapping>
std::vector<int> makeData (const Mapping& mapping)
{
  std::vector<int> data(mapping.required_span_size(), -1);
  const auto& e = mapping.extents();
  if constexpr(Mapping::extents_type::rank() == 3) {
    for (int i = 0; i < e.extent(0); ++i)
      for (int j = 0; j < e.extent(1); ++j)
        for (int k = 0; k < e.extent(2); ++k)
          data[mapping(i,j,k)] = 100*i + 10*j + k;
  } else if constexpr(Mapping::extents_type::rank() == 2) {
    for (int i = 0; i < e.extent(0); ++i)
      for (int j = 0; j < e.extent(1); ++j)
        data[mapping(i,j)] = 10*i + j;
  }
  return data;
}

void testLayoutRightRowSlice (Dune::TestSuite& testSuite)
{
  Dune::TestSuite subTest("layout_right row slice");

  using E = Dune::Std::extents<int,2,3,4>;
  using M = Dune::Std::layout_right::mapping<E>;
  M mapping(E{});
  auto data = makeData(mapping);
  Dune::Std::mdspan<int,E,Dune::Std::layout_right> span(data.data(), mapping);

  auto row = Dune::Std::submdspan(span, 1, Dune::Std::full_extent, Dune::Std::full_extent);
  static_assert(std::is_same_v<typename decltype(row)::layout_type,Dune::Std::layout_right>);
  static_assert(decltype(row)::rank() == 2);
  static_assert(decltype(row)::static_extent(0) == 3);
  static_assert(decltype(row)::static_extent(1) == 4);

  subTest.check(row.extent(0) == 3, "row extent 0");
  subTest.check(row.extent(1) == 4, "row extent 1");
  subTest.check(row(2,3) == span(1,2,3), "row value aliases source");
  row(0,0) = 77;
  subTest.check(span(1,0,0) == 77, "row writes source");

  testSuite.subTest(subTest);
}

void testLayoutLeftColumnSlice (Dune::TestSuite& testSuite)
{
  Dune::TestSuite subTest("layout_left column slice");

  using E = Dune::Std::extents<int,2,3,4>;
  using M = Dune::Std::layout_left::mapping<E>;
  M mapping(E{});
  auto data = makeData(mapping);
  Dune::Std::mdspan<int,E,Dune::Std::layout_left> span(data.data(), mapping);

  auto column = Dune::Std::submdspan(span, Dune::Std::full_extent, Dune::Std::full_extent, 2);
  static_assert(std::is_same_v<typename decltype(column)::layout_type,Dune::Std::layout_left>);
  static_assert(decltype(column)::rank() == 2);

  subTest.check(column.extent(0) == 2, "column extent 0");
  subTest.check(column.extent(1) == 3, "column extent 1");
  subTest.check(column(1,2) == span(1,2,2), "column value aliases source");
  column(0,1) = 88;
  subTest.check(span(0,1,2) == 88, "column writes source");

  testSuite.subTest(subTest);
}

void testGeneralStridedSlices (Dune::TestSuite& testSuite)
{
  Dune::TestSuite subTest("general strided slices");

  using E = Dune::Std::extents<int,2,4,5>;
  using M = Dune::Std::layout_right::mapping<E>;
  M mapping(E{});
  auto data = makeData(mapping);
  Dune::Std::mdspan<int,E,Dune::Std::layout_right> span(data.data(), mapping);

  auto middle = Dune::Std::submdspan(span, Dune::Std::full_extent, 2, Dune::Std::full_extent);
  static_assert(std::is_same_v<typename decltype(middle)::layout_type,Dune::Std::layout_stride>);
  subTest.check(middle(1,4) == span(1,2,4), "fixed middle value");

  auto range = Dune::Std::submdspan(span,
    Dune::Std::full_extent,
    Dune::Std::range_slice{1,4},
    Dune::Std::full_extent);
  static_assert(std::is_same_v<typename decltype(range)::layout_type,Dune::Std::layout_stride>);
  subTest.check(range.extent(1) == 3, "range extent");
  subTest.check(range(1,2,4) == span(1,3,4), "range value");

  auto pairRange = Dune::Std::submdspan(span,
    Dune::Std::full_extent,
    std::pair{1,4},
    Dune::Std::full_extent);
  static_assert(std::is_same_v<typename decltype(pairRange)::layout_type,Dune::Std::layout_stride>);
  subTest.check(pairRange.extent(1) == 3, "pair range extent");
  subTest.check(pairRange(1,2,4) == span(1,3,4), "pair range value");

  auto se = Dune::Std::subextents(E{}, Dune::Std::full_extent, Dune::Std::range_slice{1,4}, 2);
  static_assert(decltype(se)::rank() == 2);
  subTest.check(se.extent(0) == 2, "subextents extent 0");
  subTest.check(se.extent(1) == 3, "subextents extent 1");

  auto strided = Dune::Std::submdspan(span,
    Dune::Std::full_extent,
    Dune::Std::extent_slice{0,2,2},
    Dune::Std::full_extent);
  static_assert(std::is_same_v<typename decltype(strided)::layout_type,Dune::Std::layout_stride>);
  subTest.check(strided.extent(1) == 2, "strided extent");
  subTest.check(strided(1,1,4) == span(1,2,4), "strided value");

  testSuite.subTest(subTest);
}

void testPaddedSubmdspanLayouts (Dune::TestSuite& testSuite)
{
  Dune::TestSuite subTest("padded submdspan layouts");

  {
    using E = Dune::Std::extents<int,4,5>;
    using M = Dune::Std::layout_right::mapping<E>;
    M mapping(E{});
    auto data = makeData(mapping);
    Dune::Std::mdspan<int,E,Dune::Std::layout_right> span(data.data(), mapping);

    auto sub = Dune::Std::submdspan(span, std::pair{1,3}, std::pair{1,4});
    static_assert(std::is_same_v<typename decltype(sub)::layout_type,Dune::Std::layout_right_padded<5>>);
    subTest.check(sub.extent(0) == 2, "right padded extent 0");
    subTest.check(sub.extent(1) == 3, "right padded extent 1");
    subTest.check(sub.stride(0) == span.stride(0), "right padded stride");
    subTest.check(sub(1,2) == span(2,3), "right padded value");
  }

  {
    using E = Dune::Std::extents<int,4,5>;
    using M = Dune::Std::layout_left::mapping<E>;
    M mapping(E{});
    auto data = makeData(mapping);
    Dune::Std::mdspan<int,E,Dune::Std::layout_left> span(data.data(), mapping);

    auto sub = Dune::Std::submdspan(span, std::pair{1,3}, std::pair{1,4});
    static_assert(std::is_same_v<typename decltype(sub)::layout_type,Dune::Std::layout_left_padded<4>>);
    subTest.check(sub.extent(0) == 2, "left padded extent 0");
    subTest.check(sub.extent(1) == 3, "left padded extent 1");
    subTest.check(sub.stride(1) == span.stride(1), "left padded stride");
    subTest.check(sub(1,2) == span(2,3), "left padded value");
  }

  {
    using E = Dune::Std::dextents<int,2>;
    Dune::Std::layout_right_padded<>::mapping<E> mapping(E{4,5}, 8);
    auto data = makeData(mapping);
    Dune::Std::mdspan<int,E,Dune::Std::layout_right_padded<>> span(data.data(), mapping);

    auto sub = Dune::Std::submdspan(span, std::pair{1,3}, std::pair{1,4});
    static_assert(std::is_same_v<typename decltype(sub)::layout_type,Dune::Std::layout_right_padded<>>);
    subTest.check(sub.stride(0) == 8, "dynamic right padded stride");
    subTest.check(sub(1,2) == span(2,3), "dynamic right padded value");
  }

  {
    using E = Dune::Std::dextents<int,2>;
    Dune::Std::layout_left_padded<>::mapping<E> mapping(E{4,5}, 7);
    auto data = makeData(mapping);
    Dune::Std::mdspan<int,E,Dune::Std::layout_left_padded<>> span(data.data(), mapping);

    auto sub = Dune::Std::submdspan(span, std::pair{1,3}, std::pair{1,4});
    static_assert(std::is_same_v<typename decltype(sub)::layout_type,Dune::Std::layout_left_padded<>>);
    subTest.check(sub.stride(1) == 7, "dynamic left padded stride");
    subTest.check(sub(1,2) == span(2,3), "dynamic left padded value");
  }

  {
    using E = Dune::Std::extents<int,2,4,5>;
    using M = Dune::Std::layout_right::mapping<E>;
    M mapping(E{});
    auto data = makeData(mapping);
    Dune::Std::mdspan<int,E,Dune::Std::layout_right> span(data.data(), mapping);

    auto sub = Dune::Std::submdspan(span,
      Dune::Std::full_extent,
      Dune::Std::full_extent,
      std::pair{1,4});
    static_assert(std::is_same_v<typename decltype(sub)::layout_type,Dune::Std::layout_right_padded<5>>);
    subTest.check(sub.stride(1) == span.stride(1), "rank-3 right padded stride");
    subTest.check(sub(1,3,2) == span(1,3,3), "rank-3 right padded value");
  }

  {
    using E = Dune::Std::extents<int,4,5,2>;
    using M = Dune::Std::layout_left::mapping<E>;
    M mapping(E{});
    auto data = makeData(mapping);
    Dune::Std::mdspan<int,E,Dune::Std::layout_left> span(data.data(), mapping);

    auto sub = Dune::Std::submdspan(span,
      std::pair{1,4},
      Dune::Std::full_extent,
      Dune::Std::full_extent);
    static_assert(std::is_same_v<typename decltype(sub)::layout_type,Dune::Std::layout_left_padded<4>>);
    subTest.check(sub.stride(1) == span.stride(1), "rank-3 left padded stride");
    subTest.check(sub(2,3,1) == span(3,3,1), "rank-3 left padded value");
  }

  testSuite.subTest(subTest);
}

void testScalarAndConstSlices (Dune::TestSuite& testSuite)
{
  Dune::TestSuite subTest("scalar and const slices");

  using E = Dune::Std::extents<int,2,3>;
  using M = Dune::Std::layout_right::mapping<E>;
  M mapping(E{});
  auto data = makeData(mapping);
  Dune::Std::mdspan<int,E,Dune::Std::layout_right> span(data.data(), mapping);

  auto scalar = Dune::Std::submdspan(span, 1, 2);
  static_assert(decltype(scalar)::rank() == 0);
  subTest.check(scalar() == span(1,2), "scalar value");

  Dune::Std::mdspan<const int,E,Dune::Std::layout_right> cspan(data.data(), mapping);
  auto crow = Dune::Std::submdspan(cspan, 1, Dune::Std::full_extent);
  static_assert(std::is_same_v<typename decltype(crow)::element_type,const int>);
  subTest.check(crow(2) == cspan(1,2), "const row value");

  testSuite.subTest(subTest);
}

void testLagrangeSimplexSliceUse (Dune::TestSuite& testSuite)
{
  Dune::TestSuite subTest("LagrangeSimplex-style slice");

  using E = Dune::Std::extents<int,3,2,4>;
  using A = std::array<double,3*2*4>;
  Dune::Std::mdarray<double,E,Dune::Std::layout_right,A> values{};
  auto span = values.to_mdspan();

  auto slice = Dune::Std::submdspan(span, 2, Dune::Std::full_extent, Dune::Std::full_extent);
  static_assert(std::is_same_v<typename decltype(slice)::layout_type,Dune::Std::layout_right>);
  slice(1,3) = 5.0;
  subTest.check(span(2,1,3) == 5.0, "slice aliases mdarray span");

  testSuite.subTest(subTest);
}

int main (int argc, char** argv)
{
  Dune::TestSuite testSuite;

  testLayoutRightRowSlice(testSuite);
  testLayoutLeftColumnSlice(testSuite);
  testGeneralStridedSlices(testSuite);
  testPaddedSubmdspanLayouts(testSuite);
  testScalarAndConstSlices(testSuite);
  testLagrangeSimplexSliceUse(testSuite);

  return testSuite.exit();
}
