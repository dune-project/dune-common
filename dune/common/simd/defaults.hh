#ifndef DUNE_COMMON_SIMD_DEFAULTS_HH
#define DUNE_COMMON_SIMD_DEFAULTS_HH

/** @file
 *  @brief Default implementations for SIMD Implementations
 *  @ingroup SIMDAbstract
 *
 * This file provides default overloads for SIMD implementation functions, and
 * deleted placeholders where there are no default implementations.
 *
 * This file should never be included by users of the SIMD
 * abstraction. Include <dune/common/simd/simd.hh> instead.
 */

#include <cstddef>

#include <dune/common/simd/base.hh>
#include <dune/common/simd/interface.hh>

namespace Dune {
  namespace Simd {
    namespace Overloads {

      /**
       * @addtogroup SIMDAbstract
       * @{
       */

      /** @name Overloadable and default functions
       *
       * This group contains functions that you, as an abstraction developer,
       * must implement.  All functions that are deleted must be provided,
       * functions that have a default implementation may be left
       * unimplemented if the default behaviour is satisfactory.
       *
       * @{
       */

      //! implements Simd::lane()
      template<class V>
      decltype(auto) lane(ADLTag<0>, std::size_t l, V v) = delete;

      //! implements Simd::valueCast()
      /**
       * By default returns \c t.  That should take care of rvalues and proper
       * references -- overloading is only needed if your abstraction layer
       * uses proxies.
       */
      template<class T>
      auto valueCast(ADLTag<0>, T t)
      {
        return t;
      }

      //! implements Simd::cond()
      template<class V>
      V cond(ADLTag<0>, Mask<V> mask, V ifTrue, V ifFalse) = delete;

      //! implements Simd::anyTrue()
      template<class Mask>
      bool anyTrue(ADLTag<0>, Mask mask) = delete;

      //! implements Simd::allTrue()
      /**
       * Default uses Simd::anyTrue()
       */
      template<class Mask>
      bool allTrue(ADLTag<0>, Mask mask)
      {
        return !Dune::Simd::anyTrue(!mask);
      }

      //! implements Simd::anyFalse()
      /**
       * Default uses Simd::anyTrue()
       */
      template<class Mask>
      bool anyFalse(ADLTag<0>, Mask mask)
      {
        return Dune::Simd::anyTrue(!mask);
      }

      //! implements Simd::allFalse()
      /**
       * Default uses Simd::anyTrue()
       */
      template<class Mask>
      bool allFalse(ADLTag<0>, Mask mask)
      {
        return !Dune::Simd::anyTrue(mask);
      }

      //! @} Overloadable and default functions
      //! @} Group SIMDAbstract
    } // namespace Overloads
  } // namespace Simd
} // namespace Dune

#endif // DUNE_COMMON_SIMD_DEFAULTS_HH
