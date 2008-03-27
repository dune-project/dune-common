// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_STATIC_ASSERT_HH
#define DUNE_STATIC_ASSERT_HH

#include "deprecated.hh"

/**
 * @addtogroup Common
 *
 * @{
 */

/**
    \brief (DEPRECATED) Helper template so that compilation fails if condition is not true.

    \deprecated Use dune_static_assert instead.

    If the condition is true a static function yes is available, othewise the
    only function available is no().

    Example for compile time check whether two types are the same:
    \code
    IsTrue<is_same<int,int>::value>::yes(); //
    IsTrue<is_same<bool,int>::value>::yes(); // false, will trigger a compile time error
    \endcode

    A test that trigger a compile time error if condition is true:
    \code
    IsTrue<condition>::no()
    \endcode
 */
template <bool condition>
struct IsTrue
{
  static void no() DUNE_DEPRECATED {};
};

template <>
struct IsTrue<true>
{
  static void yes() DUNE_DEPRECATED {};
};


#include <iostream>

// Taken from BOOST
//
// Helper macro CPPMAGIC_JOIN:
// The following piece of macro magic joins the two
// arguments together, even when one of the arguments is
// itself a macro (see 16.3.1 in C++ standard).  The key
// is that macro expansion of macro arguments does not
// occur in CPPMAGIC_DO_JOIN2 but does in CPPMAGIC_DO_JOIN.
//
#define CPPMAGIC_JOIN( X, Y ) CPPMAGIC_DO_JOIN( X, Y )
#define CPPMAGIC_DO_JOIN( X, Y ) CPPMAGIC_DO_JOIN2(X,Y)
#define CPPMAGIC_DO_JOIN2( X, Y ) X ## Y

template <bool x> struct static_assert_failure;

template <> struct static_assert_failure<true> { };

template<int x> struct static_assert_test {};

/**
    \brief Helper template so that compilation fails if condition is not true.

    \code
    dune_static_assert(CONDITION, ERRORMSG);
    \endcode

    If CONDITION is not true, dune_static_assert fails.

    If the C++0x language feature static_assert is available, dune_static_assert
    should break down static_assert. Otherwise dune_static_assert implements a
    test that trigger a compile time error if condition is false.

    Example:

    \code
    dune_static_assert(1<=2, "error");
    dune_static_assert((is_same<int,int>::value),  "error msg");
    dune_static_assert((is_same<bool,int>::value), "error msg"); // false, will trigger a compile time error
    \endcode

    Be aware that...
    <ol>
    <li>dune_static_assert is not in the namespace Dune</li>
    <li>you must use extra brackets, you condition contains ','.
    This is because dune_static_assert is a preprocessor macro</li>
    </ol>

 */
#define dune_static_assert(COND,MSG) \
  typedef static_assert_test<\
    sizeof(static_assert_failure< (bool)( COND )>)\
    > CPPMAGIC_JOIN (dune_static_assert_typedef_, __LINE__)

/* @} */

#endif
