// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_TRAVERSALUTILITIES_HH
#define DUNE_COMMON_TYPETREE_TRAVERSALUTILITIES_HH

#include <type_traits>

#include <dune/common/std/apply.hh>
#include <dune/common/typetree/childextraction.hh>
#include <dune/common/typetree/traversal.hh>

namespace Dune {
  namespace TypeTree {

    namespace Impl {

      template <class R, class Arg0>
      decltype(auto) pairwiseReduction(R reduction, Arg0&& arg0)
      {
        return std::forward<Arg0>(arg0);
      }

      template <class R, class Arg0, class Arg1, class... Args>
      decltype(auto) pairwiseReduction(R reduction, Arg0&& arg0, Arg1&& arg1, Args&&... args)
      {
        return pairwiseReduction(reduction, reduction(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1)), std::forward<Args>(args)...);
      }

    } // end namespace Impl

    /** \addtogroup Tree Traversal
     *  \ingroup TypeTree
     *  \{
     */

    //! Calculate a quantity as a reduction over the leaf nodes of a TypeTree.
    template<class Tree, class T, class F, class R>
    decltype(auto) accumulateOverLeafs(const Tree& tree, T init, F functor, R reduction)
    {
      const auto flatTree = leafTreePathTuple<Tree, TreePathType::fullyStatic>();
      return Std::apply([&](auto... tp) -> decltype(auto) {
        return Impl::pairwiseReduction(reduction, init, functor(TypeTree::child(tree,tp), tp)...);
      }, flatTree);
    }

    template<class Tree, class F, class R>
    decltype(auto) accumulateOverLeafs(const Tree& tree, F functor, R reduction)
    {
      const auto flatTree = leafTreePathTuple<Tree, TreePathType::fullyStatic>();
      return Std::apply([&](auto... tp) -> decltype(auto) {
        return reduction(functor(TypeTree::child(tree,tp), tp)...);
      }, flatTree);
    }

    //! Calculate a quantity as a reduction over the leaf nodes of a TypeTree.
    /**
     * This function can be used to easily calculate a quantity that is a result of applying
     * a functor to the leaf nodes of a TypeTree and combining the functor return values.
     * The functor, reduction and result should all have cheap copy constructors to ensure
     * good performance.
     *
     * The functor must conform to the pattern
     * \code
     * struct Functor
     * {
     *   template<typename Node, typename TreePath>
     *   ResultType operator()(const Node& node, TreePath treePath) const
     *   {
     *     return ...;
     *   }
     * };
     * \endcode
     *
     * \param tree       The tree on which to perform the calculation.
     * \param functor    The functor to apply to the leaf nodes.
     * \param reduction  The operation used to combine the individual results.
     * \param value The initial value for the result.
     *
     * \returns The value obtained by combining the individual results for all leafs.
     */
    template<class ResultType, class Tree, class F, class R>
    ResultType reduceOverLeafs(const Tree& tree, F functor, R reduction, ResultType value)
    {
      forEachLeafNode(tree, [&](auto&& node, auto&& treePath) {
        value = reduction(value, functor(node, treePath));
      });
      return value;
    }

    //! \} group Tree Traversal

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_COMMON_TYPETREE_TRAVERSALUTILITIES_HH
