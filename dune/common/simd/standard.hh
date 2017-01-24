#ifndef DUNE_COMMON_SIMD_STANDARD_HH
#define DUNE_COMMON_SIMD_STANDARD_HH

/** @file
 *  @brief SIMD abstractions for the standard built-in types
 *
 * This file should never be included by users of the SIMD abstraction
 * (i.e. other Dune headers).  Neither should it be included by the
 * translation units passing built-in types to Dune headers that in turn
 * support SIMD types through the SIMD abstraction.  Dune-functionality always
 * supports built-in types.  Either because that functionality does not
 * support SIMD types and so only supports built-in types, or if it does
 * support SIMD types it must include <dune/common/simd/simd.hh>, which in
 * turn includes this header.
 */

#include <cstddef>
#include <type_traits>
#include <utility>

#include <dune/common/indices.hh>
#include <dune/common/simd/base.hh>
#include <dune/common/simd/defaults.hh> // for valueCast()
#include <dune/common/simd/isstandard.hh>

namespace Dune {
  namespace Simd {
    namespace Overloads {

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
      decltype(auto) lane(ADLTag<2, IsStandard<V>::value>,
                          std::size_t l, V &&v)
      {
        return std::forward<V>(v);
        // TODO: possibly do somthing like this unless NDEBUG?
        // return l == 0
        //   ? std::forward<V>(v)
        //   : throw Something;
      }

      // We don't use any proxies, so the default valueCast() is fine.
      // But we do need to #include defaults.hh to ensure it is available.

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

    } // namespace Overloads
  } // namespace Simd
} // namespace Dune

#endif // DUNE_COMMON_SIMD_STANDARD_HH
