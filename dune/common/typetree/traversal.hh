// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_TRAVERSAL_HH
#define DUNE_COMMON_TYPETREE_TRAVERSAL_HH

#include <utility>

#include <dune/common/hybridutilities.hh>
#include <dune/common/std/type_traits.hh>

#include <dune/common/typetree/childextraction.hh>
#include <dune/common/typetree/nodetags.hh>
#include <dune/common/typetree/treepath.hh>
#include <dune/common/typetree/visitor.hh>

namespace Dune {
  namespace TypeTree {

    /** \addtogroup Tree Traversal
     *  \ingroup TypeTree
     *  \{
     */

    namespace Impl {

      /**
       * \brief Helpfer function that returns the degree of a Tree.
       *
       * The return type is either `size_t` if it is a dynamic tree or the flag `dynamic`
       * is set to `true`, or as the type returned by the `degree()` member function of the
       * tree.
       *
       * This function allows to change the tree traversal from static to dynamic in case
       * of power nodes and uses static traversal for composite and dynamic traversal for
       * all dynamic nodes.
       **/
      template <bool dynamic = true, class Tree>
      auto traversalDegree(Tree const& tree)
      {
        if constexpr (dynamic && isPower<Tree>)
          return std::size_t(tree.degree());
        else
          return tree.degree();
      }

      // forward declaration
      template<class Tree, TreePathType::Type pathType, class Prefix, std::size_t... indices>
      constexpr auto leafTreePathTuple(Prefix prefix, std::index_sequence<indices...>);

      template<class Tree, TreePathType::Type pathType, class Prefix>
      constexpr auto leafTreePathTuple(Prefix prefix)
      {
        if constexpr (isLeaf<Tree>)
          return std::make_tuple(prefix);
        else
          return Impl::leafTreePathTuple<Tree, pathType>(prefix, std::make_index_sequence<Tree::degree()>{});
      }

      template<class Tree, TreePathType::Type pathType, class Prefix, std::size_t... indices>
      constexpr auto leafTreePathTuple(Prefix prefix, std::index_sequence<indices...>)
      {
        if constexpr (isComposite<Tree> or (isPower<Tree> and (pathType!=TreePathType::dynamic)))
          return std::tuple_cat(
            Impl::leafTreePathTuple<TypeTree::Child<Tree,indices>, pathType>(TypeTree::push_back(prefix, index_constant<indices>{}))...);
        else {
          static_assert(isPower<Tree> and (pathType==TreePathType::dynamic));
          return std::tuple_cat(
            Impl::leafTreePathTuple<TypeTree::Child<Tree,indices>, pathType>(TypeTree::push_back(prefix, indices))...);
        }
      }


      /* The signature is the same as for the public applyToTree
       * function in Dune::Typetree, despite the additionally passed
       * treePath argument. The path passed here is associated to
       * the tree and the relative paths of the children (wrt. to tree)
       * are appended to this.  Hence the behavior of the public function
       * is resembled by passing an empty treePath.
       */

      /*
       * This is the overload for leaf traversal
       */
      template<class T, class TreePath, class V,
        std::enable_if_t<isLeaf<T>, int> = 0>
      void applyToTree(T&& tree, TreePath treePath, V&& visitor)
      {
        visitor.leaf(tree, treePath);
      }

      /*
       * This is the general overload doing child traversal.
       */
      template<class T, class TreePath, class V,
        std::enable_if_t<not isLeaf<T>, int> = 0>
      void applyToTree(T&& tree, TreePath treePath, V&& visitor)
      {
        // Do we really want to take care for const-ness of the Tree
        // when instantiating VisitChild below? I'd rather expect this:
        // using Tree = std::decay_t<T>;
        // using Visitor = std::decay_t<V>;
        using Tree = std::remove_reference_t<T>;
        using Visitor = std::remove_reference_t<V>;
        visitor.pre(tree, treePath);

        // Use statically encoded degree unless tree
        // is a power node and dynamic traversal is requested.
        constexpr auto useDynamicTraversal = Visitor::treePathType == TreePathType::dynamic;
        auto indices = Dune::range(traversalDegree<useDynamicTraversal>(tree));
        Hybrid::forEach(indices, [&](auto i) {
          auto childTreePath = TypeTree::push_back(treePath, i);
          auto&& child = tree.child(i);
          using Child = std::decay_t<decltype(child)>;

          visitor.beforeChild(tree, child, treePath, i);

          // This requires that visiotor.in(...) can always be instantiated,
          // even if there's a single child only.
          if (i>0)
            visitor.in(tree, treePath);
          static const auto visitChild = Visitor::template VisitChild<Tree,Child,TreePath>::value;
          if constexpr (visitChild)
            applyToTree(child, childTreePath, visitor);

          visitor.afterChild(tree, child, treePath, i);
        });
        visitor.post(tree, treePath);
      }



