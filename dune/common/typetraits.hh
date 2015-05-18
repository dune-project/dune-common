// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_TYPETRAITS_HH
#define DUNE_TYPETRAITS_HH

#include <type_traits>

#include <dune/common/deprecated.hh>
#include <dune/common/std/utility.hh>

namespace Dune
{

  /**
   * @file
   * @brief Traits for type conversions and type information.
   * @author Markus Blatt, Christian Engwer
   */
  /** @addtogroup Common
   *
   * @{
   */

  /**
   * @brief Just an empty class
   */
  struct Empty {};

  /**
   * @brief General type traits class to check whether type is reference or
   * pointer type
   *
   * \deprecated This class will be replaced by alternatives found in the C++11 stl.
   *   - Use is_pointer<T>::value instead of TypeTraits<T>::isPointer
   *   - Use is_lvalue_reference<T>::value instead of TypeTraits<T>::isReference
   *   - Use remove_pointer<T>::type instead of TypeTraits<T>::PointeeType
   *   - Use remove_reference<T>::type instead of TypeTraits<T>::ReferredType
   */
  template <typename T>
  class TypeTraits
  {
  private:
    template <class U>
    struct PointerTraits {
      enum { result = false };
      typedef Empty PointeeType;
    };

    template <class U>
    struct PointerTraits<U*> {
      enum { result = true };
      typedef U PointeeType;
    };

    template <class U> struct ReferenceTraits
    {
      enum { result = false };
      typedef U ReferredType;
    };

    template <class U> struct ReferenceTraits<U&>
    {
      enum { result = true };
      typedef U ReferredType;
    };

  public:
    enum { isPointer = PointerTraits<T>::result };
    typedef typename PointerTraits<T>::PointeeType PointeeType DUNE_DEPRECATED_MSG("Use remove_pointer instead!");

    enum { isReference = ReferenceTraits<T>::result };
    typedef typename ReferenceTraits<T>::ReferredType ReferredType DUNE_DEPRECATED_MSG("Use remove_reference instead!");
  };

  /**
   * @brief Determines wether a type is const or volatile and provides the
   * unqualified types.
   */
  template<typename T>
  struct ConstantVolatileTraits
  {
    enum {
      /** @brief True if T has a volatile specifier. */
      isVolatile=false,
      /** @brief True if T has a const qualifier. */
      isConst=false
    };

    /** @brief The unqualified type. */
    typedef T UnqualifiedType;
    /** @brief The const type. */
    typedef const T ConstType;
    /** @brief The const volatile type. */
    typedef const volatile T ConstVolatileType;
  };

  template<typename T>
  struct ConstantVolatileTraits<const T>
  {
    enum {
      isVolatile=false, isConst=true
    };
    typedef T UnqualifiedType;
    typedef const UnqualifiedType ConstType;
    typedef const volatile UnqualifiedType ConstVolatileType;
  };


  template<typename T>
  struct ConstantVolatileTraits<volatile T>
  {
    enum {
      isVolatile=true, isConst=false
    };
    typedef T UnqualifiedType;
    typedef const UnqualifiedType ConstType;
    typedef const volatile UnqualifiedType ConstVolatileType;
  };

  template<typename T>
  struct ConstantVolatileTraits<const volatile T>
  {
    enum {
      isVolatile=true, isConst=true
    };
    typedef T UnqualifiedType;
    typedef const UnqualifiedType ConstType;
    typedef const volatile UnqualifiedType ConstVolatileType;
  };

  /** @brief Tests wether a type is volatile. */
  template<typename T>
  struct IsVolatile
  {
    enum {
      /** @brief True if The type is volatile. */
      value=ConstantVolatileTraits<T>::isVolatile
    };
  };

  /** @brief Tests wether a type is constant. */
  template<typename T>
  struct IsConst
  {
    enum {
      /** @brief True if The type is constant. */
      value=ConstantVolatileTraits<T>::isConst
    };
  };

  template<typename T, bool isVolatile>
  struct RemoveConstHelper
  {
    typedef typename ConstantVolatileTraits<T>::UnqualifiedType Type;
  };

  template<typename T>
  struct RemoveConstHelper<T,true>
  {
    typedef volatile typename ConstantVolatileTraits<T>::UnqualifiedType Type;
  };

  using std::remove_const;
  using std::remove_reference;

  /**
   * @brief Checks wether a type is convertible to another.
   *
   * @tparam From type you want to convert
   * @tparam To type you want to obtain
   *
   * Inspired by
   * <A HREF="http://www.kotiposti.net/epulkkin/instructive/base-class-determination.html"> this website</A>
   */
  template<class From, class To>
  class Conversion
  {
    typedef char Small;
    struct Big {char dummy[2];};
    static Small test(To);
    static Big test(...);
    static typename remove_reference< From >::type &makeFrom ();

