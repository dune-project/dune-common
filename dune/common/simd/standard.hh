// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
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

#include <cstddef>
#include <type_traits>
#include <utility>

#include <dune/common/indices.hh>
#include <dune/common/simd/base.hh>
#include <dune/common/simd/defaults.hh>

/** @defgroup SIMDStandard SIMD Abstraction Implementation for standard types
 *  @ingroup SIMDApp
 *
 * This implements the vectorization interface for scalar types.  It applies
 * to any type that does not have a specialized interface implementation.
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

    namespace Overloads {

      /** @name Specialized classes and overloaded functions
       *  @ingroup SIMDStandard
       *  @{
       */

      //! should have a member type \c type
      /**
       * Implements Simd::Scalar
       */
      template<class V, class>
      struct ScalarType { using type = V; };

      //! should have a member type \c type
      /**
       * Implements Simd::Rebind
       */
      template<class S, class, class>
      struct RebindType { using type = S; };

      //! should be derived from an Dune::index_constant
      /**
       * Implements Simd::lanes()
       */
      template<class, class>
      struct LaneCount : public index_constant<1> { };

      //! implements Simd::lane()
      /**
       * This binds to rvalues and const lvalues.  It would bind to non-const
       * lvalues too, but those are caught by the overload with ADLTag<3>.
       * Using a universal reference here would bind to any argument with a
       * perfect match.  This would mean ambiguous overloads with other
       * abstractions, if those only declare overloads for `const TheirType &`
       * and `TheirType &`, because because universal references match
       * perfectly.
       */
      template<class V>
      V lane(ADLTag<2>, std::size_t, V v)
      {
        return v;
      }

      template<class V>
      V &lane(ADLTag<3>, std::size_t, V &v)
      {
        return v;
      }

      // No Simd::cond() implementation, the overload for bool masks in the
      // interface is sufficient

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
