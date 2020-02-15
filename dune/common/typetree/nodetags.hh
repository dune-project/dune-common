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

#ifndef DOXYGEN

    //! Special tag used as start value in algorithms.
    struct StartTag {};

#endif // DOXYGEN

    //! \} group Nodes

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_COMMON_TYPETREE_NODETAGS_HH