      /* Traverse tree and visit each node. The signature is the same
       * as for the public forEachNode function in Dune::Typtree,
       * despite the additionally passed treePath argument. The path
       * passed here is associated to the tree and the relative
       * paths of the children (wrt. to tree) are appended to this.
       * Hence the behavior of the public function is resembled
       * by passing an empty treePath.
       */
      template<class Tree, class TreePath, class PreFunc, class LeafFunc, class PostFunc>
      void forEachNode(Tree&& tree, TreePath treePath, PreFunc&& preFunc, LeafFunc&& leafFunc, PostFunc&& postFunc)
      {
        using TreeType = std::decay_t<Tree>;
        if constexpr (isLeaf<TreeType>) {
          // If we have a leaf tree just visit it using the leaf function.
          leafFunc(tree, treePath);
        } else {
          // Otherwise visit the tree with the pre function,
          // visit all children using a static loop, and
          // finally visit the tree with the post function.
          preFunc(tree, treePath);
          Hybrid::forEach(Dune::range(traversalDegree(tree)), [&](auto i) {
            auto childTreePath = TypeTree::push_back(treePath, i);
            forEachNode(tree.child(i), childTreePath, preFunc, leafFunc, postFunc);
          });
          postFunc(tree, treePath);
        }
      }

    } // namespace Detail


    // ********************************************************************************
    // Public Interface
    // ********************************************************************************

    /**
     * \brief Create tuple of tree paths to leafs
     *
     * This creates a tuple of HybridTreePath objects refering to the
     * leaf nodes in the tree. The generated tree paths are always
     * HybridTreePath instances. If pathType is TreePathType::dynamic
     * then path entries for power nodes are of type std::size_t.
     * For all other nodes and if pathType is not TreePathType::dynamic,
     * the entries are instances of Dune::index_constant.
     *
     * \tparam Tree Type of tree to generate tree paths for
     * \tparam pathType Type of paths to generate
     */
    template<class Tree, TreePathType::Type pathType = TreePathType::dynamic>
    constexpr auto leafTreePathTuple()
    {
      return Impl::leafTreePathTuple<std::decay_t<Tree>, pathType>(hybridTreePath());
    }

    //! Apply visitor to TypeTree.
    /**
     * \code
     #include <dune/common/typetree/traversal.hh>
     * \endcode
     * This function applies the given visitor to the given tree. Both visitor and tree may be const
     * or non-const (if the compiler supports rvalue references, they may even be a non-const temporary).
     *
     * \note The visitor must implement the interface laid out by DefaultVisitor (most easily achieved by
     *       inheriting from it) and specify the required type of tree traversal (static or dynamic) by
     *       inheriting from either StaticTraversal or DynamicTraversal.
     *
     * \param tree    The tree the visitor will be applied to.
     * \param visitor The visitor to apply to the tree.
     */
    template<class Tree, class Visitor>
    void applyToTree(Tree&& tree, Visitor&& visitor)
    {
      Impl::applyToTree(tree, hybridTreePath(), visitor);
    }

    /**
     * \brief Traverse tree and visit each node
     *
     * All passed callback functions are called with the
     * node and corresponding treepath as arguments.
     *
     * \param tree The tree to traverse
     * \param preFunc This function is called for each inner node before visiting its children
     * \param leafFunc This function is called for each leaf node
     * \param postFunc This function is called for each inner node after visiting its children
     */
    template<class Tree, class PreFunc, class LeafFunc, class PostFunc>
    void forEachNode(Tree&& tree, PreFunc&& preFunc, LeafFunc&& leafFunc, PostFunc&& postFunc)
    {
      Impl::forEachNode(tree, hybridTreePath(), preFunc, leafFunc, postFunc);
    }

    /**
     * \brief Traverse tree and visit each node
     *
     * All passed callback functions are called with the
     * node and corresponding treepath as arguments.
     *
     * \param tree The tree to traverse
     * \param innerFunc This function is called for each inner node before visiting its children
     * \param leafFunc This function is called for each leaf node
     */
    template<class Tree, class InnerFunc, class LeafFunc>
    void forEachNode(Tree&& tree, InnerFunc&& innerFunc, LeafFunc&& leafFunc)
    {
      auto nop = [](auto&&... args) {};
      forEachNode(tree, innerFunc, leafFunc, nop);
    }

    /**
     * \brief Traverse tree and visit each node
     *
     * The passed callback function is called with the
     * node and corresponding treepath as arguments.
     *
     * \param tree The tree to traverse
     * \param nodeFunc This function is called for each node
     */
    template<class Tree, class NodeFunc>
    void forEachNode(Tree&& tree, NodeFunc&& nodeFunc)
    {
      forEachNode(tree, nodeFunc, nodeFunc);
    }

    /**
     * \brief Traverse tree and visit each leaf node
     *
     * The passed callback function is called with the
     * node and corresponding treepath as arguments.
     *
     * \param tree The tree to traverse
     * \param leafFunc This function is called for each leaf node
     */
    template<class Tree, class LeafFunc>
    void forEachLeafNode(Tree&& tree, LeafFunc&& leafFunc)
    {
      auto nop = [](auto&&... args) {};
      forEachNode(tree, nop, leafFunc, nop);
    }

    //! \} group Tree Traversal

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_COMMON_TYPETREE_TRAVERSAL_HH
