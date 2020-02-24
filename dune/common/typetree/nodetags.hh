// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_NODETAGS_HH
#define DUNE_COMMON_TYPETREE_NODETAGS_HH

namespace Dune {
  namespace TypeTree {

    /** \addtogroup Nodes
     *  \ingroup TypeTree
     *  \{
     */

    //! Tag designating a leaf node.
    struct LeafNodeTag {};

    //! Tag designating a power node.
    struct PowerNodeTag {};

    //! Tag designating a composite node.
    struct CompositeNodeTag {};


    //! Returns the node tag of the given Node.
    template<class Node>
    using NodeTag = typename std::decay_t<Node>::NodeTag;

    template<class T>
    constexpr bool isLeaf = std::is_convertible<NodeTag<T>, LeafNodeTag>::value;

    template<class T>
    constexpr bool isPower = std::is_convertible<NodeTag<T>, PowerNodeTag>::value;

    template<class T>
    constexpr bool isComposite = std::is_convertible<NodeTag<T>, CompositeNodeTag>::value;


#ifndef DOXYGEN

    //! Special tag used as start value in algorithms.
    struct StartTag {};

#endif // DOXYGEN

    //! \} group Nodes

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_COMMON_TYPETREE_NODETAGS_HH
