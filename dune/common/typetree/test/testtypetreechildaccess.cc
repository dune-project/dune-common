// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=8 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception OR LGPL-3.0-or-later

#include <config.h>

#include <cstdlib>
#include <utility>
#include <type_traits>
#include <tuple>

#include <dune/common/indices.hh>
#include <dune/common/hybridmultiindex.hh>
#include <dune/common/typetree/childaccess.hh>
#include <dune/common/typetree/test/testtypetreeutilities.hh>




// Bring concepts into scope
using namespace Dune::Indices;

template<class Tree, class Child, std::size_t... i>
void checkChild() {

  auto tree = Tree{};
  using TreePath = Dune::HybridMultiIndex<Dune::index_constant<i>...>;
  auto treePath = TreePath{};

  using ChildByIndices = Dune::TypeTree::Child<Tree, i...>;
  static_assert(std::is_same_v<Child, ChildByIndices>);

  using ChildByTreePath = Dune::TypeTree::ChildForTreePath<Tree, TreePath>;
  static_assert(std::is_same_v<Child, ChildByTreePath>);

  const auto& childByIndices = Dune::TypeTree::child(tree, Dune::index_constant<i>{}...);
  static_assert(std::is_same_v<Child, std::decay_t<decltype(childByIndices)>>);

  const auto& childByTreePath = Dune::TypeTree::child(tree, treePath);
  static_assert(std::is_same_v<Child, std::decay_t<decltype(childByTreePath)>>);
}

template<class Tree, class... Child>
void checkChildren() {
  using DeducedChildren = Dune::TypeTree::Impl::Children<Tree>;
  if constexpr (sizeof...(Child)>0)
    static_assert(std::is_same_v<DeducedChildren, std::tuple<Child...>>);
  else
    static_assert(std::is_same_v<DeducedChildren, void>);
}

int main() {

  // Use tags to generate different type nodes
  using TagA = Dune::index_constant<23>;
  using TagB = Dune::index_constant<42>;
  using TagC = Dune::index_constant<237>;
  using TagD = Dune::index_constant<47>;

  using Tree = NonUniformInner<TagA,
      UniformStaticInner<TagB,
        Leaf<TagA>
      , 3>,
      Leaf<TagB>,
      NonUniformInner<TagC,
        Leaf<TagC>,
        Leaf<TagD>
      >
    >;

  checkChild<Tree,
    Tree
    >();

  checkChildren<Tree,
      UniformStaticInner<TagB,
        Leaf<TagA>
      , 3>,
      Leaf<TagB>,
      NonUniformInner<TagC,
        Leaf<TagC>,
        Leaf<TagD>
      >
    >();

  checkChild<Tree,
    UniformStaticInner<TagB,
      Leaf<TagA>
    , 3>
    , 0>();

  checkChildren<Dune::TypeTree::Child<Tree, 0>>();

  checkChild<Tree,
    Leaf<TagA>
    , 0, 0>();

  checkChildren<Dune::TypeTree::Child<Tree, 0, 0>>();

  checkChild<Tree,
    Leaf<TagB>
    , 1>();

  checkChildren<Dune::TypeTree::Child<Tree, 1>>();

  checkChild<Tree,
    NonUniformInner<TagC,
      Leaf<TagC>,
      Leaf<TagD>
    >
    , 2>();

  checkChildren<Dune::TypeTree::Child<Tree, 2>,
      Leaf<TagC>,
      Leaf<TagD>
    >();

  checkChild<Tree,
    Leaf<TagC>
    , 2, 0>();

  checkChildren<Dune::TypeTree::Child<Tree, 2, 0>>();

  checkChild<Tree,
    Leaf<TagD>
    , 2, 1>();

  checkChildren<Dune::TypeTree::Child<Tree, 2, 1>>();

  return EXIT_SUCCESS;
}
