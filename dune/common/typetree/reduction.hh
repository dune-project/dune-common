// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_TRAVERSALUTILITIES_HH
#define DUNE_COMMON_TYPETREE_TRAVERSALUTILITIES_HH

#include <dune/common/typetree/traversal.hh>

namespace Dune {
  namespace TypeTree {

    /** \addtogroup Tree Traversal
     *  \ingroup TypeTree
     *  \{
     */

    namespace Impl {

      //! Visitor that applies a functor and an associated reduction to a TypeTree.
      /**
       * \tparam F The functor to apply to leaf nodes. Must return a ResultType.
       * \tparam R The reduction used to combine the results.
       * \tparam ResultType The result type of the operation.
       */
      template<typename F, typename R, typename ResultType>
      struct LeafReductionVisitor
        : public TypeTree::TreeVisitor
      {

        static const TreePathType::Type treePathType = TreePathType::dynamic;

        template<typename Node, typename TreePath>
        void leaf(const Node& node, TreePath treePath)
        {
          _value = _reduction(_value,_functor(node,treePath));
        }

        LeafReductionVisitor(F functor, R reduction, ResultType startValue)
          : _functor(functor)
          , _reduction(reduction)
          , _value(startValue)
        {}

        ResultType result() { return _value; }

        F _functor;
        R _reduction;
        ResultType _value;

      };

    } // end namespace Impl

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
       * \param startValue The initial value for the result.
       *
       * \returns The value obtained by combining the individual results for all leafs.
       */
    template<typename ResultType, typename Tree, typename F, typename R>
    ResultType reduceOverLeafs(const Tree& tree, F functor, R reduction, ResultType startValue)
    {
      Impl::LeafReductionVisitor<F,R,ResultType> visitor(functor,reduction,startValue);
      TypeTree::applyToTree(tree,visitor);
      return visitor.result();
    }

    //! \} group Tree Traversal

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_COMMON_TYPETREE_TRAVERSALUTILITIES_HH
