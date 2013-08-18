// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_TYPETRAITS_HH
#define DUNE_TYPETRAITS_HH

#if defined HAVE_TYPE_TRAITS
#include <type_traits>
#elif defined HAVE_TR1_TYPE_TRAITS
#include <tr1/type_traits>
#endif

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
    typedef typename PointerTraits<T>::PointeeType PointeeType;

    enum { isReference = ReferenceTraits<T>::result };
    typedef typename ReferenceTraits<T>::ReferredType ReferredType;
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

#if defined HAVE_TYPE_TRAITS
  using std::remove_const;
#elif defined HAVE_TR1_TYPE_TRAITS
  using std::tr1::remove_const;
#else
  /**
   * @brief Removes a const qualifier while preserving others.
   */
  template<typename T>
  struct remove_const
  {
    typedef typename RemoveConstHelper<T, IsVolatile<T>::value>::Type type;
  };
#endif

#if defined HAVE_TYPE_TRAITS
  using std::remove_reference;
#elif defined HAVE_TR1_TYPE_TRAITS
  using std::tr1::remove_reference;
#else
  //! Remove a reference from a type
  /**
   * If the template parameter \c T matches \c T1&, then the member typedef \c
   * type is \c T1, otherwise it is \c T.
   */
  template<typename T> struct remove_reference {
    //! T with references removed
    typedef T type;
  };
#  ifndef DOXYGEN
  template<typename T> struct remove_reference<T&> {
    typedef T type;
  };
#  endif // ! defined(DOXYGEN)
#endif

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
    static typename TypeTraits< From >::ReferredType &makeFrom ();

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
    typedef typename ConstantVolatileTraits< typename TypeTraits< Base >::ReferredType >::UnqualifiedType RawBase;
    typedef typename ConstantVolatileTraits< typename TypeTraits< Derived >::ReferredType >::UnqualifiedType RawDerived;
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

#ifdef HAVE_TYPE_TRAITS
  using std::enable_if;
#else
  /**
   * @brief Enable typedef if condition is met.
   *
   * Replacement implementation for compilers without this in the stl.
   * Depending on the value of b the type T is provided as typedef type.
   */
  template<bool b, typename T=void>
  struct enable_if
  {
    typedef T type;
  };

  template<typename T>
  struct enable_if<false,T>
  {};
#endif


  /**
   * @brief Enable typedef if two types are interoperable.
   *
   * (also see IsInteroperable)
   */
  template<class T1, class T2, class Type>
  struct EnableIfInterOperable
    : public enable_if<IsInteroperable<T1,T2>::value, Type>
  {};

#if defined HAVE_TYPE_TRAITS
  using std::is_same;
#elif defined HAVE_TR1_TYPE_TRAITS
  using std::tr1::is_same;
#else
  /**
   * @brief Compile time test for testing whether
   * two types are the same.
   */
  template<typename T1, typename T2>
  struct is_same
  {
    //! Whether T1 is the same type as T2.
    enum {
      /* @brief Whether T1 is the same type as T2. */
      value=false
    };
  };


  template<typename T>
  struct is_same<T,T>
  {
    enum { value=true};
  };
#endif

  /**
   * @brief Select a type based on a condition.
   *
   * If template parameter first is true T1 is selected
   * otherwise T2 will be selected.
   * The selected type is accessible through the typedef
   * Type.
   *
   * \deprecated Will be removed after dune-common-2.3, use 'conditional' instead.
   */
  template<bool first, class T1, class T2>
  struct SelectType
  {
    /**
     * @brief The selected type.
     *
     * if first is true this will be type T1 and
     * otherwise T2
     */
    typedef T1 Type DUNE_DEPRECATED_MSG("Use Dune::conversion::type instead");
  } DUNE_DEPRECATED;

  template<class T1, class T2>
  struct SelectType<false,T1,T2>
  {
    typedef T2 Type DUNE_DEPRECATED_MSG("Use Dune::conversion::type instead");
  };

   /**
   * @brief Select a type based on a condition.
   *
   * If template parameter first is true T1 is selected
   * otherwise T2 will be selected.
   * The selected type is accessible through the typedef
   * type.
   *
   * \note This is a reimplementation of the C++11 stl feature of the same name.
   */
  template<bool first, class T1, class T2>
  struct conditional
  {
    /**
     * @brief The selected type
     *
     * if first is true this will be type T1 and
     * T2 otherwise
     */
    typedef T1 type;
  };

  template<class T1, class T2>
  struct conditional<false,T1,T2>
  {
    typedef T2 type;
  };

  ////////////////////////////////////////////////////////////////////////
  //
  // integral_constant (C++0x 20.7.3 "Helper classes")
  //
#if HAVE_INTEGRAL_CONSTANT
  using std::integral_constant;
  using std::true_type;
  using std::false_type;
#else // #if HAVE_INTEGRAL_CONSTANT
  //! Generate a type for a given integral constant
  /**
   * \tparam T Type of the constant.
   * \tparam v Value of the constant.
   */
  template <class T, T v>
  struct integral_constant {
    //! value this type was generated for
    static const T value = v;
    //! type of value
    typedef T value_type;
    //! type of this class itself
    typedef integral_constant<T,v> type;
    //! conversion to value_type/T
    operator value_type() { return value; }
  };

  //! type for true
  typedef integral_constant<bool, true> true_type;
  //! type for false
  typedef integral_constant<bool, false> false_type;
#endif // #else // #if HAVE_INTEGRAL_CONSTANT

  /** @} */
}
#endif
