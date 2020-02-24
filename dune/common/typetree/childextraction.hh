// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=8 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_CHILDEXTRACTION_HH
#define DUNE_COMMON_TYPETREE_CHILDEXTRACTION_HH

#include <utility>

#include <dune/common/concept.hh>
#include <dune/common/documentation.hh>
#include <dune/common/shared_ptr.hh>
#include <dune/common/typetraits.hh>

#include <dune/common/typetree/nodeinterface.hh>
#include <dune/common/typetree/nodetags.hh>
#include <dune/common/typetree/treepath.hh>


namespace Dune {
  namespace TypeTree {

    //! \addtogroup TypeTreeChildExtraction Child Extraction
    //! Utility functions and metafunctions for extracting children from a TypeTree.
    //! \ingroup TypeTree
    //! \{

#ifndef DOXYGEN

    namespace Impl {

      // forward declaration
      template<class Node>
      struct Extractor;

      // ********************************************************************************
      // end of the recursion, there are no child indices, so just return the node itself
      // ********************************************************************************

      template<class Node>
      decltype(auto) childImpl (Node&& node)
      {
        return std::forward<Node>(node);
      }


      // ********************************************************************************
      // next index is a run-time value
      // ********************************************************************************

      struct HasDynamicChildMethod
      {
        template<class Node>
        auto require (const Node& node) -> decltype(node.child(0u));
      };

      // The actual implemention here overloads on std::size_t.
      template<class Node, class... J,
        std::enable_if_t<Dune::models<HasDynamicChildMethod, Node>(), int> = 0>
      decltype(auto) childImpl (Node&& node, std::size_t i, J... j)
      {
        assert(i < node.degree() && "Child index out of range");
        return Extractor<Node>::child(node.child(i),j...);
      }


      // ********************************************************************************
      // next index is a compile-time constant
      // ********************************************************************************

      // we need a concept to make sure that the node has a templated child()
      // method
      struct HasTemplateChildMethod
      {
        template<class Node>
        auto require (const Node& node) -> decltype(node.template child<0>());
      };

      // The actual implementation is rather simple, we just use an overload that requires
      // the first index to be an index_constant, get the child and then recurse.
      template<class Node, std::size_t i, class... J>
      decltype(auto) childImpl (Node&& node, index_constant<i> ii, J... j)
      {
        if constexpr (Dune::models<HasTemplateChildMethod, Node>() && i < StaticDegree<Node>::value)
          return Extractor<Node>::child(node.child(ii),j...);
        else {
          static_assert(Dune::models<HasTemplateChildMethod, Node>(),
            "Node does not have a template method child()");
          static_assert(i < StaticDegree<Node>::value,
            "Child index out of range");
        }
      }

      // helper struct that forwards to the implementation of child and childStorage
      template<class>
      struct Extractor
      {
        template<class Node, class... Indices>
        static decltype(auto) child (Node&& node, Indices... indices)
        {
          return childImpl(std::forward<Node>(node), indices...);
        }
      };


      // forward to the impl methods by extracting the indices from the treepath
      template<class Node, class... Indices, std::size_t... i>
      decltype(auto) child (Node&& node, HybridTreePath<Indices...> tp, std::index_sequence<i...>)
      {
        return childImpl(std::forward<Node>(node),treePathEntry<i>(tp)...);
      }

    } // end namespace Impl


#endif // DOXYGEN

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
     * \param indices...  A list of indices that describes the path into the tree to the
     *                    wanted child. These parameters can be a combination of run time
     *                    indices (for tree nodes that allow accessing their children using
     *                    run time information, like PowerNode) and instances of index_constant,
     *                    which work for all types of inner nodes.
     * \return            A reference to the child, its cv-qualification depends on the
     *                    passed-in node.
     */
    template<class Node, class... Indices>
#ifdef DOXYGEN
    ImplementationDefined child (Node&& node, Indices... indices)
#else
    decltype(auto) child (Node&& node, Indices... indices)
#endif
    {
      return Impl::childImpl(std::forward<Node>(node),indices...);
    }


