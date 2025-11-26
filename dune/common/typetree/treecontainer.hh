// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception OR LGPL-3.0-or-later

#ifndef DUNE_COMMON_TYPETREE_TREECONTAINER_HH
#define DUNE_COMMON_TYPETREE_TREECONTAINER_HH

#include <type_traits>
#include <utility>
#include <functional>
#include <array>

#include <dune/common/indices.hh>
#include <dune/common/hybridutilities.hh>
#include <dune/common/rangeutilities.hh>
#include <dune/common/tuplevector.hh>

#include <dune/common/typetree/nodeconcepts.hh>
#include <dune/common/typetree/treepath.hh>

namespace Dune::TypeTree {

  namespace Impl {

    /**
     * \brief A factory class creating a hybrid container compatible with a type tree
     *
     * This class allows to create a nested hybrid container having the same structure
     * as a given type tree. Power nodes are represented as std::array's while composite
     * nodes are represented as Dune::TupleVector's. The stored values for the leaf nodes
     * are creating using a given predicate. Once created, the factory provides an
     * operator() creating the container for the tree given as argument.
     *
     * \tparam LeafToValue Type of a predicate that determines the stored values at the leafs
     */
    template<class LeafToValue>
    class ContainerFactory
    {
    public:

      /**
       * \brief Create ContainerFactory
       *
       * The given predicate will be stored by value.
       *
       * \param leafToValue A predicate used to generate the stored values for the leaves
       */
      ContainerFactory(LeafToValue leafToValue) :
        leafToValue_(leafToValue)
      {}

      template<class Node>
      requires Dune::TypeTree::Concept::TreeNode<Node>
      auto operator()(const Node& node)
      {
        if constexpr (Dune::TypeTree::Concept::LeafTreeNode<Node>)
          return leafToValue_(node);
        else
        {
          if constexpr (Dune::TypeTree::Concept::UniformInnerTreeNode<Node>)
          {
            if constexpr (Dune::TypeTree::Concept::StaticDegreeInnerTreeNode<Node>)
            {
              return Dune::unpackIntegerSequence([&](auto... indices) {
                  return std::array{(*this)(node.child(indices))...};
                }, std::make_index_sequence<std::size_t(Node::degree())>());
            }
            else
            {
              using TransformedChild = decltype((*this)(node.child(0)));
              std::vector<TransformedChild> container;
              container.reserve(node.degree());
              for (std::size_t i = 0; i < node.degree(); ++i)
                container.emplace_back((*this)(node.child(i)));
              return container;
            }
          }
          else
          {
            return Dune::unpackIntegerSequence([&](auto... indices) {
                return Dune::makeTupleVector((*this)(node.child(indices))...);
              }, std::make_index_sequence<std::size_t(Node::degree())>());
          }
        }
      }

    private:
      LeafToValue leafToValue_;
    };


    /*
     * \brief Wrap nested container to provide a VectorBackend
     */
    template<class Container>
    class TreeContainerVectorBackend
    {
      template<class C>
      static constexpr decltype(auto) accessByTreePath(C&& container, const TreePath<>& path)
      {
        return container;
      }

      template<class C, class... T>
      static constexpr decltype(auto) accessByTreePath(C&& container, const TreePath<T...>& path)
      {
        auto head = path[Dune::Indices::_0];
        auto tailPath = Dune::unpackIntegerSequence([&](auto... i){
                      return treePath(path[Dune::index_constant<i+1>{}]...);
                    }, std::make_index_sequence<sizeof...(T)-1>());
        return accessByTreePath(container[head], tailPath);
      }


      template<class C, class Tree>
      static void recursiveResize(C& container, const Tree& tree)
      {
        if constexpr (not Dune::TypeTree::Concept::LeafTreeNode<Tree>)
        {
          if constexpr (requires { container.resize(0u); })
            container.resize(tree.degree());
          Dune::Hybrid::forEach(Dune::range(tree.degree()), [&](auto i) {
            recursiveResize(container[i], tree.child(i));
          });
        }
      }

    public:
      //! Move the passed container into the internal storage
      TreeContainerVectorBackend(Container&& container) :
        container_(std::move(container))
      {}

