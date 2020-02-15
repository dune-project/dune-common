// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_POWERNODE_HH
#define DUNE_COMMON_TYPETREE_POWERNODE_HH

#include <cassert>
#include <array>
#include <memory>
#include <type_traits>

#include <dune/common/typetraits.hh>
#include <dune/common/std/type_traits.hh>

#include <dune/common/typetree/nodetags.hh>
#include <dune/common/typetree/childextraction.hh>
#include <dune/common/typetree/typetraits.hh>

namespace Dune {
  namespace TypeTree {

    /** \addtogroup Nodes
     *  \ingroup TypeTree
     *  \{
     */

    /** \brief Collect k instances of type T within a \ref TypeTree.
     *
     *  \tparam T The base type
     *  \tparam k The number of instances this node should collect
     */
    template<typename T, std::size_t k>
    class PowerNode
    {
    public:

      //! Mark this class as non leaf in the \ref TypeTree.
      static const bool isLeaf = false;

      //! Mark this class as a power in the \ref TypeTree.
      static const bool isPower = true;

      //! Mark this class as a non composite in the \ref TypeTree.
      static const bool isComposite = false;

      //! The number of children.
      static const std::size_t CHILDREN = k;

      static constexpr std::size_t degree ()
      {
        return k;
      }

      //! The type tag that describes a PowerNode.
      typedef PowerNodeTag NodeTag;

      //! The type of each child.
      typedef T ChildType;

      //! The storage type of each child.
      typedef std::shared_ptr<T> ChildStorageType;

      //! The const version of the storage type of each child.
      typedef std::shared_ptr<const T> ChildConstStorageType;

      //! The type used for storing the children.
      typedef std::array<ChildStorageType,k> NodeStorage;


      //! Access to the type and storage type of the i-th child.
      template<std::size_t i>
      struct Child
      {
        static_assert((i < CHILDREN), "child index out of range");

        //! The type of the child.
        typedef T Type;

        //! The type of the child.
        typedef T type;

        //! The storage type of the child.
        typedef ChildStorageType Storage;

        //! The const storage type of the child.
        typedef ChildConstStorageType ConstStorage;
      };

      //! @name Child Access (templated methods)
      //! @{

      //! Returns the i-th child.
      /**
       * \returns a reference to the i-th child.
       */
      template<std::size_t i>
      T& child (index_constant<i> = {})
      {
        static_assert((i < CHILDREN), "child index out of range");
        return *_children[i];
      }

      //! Returns the i-th child (const version).
      /**
       * \returns a const reference to the i-th child.
       */
      template<std::size_t i>
      const T& child (index_constant<i> = {}) const
      {
        static_assert((i < CHILDREN), "child index out of range");
        return *_children[i];
      }

      //! Returns the storage of the i-th child.
      /**
       * \returns a copy of the object storing the i-th child.
       */
      template<std::size_t i>
      ChildStorageType childStorage (index_constant<i> = {})
      {
        static_assert((i < CHILDREN), "child index out of range");
        return _children[i];
      }

      //! Returns the storage of the i-th child (const version).
      /**
       * This method is only important if the child is stored as
       * some kind of pointer, as this allows the pointee type to
       * become const.
       * \returns a copy of the object storing the i-th child.
       */
      template<std::size_t i>
      ChildConstStorageType childStorage (index_constant<i> = {}) const
      {
        static_assert((i < CHILDREN), "child index out of range");
        return _children[i];
      }

      //! Sets the i-th child to the passed-in value.
      template<std::size_t i>
      void setChild (T& t, index_constant<i> = {})
      {
        static_assert((i < CHILDREN), "child index out of range");
        _children[i] = stackobject_to_shared_ptr(t);
      }

      //! Store the passed value in i-th child.
      template<std::size_t i>
      void setChild (T&& t, index_constant<i> = {})
      {
        static_assert((i < CHILDREN), "child index out of range");
        _children[i] = std::make_shared<T>(std::move(t));
      }

      //! Sets the stored value representing the i-th child to the passed-in value.
      template<std::size_t i>
      void setChild (ChildStorageType st, index_constant<i> = {})
      {
        static_assert((i < CHILDREN), "child index out of range");
        _children[i] = std::move(st);
      }

      //! @}


      //! @name Child Access (Dynamic methods)
      //! @{

      //! Returns the i-th child.
      /**
       * \returns a reference to the i-th child.
       */
      T& child (std::size_t i)
      {
        assert(i < CHILDREN && "child index out of range");
        return *_children[i];
      }

      //! Returns the i-th child (const version).
      /**
       * \returns a const reference to the i-th child.
       */
      const T& child (std::size_t i) const
      {
        assert(i < CHILDREN && "child index out of range");
        return *_children[i];
      }

      //! Returns the storage of the i-th child.
      /**
       * \returns a copy of the object storing the i-th child.
       */
      ChildStorageType childStorage (std::size_t i)
      {
        assert(i < CHILDREN && "child index out of range");
        return _children[i];
      }

      //! Returns the storage of the i-th child (const version).
      /**
       * This method is only important if the child is stored as
       * some kind of pointer, as this allows the pointee type to
       * become const.
       * \returns a copy of the object storing the i-th child.
       */
      ChildConstStorageType childStorage (std::size_t i) const
      {
        assert(i < CHILDREN && "child index out of range");
        return (_children[i]);
      }

