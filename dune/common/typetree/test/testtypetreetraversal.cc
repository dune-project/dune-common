// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception OR LGPL-3.0-or-later

#include <config.h>

#include <cstddef>
#include <type_traits>
#include <utility>

#include <dune/common/indices.hh>
#include <dune/common/test/testsuite.hh>
#include <dune/common/typetree/test/testtypetreeutilities.hh>
#include <dune/common/typetree/traversal.hh>

int main()
{
  Dune::TestSuite test("tree traversal check");

  using Payload = int;

  auto tree = NonUniformInner(
                Payload(0),
                UniformStaticInner(
                  Payload(0),
                  Leaf(Payload(0)),
                  Dune::Indices::_3
                ),
                Leaf(Payload(0)));

  {
    std::size_t all = 0;
    Dune::TypeTree::forEachNode(tree, [&](auto&& node, auto&& path) {
      ++all;
    });
    test.check(all==6)
      << "Counting all nodes with forEachNode failed. Result is " << all << " but should be " << 6;
  }

  {
    std::size_t all = 0;
    auto countNode = [&](auto&& node, auto&& path) {
      ++all;
    };
    auto nop = [&](auto&& node, auto&& path) {};
    Dune::TypeTree::forEachNode(tree, countNode, countNode, nop);
    test.check(all==6)
      << "Counting all nodes with forEachNode failed. Result is " << all << " but should be " << 6;
  }

  {
    std::size_t all = 0;
    auto countNode = [&](auto&& node, auto&& path) {
      ++all;
    };
    auto nop = [&](auto&& node, auto&& path) {};
    Dune::TypeTree::forEachNode(tree, nop, countNode, countNode);
    test.check(all==6)
      << "Counting all nodes with forEachNode failed. Result is " << all << " but should be " << 6;
  }

  {
    std::size_t all = 0;
    auto countNode = [&](auto&& node, auto&& path) {
      ++all;
    };
    auto nop = [&](auto&& node, auto&& path) {};
    Dune::TypeTree::forEachNode(tree, nop, countNode, nop);
    test.check(all==4)
      << "Counting all nodes with forEachNode failed. Result is " << all << " but should be " << 4;
  }

  {
    std::size_t all = 0;
    auto countNode = [&](auto&& node, auto&& path) {
      ++all;
    };
    Dune::TypeTree::forEachNode(tree, countNode, countNode, countNode);
    test.check(all==8)
      << "Counting all nodes with forEachNode failed. Result is " << all << " but should be " << 8;
  }

  {
    std::size_t leaf = 0;
    Dune::TypeTree::forEachLeafNode(tree, [&](auto&& node, auto&& path) {
      ++leaf;
    });
    test.check(leaf==4)
      << "Counting leaf nodes with forEachLeafNode failed. Result is " << leaf << " but should be " << 4;
  }

  {
    auto countVisit = [] (auto&& node, auto&& path) {
      ++(node.value());
    };
    Dune::TypeTree::forEachNode(tree, countVisit, countVisit, countVisit);

    std::size_t visits=0;
    Dune::TypeTree::forEachNode(tree, [&](auto&& node, auto&& path) {
      visits += node.value();
    });

    test.check(visits==8)
      << "Counting all node visitations failed. Result is " << visits << " but should be " << 8;
  }

  return test.exit();
}
