#ifndef DUNE_COMMON_SIMD_STANDARD_HH
#define DUNE_COMMON_SIMD_STANDARD_HH

/** @file
 *  @ingroup SIMDStandard
 *  @brief SIMD abstractions for the standard built-in types
 *
 * This file should not normally be included by users of the SIMD abstraction
 * (i.e. other Dune headers).  Neither should it be included by the
 * translation units passing built-in types to Dune headers that in turn
 * support SIMD types through the SIMD abstraction.  Dune-functionality always
 * supports built-in types.  Either because that functionality does not
 * support SIMD types and so only supports built-in types, or if it does
 * support SIMD types it must include `<dune/common/simd/simd.hh>`, which in
 * turn includes this header.
 */

#include <complex>
#include <cstddef>
#include <type_traits>
#include <utility>

#include <dune/common/indices.hh>
#include <dune/common/simd/base.hh>
#include <dune/common/simd/defaults.hh>
#include <dune/common/simd/isstandard.hh>

/** @defgroup SIMDStandard SIMD Abstraction Implementation for standard types
 *  @ingroup SIMDApp
 *
 * This implements the vectorization interface for scalar types, in particular
 * the standard arithmetic types as per `std::is_arithmetic`, as well as
 * `std::complex<T>` for any supported type `T`.
 *
 * If you have a type that provides an interface sufficiently close to the
 * standard types, you can enable support for that type in this abstraction
 * implementation by specializing `Dune::Simd::IsStandard`.  Candidates for
 * this include extended precision types and automatic differentiation types.
 *
 * As an application developer, there is nothing special you need to do to get
 * support for standard types in the vectorization abstraction.  If the dune
 * classes you are using provide support for vectorization, they will include
 * `<dune/common/simd/simd.hh>`, which will pull in the abstraction for
 * standard types automatically.  You simply need to make sure that the types
 * themselves are supported:
 * - for built-in types there is nothing you need to do,
 * - for `std::complex`, you need to `#include <complex>`
 * - etc.
 */

namespace Dune {
  namespace Simd {

    //! Declare std::complex a standard type
    /**
     * \note This is done in `<dune/common/simd/standard.hh>` rather than
     *       `<dune/common/simd/isstandard.hh>` to in order not to have to
     *       include `<complex>` from the latter.
     */
    template<class T>
    struct IsStandard<std::complex<T> > : IsStandard<T> {};

    namespace Overloads {

      /** @name Specialized classes and overloaded functions
       *  @ingroup SIMDStandard
       *  @{
       */

      //! should have a member type \c type
      /**
       * Implements Simd::Scalar
       */
      template<class V>
      struct ScalarType<V, std::enable_if_t<IsStandard<V>::value> >
      {
        using type = V;
      };

      //! should have a member type \c type
      /**
       * Implements Simd::Index
       */
      template<class V>
      struct IndexType<V, std::enable_if_t<IsStandard<V>::value> >
      {
        using type = std::size_t;
      };

      //! should have a member type \c type
      /**
       * Implements Simd::Mask
       */
      template<class V>
      struct MaskType<V, std::enable_if_t<IsStandard<V>::value> >
      {
        using type = bool;
      };

      //! should be derived from an Dune::index_constant
      /**
       * Implements Simd::lanes()
       */
      template<class V>
      struct LaneCount<V, std::enable_if_t<IsStandard<V>::value> >
        : public index_constant<1>
      { };

      //! implements Simd::lane()
      template<class V>
      decltype(auto) lane(ADLTag<2, IsStandard<std::decay_t<V>>::value>,
                          std::size_t l, V &&v)
      {
        return std::forward<V>(v);
      }

      //! implements Simd::entries()
      template<class V>
      V cond(ADLTag<2, IsStandard<V>::value>,
             bool mask, V ifTrue, V ifFalse)
      {
        return mask ? ifTrue : ifFalse;
      }

      //! implements Simd::anyTrue()
      inline bool anyTrue(ADLTag<2>, bool mask) { return mask; }

      //! implements Simd::allTrue()
      inline bool allTrue(ADLTag<2>, bool mask) { return mask; }

      //! implements Simd::anyFalse()
      inline bool anyFalse(ADLTag<2>, bool mask) { return !mask; }

      //! implements Simd::allFalse()
      inline bool allFalse(ADLTag<2>, bool mask) { return !mask; }

      //! @} group SIMDStandard

    } // namespace Overloads
  } // namespace Simd
} // namespace Dune

#endif // DUNE_COMMON_SIMD_STANDARD_HH
