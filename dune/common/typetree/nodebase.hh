// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_NODEBASE_HH
#define DUNE_COMMON_TYPETREE_NODEBASE_HH

#include <dune/common/typetree/nodetags.hh>

namespace Dune {
  namespace TypeTree {

    /** \addtogroup Nodes
     *  \ingroup TypeTree
     *  \{
     */

    template <class Derived>
    class NodeBase
    {
    public:
      static constexpr bool isLeaf = TypeTree::isLeaf<Derived>;
      static constexpr bool isPower = TypeTree::isPower<Derived>;
      static constexpr bool isComposite = TypeTree::isComposite<Derived>;
    };

    //! \} group Nodes

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_COMMON_TYPETREE_NODETAGS_HH
