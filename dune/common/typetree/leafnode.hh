// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_LEAFNODE_HH
#define DUNE_COMMON_TYPETREE_LEAFNODE_HH

#include <dune/common/typetree/nodetags.hh>
#include <cstddef>

namespace Dune {
  namespace TypeTree {

    /** \addtogroup Nodes
     *  \ingroup TypeTree
     *  \{
     */

    /** \brief Base class for leaf nodes in a \ref TypeTree.
     *
     *  Every leaf type in a \ref TypeTree must be derived from this
     *  class.
     */

    class LeafNode
    {

    public:

      //! Mark this class as a leaf in a \ref TypeTree.
      static const bool isLeaf = true;

      //! Mark this class as a non power in the \ref TypeTree.
      static const bool isPower = false;

      //! Mark this class as a non composite in the \ref TypeTree.
      static const bool isComposite = false;

      //! Leafs have no children.
      static const std::size_t CHILDREN = 0;

      //! The type tag that describes a LeafNode.
      typedef LeafNodeTag NodeTag;

      static constexpr std::size_t degree()
      {
        return 0;
      }

    protected:

      //! Default constructor.
      /**
       * The default constructor is protected, as LeafNode is a utility
       * class that needs to be filled with meaning by subclassing it
       * and adding useful functionality to the subclass.
       */
      LeafNode() {}
    };

    //! \} group Nodes

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_COMMON_TYPETREE_POWERNODE_HH
