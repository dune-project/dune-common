// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_HASH_HH
#define DUNE_COMMON_HASH_HH

#include <dune/common/typetraits.hh>
#include <dune/common/static_assert.hh>

#if HAVE_STD_HASH
#include <functional>
#endif

#if HAVE_TR1_HASH
#include <tr1/functional>
#endif

#if HAVE_DUNE_BOOST

#include <boost/version.hpp>

// Boost 1.34.0 seems to be the first usable version of boost::functional::hash
#if BOOST_VERSION >= 103400
#define HAVE_BOOST_HASH 1

// only pull in boost if really necessary
#if !HAVE_STD_HASH && !HAVE_TR1_HASH

#include <boost/functional/hash.hpp>

#endif // !HAVE_STD_HASH && !HAVE_TR1_HASH
#endif // BOOST_VERSION >= 103400
#endif // HAVE_DUNE_BOOST

/**
 * \file
 * \brief Support for calculating hash values of objects.
 *
 * This file provides the functor Dune::hash to calculate hash values and
 * some infrastructure to simplify extending Dune::hash for user-defined types,
 * independent of the actual underlying implementation.
 *
 */



// ********************************************************************************
// Doxygen documentation
// ********************************************************************************

#ifdef DOXYGEN

namespace Dune {

  //! Functor for hashing objects of type T.
  /**
   * The interface outlined below is compatible with std::hash, std::tr1::hash and
   * boost::hash, so it is possible to use Dune::hash in associative containers from
   * those libraries.
   *
   * The current implementation piggybacks on top of C++11, TR1 or Boost, in that order.
   * As there is no local fallback implementation, hashing will not work without at least
   * one of those dependencies installed.
   */
  template<typename T>
  struct hash
  {

    //! Calculates the hash of t.
    std::size_t operator()(const T& t) const
    {
      return hash(t);
    }

  };

}

//! Defines the required struct specialization to make type hashable via `Dune::hash`.
/**
 * In order to calculate the hash, operator() of the generated specialization will
 * return the result of an unqualified call to the global function `hash_value(const type&)`.
 * As the call is not qualified, the function will be found using argument-dependent lookup,
 * allowing implementors to conveniently place it inside the class body.
 *
 * Consider the following type:
 *
 * \code
 * namespace ns {
 *   template<typename A, int i>
 *   class Foo
 *   {
 *     ...
 *   };
 * }
 * \endcode
 *
 * In order to add support for `Dune::hash`, you need to extend the definition like this:
 *
 * \code
 * namespace ns {
 *   template<typename A, int i>
 *   class Foo
 *   {
 *     ...
 *     // The keyword "friend" turns this into a global function that is a friend of Foo.
 *     inline friend std::size_t hash_value(const Foo& arg)
 *     {
 *       return ...;
 *     }
 *   };
 * }
 *
 * // Define hash struct specialization
 * DUNE_DEFINE_HASH(DUNE_HASH_TEMPLATE_ARGS(typename A, int i),DUNE_HASH_TYPE(Foo<A,i>))
 * \endcode
 *
 * \warning
 * As the specialization has to be placed in the original namespace of the
 * `hash` struct (e.g. `std`), this macro *must* be called from the global namespace!
 *
 * \param template_args  The template arguments required by the hash struct specialization,
 *                       wrapped in a call to DUNE_HASH_TEMPLATE_ARGS. If this is a complete
 *                       specialization, call DUNE_HASH_TEMPLATE_ARGS without arguments.
 * \param type           The exact type of the specialization, wrapped in a call to DUNE_HASH_TYPE.
 */
#define DUNE_DEFINE_HASH(template_args,type)


//! Wrapper macro for the template arguments in DUNE_DEFINE_HASH.
/**
 * This macro should always be used as a wrapper for the template arguments when calling DUNE_DEFINE_HASH.
 * It works around some preprocessor limitations when the template arguments contain commas or the list
 * is completely empty.
 */
