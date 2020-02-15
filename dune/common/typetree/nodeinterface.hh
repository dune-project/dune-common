// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=8 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_NODEINTERFACE_HH
#define DUNE_COMMON_TYPETREE_NODEINTERFACE_HH

#include <cstddef>
#include <type_traits>

#include <dune/common/documentation.hh>

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
      //! Whether this is a leaf node in a \ref TypeTree.
      static const bool isLeaf = implementationDefined;

      //! Whether this is a power node in the \ref TypeTree.
      static const bool isPower = implementationDefined;

      //! Whether this is a composite node in the \ref TypeTree.
      static const bool isComposite = implementationDefined;

      //! Number of children of this node in the \ref TypeTree
      static const std::size_t CHILDREN = implementationDefined;

      //! The type tag that describes what kind of node this is
      /**
       * One of LeafNodeTag, PowerNodeTag or CompositeNodeTag.
       * Other tags are also possible when new
       * kinds of nodes are defined.
       */
      typedef ImplementationDefined NodeTag;

      //! container type to pass around a collection of children
      /**
       * \note This typedef is not present for leaf nodes.
       */
      typedef ImplementationDefined NodeStorage;
    };

    //! Returns the node tag of the given Node.
    template<typename Node>
    using NodeTag = typename std::decay_t<Node>::NodeTag;

    //! Returns the implementation tag of the given Node.
    template<typename T>
    using ImplementationTag = typename std::decay_t<T>::ImplementationTag;


    //! Returns the degree of node as run time information.
    template<typename Node>
    std::size_t degree(const Node& node)
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
    template<typename Node, typename NodeTag>
    constexpr std::size_t degree(const Node* node, NodeTag)
    {
      return Node::degree();
    }

#endif // DOXYGEN

    //! Returns the statically known degree of the given Node type as a std::integral_constant.
    /**
     * \note If you are only interested in the numeric value, take a look at staticDegree<Node>
     *       instead.
     */
    template<typename Node>
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
