// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

#include <array>
#include <type_traits>
#include <vector>

#include <dune/common/test/testsuite.hh>
#include <dune/common/std/span.hh>


template <class T, std::size_t E>
void test_span (Dune::TestSuite& testSuite, std::string name, std::size_t size)
{
  Dune::TestSuite subTestSuite(name);
  using Span = Dune::Std::span<T, E>;

  std::vector<T> coefficients(size, 42.0);
  auto dh = coefficients.data();

  // construction
  Span span2{dh, size};
  subTestSuite.check(span2.size() == size);

  // check subspans
  auto span3a = span2.template first<2>();
  subTestSuite.check(span3a.size() == 2);
  auto span3b = span2.template last<2>();
  subTestSuite.check(span3b.size() == 2);

  auto span4a = span2.first(2);
  subTestSuite.check(span4a.size() == 2);
  auto span4b = span2.last(2);
  subTestSuite.check(span4b.size() == 2);

  auto span5a = span2.template subspan<2>();
  subTestSuite.check(span5a.size() == size-2);
  auto span5b = span2.subspan(2);
  subTestSuite.check(span5b.size() == size-2);
  auto span5c = span2.template subspan<2,3>();
  subTestSuite.check(span5c.size() == 3);
  auto span5d = span2.subspan(2,3);
  subTestSuite.check(span5d.size() == 3);

  // check element access
  for (std::size_t i = 0; i < span2.size(); ++i)
    subTestSuite.check(span2[i] == 42.0);

  // check iterators
  for (auto const& s_i : span2)
    subTestSuite.check(s_i == 42.0);

  subTestSuite.check(std::distance(span2.begin(), span2.end()) == std::ptrdiff_t(size));
  subTestSuite.check(std::distance(span2.rbegin(), span2.rend()) == std::ptrdiff_t(size));

  testSuite.subTest(subTestSuite);
}

void deduce_span (Dune::TestSuite& testSuite, std::string name, Dune::Std::span<int> s, std::size_t size)
{
  Dune::TestSuite subTestSuite(name);
  subTestSuite.check(s.size() == size);
  testSuite.subTest(subTestSuite);
}

int main(int argc, char** argv)
{
  Dune::TestSuite testSuite;

  // definition of some extents
  test_span<double,7>(testSuite, "static", 7);
  test_span<double,Dune::Std::dynamic_extent>(testSuite, "dynamic", 7);


  int arr[]{1, 2, 3, 4};
  Dune::Std::span s1{arr};
  deduce_span(testSuite, "CArray", arr, 4);

  std::vector vec{1, 2, 3, 4, 5};
  Dune::Std::span s2{vec};
  deduce_span(testSuite, "std::vector", vec, 5);

  std::array arr2{1, 2, 3, 4, 5, 6};
  Dune::Std::span s3{arr2};
  deduce_span(testSuite, "std::array", arr2, 6);

  return testSuite.exit();
}