      //! Sets the i-th child to the passed-in value.
      void setChild (std::size_t i, T& t)
      {
        assert(i < CHILDREN && "child index out of range");
        _children[i] = stackobject_to_shared_ptr(t);
      }

      //! Store the passed value in i-th child.
      void setChild(std::size_t i, T&& t)
      {
        assert(i < CHILDREN && "child index out of range");
        _children[i] = std::make_shared<T>(std::move(t));
      }

      //! Sets the stored value representing the i-th child to the passed-in value.
      void setChild (std::size_t i, ChildStorageType st)
      {
        assert(i < CHILDREN && "child index out of range");
        _children[i] = std::move(st);
      }

      const NodeStorage& nodeStorage() const
      {
        return _children;
      }

      //! @}

      //! @name Nested Child Access
      //! @{

      // The following two methods require a little bit of SFINAE trickery to work correctly:
      // We have to make sure that they don't shadow the methods for direct child access because
      // those get called by the generic child() machinery. If that machinery picks up the methods
      // defined below, we have an infinite recursion.
      // So the methods make sure that either
      //
      // * there are more than one argument. In that case, we got multiple indices and can forward
      //   to the general machine.
      //
      // * the first argument is not a valid flat index, i.e. either a std::size_t or an index_constant.
      //   The argument thus has to be some kind of TreePath instance that we can also pass to the
      //   generic machine.
      //
      // The above SFINAE logic works, but there is still a problem with the return type deduction.
      // We have to do a lazy lookup of the return type after SFINAE has succeeded, otherwise the return
      // type deduction will trigger the infinite recursion.

      //! Returns the child given by the list of indices.
      /**
       * This method simply forwards to the freestanding function child(). See that
       * function for further information.
       */
#ifdef DOXYGEN
      template<typename... Indices>
      ImplementationDefined& child(Indices... indices)
#else
      template<typename I0, typename... I,
        std::enable_if_t<(sizeof...(I) > 0) || IsTreePath<I0>::value, int > = 0>
      decltype(auto) child(I0 i0, I... i)
#endif
      {
        static_assert(sizeof...(I) > 0 || Impl::_non_empty_tree_path(I0{}),
          "You cannot use the member function child() with an empty TreePath, use the freestanding version child(node,treePath) instead."
          );
        return TypeTree::child(*this,i0,i...);
      }

      //! Returns the child given by the list of indices.
      /**
       * This method simply forwards to the freestanding function child(). See that
       * function for further information.
       */
#ifdef DOXYGEN
      template<typename... Indices>
      const ImplementationDefined& child(Indices... indices)
#else
      template<typename I0, typename... I,
        std::enable_if_t<(sizeof...(I) > 0) || IsTreePath<I0>::value, int > = 0>
      decltype(auto) child(I0 i0, I... i) const
#endif
      {
        static_assert(sizeof...(I) > 0 || Impl::_non_empty_tree_path(I0{}),
          "You cannot use the member function child() with an empty TreePath, use the freestanding version child(node,treePath) instead."
          );
        return TypeTree::child(*this,i0,i...);
      }

      //! @}

      //! @name Constructors
      //! @{

    protected:

      //! Default constructor.
      /**
       * The default constructor is protected, as PowerNode is a utility
       * class that needs to be filled with meaning by subclassing it
       * and adding useful functionality to the subclass.
       *
       * \warning When using the default constructor, make sure to set ALL children
       * by means of the setChild() methods!
       */
      PowerNode()
      {}

      //! Initialize the PowerNode with a copy of the passed-in storage type.
      explicit PowerNode(const NodeStorage& children)
        : _children(children)
      {}

      //! Initialize all children with copies of a storage object constructed from the parameter \c t.
      explicit PowerNode (T& t, bool distinct_objects = true)
      {
        if (distinct_objects)
        {
          for (typename NodeStorage::iterator it = _children.begin(); it != _children.end(); ++it)
            *it = std::make_shared<T>(t);
        }
        else
        {
          std::shared_ptr<T> sp = stackobject_to_shared_ptr(t);
          std::fill(_children.begin(),_children.end(),sp);
        }
      }

#ifdef DOXYGEN

      //! Initialize all children with the passed-in objects.
      PowerNode(T& t1, T& t2, ...)
      {}

#else

      template<typename... Children,
        std::enable_if_t<(std::is_same_v<ChildType, std::decay_t<Children>> ||...), int> = 0>
      PowerNode (Children&&... children)
      {
        static_assert(CHILDREN == sizeof...(Children),
          "PowerNode constructor is called with incorrect number of children");
        _children = NodeStorage{copy_or_wrap(std::forward<Children>(children))...};
      }

      template<typename... Children,
        std::enable_if_t<(std::is_same_v<ChildType, Children> ||...), int> = 0>
      PowerNode (std::shared_ptr<Children>... children)
      {
        static_assert(CHILDREN == sizeof...(Children),
          "PowerNode constructor is called with incorrect number of children");
        _children = NodeStorage{std::move(children)...};
      }

#endif // DOXYGEN

      //! @}

    private:
      NodeStorage _children;
    };

    //! \} group Nodes

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_COMMON_TYPETREE_POWERNODE_HH
