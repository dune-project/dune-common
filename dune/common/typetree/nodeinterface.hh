// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=8 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_NODEINTERFACE_HH
#define DUNE_COMMON_TYPETREE_NODEINTERFACE_HH

#include <cstddef>
#include <type_traits>

#include <dune/common/documentation.hh>
#include <dune/common/typetree/nodetags.hh>

namespace Dune {
  namespace TypeTree {

    /** \addtogroup Nodes
     *  \ingroup TypeTree
     *  \{
     */

    /** \brief Interface for nodes in a \ref TypeTree.
     *
     * This class cannot be used itself, it is for documentation purposes
     * only.
     *
     * \note Constructor signatures are explicitly not specified by this
     *       interface.
     * \note In addition, every node in a tree must be derived from one of
     *       the node base classes LeafNode, PowerNode, or CompositeNode,
     *       or from a base class for a yet-to-be-defined new node type.
     */
    struct NodeInterface
    {
      //! The type tag that describes what kind of node this is
      /**
       * One of LeafNodeTag, PowerNodeTag or CompositeNodeTag.
       * Other tags are also possible when new
       * kinds of nodes are defined.
       */
      typedef ImplementationDefined NodeTag;
    };

    //! Returns the degree of node as run time information.
    template<class Node>
    std::size_t degree (const Node& node)
    {
      return degree(&node,NodeTag<Node>());
    }

#ifndef DOXYGEN

    //! Default implementation of degree dispatch function.
    /**
     * This dispatches using a pointer to the node instead of a reference,
     * as we can easily create a constexpr pointer to the node, while a constexpr
     * reference might not even be possible to manufacture (std::declval is not
     * constexpr).
     */
    template<class Node, class NodeTag>
    constexpr std::size_t degree (const Node* node, NodeTag)
    {
      return Node::degree();
    }

#endif // DOXYGEN

    //! Returns the statically known degree of the given Node type as a std::integral_constant.
    template<class Node>
    using StaticDegree = std::integral_constant<
      std::size_t,
      degree(
        static_cast<std::decay_t<Node>*>(nullptr),
        NodeTag<std::decay_t<Node>>()
        )
      >;

    //! \} group Nodes

  } // namespace TypeTree
} //namespace Dune

#endif //  DUNE_COMMON_TYPETREE_NODEINTERFACE_HH
