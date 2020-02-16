// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_TREEINFO_HH
#define DUNE_COMMON_TYPETREE_TREEINFO_HH

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include <dune/common/shared_ptr.hh>
#include <dune/common/indices.hh>
#include <dune/common/typetree/nodeinterface.hh>
#include <dune/common/typetree/nodetags.hh>

namespace Dune {
  namespace TypeTree {

    //! Struct for obtaining some basic structural information about a TypeTree.
    /**
     * This struct extracts basic information about the passed TypeTree and
     * presents them in a static way suitable for use as compile-time constants.
     *
     * \tparam Tree  The TypeTree to examine.
     * \tparam Tag   Internal parameter, leave at default value.
     */
    template<class Tree, class Tag = StartTag>
    struct TreeInfo
    {
    private:
      // Start the tree traversal
      typedef TreeInfo<Tree,NodeTag<Tree>> NodeInfo;

    public:

      //! The depth of the TypeTree.
      static const std::size_t depth = NodeInfo::depth;

      //! The total number of nodes in the TypeTree.
      static const std::size_t nodeCount = NodeInfo::nodeCount;

      //! The number of leaf nodes in the TypeTree.
      static const std::size_t leafCount = NodeInfo::leafCount;
    };


#ifndef DOXYGEN

    // ********************************************************************************
    // TreeInfo specializations for the different node types
    // ********************************************************************************

    // leaf node
    template<class Node>
    struct TreeInfo<Node,LeafNodeTag>
    {
      static const std::size_t depth = 1;

      static const std::size_t nodeCount = 1;

      static const std::size_t leafCount = 1;
    };


    // power node - exploit the fact that all children are identical
    template<class Node>
    struct TreeInfo<Node,PowerNodeTag>
    {
      typedef TreeInfo<typename Node::ChildType,NodeTag<typename Node::ChildType>> ChildInfo;

      static const std::size_t depth = 1 + ChildInfo::depth;

      static const std::size_t nodeCount = 1 + StaticDegree<Node>::value * ChildInfo::nodeCount;

      static const std::size_t leafCount = StaticDegree<Node>::value * ChildInfo::leafCount;
    };


    namespace Impl {

      // TMP for iterating over the children of a composite node
      // identical for both composite node implementations
      template<class Node, std::size_t k, std::size_t n>
      struct generic_compositenode_children_info
      {
        typedef generic_compositenode_children_info<Node,k+1,n> NextChild;

        // extract child info
        typedef typename Node::template Child<k>::Type Child;
        typedef NodeTag<Child> ChildTag;
        typedef TreeInfo<Child,ChildTag> ChildInfo;

        // combine information of current child with info about following children
        static const std::size_t maxDepth = ChildInfo::depth > NextChild::maxDepth ? ChildInfo::depth : NextChild::maxDepth;

        static const std::size_t nodeCount = ChildInfo::nodeCount + NextChild::nodeCount;

        static const std::size_t leafCount = ChildInfo::leafCount + NextChild::leafCount;
      };

      // End of recursion
      template<class Node, std::size_t n>
      struct generic_compositenode_children_info<Node,n,n>
      {
        static const std::size_t maxDepth = 0;

        static const std::size_t nodeCount = 0;

        static const std::size_t leafCount = 0;
      };

    } // end namespace Impl


    // Struct for building information about composite node
    template<class Node>
    struct GenericCompositeNodeInfo
    {
      typedef Impl::generic_compositenode_children_info<Node,0,StaticDegree<Node>::value> Children;

      static const std::size_t depth = 1 + Children::maxDepth;

      static const std::size_t nodeCount = 1 + Children::nodeCount;

      static const std::size_t leafCount = Children::leafCount;
    };


    // CompositeNode: delegate to GenericCompositeNodeInfo
    template<class Node>
    struct TreeInfo<Node,CompositeNodeTag>
      : public GenericCompositeNodeInfo<Node>
    {};

#endif // DOXYGEN

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_COMMON_TYPETREE_TREEINFO_HH