  public:
    enum {
      /** @brief True if the conversion exists. */
      exists =  sizeof(test(makeFrom())) == sizeof(Small),
      /** @brief Whether the conversion exists in both ways. */
      isTwoWay = exists && Conversion<To,From>::exists,
      /** @brief True if To and From are the same type. */
      sameType = false
    };
    Conversion(){}

  };

  template <class From>
  class Conversion<From, void>
  {
  public:
    enum {
      exists = false,
      isTwoWay = false,
      sameType = false
    };
  };

  template <class To>
  class Conversion<void, To>
  {
  public:
    enum {
      exists = false,
      isTwoWay = false,
      sameType = false
    };
  };

  template<>
  class Conversion< int, double >
  {
  public:
    enum {
      exists = true,
      isTwoWay = false,
      sameType = false
    };
  };

  template<class T>
  class Conversion<T,T>{
  public:
    enum { exists=true, isTwoWay=true, sameType=true};
  };

  /**
   * @brief Checks wether a type is derived from another.
   *
   * @tparam Base the potential base class you want to test for
   * @tparam Derived type you want to test
   *
   * Similar idea to
   * <A HREF="http://www.kotiposti.net/epulkkin/instructive/base-class-determination.html"> this website</A>
   */
  template <class Base, class Derived>
  class IsBaseOf
  {
    typedef typename ConstantVolatileTraits< typename remove_reference< Base >::type >::UnqualifiedType RawBase;
    typedef typename ConstantVolatileTraits< typename remove_reference< Derived >::type >::UnqualifiedType RawDerived;
    typedef char Small;
    struct Big {char dummy[2];};
    static Small test(RawBase*);
    static Big test(...);
    static RawDerived* &makePtr ();
  public:
    enum {
      /** @brief True if Base is a base class of Derived. */
      value = sizeof(test(makePtr())) == sizeof(Small)
    };
    IsBaseOf(){}

  };

  /**
   * @brief Checks wether two types are interoperable.
   *
   * Two types are interoperable if conversions in either directions
   * exists.
   */
  template<class T1, class T2>
  struct IsInteroperable
  {
    enum {
      /**
       * @brief True if either a conversion from T1 to T2 or vice versa
       * exists.
       */
      value = Conversion<T1,T2>::exists || Conversion<T2,T1>::exists
    };
  };

  using std::enable_if;

  /**
   * @brief Enable typedef if two types are interoperable.
   *
   * (also see IsInteroperable)
   */
  template<class T1, class T2, class Type>
  struct EnableIfInterOperable
    : public enable_if<IsInteroperable<T1,T2>::value, Type>
  {};

  // pull in default implementation
  using std::is_same;
  using std::conditional;
  using std::integral_constant;
  using std::true_type;
  using std::false_type;


  template<typename>
  struct __is_pointer_helper
  : public false_type { };

  template<typename T>
  struct __is_pointer_helper<T*>
  : public true_type { };

  /// is_pointer
  template<typename T>
  struct is_pointer
  : public integral_constant<bool, (__is_pointer_helper<T>::value)>
    { };

  // Helper class for is_lvalue_reference
  template<typename>
  struct __is_lvalue_reference_helper
  : public false_type { };

  template<typename T>
  struct __is_lvalue_reference_helper<T&>
  : public true_type { };

  /** \brief Determine whether a type is a lvalue reference type */
  template<typename T>
  struct is_lvalue_reference
  : public integral_constant<bool, (__is_lvalue_reference_helper<T>::value)>
    { };

  template<typename _Tp>
    struct __remove_pointer_helper
    { typedef _Tp     type; };

  template<typename _Tp>
    struct __remove_pointer_helper<_Tp*>
    { typedef _Tp     type; };

  /** \brief Return the type a pointer type points to
   *
   * \note When the argument T is not a pointer, TypeTraits::PointeeType returns Dune::Empty,
   * while Dune::remove_pointer (as std::remove_pointer), returns T itself.
   */
  template<typename _Tp>
    struct remove_pointer
    : public __remove_pointer_helper<typename remove_const<_Tp>::type >
    { };

