// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_TYPELIST_HH
#define DUNE_COMMON_TYPELIST_HH

#include <type_traits>


namespace Dune {

  /**
   * \brief A simple type list
   *
   * \ingroup TypeUtilities
   *
   * The purpose of this is to encapsulate a list of types.
   * This allows, e.g., to pack an argument-pack into one type.
   * In contrast to a std::tuple a TypeList can be created
   * without creating any object of the stored types.
   *
   * This can, e.g., be used for overload resolution
   * with tag-dispatch where TypeList is used as tag.
   * In combination with PriorityTag this allows to emulate
   * partial specialization of function templates in
   * a sane way, i.e., without the hassle of classic
   * specialization of function templates
   */
  template<class... T>
  struct TypeList
  {};



  /**
   * \brief Check if given type is a TypeList
   *
   * \ingroup Utility
   *
   * The result of the check is encoded in the
   * base class of type std::integral_constant<bool, result>.
   */
  template<class T>
  struct IsTypeList : std::false_type {};

  template<class... T>
  struct IsTypeList<TypeList<T...> > : std::true_type {};



  /**
   * \brief Check if given type is an empty TypeList
   *
   * \ingroup Utility
   *
   * The result of the check is encoded in the
   * base class of type std::integral_constant<bool, result>.
   */
  template<class T>
  struct IsEmptyTypeList : std::integral_constant<bool, IsTypeList<T>() and std::is_same<T, TypeList<> >() > {};



  template<class T>
  struct TypeListSize {};

  /**
   * \brief Get size of TypeList
   *
   * \ingroup Utility
   *
   * The result of is encoded in the base class of
   * type std::integral_constant<std::size_t, result>.
   */
  template<class... T>
  struct TypeListSize<TypeList<T...>> : std::integral_constant<std::size_t, sizeof...(T)> {};



  template<std::size_t i, class T>
  struct TypeListElement {};

  /**
   * \brief Get element of TypeList
   *
   * \ingroup Utility
   */
  template<std::size_t i, class... T>
  struct TypeListElement<i, TypeList<T...>>
  {
    /**
     * \brief Export type of i-th element in TypeList
     *
     * \todo Implement without using std::tuple.
     */
    using Type = typename std::tuple_element<i, std::tuple<T...>>::type;
  };

  /**
   * \brief Shortcut for TypeListElement<i, T>::Type;
   */
  template<std::size_t i, class T>
  using TypeListEntry_t = typename TypeListElement<i, T>::Type;


} // namespace Dune

#endif // DUNE_COMMON_TYPELIST_HH
