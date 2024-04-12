// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

#include <numeric>
#include <vector>

#include <dune/common/indexediterator.hh>
#include <dune/common/iteratorrange.hh>
#include <dune/common/rangeutilities.hh>
#include <dune/common/test/testsuite.hh>

auto testSparseRange()
{
  using namespace Dune;
  TestSuite suite("Check sparseRange()");
  std::vector<int> vec(10);
  std::iota(vec.begin(), vec.end(), 0);

  auto indexedRange = IteratorRange{IndexedIterator{vec.begin()}, IndexedIterator{vec.end()}};
  for (auto&& [vi,i] : Dune::sparseRange(indexedRange))
    suite.check(vi == i);

  return suite;
}

int main()
{
  Dune::TestSuite suite;
  suite.subTest(testSparseRange());

  return suite.exit();
}