  /**
     \brief template which always yields a false value
     \tparam T Some type.  It sould be a type expression involving template
               parameters of the class or function using AlwaysFalse.

     Suppose you have a template class.  You want to document the required
     members of this class in the non-specialized template, but you know that
     actually instantiating the non-specialized template is an error.  You
     can try something like this:
     \code
     template<typename T>
     struct Traits {
       static_assert(false,
                          "Instanciating this non-specialized template is an "
                          "error.  You should use one of the specializations "
                          "instead.");
       //! The type used to frobnicate T
       typedef void FrobnicateType;
     };
     \endcode
     This will trigger static_assert() as soon as the compiler reads the
     definition for the Traits template, since it knows that "false" can
     never become true, no matter what the template parameters of Traits are.
     As a workaround you can use AlwaysFalse: replace <tt>false</tt> by
     <tt>AlwaysFalse<T>::value</tt>, like this:
     \code
     template<typename T>
     struct Traits {
       static_assert(AlwaysFalse<T>::value,
                          "Instanciating this non-specialized template is an "
                          "error.  You should use one of the specializations "
                          "instead.");
       //! The type used to frobnicate T
       typedef void FrobnicateType;
     };
     \endcode
     Since there might be an specialization of AlwaysFalse for template
     parameter T, the compiler cannot trigger static_assert() until the
     type of T is known, that is, until Traits<T> is instantiated.
   */
  template<typename T>
  struct AlwaysFalse {
    //! always a false value
    static const bool value = false;
  };

  /**
     \brief template which always yields a true value
     \tparam T Some type.  It sould be a type expression involving template
               parameters of the class or function using AlwaysTrue.

     \note This class exists mostly for consistency with AlwaysFalse.
   */
  template<typename T>
  struct AlwaysTrue {
    //! always a true value
    static const bool value = true;
  };


#if defined(DOXYGEN) or HAVE_IS_INDEXABLE_SUPPORT

#ifndef DOXYGEN

  namespace detail {

    template<typename T, typename I, typename = int>
    struct _is_indexable
      : public std::false_type
    {};

    template<typename T, typename I>
    struct _is_indexable<T,I,typename std::enable_if<(sizeof(Std::declval<T>()[Std::declval<I>()]) > 0),int>::type>
      : public std::true_type
    {};

  }

#endif // DOXYGEN

  //! Type trait to determine whether an instance of T has an operator[](I), i.e. whether
  //! it can be indexed with an index of type I.
  /**
   * \warning Not all compilers support testing for arbitrary index types. In particular, there
   *          are problems with GCC 4.4 and 4.5.
   */
  template<typename T, typename I = std::size_t>
  struct is_indexable
    : public detail::_is_indexable<T,I>
  {};


#else // defined(DOXYGEN) or HAVE_IS_INDEXABLE_SUPPORT


  // okay, here follows a mess of compiler bug workarounds...
  // GCC 4.4 dies if we try to subscript a simple type like int and
  // both GCC 4.4 and 4.5 don't like using arbitrary types as subscripts
  // for macros.
  // So we make sure to only ever attempt the SFINAE for operator[] for
  // class types, and to make sure the compiler doesn't become overly eager
  // we have to do some lazy evaluation tricks with nested templates and
  // stuff.
  // Let's get rid of GCC 4.4 ASAP!


  namespace detail {

    // simple wrapper template to support the lazy evaluation required
    // in _is_indexable
    template<typename T>
    struct _lazy
    {
      template<typename U>
      struct evaluate
      {
        typedef T type;
      };
    };

    // default version, gets picked if SFINAE fails
    template<typename T, typename = int>
    struct _is_indexable
      : public std::false_type
    {};

    // version for types supporting the subscript operation
    template<typename T>
    struct _is_indexable<T,decltype(Std::declval<T>()[0],0)>
      : public std::true_type
    {};

    // helper struct for delaying the evaluation until we are sure
    // that T is a class (i.e. until we are outside std::conditional
    // below)
    struct _check_for_index_operator
    {

      template<typename T>
      struct evaluate
        : public _is_indexable<T>
      {};

    };

  }

  // The rationale here is as follows:
  // 1) If we have an array, we assume we can index into it. That isn't
  //    true if I isn't an integral type, but that's why we have the static assertion
  //    in the body - we could of course try and check whether I is integral, but I
  //    can't be arsed and want to provide a motivation to switch to a newer compiler...
  // 2) If we have a class, we use SFINAE to check for operator[]
  // 3) Otherwise, we assume that T does not support indexing
  //
  // In order to make sure that the compiler doesn't accidentally try the SFINAE evaluation
  // on an array or a scalar, we have to resort to lazy evaluation.
  template<typename T, typename I = std::size_t>
  struct is_indexable
    : public std::conditional<
               std::is_array<T>::value,
               detail::_lazy<std::true_type>,
               typename std::conditional<
                 std::is_class<T>::value,
                 detail::_check_for_index_operator,
                 detail::_lazy<std::false_type>
                 >::type
               >::type::template evaluate<T>::type
  {
    static_assert(std::is_same<I,std::size_t>::value,"Your compiler is broken and does not support checking for arbitrary index types");
  };


#endif // defined(DOXYGEN) or HAVE_IS_INDEXABLE_SUPPORT


  /** @} */
}
#endif
