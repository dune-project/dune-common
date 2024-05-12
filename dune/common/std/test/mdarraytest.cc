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
#include <dune/common/std/mdarray.hh>

template <class Tensor>
void checkAccess(Dune::TestSuite& testSuite, const Tensor& tensor)
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

template <class C, class M>
void test_container (Dune::TestSuite& testSuite, std::string name, const M& mapping)
{
  Dune::TestSuite subTestSuite(name);
  using Tensor = Dune::Std::mdarray<typename C::value_type, typename M::extents_type, typename M::layout_type, C>;

  // construction
  [[maybe_unused]] Tensor tensor0{};
  [[maybe_unused]] Tensor tensor1{mapping.extents()};
  [[maybe_unused]] Tensor tensor2{mapping};
  Tensor tensor3{mapping.extents(), 42.0};
  [[maybe_unused]] Tensor tensor4{mapping, 42.0};

  // construction with explicitly listed extents
  auto const& e = mapping.extents();
  if constexpr(Tensor::rank() == 1)
    [[maybe_unused]] Tensor tensor5{e.extent(0)};
  else if constexpr(Tensor::rank() == 2)
    [[maybe_unused]] Tensor tensor5{e.extent(0), e.extent(1)};
  else if constexpr(Tensor::rank() == 3)
    [[maybe_unused]] Tensor tensor5{e.extent(0), e.extent(1), e.extent(2)};


  // convert to mdspan
  auto tensor_span = tensor3.to_mdspan();

  // construct from mdspan
  Tensor tensor6{tensor_span};

  checkAccess(subTestSuite, tensor3);
  testSuite.subTest(subTestSuite);
}


template <class L, class E>
void test_layout (Dune::TestSuite& testSuite, std::string name, const E& extent)
{
  using Mapping = typename L::template mapping<E>;
  Mapping mapping(extent);

  Dune::TestSuite subTestSuite(name);
  test_container<std::vector<double>>(subTestSuite, "std::vector<double>", mapping);
  if constexpr(E::rank_dynamic() == 0) {
    if constexpr (E::rank() == 0)
        test_container<std::array<double,1>>(subTestSuite, "std::array<double,1>", mapping);
    else if constexpr (E::rank() == 1) {
        static constexpr std::size_t N = E::static_extent(0);
        test_container<std::array<double,N>>(subTestSuite, "std::array<double,n>", mapping);
    } else if constexpr (E::rank() == 2) {
        static constexpr std::size_t N = E::static_extent(0)*E::static_extent(1);
        test_container<std::array<double,N>>(subTestSuite, "std::array<double,n*n>", mapping);
    } else if constexpr (E::rank() == 3) {
        static constexpr std::size_t N = E::static_extent(0)*E::static_extent(1)*E::static_extent(2);
        test_container<std::array<double,N>>(subTestSuite, "std::array<double,n*n*n>", mapping);
    }
  }
  testSuite.subTest(subTestSuite);
}


template <class E>
void test_extents (Dune::TestSuite& testSuite, std::string name)
{
  E extent{};

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

  return testSuite.exit();
}
