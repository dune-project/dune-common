// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_TREECONTAINER_HH
#define DUNE_COMMON_TYPETREE_TREECONTAINER_HH

#include <array>
#include <functional>
#include <type_traits>
#include <utility>

#include <dune/common/indices.hh>
#include <dune/common/tuplevector.hh>
#include <dune/common/typetree/treepath.hh>

namespace Dune {
  namespace TypeTree {

    namespace Impl {

      /**
       * \brief A factory class creating a hybrid container compatible with a type tree
       *
       * This class allows to create a nested hybrid container having the same structure
       * as a given type tree. Power nodes are represented as std::array's while composite
       * nodes are represented as Dune::TupleVector's. The stored values for the leaf nodes
       * are created using a given functor. Once created, the factory provides an
       * operator() creating the container for the tree given as argument.
       *
       * \tparam LeafToValue Type of a functor that determines the stored values at the leafs
       */
      template<class LeafToValue>
      class ContainerFactory
      {
      public:

        /**
         * \brief Create ContainerFactory
         *
         * The given functor will be stored by value.
         *
         * \param leafToValue  A functor used to generate the stored values for the leaves
         */
        ContainerFactory (LeafToValue leafToValue) :
          leafToValue_(std::move(leafToValue))
        {}

        template<class Node>
        auto operator() (const Node& node) const
        {
          if constexpr (Node::isLeaf)
            return leafToValue_(node);
          else
          if constexpr (Node::isPower) {
            using TransformedChild = decltype((*this)(node.child(0)));
            return std::array<TransformedChild, Node::degree()>();
          }
          else
          if constexpr (Node::isComposite) {
            auto indices = std::make_index_sequence<Node::degree()>{};
            return Dune::unpackIntegerSequence(
              [&](auto... i) { return Dune::makeTupleVector((*this)(node.child(i))...); }, indices);
          }
          else {
            static_assert(Node::isLeaf || Node::isPower || Node::isComposite,
              "Node must be one of leaf,power,composite.");
            return;
          }
        }

      private:
        LeafToValue leafToValue_;
      };


      /**
       * \brief Wrap nested container to provide a VectorBackend
       */
      template<class Container>
      class TreeContainerVectorBackend
      {
        template<class C>
        static constexpr decltype(auto) accessByTreePath (C&& container, const HybridTreePath<>& path)
        {
          return container;
        }

        template<class C, class... T>
        static constexpr decltype(auto) accessByTreePath (C&& container, const HybridTreePath<T...>& path)
        {
          auto head = path[Dune::Indices::_0];
          auto indices = std::make_index_sequence<sizeof...(T)-1>();
          auto tailPath = Dune::unpackIntegerSequence(
            [&](auto... i){ return treePath(path[i+1]...); }, indices);
          return accessByTreePath(container[head], tailPath);
        }

      public:
        /// \brief Create the container by default construction of the underlying
        /// container type
        TreeContainerVectorBackend () = default;

        /// \brief Create the container by initialization from the given container
        /// created in the \ref ContainerFactory
        TreeContainerVectorBackend (Container&& container) :
          container_(std::move(container))
        {}

        /// \brief Access a const entry in the tree container by given treepath
        template<class... T>
        decltype(auto) operator[] (const HybridTreePath<T...>&  path) const
        {
          return accessByTreePath(container_, path);
        }

        /// \brief Access an entry in the tree container by given treepath
        template<class... T>
        decltype(auto) operator[] (const HybridTreePath<T...>&  path)
        {
          return accessByTreePath(container_, path);
        }

        /// \brief Obtain the underlying const-container
        const Container& data () const
        {
          return container_;
        }

        /// \brief Obtain the underlying container
        Container& data ()
        {
          return container_;
        }

      private:
        Container container_;
      };

      /// \brief Generator functor for \ref TreeContainerVectorBackend
      template<class C>
      auto makeTreeContainerVectorBackend (C&& container)
      {
        return TreeContainerVectorBackend<std::decay_t<C>>(std::forward<C>(container));
      }


      /**
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
        auto operator() (const Node& node) const
        {
          return LeafToValue<Node>{};
        }
      };

    } // end namespace Impl


    /** \addtogroup TypeTree
     *  \{
     */

    /**
     * \brief Create container having the same structure as the given tree
     *
     * This class allows to create a nested hybrid container having the same structure
     * as a given type tree. Power nodes are represented as std::array's while composite
     * nodes are represented as Dune::TupleVector's. The stored values for the leaf nodes
     * are creating using a given functor. For convenience the created container is
     * not returned directly. Instead, the returned object stores the container and
     * provides operator[] access using a HybridTreePath.
     *
     * \param tree The tree which should be mapper to a container
     * \param leafToValue A functor used to generate the stored values for the leaves
     *
     * \returns A container matching the tree structure
     */
    template<class Tree, class LeafToValue>
    auto makeTreeContainer (const Tree& tree, LeafToValue&& leafToValue)
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
     * provides operator[] access using a HybridTreePath.
     *
     * \tparam Value  Type of the values to be stored for the leafs. Should be default
     *                constructible.
     * \param tree    The tree which should be mapper to a container
     *
     * \returns A container matching the tree structure
     */
    template<class Value, class Tree>
    auto makeTreeContainer (const Tree& tree)
    {
      return makeTreeContainer(tree, [](const auto&) {return Value{};});
    }

    /**
     * \brief Create container having the same structure as the given tree assuming the
     * LeafToValue type is default constructible.
     **/
    template<template<class Node> class LeafToValue, class Tree>
    auto makeTreeContainer (const Tree& tree)
    {
      return makeTreeContainer(tree, Impl::LeafToDefaultConstructibleValue<LeafToValue>{});
    }


    /**
     * \brief Alias to container type generated by makeTreeContainer for given tree type
     * and uniform value type.
     */
    template<class Value, class Tree>
    using UniformTreeContainer = decltype(makeTreeContainer<Value>(std::declval<const Tree&>()));

    /**
     * \brief Alias to container type generated by makeTreeContainer for give tree type
     * and when using LeafToValue to create values.
     */
    template<template<class Node> class LeafToValue, class Tree>
    using TreeContainer = decltype(makeTreeContainer<LeafToValue>(std::declval<const Tree&>()));

    //! \} group TypeTree

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_COMMON_TYPETREE_TREECONTAINER_HH