#define DUNE_HASH_TEMPLATE_ARGS(...)

//! Wrapper macro for the type to be hashed in DUNE_DEFINE_HASH.
/**
 * This macro should always be used as a wrapper for the type of the specialization when calling
 * DUNE_DEFINE_HASH.
 * It works around some preprocessor limitations when the type contains commas.
 */
#define DUNE_HASH_TYPE(...)

#else // DOXYGEN - hide all the ugly implementation



// ********************************************************************************
// C++11 support
// ********************************************************************************

#if HAVE_STD_HASH
// We have std::hash from C++11

// Announce that we provide Dune::hash
#define HAVE_DUNE_HASH 1

// import std::hash into Dune namespace
namespace Dune {

  using std::hash;

}

// Macro for defining a std::hash specialization for type.
// This should not be called directly. Call DUNE_DEFINE_HASH
// instead.
#define DUNE_DEFINE_STD_HASH(template_args,type) \
  namespace std {                                  \
                                                 \
    template<template_args>                          \
    struct hash<type>                                \
    {                                                \
      std::size_t operator()(const type& arg) const  \
      {                                              \
        return hash_value(arg);                      \
      }                                              \
    };                                               \
                                                 \
  }                                                \

#else // HAVE_STD_HASH

// We do not support std::hash, so don't do anything here.
#define DUNE_DEFINE_STD_HASH(template_args,type)

#endif // HAVE_STD_HASH



// ********************************************************************************
// TR1 support
// ********************************************************************************

#if HAVE_TR1_HASH
// We have std::tr1::hash from TR1

#ifndef HAVE_DUNE_HASH
// std::hash wasn't found, so use std::tr1::hash

// Announce that we provide Dune::hash
#define HAVE_DUNE_HASH 1

// import std::tr1::hash into Dune namespace
namespace Dune {

  using std::tr1::hash;

}

#endif // HAVE_DUNE_HASH

// Macro for defining a std::tr1::hash specialization for type.
// This should not be called directly. Call DUNE_DEFINE_HASH
// instead.
#define DUNE_DEFINE_TR1_HASH(template_args,type) \
  namespace std {                                  \
    namespace tr1 {                                  \
                                                 \
      template<template_args>                          \
      struct hash<type>                                \
      {                                                \
        std::size_t operator()(const type& arg) const  \
        {                                              \
          return hash_value(arg);                      \
        }                                              \
      };                                               \
                                                 \
    }                                                \
  }                                                \

#else // HAVE_TR1_HASH

// We do not support std::tr1::hash, so don't do anything here.
#define DUNE_DEFINE_TR1_HASH(template_args,type)

#endif // HAVE_TR1_HASH



// ********************************************************************************
// common macros for both C++11 and TR1 support
// ********************************************************************************

#if HAVE_STD_HASH || HAVE_TR1_HASH

// Wrapper macro for template arguments.
// This is required because the template arguments can contain commas,
// which will create a macro argument list of unknown length. That in itself
// would not be a problem, but DUNE_DEFINE_HASH has to be called with two argument
// lists of unknown length. So this macro wraps its arguments with parentheses,
// turning it into a single argument. The result is used as the parameter list of
// an expansion macro in the calls to the implementation-specific macros
// for C++11 and TR1. Noto that technically, this trick is only legal for C++11,
// but pretty much every compiler supports variadic macros in C++03 mode, as they
// are part of C99.
#define DUNE_HASH_TEMPLATE_ARGS(...) (__VA_ARGS__)

// Wrapper macro for type to be hashed.
// See above for rationale.
#define DUNE_HASH_TYPE(...) (__VA_ARGS__)

// Expansion macro for the parenthesed argument lists created by
// DUNE_HASH_TEMPLATE_ARGS and DUNE_HASH_TYPE.
#define DUNE_HASH_EXPAND_VA_ARGS(...) __VA_ARGS__

