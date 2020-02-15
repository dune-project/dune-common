// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_TREECONTAINER_HH
#define DUNE_COMMON_TYPETREE_TREECONTAINER_HH

#include <type_traits>
#include <utility>
#include <functional>
#include <array>

#include <dune/common/indices.hh>
#include <dune/common/tuplevector.hh>
#include <dune/common/typetree/treepath.hh>

namespace Dune {
  namespace TypeTree {

    namespace Impl {

      /*
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
         * \param A predicate used to generate the stored values for the leaves
         */
        ContainerFactory(LeafToValue leafToValue) :
          leafToValue_(std::move(leafToValue))
        {}

        template<class Node>
        auto operator()(const Node& node)
        {
          if constexpr (Node::isLeaf)
          {
            return leafToValue_(node);
          }
          else if constexpr (Node::isPower)
          {
            using TransformedChild = decltype((*this)(node.child(0)));
            return std::array<TransformedChild, Node::degree()>();
          }
          else if constexpr (Node::isComposite)
          {
            return Dune::unpackIntegerSequence([&](auto... indices) {
                return Dune::makeTupleVector((*this)(node.child(indices))...);
              }, std::make_index_sequence<Node::degree()>());
          }
          else
          {
            return 0;
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
        static constexpr decltype(auto) accessByTreePath(C&& container, const HybridTreePath<>& path)
        {
          return container;
        }

        template<class C, class... T>
        static constexpr decltype(auto) accessByTreePath(C&& container, const HybridTreePath<T...>& path)
        {
          auto head = path[Dune::Indices::_0];
          auto tailPath = Dune::unpackIntegerSequence([&](auto... i){
                        return treePath(path[i+1]...);
                      }, std::make_index_sequence<sizeof...(T)-1>());
          return accessByTreePath(container[head], tailPath);
        }

      public:
        TreeContainerVectorBackend() = default;

        TreeContainerVectorBackend(Container&& container) :
          container_(std::move(container))
        {}

        template<class... T>
        decltype(auto) operator[](const HybridTreePath<T...>&  path) const
        {
          return accessByTreePath(container_, path);
        }

        template<class... T>
        decltype(auto) operator[](const HybridTreePath<T...>&  path)
        {
          return accessByTreePath(container_, path);
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

    } // end namespace Impl


    /** \addtogroup TypeTree
     *  \{
     */

    /**
     * \brief Create container havin the same structure as the given tree
     *
     * This class allows to create a nested hybrid container having the same structure
     * as a given type tree. Power nodes are represented as std::array's while composite
     * nodes are represented as Dune::TupleVector's. The stored values for the leaf nodes
     * are creating using a given predicate. For convenience the created container is
     * not returned directly. Instead, the returned object stores the container and
     * provides operator[] access using a HybridTreePath.
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
     * \brief Create container havin the same structure as the given tree
     *
     * This class allows to create a nested hybrid container having the same structure
     * as a given type tree. Power nodes are represented as std::array's while composite
     * nodes are represented as Dune::TupleVector's. The stored values for the leaf nodes
     * are of the given type Value. For convenience the created container is
     * not returned directly. Instead, the returned object stores the container and
     * provides operator[] access using a HybridTreePath.
     *
     * \tparam Value Type of the values to be stored for the leafs. Should be default constructible.
     * \param leafToValue A predicate used to generate the stored values for the leaves
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

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_COMMON_TYPETREE_TREECONTAINER_HH
