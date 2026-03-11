// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=8 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception OR LGPL-3.0-or-later

#ifndef DUNE_COMMON_TYPETREE_CHILDACCESS_HH
#define DUNE_COMMON_TYPETREE_CHILDACCESS_HH

#include <cstddef>
#include <type_traits>
#include <utility>
#include <tuple>

#include <dune/common/indices.hh>
#include <dune/common/typelist.hh>
#include <dune/common/typetraits.hh>

#include <dune/common/hybridmultiindex.hh>
#include <dune/common/typetree/nodeconcepts.hh>
#include <dune/common/typetree/treepath.hh>


namespace Dune::TypeTree {

  //! \addtogroup TypeTreeChildExtraction Child Extraction
  //! Utility functions and metafunctions for extracting children from a TypeTree.
  //! \ingroup TypeTree
  //! \{

  //! Extracts the child of a node given by a TreePath object.
  /**
   * Use this function to extract a (possibly indirect) child of a TypeTree node.
   *
   * Example:
   *
   * \code{.cc}
   * using namespace Dune::Indices; // for compile-time indices
   * auto tp = Dune::TypeTree::TreePath(_4,2,_0,1);
   * auto&& c = child(node,tp);
   * \endcode
   *
   * returns the second child of the first child of the third child
   * of the fifth child of node, where some child lookups were done using
   * a compile-time index and some using a run-time index.
   *
   * \param node        The node from which to extract the child.
   * \param treePath    A TreePath that describes the path into the tree to the
   *                    wanted child. This tree path object  can be a combination of run time indices
   *                    (for tree nodes that allow accessing their children using run time information,
   *                    like PowerNode) and instances of index_constant, which work for all types of inner
   *                    nodes.
   * \return            A reference to the child, its cv-qualification depends on the passed-in node.
   */
  template<typename Node, typename... Indices>
  decltype(auto) child (Node&& node, TreePath<Indices...> treePath)
  {
    if constexpr (sizeof...(Indices) == 0)
      return std::forward<Node>(node);
    else
    {
      using I0 = std::tuple_element_t<0, TreePath<Indices...>>;
      if constexpr (Dune::IsIntegralConstant<I0>::value and Concept::StaticDegreeInnerTreeNode<Node>)
        static_assert(I0::value < std::decay_t<Node>::degree(), "Child index out of range");
      else
        assert(std::size_t(treePath.front()) < node.degree() && "Child index out of range");
      if constexpr (sizeof...(Indices) == 1)
        return node.child(treePath.front());
      else
        return child(node.child(treePath.front()), pop_front(treePath));
    }
  }

  //! Extracts the child of a node given by a sequence of compile-time and run-time indices.
  /**
   * Use this function to extract a (possibly indirect) child of a TypeTree node.
   *
   * Example:
   *
   * \code{.cc}
   * using namespace Dune::Indices;   // for compile-time indices
   * auto&& c = child(node,_4,2,_0,1);
   * \endcode
   *
   * returns the second child of the first child of the third child
   * of the fifth child of node, where some child lookups were done using
   * a compile-time index and some using a run-time index.
   *
   * \param node        The node from which to extract the child.
   * \param indices     A list of indices that describes the path into the tree to the
   *                    wanted child. These parameters can be a combination of run time indices
   *                    (for tree nodes that allow accessing their children using run time information,
   *                    like PowerNode) and instances of index_constant, which work for all types of inner
   *                    nodes.
   * \return            A reference to the child, its cv-qualification depends on the passed-in node.
   */
  template<typename Node, typename... Indices>
  decltype(auto) child (Node&& node, Indices... indices)
  {
    return child(node, Dune::HybridMultiIndex{indices...});
  }

  namespace Impl {

    // We could directly implement Child as
    //
    //   using Child = std::decay_t<decltype(child(std::declval<Node>(), Dune::index_constant<indices>()...))>;
    //
    // but this triggers an internal compiler error in
    // gcc 11, 12, and 13 while it does work with gcc 10
    // and 14 and clang. This can be avoided by extracting
    // this into a traits class.
    template<typename Node, std::size_t... indices>
    struct ChildTraits
    {
      using type = std::decay_t<decltype(child(std::declval<Node>(), Dune::index_constant<indices>()...))>;
    };

  }

  //! Template alias for the type of a child node given by a list of child indices.
  /**
   * This template alias is implemented in terms of the free-standing child() functions and uses those
   * in combination with decltype() to extract the child type.
   *
   * \tparam Node     The type of the parent node.
   * \tparam indices  A list of index values the describes the path to the wanted child.
   */
  template<typename Node, std::size_t... indices>
  using Child = typename Impl::ChildTraits<Node, indices...>::type;

  //! Template alias for the type of a child node given by a TreePath type.
  /**
   * This template alias is implemented in terms of the free-standing child() functions and uses those
   * in combination with decltype() to extract the child type.
   *
   * \tparam Node      The type of the parent node.
   * \tparam TreePath  The type of a TreePath that describes the path to the wanted child.
   */
  template<typename Node, typename TreePath>
  using ChildForTreePath = std::decay_t<decltype(child(std::declval<Node>(), std::declval<TreePath>()))>;



  namespace Impl {

    template<class N>
    static constexpr auto childTypes()
    {
      if constexpr (Dune::TypeTree::Concept::StaticDegreeInnerTreeNode<N>)
      {
        return Dune::unpackIntegerSequence([&](auto... i) {
          return Dune::MetaType<std::tuple<Dune::TypeTree::Child<N, i>...>>{};
        }, std::make_index_sequence<N::degree()>{});
      }
      else
        return Dune::MetaType<void>{};
    }

    //! Template alias to extract the types of direct children of a node
    /**
     * For a node satisfying the \ref StaticDegreeInnerTreeNode concept,
     * this alias provides a tuple of the types of the direct children.
     * For nodes not satisfying the concept this is an alias for void.
     * In these cases the node is either a leaf node and has no children
     * or has a dynamic degree such that all children have the same
     * type available as `Child<Node,0>`.
     *
     * \tparam Node     The type of the parent node.
     */
    template<class N>
    using Children = typename decltype(Impl::childTypes<N>())::type;

  }

  //! \} group TypeTree

} //namespace Dune::TypeTree

#endif // DUNE_COMMON_TYPETREE_CHILDACCESS_HH