      //! Default construct the container and perform a resize depending on the tree-node degrees.
      template <class Tree>
      requires Dune::TypeTree::Concept::TreeNode<Tree>
      TreeContainerVectorBackend(const Tree& tree) :
        TreeContainerVectorBackend()
      {
        this->resize(tree);
      }

      //! Default constructor. The stored container might need to be resized before usage.
      template <class C = Container,
        std::enable_if_t<std::is_default_constructible_v<C>, bool> = true>
      TreeContainerVectorBackend() :
        container_()
      {}

      template<class... T>
      decltype(auto) operator[](const TreePath<T...>&  path) const
      {
        return accessByTreePath(container_, path);
      }

      template<class... T>
      decltype(auto) operator[](const TreePath<T...>&  path)
      {
        return accessByTreePath(container_, path);
      }

      //! Resize the (nested) container depending on the degree of the tree nodes
      template<class Tree>
      requires Dune::TypeTree::Concept::TreeNode<Tree>
      void resize(const Tree& tree)
      {
        recursiveResize(container_, tree);
      }

      const Container& data() const
      {
        return container_;
      }

      Container& data()
      {
        return container_;
      }

    private:
      Container container_;
    };

    template<class Container>
    auto makeTreeContainerVectorBackend(Container&& container)
    {
      return TreeContainerVectorBackend<std::decay_t<Container>>(std::forward<Container>(container));
    }

    /*
     * \brief A simple lambda for creating default constructible values from a node
     *
     * This simply returns LeafToValue<Node>{} for a given Node. It's needed
     * because using a lambda expression in a using declaration is not allowed
     * because it's an unevaluated context.
     */
    template<template<class Node> class LeafToValue>
    struct LeafToDefaultConstructibleValue
    {
      template<class Node>
      auto operator()(const Node& node) const
      {
        return LeafToValue<Node>{};
      }
    };

  } // namespace Impl

  /** \addtogroup TypeTree
   *  \{
   */

  /**
   * \brief Create container having the same structure as the given tree
   *
   * This class allows to create a nested hybrid container having the same structure
   * as a given type tree. Power nodes are represented as std::array's while composite
   * nodes are represented as Dune::TupleVector's. The stored values for the leaf nodes
   * are creating using a given predicate. For convenience the created container is
   * not returned directly. Instead, the returned object stores the container and
   * provides operator[] access using a TreePath.
   *
   * \param tree The tree which should be mapper to a container
   * \param leafToValue A predicate used to generate the stored values for the leaves
   *
   * \returns A container matching the tree structure
   */
  template<class Tree, class LeafToValue>
  auto makeTreeContainer(const Tree& tree, LeafToValue&& leafToValue)
  {
    auto f = std::ref(leafToValue);
    auto factory = Impl::ContainerFactory<decltype(f)>(f);
    return Impl::makeTreeContainerVectorBackend(factory(tree));
  }

  /**
   * \brief Create container having the same structure as the given tree
   *
   * This class allows to create a nested hybrid container having the same structure
   * as a given type tree. Power nodes are represented as std::array's while composite
   * nodes are represented as Dune::TupleVector's. The stored values for the leaf nodes
   * are of the given type Value. For convenience the created container is
   * not returned directly. Instead, the returned object stores the container and
   * provides operator[] access using a TreePath.
   *
   * \tparam Value Type of the values to be stored for the leafs. Should be default constructible.
   * \param tree The tree which should be mapper to a container
   *
   * \returns A container matching the tree structure
   */
  template<class Value, class Tree>
  auto makeTreeContainer(const Tree& tree)
  {
    return makeTreeContainer(tree, [](const auto&) {return Value{};});
  }

  /**
   * \brief Alias to container type generated by makeTreeContainer for given tree type and uniform value type
   */
  template<class Value, class Tree>
  using UniformTreeContainer = std::decay_t<decltype(makeTreeContainer<Value>(std::declval<const Tree&>()))>;

  /**
   * \brief Alias to container type generated by makeTreeContainer for give tree type and when using LeafToValue to create values
   */
  template<template<class Node> class LeafToValue, class Tree>
  using TreeContainer = std::decay_t<decltype(makeTreeContainer(std::declval<const Tree&>(), std::declval<Impl::LeafToDefaultConstructibleValue<LeafToValue>>()))>;

  //! \} group TypeTree

} //namespace Dune::TypeTree

#endif // DUNE_COMMON_TYPETREE_TREECONTAINER_HH
