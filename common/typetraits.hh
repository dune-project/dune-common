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
    /** @brief True if T has a volatile specifier. */
    static const bool isVolatile=false;
    /** @brief True if T has a const qualifier. */
    static const bool isConst=false;
    /** @brief The unqualified type. */
    typedef T UnqualifiedType;
    /** @brief The const type. */
    typedef const T ConstType;
    /** @brief The const volatile Type. */
    typedef const volatile T ConstVolatileType;
  };

  template<typename T>
  struct ConstantVolatileTraits<const T>
  {
    static const bool isVolatile=false;
    static const bool isConst=true;
    typedef T UnqualifiedType;
    typedef const UnqualifiedType ConstType;
    typedef const volatile UnqualifiedType ConstVolatileType;
  };


  template<typename T>
  struct ConstantVolatileTraits<volatile T>
  {
    static const bool isVolatile=true;
    static const bool isConst=false;
    typedef T UnqualifiedType;
    typedef const UnqualifiedType ConstType;
    typedef const volatile UnqualifiedType ConstVolatileType;
  };

  template<typename T>
  struct ConstantVolatileTraits<const volatile T>
  {
    static const bool isVolatile=true;
    static const bool isConst=true;
    typedef T UnqualifiedType;
    typedef const UnqualifiedType ConstType;
    typedef const volatile UnqualifiedType ConstVolatileType;
  };

  /** @brief Tests wether a type is volatile. */
  template<typename T>
  struct IsVolatile
  {
    /** @brief True if The type is volatile. */
    const static bool value=ConstantVolatileTraits<T>::isVolatile;
  };

  /** @brief Tests wether a type is constant. */
  template<typename T>
  struct IsConst
  {
    /** @brief True if The type is constant. */
    const static bool value=ConstantVolatileTraits<T>::isConst;
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
   * @url{http://www.kotiposti.net/epulkkin/instructive/base-class-determination.html}
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
    /** @brief True if the conversion exists. */
    const static bool exists =  sizeof(test(makeFrom())) ==
                               sizeof(Small);
    /** @brief Wether the conversion exists in both ways. */
    const static bool esistsTowWay = exists && Conversion<To,From>::exists;
    /** @brief True if To and From are the same type. */
    const static bool sameType = false;
  };

  template<class T>
  class Conversion<T,T>{
  public:
    static const bool exists=true, isTwoWay=true, sameType=true;
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
    /**
     * @brief True if either a conversion from T1 to T2 or vice versa
     * exists.
     */
    const static bool value=Or<Conversion<T1,T2>::exists,
        Conversion<T2,T1>::exists>::value;
  };

  template<bool b, typename T=void>
  struct EnableIf
  {
    typedef T type;
  };

  template<typename T>
  struct EnableIf<false,T>
  {};

  template<class T1, class T2, class Type>
  struct EnableIfInterOperable
    : public EnableIf<IsInteroperable<T1,T2>::value, Type>
  {};


  /** @} */
}
#endif
