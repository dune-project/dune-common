// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_COMPOSITENODE_HH
#define DUNE_COMMON_TYPETREE_COMPOSITENODE_HH

#include <memory>
#include <tuple>

#include <dune/common/typetree/childextraction.hh>
#include <dune/common/typetree/nodebase.hh>
#include <dune/common/typetree/nodetags.hh>
#include <dune/common/typetree/typetraits.hh>

namespace Dune {
  namespace TypeTree {

    /** \addtogroup Nodes
     *  \ingroup TypeTree
     *  \{
     */

    //! Base class for composite nodes based on variadic templates.
    template<class... Children>
    class CompositeNode
        : public NodeBase<CompositeNode<Children...>>
    {
    public:

      //! The type tag that describes a CompositeNode.
      typedef CompositeNodeTag NodeTag;

      //! Tuple containing all childs
      typedef std::tuple<Children...> ChildTypes;

      //! The type used for storing the children.
      typedef std::tuple<std::shared_ptr<Children>... > NodeStorage;

      //! The number of children in this node
      static constexpr auto degree ()
      {
        return index_constant<sizeof...(Children)>{};
      }

      //! Access to the type and storage type of the i-th child.
      template<std::size_t k>
      using Child = std::tuple_element_t<k, ChildTypes>;


      //! @name Child Access
      //! @{

      //! Returns the i-th child.
      /**
       * \returns a reference to the i-th child.
       */
      template<std::size_t k>
      Child<k>& child (index_constant<k> = {})
      {
        return *std::get<k>(children_);
      }

      //! Returns the i-th child (const version).
      /**
       * \returns a const reference to the i-th child.
       */
      template<std::size_t k>
      const Child<k>& child (index_constant<k> = {}) const
      {
        return *std::get<k>(children_);
      }

      //! Sets the i-th child to the passed-in value.
      template<std::size_t k, class C>
      void setChild (C&& child, index_constant<k> = {})
      {
        std::get<k>(children_) = copy_or_wrap_or_share(std::forward<C>(child));
      }

      const NodeStorage& nodeStorage() const
      {
        return children_;
      }

      //! @}

      //! @name Nested Child Access
      //! @{

      // The following two methods require a little bit of SFINAE trickery to work correctly:
      // We have to make sure that they don't shadow the methods for direct child access
      // because those get called by the generic child() machinery. If that machinery picks
      // up the methods defined below, we have an infinite recursion.
      // So the methods make sure that either
      //
      // * there are more than one argument. In that case, we got multiple indices and can
      //   forward to the general machine.
      //
      // * the first argument is not a valid flat index, i.e. either a std::size_t or an
      //   index_constant. The argument thus has to be some kind of TreePath instance that
      //   we can also pass to the generic machine.
      //
      // The above SFINAE logic works, but there is still a problem with the return type
      // deduction. We have to do a lazy lookup of the return type after SFINAE has succeeded,
      // otherwise the return type deduction will trigger the infinite recursion.

      //! Returns the child given by the list of indices.
      /**
       * This method simply forwards to the freestanding function child(). See that
       * function for further information.
       */
#ifdef DOXYGEN
      template<class... Indices>
      ImplementationDefined& child (Indices... indices)
#else
      template<class I0, class... I,
        std::enable_if_t<(sizeof...(I) > 0) || IsTreePath<I0>::value, int> = 0>
      decltype(auto) child (I0 i0, I... i)
#endif
      {
        static_assert(sizeof...(I) > 0 || Impl::_non_empty_tree_path(I0{}),
          "You cannot use the member function child() with an empty TreePath, use the "
          "freestanding version child(node,treePath) instead."
          );
        return TypeTree::child(*this,i0,i...);
      }

      //! Returns the child given by the list of indices.
      /**
       * This method simply forwards to the freestanding function child(). See that
       * function for further information.
       */
#ifdef DOXYGEN
      template<class... Indices>
      const ImplementationDefined& child (Indices... indices)
#else
      template<class I0, class... I,
        std::enable_if_t<(sizeof...(I) > 0) || IsTreePath<I0>::value, int> = 0>
      decltype(auto) child (I0 i0, I... i) const
#endif
      {
        static_assert(sizeof...(I) > 0 || Impl::_non_empty_tree_path(I0{}),
          "You cannot use the member function child() with an empty TreePath, use the "
          "freestanding version child(node,treePath) instead."
          );
        return TypeTree::child(*this,i0,i...);
      }

      //! @}

    protected:

      //! @name Constructors
      //! @{

      //! Default constructor.
      /**
       * This constructor requires the storage type to be default
       * constructible.
       * \warning If the storage type is a pointer, the resulting object
       * will not be usable before its children are set using any of the
       * setChild(...) methods!
       */
      CompositeNode ()
      {}

      //! Initialize all children with the passed-in objects.
      template<class... Args,
        std::enable_if_t<(sizeof...(Args) == degree()), int> = 0>
      CompositeNode (Args&&... args)
        : children_(copy_or_wrap_or_share(std::forward<Args>(args))...)
      {}

      //! Initialize the CompositeNode with a copy of the passed-in storage type.
      CompositeNode (NodeStorage children)
        : children_(std::move(children))
      {}

      //! @}

    private:
      NodeStorage children_;
    };

    //! \} group Nodes

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_COMMON_TYPETREE_COMPOSITENODE_HH
