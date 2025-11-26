// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception OR LGPL-3.0-or-later

#ifndef DUNE_COMMON_TYPETREE_TRAVERSAL_HH
#define DUNE_COMMON_TYPETREE_TRAVERSAL_HH

#include <utility>

#include <dune/common/hybridutilities.hh>
#include <dune/common/indices.hh>
#include <dune/common/std/type_traits.hh>

#include <dune/common/typetree/nodeconcepts.hh>
#include <dune/common/typetree/childaccess.hh>
#include <dune/common/typetree/treepath.hh>

namespace Dune::TypeTree {

  namespace Impl {

    template<class Callable, class Arg0, class Arg1>
    constexpr void invokeWithTwoOrOneArg(Callable&& callable, Arg0&& arg0, Arg1&& arg1) {
      static_assert(std::invocable<Callable&&, Arg0&&, Arg1&&> || std::invocable<Callable&&, Arg0&&>);
      if constexpr (std::invocable<Callable&&, Arg0&&, Arg1&&>)
        callable(arg0, arg1);
      else if constexpr (std::invocable<Callable&&, Arg0&&>)
        callable(arg0);
    };

  } // namespace Impl

  /** \addtogroup Tree Traversal
   *  \ingroup TypeTree
   *  \{
   */

#ifndef DOXYGEN
  /// A functor with no operation
  struct NoOp
  {
    template<class... T>
    constexpr void operator()(T&&...) const { /* do nothing */ }
  };
#endif

  /**
   * @brief Traverse each child of a tree and apply a callable function.
   *
   * This function iterates over each child node of a given tree and
   * applies a callable function to each child. The callable function can accept
   * either one or two arguments: the child node itself, and optionally, its index.
   *
   * @tparam Tree The type of the tree container to be traversed. This must
   *                   satisfy the Concept::InnerTreeNode concept.
   * @tparam Callable  The type of the callable function (functor) to be applied
   *                   to each child node. This can be a lambda, function pointer,
   *                   or any callable object.
   *
   * @param container The tree container whose children will be traversed.
   * @param at_value  The callable function to be applied to each child node.
   *                  This function can accept either one argument (the child node)
   *                  or two arguments (the child node and its index).
   */
  template<Concept::InnerTreeNode Tree, class Callable>
  constexpr void forEachChild(Tree&& container, Callable&& at_value)
  {
    if constexpr (Concept::UniformInnerTreeNode<Tree>)
      for (std::size_t i = 0; i != container.degree(); ++i)
        Impl::invokeWithTwoOrOneArg(at_value, std::forward<Tree>(container).child(i), i);
    else
      Dune::unpackIntegerSequence(
        [&](auto... i) { (Impl::invokeWithTwoOrOneArg(at_value, std::forward<Tree>(container).child(i), i), ...); },
        std::make_index_sequence<std::remove_cvref_t<Tree>::degree()>{});
  }

  namespace Impl {

    /* Traverse tree and visit each node. The signature is the same
     * as for the public forEachNode function in Dune::Typtree,
     * despite the additionally passed treePath argument. The path
     * passed here is associated to the tree and the relative
     * paths of the children (wrt. to tree) are appended to this.
     * Hence the behavior of the public function is resembled
     * by passing an empty treePath.
     */
    template<Concept::TreeNode Tree, class TreePath, class PreFunc, class LeafFunc, class PostFunc>
    void forEachNode(Tree&& tree, TreePath treePath, PreFunc&& preFunc, LeafFunc&& leafFunc, PostFunc&& postFunc)
    {
      if constexpr(Concept::LeafTreeNode<std::decay_t<Tree>>) {
        Impl::invokeWithTwoOrOneArg(leafFunc, tree, treePath);
      } else {
        Impl::invokeWithTwoOrOneArg(preFunc, tree, treePath);
        forEachChild(
          tree,
          [&]<class Child>(Child&& child, auto i) {
            forEachNode(
              std::forward<Child>(child),
              push_back(treePath, i),
              preFunc,
              leafFunc,
              postFunc
            );
          });
        Impl::invokeWithTwoOrOneArg(postFunc, tree, treePath);
      }
    }

  } // namespace Impl


  // ********************************************************************************
  // Public Interface
  // ********************************************************************************

  /**
   * \brief Traverse tree and visit each node
   *
   * This does an in-order traversal of the tree.
   * For leaf node the leafNodeFunc callback is called.
   * For each inner node this function first calls the preNodeFunc,
   * then it traverses the children, and then it calls the postNodeFunc.
   * All callback functions are called with the
   * node and corresponding treepath as arguments.
   *
   * \param tree The tree to traverse
   * \param preNodeFunc This function is called for each inner node
   * \param leafNodeFunc This function is called for each leaf node
   * \param postNodeFunc This function is called for each inner node
   */
  template<class Tree, class PreNodeFunc, class LeafNodeFunc, class PostNodeFunc>
  void forEachNode(Tree&& tree, PreNodeFunc&& preNodeFunc, LeafNodeFunc&& leafNodeFunc, PostNodeFunc&& postNodeFunc)
  {
    Impl::forEachNode(tree, treePath(), preNodeFunc, leafNodeFunc, postNodeFunc);
  }

  /**
   * \brief Traverse tree and visit each node
   *
   * The passed callback function is called with the
   * node and corresponding treepath as arguments.
   *
   * \param tree The tree to traverse
   * \param nodeFunc This function is called for each node
   */
  template<Concept::TreeNode Tree, class NodeFunc>
  void forEachNode(Tree&& tree, NodeFunc&& nodeFunc)
  {
    forEachNode(tree, nodeFunc, nodeFunc, NoOp{});
  }

  /**
   * \brief Traverse tree and visit each leaf node
   *
   * The passed callback function is called with the
   * node and corresponding treepath as arguments.
   *
   * \param tree The tree to traverse
   * \param leafFunc This function is called for each leaf node
   */
  template<Concept::TreeNode Tree, class LeafFunc>
  void forEachLeafNode(Tree&& tree, LeafFunc&& leafFunc)
  {
    forEachNode(tree, NoOp{}, leafFunc, NoOp{});
  }

  //! \} group Tree Traversal

} //namespace Dune::TypeTree

#endif // DUNE_COMMON_TYPETREE_TRAVERSAL_HH
