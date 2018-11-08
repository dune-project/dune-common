// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DEPRECATED_HH
#define DUNE_DEPRECATED_HH

/** \file
 * \brief Definition of the DUNE_DEPRECATED macro for the case that config.h
 *      is not available
 */

//! @addtogroup CxxUtilities
//! @{
#if defined(DOXYGEN) || !defined(HAS_ATTRIBUTE_DEPRECATED)
//! Mark some entity as deprecated
/**
 * \code
 *#include <dune/common/deprecated.hh>
 * \endcode
 *
 * This is a preprocessor define which can be used to mark functions,
 * typedefs, enums and other stuff deprecated.  If something is marked
 * deprecated, users are advised to migrate to the new interface, since it
 * will probably be removed in the next release of Dune.
 *
 * DUNE_DEPRECATED currently works with g++ and clang++.  For other compilers it will
 * be defined empty.  This way the user will not get any deprecation warning,
 * but at least his code still compiles (well, until the next Dune release,
 * that is).
 *
 * Here are some examples how to mark different stuff deprecated:
 *  - Classes
 *    \code
   class DUNE_DEPRECATED Class {}; // 1)
   class Class {} DUNE_DEPRECATED; // 2)
 *    \endcode
 *    Both forms do not work properly with g++-4.1: no deprecation warning
 *    will be given, although the code still compiles.  1) should be preferred
 *    over 2) since 2) does not work with clang++-1.1 (again, no warning given
 *    but code still compiles, works with clang++-3.1)
 *  - Template classes
 *    \code
   template<class T>
   class DUNE_DEPRECATED Class {}; // 1)
   template<class T>
   class Class {} DUNE_DEPRECATED; // 2)
 *    \endcode
 *    This works works with g++ >=4.3 only; g++-4.1 and
 *    clang++ compile the code without warning in both cases.  Furthermore,
 *    the warning is only triggered when copying an object of that template
 *    class, neither making a typedef nor simply creating such an object emit
 *    the warning.  It is thus recommended that some essential class member be
 *    marked deprecated as well, if possible.
 *  - Member constants
 *    \code
   template<typename T> struct Class {
   static const int c0 DUNE_DEPRECATED = 0;
   static const int DUNE_DEPRECATED c1 = 1;
   };
 *    \endcode
 *    Works with g++-4.1, g++ >=4.3 and clang++3.1.
 *    No warning but clean compile with clang++-1.1.
 *  - Member enumerators
 *    \code
   template<typename T> struct Class {
   enum enumeration { enumerator = 0 };
   };
 *    \endcode
 *    No form of deprecation is known that does not trigger an error on most
 *    compilers.
 *  - Member functions
 *    \code
   template<typename T> struct Class {
   void frob() DUNE_DEPRECATED {}
   }; // 1)
   template<typename T> struct Class {
   void DUNE_DEPRECATED frob() {}
   }; // 2)
   template<typename T> struct Class {
   DUNE_DEPRECATED void frob() {}
   }; // 3)
 *    \endcode
 *    With g++ only 2) emits a warning for templated member functions.
 */
#define DUNE_DEPRECATED
#else // defined(HAS_ATTRIBUTE_DEPRECATED)
#define DUNE_DEPRECATED __attribute__((deprecated))
#endif