// Define specializations for all discovered hash implementations.
#define DUNE_DEFINE_HASH(template_args,type)                                                  \
  DUNE_DEFINE_STD_HASH(DUNE_HASH_EXPAND_VA_ARGS template_args, DUNE_HASH_EXPAND_VA_ARGS type) \
  DUNE_DEFINE_TR1_HASH(DUNE_HASH_EXPAND_VA_ARGS template_args,DUNE_HASH_EXPAND_VA_ARGS type)  \


#else // HAVE_STD_HASH || HAVE_TR1_HASH


// Fallback implementation that doesn't do anything.
#define DUNE_DEFINE_HASH(template_args,type)

// Consider DUNE_HASH_TEMPLATE_ARGS as an argument-less macro and
// replace it with an empty token.
// This will leave its arguments in parentheses, causing them
// to be considered as a single argument to DUNE_DEFINE_HASH, which
// will ignore them anyway.
#define DUNE_HASH_TEMPLATE_ARGS

// Replace DUNE_HASH_TYPE with an empty token. See above for rationale.
#define DUNE_HASH_TYPE

#endif // HAVE_STD_HASH || HAVE_TR1_HASH



// ********************************************************************************
// Boost support
// ********************************************************************************

#if !HAVE_DUNE_HASH && HAVE_BOOST_HASH
// We haven't found a hash implementation yet and Boost is available

// Announce that we provide Dune::hash
#define HAVE_DUNE_HASH 1

// import boost::hash into Dune namespace
namespace Dune {

  using boost::hash;

}

// We no not need to register our types with boost::hash, as its extension
// mechanism will automatically pick up the global hash_value() functions.

#endif // !HAVE_DUNE_HASH && HAVE_BOOST_HASH

#endif // DOXYGEN



// ********************************************************************************
// Some utility functions for combining hashes of member variables.
// ********************************************************************************

#if HAVE_DUNE_HASH || defined(DOXYGEN)

namespace Dune {

  // The following functions are an implementation of the proposed hash extensions for
  // the C++ standard by Peter Dimov
  // (cf. http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2005/n1756.pdf, issue 6.18).
  // They are also contained in the boost::functional::hash library by Daniel James, but
  // that implementation uses boost::hash internally, while we want to use Dune::hash. They
  // are also considered for inclusion in TR2 (then based on std::hash, of course).

#ifndef DOXYGEN

  // helper struct for providing different hash combining algorithms dependent on
  // the size of size_t.
  // hash_combiner has to be specialized for the size (in bytes) of std::size_t.
  // Specialized versions should provide a method
  //
  // template <typename typeof_size_t, typename T>
  // void operator()(typeof_size_t& seed, const T& arg) const;
  //
  // that will be called by the interface function hash_combine() described further below.
  // The redundant template parameter typeof_size_t is needed to avoid warnings for the
  // unused 64-bit specialization on 32-bit systems.
  //
  // There is no default implementation!
  template<int sizeof_size_t>
  struct hash_combiner;


  // hash combining for 64-bit platforms.
  template<>
  struct hash_combiner<8>
  {

    template<typename typeof_size_t, typename T>
    void operator()(typeof_size_t& seed, const T& arg) const
    {
      dune_static_assert(sizeof(typeof_size_t)==8, "hash_combiner::operator() instantiated with nonmatching type and size");

      // The following algorithm for combining two 64-bit hash values is inspired by a similar
      // function in CityHash (http://cityhash.googlecode.com/svn-history/r2/trunk/src/city.h),
      // which is in turn based on ideas from the MurmurHash library. The basic idea is easy to
      // grasp, though: New information is XORed into the existing hash multiple times at different
      // places (using shift operations), and the resulting pattern is spread over the complete
      // range of available bits via multiplication with a "magic" constant. The constants used
      // below (47 and  0x9ddfea08eb382d69ULL) are taken from the CityHash implementation.
      //
      // We opted not to use the mixing algorithm proposed in the C++ working group defect list at
      // http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2005/n1756.pdf, p. 57f. because it
      // has very bad hash distribution properties if you apply it to lists of very small numbers,
      // an application that is frequent in PDELab's ordering framework.

      Dune::hash<T> hasher;
      const typeof_size_t kMul = 0x9ddfea08eb382d69ULL;
      typeof_size_t h = hasher(arg);
      typeof_size_t a = (seed ^ h) * kMul;
      a ^= (a >> 47);
      typeof_size_t b = (h ^ a) * kMul;
      b ^= (b >> 47);
      b *= kMul;
      seed = b;
    }

  };