    //! Extracts the child of a node given by a HybridTreePath object.
    /**
     * Use this function to extract a (possibly indirect) child of a TypeTree node.
     *
     * Example:
     *
     * \code{.cc}
     * using namespace Dune::TypeTree::Indices; // for compile-time indices
     * auto tp = Dune::TypeTree::hybridTreePath(_4,2,_0,1);
     * auto&& c = child(node,tp);
     * \endcode
     *
     * returns the second child of the first child of the third child
     * of the fifth child of node, where some child lookups were done using
     * a compile-time index and some using a run-time index.
     *
     * \param node        The node from which to extract the child.
     * \param treePath    A HybridTreePath that describes the path into the tree to the
     *                    wanted child. This tree path object  can be a combination of run
     *                    time indices (for tree nodes that allow accessing their children
     *                    using run time information, like PowerNode) and instances of
     *                    index_constant, which work for all types of inner nodes.
     * \return            A reference to the child, its cv-qualification depends on the
     *                    passed-in node.
     */
    template<class Node, class... Indices>
#ifdef DOXYGEN
    ImplementationDefined child (Node&& node, HybridTreePath<Indices...> treePath)
#else
    decltype(auto) child (Node&& node, HybridTreePath<Indices...> tp)
#endif
    {
      return Impl::child(std::forward<Node>(node),tp,std::index_sequence_for<Indices...>{});
    }


#ifndef DOXYGEN

    namespace Impl {

      template<class T>
      struct filter_void
      {
        using type = T;
      };

      template<>
      struct filter_void<void>
      {};

      template<class Node, std::size_t... indices>
      struct ChildImpl
        : public filter_void<std::decay_t<decltype(child(std::declval<Node>(),index_constant<indices>{}...))>>
      {};

    } // end namespace Impl

#endif // DOXYGEN

    //! Template alias for the type of a child node given by a list of child indices.
    /**
     * This template alias is implemented in terms of the free-standing child() functions
     * and uses those in combination with decltype() to extract the child type.

     * \tparam Node     The type of the parent node.
     * \tparam indices  A list of index values the describes the path to the wanted child.
     */
    template<class Node, std::size_t... indices>
    using Child = typename Impl::ChildImpl<Node,indices...>::type;


#ifndef DOXYGEN

    namespace Impl {

      template<class Node, class TreePath>
      struct ChildForTreePathImpl
      {
        using type = std::decay_t<decltype(child(std::declval<Node>(),std::declval<TreePath>()))>;
      };

    } // end namespace Impl

#endif // DOXYGEN

    //! Template alias for the type of a child node given by a TreePath or a HybridTreePath type.
    /**
     * This template alias is implemented in terms of the free-standing child() functions
     * and uses those in combination with decltype() to extract the child type. It supports
     * both TreePath and HybridTreePath.
     *
     * \tparam Node      The type of the parent node.
     * \tparam TreePath  The type of a TreePath or a HybridTreePath that describes the path
     *                   to the wanted child.
     */
    template<class Node, class TreePath>
    using ChildForTreePath = typename Impl::ChildForTreePathImpl<Node,TreePath>::type;


#ifndef DOXYGEN

    namespace Impl {

      // By default, types are flat indices if they are integral
      template<class T>
      struct _is_flat_index
      {
        using type = std::is_integral<T>;
      };

      // And so is any index_constant
      template<std::size_t i>
      struct _is_flat_index<index_constant<i>>
      {
        using type = std::true_type;
      };

    } // end namespace Impl

#endif // DOXYGEN

    //! Type trait that determines whether T is a flat index in the context of child extraction.
    /*
     * This type trait can be used to check whether T is a flat index (i.e. either `std::size_t`
     * or `index_constant`). The type trait normalizes T before doing the check, so it will also
     * work correctly for references and cv-qualified types.
     */
    template<class T>
    using is_flat_index = typename Impl::_is_flat_index<std::decay_t<T>>::type;


#ifndef DOXYGEN

    namespace Impl {

      // helper function for check in member child() functions that tolerates being passed
      // something that isn't a TreePath. It will just return 0 in that case
      template<class T>
      constexpr bool _non_empty_tree_path(const T& t)
      {
        DUNE_UNUSED_PARAMETER(t);
        if constexpr (TypeTree::is_flat_index<T>::value)
          return false;
        else
          return treePathSize(t) > 0;
      }

    } // end namespace Impl

#endif // DOXYGEN

    //! \} group TypeTree

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_COMMON_TYPETREE_CHILDEXTRACTION_HH
