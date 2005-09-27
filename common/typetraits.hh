// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_TYPETRAITS_HH__
#define __DUNE_TYPETRAITS_HH__

#include <dune/common/logictraits.hh>

namespace Dune
{

  /**
   * @file
   * @brief Traits for type conversions and type information.
   * @author Markus Blatt
   */
  /** @addtogroup Common
   *
   * @{
   */
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


  /**
   * @brief Removes a const qualifier while preserving others.
   */
  template<typename T>
  struct RemoveConst
  {
    typedef typename RemoveConstHelper<T, IsVolatile<T>::value>::Type Type;
  };

  /**
   * @brief Checks wether a type is derived from another.
   *
   * Inspired by
   * @link http://www.kotiposti.net/epulkkin/instructive/base-class-determination.html @endlink
   */
  template<class From, class To>
  class Conversion
  {
    typedef char Small;
    struct Big {char dummy[2];};
    static Small test(To);
    static Big test(...);
    static From makeFrom();
  public:
    enum {
      /** @brief True if the conversion exists. */
      exists =  sizeof(test(makeFrom())) == sizeof(Small),
      /** @brief Wether the conversion exists in both ways. */
      isTwoWay = exists && Conversion<To,From>::exists,
      /** @brief True if To and From are the same type. */
      sameType = false
    };
  };

  template<class T>
  class Conversion<T,T>{
  public:
    enum { exists=true, isTwoWay=true, sameType=true};
  };

  /**
   * @brief Checks wether two types are interoperable.
   *
   * Two types are interoperable conversions in either directions
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
      value=Or<Conversion<T1,T2>::exists,
          Conversion<T2,T1>::exists>::value
    };
  };

  /** \todo Please doc me! */
  template<bool b, typename T=void>
  struct EnableIf
  {
    typedef T type;
    typedef T Type;
  };

  template<typename T>
  struct EnableIf<false,T>
  {};

  /** \todo Please doc me! */
  template<class T1, class T2, class Type>
  struct EnableIfInterOperable
    : public EnableIf<IsInteroperable<T1,T2>::value, Type>
  {};


  /**
   * @brief Compile time test for testing whether
   * two types are the same.
   */
  template<typename T1, typename T2>
  struct SameType
  {
    enum { value=false};
  };


  template<typename T>
  struct SameType<T,T>
  {
    enum { value=true};
  };

  /**
   * @brief Select a type based on a condition.
   *
   * If template parameter first is true T1 is selected
   * otherwise T2 will be selected.
   * The selected type id accessible through the typedef
   * Type.
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
    typedef T1 Type;
  };

  template<class T1, class T2>
  struct SelectType<false,T1,T2>
  {
    typedef T2 Type;
  };
  /** @} */
}
#endif
