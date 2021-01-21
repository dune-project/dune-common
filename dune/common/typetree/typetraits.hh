// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_TYPETREE_TYPETRAITS_HH
#define DUNE_COMMON_TYPETREE_TYPETRAITS_HH

#include <type_traits>

#include <dune/common/typetree/treepath.hh>

namespace Dune {
  namespace TypeTree {

    namespace Impl {

      // Check if type is a or is derived from one of the tree path types

      // Default overload for types not representing a tree path
      constexpr auto isTreePath (...)
        -> std::false_type
      {
        return {};
      }

      // Overload for instances of HybridTreePath<...>
      template<class... I>
      constexpr auto isTreePath (const HybridTreePath<I...>&)
        -> std::true_type
      {
        return {};
      }

    } // end namespace Impl


    /**
     * \brief Check if type represents a tree path
     *
     * If T is a or derived from one of the tree path types this
     * struct derives from std::true_type, otherwise from std::false_type.
     * Hence the result of the check is available via ::value, cast to bool,
     * or operator().
     *
     * \tparam T Check if this type represents a tree path
     */
    template<class T>
    struct IsTreePath
      : public decltype(Impl::isTreePath(std::declval<std::decay_t<T>>()))
    {};

    /**
     * \brief Check if given object represents a tree path
     *
     * \tparam T Check if this type represents a tree path
     * \returns std::true_type if argument is a tree path and std::false_type if not
     */
    template<class T>
    constexpr auto isTreePath (const T&)
      -> IsTreePath<T>
    {
      return IsTreePath<T>{};
    }

  } // end namespace TypeTree
} // end namespace Dune

#endif // DUNE_COMMON_TYPETREE_TYPETRAITS_HH
