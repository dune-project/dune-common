// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=8 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception OR LGPL-3.0-or-later

#include <config.h>

#include <utility>
#include <vector>

#include <dune/common/test/testsuite.hh>
#include <dune/common/typetree/treecontainer.hh>
#include <dune/common/typetree/traversal.hh>
#include <dune/common/typetree/test/testtypetreeutilities.hh>



template<class F>
bool notThrown(F&& f)
{
  try {
    f();
    return true;
  }
  catch(...) {}
  return false;
}

template <class Value, class Tree>
using UniformTreeMatrix
  = Dune::TypeTree::UniformTreeContainer<
      Dune::TypeTree::UniformTreeContainer<Value,Tree>,Tree>;

template<class Tree, class Value>
Dune::TestSuite checkTreeContainer(const Tree& tree, const Value& value)
{
  Dune::TestSuite test(treeName(tree));

  // construct a container using a factory function
  auto container = Dune::TypeTree::makeTreeContainer<Value>(tree);

  // copy construct the container
  auto container2{container};
  auto container3{container};

  // copy-assign the container
  container2 = container;

  // move-construct the container
  auto container4{std::move(container2)};

  // move-assign the container
  container4 = std::move(container3);

  Dune::TypeTree::forEachLeafNode(tree, [&] (auto&& node, auto treePath) {
      test.check(notThrown([&]() {
        container[treePath] = value;
      })) << "Assigning desired value to tree container entry failed";
    });

  Dune::TypeTree::forEachLeafNode(tree, [&] (auto&& node, auto treePath) {
      test.check(container[treePath] == value)
        << "Value in tree container does not match assigned value";
    });

  // default construct a container
  decltype(container) container5{};
  container5.resize(tree);

  Dune::TypeTree::forEachLeafNode(tree, [&] (auto&& node, auto treePath) {
      test.check(notThrown([&]() {
        container5[treePath] = value;
      })) << "Assigning desired value to tree container entry failed";
    });

  Dune::TypeTree::forEachLeafNode(tree, [&] (auto&& node, auto treePath) {
      test.check(container5[treePath] == value)
        << "Value in tree container does not match assigned value";
    });


  // default construct a container with size information from tree
  decltype(container) container6{tree};

  Dune::TypeTree::forEachLeafNode(tree, [&] (auto&& node, auto treePath) {
      test.check(notThrown([&]() {
        container6[treePath] = value;
      })) << "Assigning desired value to tree container entry failed";
    });

  Dune::TypeTree::forEachLeafNode(tree, [&] (auto&& node, auto treePath) {
      test.check(container6[treePath] == value)
        << "Value in tree container does not match assigned value";
    });


  // construct a matrix-like container
  auto matrix = Dune::TypeTree::makeTreeContainer(tree,
    [&](auto const&) { return Dune::TypeTree::makeTreeContainer<Value>(tree); });

  Dune::TypeTree::forEachLeafNode(tree, [&] (auto&& rowNode, auto rowTreePath) {
    Dune::TypeTree::forEachLeafNode(tree, [&] (auto&& colNode, auto colTreePath) {
      test.check(notThrown([&]() {
        matrix[rowTreePath][colTreePath] = value;
      })) << "Assigning desired value to tree matrix-container entry failed";
    });
  });

  Dune::TypeTree::forEachLeafNode(tree, [&] (auto&& rowNode, auto rowTreePath) {
    Dune::TypeTree::forEachLeafNode(tree, [&] (auto&& colNode, auto colTreePath) {
      test.check(matrix[rowTreePath][colTreePath] == value)
        << "Value in tree matrix-container does not match assigned value";
    });
  });

  return test;
}



int main(int argc, char** argv)
{

  Dune::TestSuite test;

  using namespace Dune::Indices;

  // Use tags to generate different type nodes
  auto tagA = Dune::index_constant<23>{};
  auto tagB = Dune::index_constant<42>{};
  auto tagC = Dune::index_constant<237>{};
  auto tagD = Dune::index_constant<47>{};


  int v1 = 42;
  std::vector<double> v2{1,2,3,4};

  auto l1 = Leaf(tagA);
  test.subTest(checkTreeContainer(l1, v1));
  test.subTest(checkTreeContainer(l1, v2));

  auto us3_l1 = UniformStaticInner(tagA, Leaf(tagB), _3);
  test.subTest(checkTreeContainer(us3_l1, v1));
  test.subTest(checkTreeContainer(us3_l1, v2));

  auto ud3_l1 = UniformDynamicInner(tagA, Leaf(tagB), 3);
  test.subTest(checkTreeContainer(ud3_l1, v1));
  test.subTest(checkTreeContainer(ud3_l1, v2));

  auto n_l1_us2_l2_l3 = NonUniformInner(tagA, Leaf(tagB), UniformStaticInner(tagC, Leaf(tagD), _2), Leaf(tagA));
  test.subTest(checkTreeContainer(n_l1_us2_l2_l3, v1));
  test.subTest(checkTreeContainer(n_l1_us2_l2_l3, v2));

  test.report();

  return test.exit();
}
