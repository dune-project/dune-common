// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception OR LGPL-3.0-or-later

#ifndef DUNE_COMMON_TYPETREE_NODECONCEPTS_HH
#define DUNE_COMMON_TYPETREE_NODECONCEPTS_HH

#include <dune/common/typetraits.hh>
#include <dune/common/indices.hh>

#include <concepts>
#include <type_traits>
#include <utility>


namespace Dune::TypeTree::Concept {

  namespace Impl {

    template<class Node>
    concept HasConstExprDegree = requires(Node node)
    {
      std::bool_constant<(std::remove_cvref_t<Node>::degree(), true)>();
    };

    template<class Node>
    concept HasPositiveDegree = ( std::remove_cvref_t<Node>::degree() > 0);

  }

  //!@brief Model of a node of a typetree
  template<class Node>
  concept TreeNode = requires(Node node)
  {
    { std::as_const(node).degree() } -> std::convertible_to<std::size_t>;
  };

  //!@brief Model of an inner node of a typetree with compile time known degree and child access via index_constant
  template<class Node>
  concept StaticDegreeInnerTreeNode = TreeNode<Node> && requires(Node node)
  {
    std::remove_cvref_t<Node>::degree();
    requires IsCompileTimeConstant<decltype(std::remove_cvref_t<Node>::degree())>::value;
    requires Impl::HasConstExprDegree<Node> and Impl::HasPositiveDegree<Node>;
    node.child(index_constant<0>());
    node.child(index_constant<(std::remove_cvref_t<Node>::degree()-1)>());
  };

  //!@brief Model of an inner node of a typetree with uniform nodes accessible via runtime index
  template<class Node>
  concept UniformInnerTreeNode = TreeNode<Node> && requires(Node node, std::size_t index)
  {
    node.child(index);
  };

  //!@brief Model of an inner tree node of a typetree (either static degree or uniform)
  template<class Node>
  concept InnerTreeNode = StaticDegreeInnerTreeNode<Node> || UniformInnerTreeNode<Node>;

  //!@brief Model of a leaf tree node of a typetree
  template<class Node>
  concept LeafTreeNode = TreeNode<Node> && not InnerTreeNode<Node>;

} // namespace Dune::TypeTree::Concept

#endif // DUNE_COMMON_TYPETREE_NODECONCEPTS_HH