#if defined(DOXYGEN) || !defined(HAS_ATTRIBUTE_DEPRECATED_MSG)
//! Mark some entity as deprecated
/**
 * \code
 *#include <dune/common/deprecated.hh>
 * \endcode
 *
 * This is a preprocessor define which can be used to mark functions,
 * typedefs, enums and other stuff deprecated and to also specify a
 * hint what replaces the given functionality.  If something is marked
 * deprecated, users are advised to migrate to the new interface,
 * since it will probably be removed in the next release of Dune.
 *
 * DUNE_DEPRECATED_MSG currently works only for compilers which
 * support the attribute __attribute__((deprecated("message")).  For
 * other compilers it will be defined empty.  This way the user will
 * not get any deprecation warning, but at least his code still
 * compiles (well, until the next Dune release, that is).
 *
 * Here are some examples how to mark different stuff deprecated:
 *  - Classes
 *    \code
   class DUNE_DEPRECATED_MSG("In the future, please use 'Glass'") Class {}; // 1)
   class Class {} DUNE_DEPRECATED_MSG("In the future, please use 'Glass'"); // 2)
 *    \endcode
 *    For both forms, deprecation warnings and the additional hint "In
 *    the future, please use 'Glass'" will be printed on compilers
 *    which support it (e.g. G++ >= 4.5.0, clang++ >= 1.1). For compilers
 *    which support deprecating but do not take an additional hint
 *    (e.g. G++ < 4.5.0), only the deprecation warning is printed, and
 *    finally compilers which do not support deprecation of code won't
 *    print anything, but the code will still compile.  1) should be
 *    preferred over 2) since 2) does not work with clang++-1.1
 *    (again, no warning will be given but code still compiles)
 *  - Template classes
 *    \code
   template<class T>
   class DUNE_DEPRECATED_MSG("In the future, please use 'Glass'") Class {}; // 1)
   template<class T>
   class Class {} DUNE_DEPRECATED_MSG("In the future, please use 'Glass'"); // 2)
 *    \endcode
 *    This works works with g++ >= 4.5, clang++ until at least version
 *    1.1 will compile the code without warning in both cases.
 *    Furthermore, the warning is only triggered when copying an
 *    object of that template class, neither making a typedef nor
 *    simply creating such an object emit the warning.  It is thus
 *    recommended that some essential class member be marked
 *    deprecated as well, if possible.
 *  - Member constants
 *    \code
   template<typename T> struct Class {
   static const int c0 DUNE_DEPRECATED_MSG("c2 is the new hype") = 0;
   static const int DUNE_DEPRECATED_MSG("c2 is the new hype") c1 = 1;
   };
 *    \endcode
 *    Works without printing the hint on g++-4.1, g++-4.3, g++-4.4 and
 *    fully on g++ >= 4.5.  Works for clang++-3.1.
 *    No warning but clean compile with clang++-1.1.
 *  - Member enumerators
 *    \code
   template<typename T> struct Class {
   enum enumeration { enumerator = 0 };
   };
 *    \endcode
 *    No form of deprecation is known that does not trigger an error on most
 *    compilers.
 *  - Member functions
 *    \code
   template<typename T> struct Class {
   void frob() DUNE_DEPRECATED_MSG("use frog") {}
   }; // 1)
   template<typename T> struct Class {
   void DUNE_DEPRECATED_MSG("use frog") frob() {}
   }; // 2)
   template<typename T> struct Class {
   DUNE_DEPRECATED_MSG("use frog") void frob() {}
   }; // 3)
 *    \endcode
 *    With g++ only 2) emits a warning for templated member functions.
 */
#define DUNE_DEPRECATED_MSG(text) DUNE_DEPRECATED
#else // defined(HAS_ATTRIBUTE_DEPRECATED_MSG)
#define DUNE_DEPRECATED_MSG(text) __attribute__((deprecated(# text)))
#endif

#ifdef DOXYGEN
/**
 * \brief Ignore deprecation warnings (start)
 *
 * This macro can be used together with `DUNE_NO_DEPRECATED_END` to mark a
 * block in which deprecation warnings are ignored.  This can be useful for
 * implementations of deprecated methods that call other deprecated methods
 * or for testing deprecated methods in the testsuite.
 *
 * \code
     DUNE_NO_DEPRECATED_BEGIN
     some_deprecated_function();
     another_deprecated_function();
     DUNE_NO_DEPRECATED_END
 * \endcode
 *
 * \warning This macro must always be used together with `DUNE_NO_DEPRECATED_END`
 */
#define DUNE_NO_DEPRECATED_BEGIN ...
/**
 * \brief Ignore deprecation warnings (end)
 *
 * \warning This macro must always be used together with `DUNE_NO_DEPRECATED_BEGIN`
 */
#define DUNE_NO_DEPRECATED_END   ...
#else
#  if defined __clang__
#    define DUNE_NO_DEPRECATED_BEGIN \
         _Pragma("clang diagnostic push") \
         _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#    define DUNE_NO_DEPRECATED_END _Pragma("clang diagnostic pop")
#  elif defined __INTEL_COMPILER
#    define DUNE_NO_DEPRECATED_BEGIN \
         _Pragma("warning push") \
         _Pragma("warning(disable:1478)") \
         _Pragma("warning(disable:1786)")
#    define DUNE_NO_DEPRECATED_END _Pragma("warning pop")
#  elif defined __GNUC__
#    define DUNE_NO_DEPRECATED_BEGIN \
         _Pragma("GCC diagnostic push") \
         _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#    define DUNE_NO_DEPRECATED_END _Pragma("GCC diagnostic pop")
#  else
#    define DUNE_NO_DEPRECATED_BEGIN /* Noop. */
#    define DUNE_NO_DEPRECATED_END   /* Noop. */
#  endif
#endif

//! @}

#endif