  // hash combining for 32-bit platforms.
  template<>
  struct hash_combiner<4>
  {

    template<typename typeof_size_t, typename T>
    void operator()(typeof_size_t& seed, const T& arg) const
    {
      dune_static_assert(sizeof(typeof_size_t)==4, "hash_combiner::operator() instantiated with nonmatching type and size");

      // The default algorithm above requires a 64-bit std::size_t. The following algorithm is a
      // 32-bit compatible fallback, again inspired by CityHash and MurmurHash
      // (http://cityhash.googlecode.com/svn-history/r2/trunk/src/city.cc).
      // It uses 32-bit constants and relies on rotation instead of multiplication to spread the
      // mixed bits as that is apparently more efficient on IA-32. The constants used below are again
      // taken from CityHash, in particular from the file referenced above.

      Dune::hash<T> hasher;
      const typeof_size_t c1 = 0xcc9e2d51;
      const typeof_size_t c2 = 0x1b873593;
      const typeof_size_t c3 = 0xe6546b64;
      typeof_size_t h = hasher(arg);
      typeof_size_t a = seed * c1;
      a = (a >> 17) | (a << (32 - 17));
      a *= c2;
      h ^= a;
      h = (h >> 19) | (h << (32 - 19));
      seed = h * 5 + c3;
    }

  };

#endif // DOXYGEN

  //! Calculates the hash value of arg and combines it in-place with seed.
  /**
   * \note This function is only available if the macro `HAVE_DUNE_HASH` is defined.
   *
   * \param seed  The hash value that will be combined with the hash of arg.
   * \param arg   The object for which to calculate a hash value and combine it with seed.
   */
  template<typename T>
  inline void hash_combine(std::size_t& seed, const T& arg)
  {
    hash_combiner<sizeof(std::size_t)>()(seed,arg);
  }

  //! Hashes all elements in the range [first,last) and returns the combined hash.
  /**
   * \note This function is only available if the macro `HAVE_DUNE_HASH` is defined.
   *
   * \param first  Iterator pointing to the first object to hash.
   * \param last   Iterator pointing one past the last object to hash.

   * \returns      The result of hashing all objects in the range and combining them
   *               using hash_combine() in sequential fashion, starting with seed 0.
   */
  template<typename It>
  inline std::size_t hash_range(It first, It last)
  {
    std::size_t seed = 0;
    for (; first != last; ++first)
    {
      hash_combine(seed,*first);
    }
    return seed;
  }

  //! Hashes all elements in the range [first,last) and combines the hashes in-place with seed.
  /**
   * \note This function is only available if the macro `HAVE_DUNE_HASH` is defined.
   *
   * \param seed   Start value that will be combined with the hash values of all objects in
   *               the range using hash_combine() in sequential fashion.
   * \param first  Iterator pointing to the first ojbect to hash.
   * \param last   Iterator pointing one past the last object to hash.
   */
  template<typename It>
  inline void hash_range(std::size_t& seed, It first, It last)
  {
    for (; first != last; ++first)
    {
      hash_combine(seed,*first);
    }
  }

} // end namespace Dune

#endif // HAVE_DUNE_HASH || defined(DOXYGEN)

#endif // DUNE_COMMON_HASH_HH
