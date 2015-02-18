#ifndef DUNE_COMMON_STD_TYPE_TRAITS_HH
#define DUNE_COMMON_STD_TYPE_TRAITS_HH

#include <type_traits>

namespace Dune
{

namespace Std
{

  // to_false_type
  // -------------

  /** \class to_false_type
   *
   *  \brief template mapping a type to <tt>std::false_type</tt>
   *
   *  \tparam T Some type
   *
   *  Suppose you have a template class. You want to document the required
   *  members of this class in the non-specialized template, but you know that
   *  actually instantiating the non-specialized template is an error. You can
   *  try something like this:
   *  \code
   *  template<typename T>
   *  struct Traits
   *  {
   *    static_assert(false,
   *                  "Instanciating this non-specialized template is an "
   *                  "error. You should use one of the specializations "
   *                  "instead.");
   *    //! The type used to frobnicate T
   *    typedef void FrobnicateType;
   *  };
   *  \endcode
   *  This will trigger static_assert() as soon as the compiler reads the
   *  definition for the Traits template, since it knows that "false" can never
   *  become true, no matter what the template parameters of Traits are. As a
   *  workaround you can use to_false_type: replace <tt>false</tt> by
   *  <tt>to_false_type<T>::value</tt>, like this:
   *  \code
   *  template<typename T>
   *  struct Traits
   *  {
   *    static_assert(Std::to_false_type<T>::value,
   *                  "Instanciating this non-specialized template is an "
   *                  "error. You should use one of the specializations "
   *                  "instead.");
   *    //! The type used to frobnicate T
   *    typedef void FrobnicateType;
   *  };
   *  \endcode
   *  Since there might be an specialization of to_false_type for template
   *  parameter T, the compiler cannot trigger static_assert() until the type
   *  of T is known, that is, until Traits<T> is instantiated.
   */
  template< typename T >
  struct to_false_type : public std::false_type {};



  // to_true_type
  // ------------

  /** \class to_true_type
   *
   *  \brief template mapping a type to <tt>std::true_type</tt>
   *
   *  \tparam T Some type
   *
   *  \note This class exists mostly for consistency with to_false_type.
   */
  template< typename T >
  struct to_true_type : public std::true_type {};

} // namespace Std

} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_TYPE_TRAITS_HH
