// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

#include <array>
#include <type_traits>
#include <vector>

#include <dune/common/test/testsuite.hh>
#include <dune/common/std/default_accessor.hh>
#include <dune/common/std/extents.hh>
#include <dune/common/std/layout_left.hh>
#include <dune/common/std/layout_right.hh>
#include <dune/common/std/mdspan.hh>

template <class Tensor>
void checkAccess(Dune::TestSuite& testSuite, Tensor tensor)
{
  Dune::TestSuite subTestSuite("checkAccess");

  if constexpr(Tensor::rank() == 0) {
    subTestSuite.check(tensor[std::array<int,0>{}] == 42.0);
#if __cpp_multidimensional_subscript >= 202110L
    subTestSuite.check(tensor[] == 42.0);
#endif
  }
  else if constexpr(Tensor::rank() == 1) {
    for (int i = 0; i < tensor.extent(0); ++i) {
      subTestSuite.check(tensor[std::array{i}] == 42.0);
      subTestSuite.check(tensor[i] == 42.0);
    }
  }
  else if constexpr(Tensor::rank() == 2) {
    for (int i = 0; i < tensor.extent(0); ++i) {
      for (int j = 0; j < tensor.extent(1); ++j) {
        subTestSuite.check(tensor[std::array{i,j}] == 42.0);
#if __cpp_multidimensional_subscript >= 202110L
        subTestSuite.check(tensor[i,j] == 42.0);
#endif
      }
    }
  }
  else if constexpr(Tensor::rank() == 3) {
    for (int i = 0; i < tensor.extent(0); ++i) {
      for (int j = 0; j < tensor.extent(1); ++j) {
        for (int k = 0; k < tensor.extent(2); ++k) {
          subTestSuite.check(tensor[std::array{i,j,k}] == 42.0);
#if __cpp_multidimensional_subscript >= 202110L
          subTestSuite.check(tensor[i,j,k] == 42.0);
#endif
        }
      }
    }
  }

  testSuite.subTest(subTestSuite);
}

template <class A, class M>
void test_accessor (Dune::TestSuite& testSuite, std::string name, const M& mapping)
{
  Dune::TestSuite subTestSuite(name);
  using Span = Dune::Std::mdspan<typename A::element_type, typename M::extents_type, typename M::layout_type, A>;

  std::vector<double> coefficients(mapping.required_span_size(), 42.0);
  auto dh = coefficients.data();

  // construction
  if constexpr(Span::rank() == 0 || Span::rank_dynamic() == 0)
    Span span1{dh};
  Span span2{dh, mapping.extents()};
  Span span3{dh, mapping};
  Span span4{dh, mapping, A{}};

  // construction with explicitly listed extents
  const typename M::extents_type& e = mapping.extents();
  if constexpr(Span::rank() == 1)
    Span span5{dh, e.extent(0)};
  else if constexpr(Span::rank() == 2)
    Span span5{dh, e.extent(0), e.extent(1)};
  else if constexpr(Span::rank() == 3)
    Span span5{dh, e.extent(0), e.extent(1), e.extent(2)};

  checkAccess(subTestSuite, span2);
  testSuite.subTest(subTestSuite);
}


template <class L, class E>
void test_layout (Dune::TestSuite& testSuite, std::string name, const E& extent)
{
  using Mapping = typename L::template mapping<E>;
  Mapping mapping(extent);

  Dune::TestSuite subTestSuite(name);
  test_accessor<Dune::Std::default_accessor<double>>(subTestSuite, "default_accessor<double>", mapping);
  test_accessor<Dune::Std::default_accessor<const double>>(subTestSuite, "default_accessor<const double>", mapping);
  testSuite.subTest(subTestSuite);
}


template <class E, class... Args>
void test_extents (Dune::TestSuite& testSuite, std::string name, Args&&... args)
{
  E extent{std::forward<Args>(args)...};

  Dune::TestSuite subTestSuite(name);
  test_layout<Dune::Std::layout_left>(subTestSuite, "layout_left", extent);
  test_layout<Dune::Std::layout_right>(subTestSuite, "layout_right", extent);
  testSuite.subTest(subTestSuite);
}

int main(int argc, char** argv)
{
  Dune::TestSuite testSuite;

  // definition of some extents
  test_extents<Dune::Std::extents<int>>(testSuite, "rank=0");
  test_extents<Dune::Std::extents<int,7>>(testSuite, "rank=1");
  test_extents<Dune::Std::extents<int,7,7>>(testSuite, "rank=2");
  test_extents<Dune::Std::extents<int,7,7,7>>(testSuite, "rank=3");

  test_extents<Dune::Std::dextents<int,1>>(testSuite, "rank=1", 7);
  test_extents<Dune::Std::dextents<int,2>>(testSuite, "rank=2", 7,7);
  test_extents<Dune::Std::dextents<int,3>>(testSuite, "rank=3", 7,7,7);

  return testSuite.exit();
}
