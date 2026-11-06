// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_SIMD_STDSIMD_HH
#define DUNE_COMMON_SIMD_STDSIMD_HH

/** @file
 *  @ingroup SIMDStdSimd
 *  @brief SIMD abstractions for `std::experimental::simd`
 */

#include <dune-common-config.hh> // DUNE_HAVE_CXX_EXPERIMENTAL_SIMD

#if DUNE_HAVE_CXX_EXPERIMENTAL_SIMD

#include <cstddef>
#include <type_traits>
#include <utility>

#include <experimental/simd>

#include <dune/common/indices.hh>
#include <dune/common/simd/base.hh>
#include <dune/common/simd/defaults.hh> // for anyFalse()
#include <dune/common/simd/loop.hh>
#include <dune/common/simd/valueproxy.hh>
#include <dune/common/typetraits.hh>

/** @defgroup SIMDStdSimd SIMD Abstraction Implementation for std::experimental::simd
 *  @ingroup SIMDApp
 *
 * This implements the vectorization interface for the types of the C++
 * Parallelism TS v2, namely `std::experimental::simd<T, Abi>` and
 * `std::experimental::simd_mask<T, Abi>`, e.g. `native_simd<double>` or
 * `fixed_size_simd<double, 4>`.
 *
 * The types are specified in section 9 "Data-Parallel Types" of the
 * Technical Specification "C++ Extensions for Parallelism Version 2"
 * (ISO/IEC TS 19570:2018).  A freely available reference is
 * https://en.cppreference.com/w/cpp/experimental/simd.  The types have been
 * adopted into C++26 as `std::simd`.
 *
 * As an application developer, you need to `#include
 * <dune/common/simd/stdsimd.hh>`.  It is only available if the standard
 * library provides `std::experimental::simd`, which is indicated by the
 * preprocessor macro `DUNE_HAVE_CXX_EXPERIMENTAL_SIMD`:
 *
 * - If your program works both with and without it, wrap the use in
 *   `#if DUNE_HAVE_CXX_EXPERIMENTAL_SIMD` and `#endif`.
 *
 * - If you write a unit test, in your `CMakeLists.txt` use
 *   `dune_add_test(... CMAKE_GUARD DUNE_HAVE_CXX_EXPERIMENTAL_SIMD)`.
 *
 * No library or special compiler flags are needed.  The number of lanes of
 * `native_simd<T>` depends on the target architecture, so compile with
 * e.g. `-march=native` to make use of the vector instructions of your CPU.
 *
 * Rebinding a vector type to another scalar type yields
 * `std::experimental::rebind_simd_t`, which may change the ABI tag,
 * e.g. from `native_simd<float>` to a `fixed_size_simd<double, N>`.  A
 * rebind round trip therefore does not necessarily yield the original type.
 *
 * @section SIMDStdSimdRestrictions Restrictions
 *
 * The types follow the rules of the Parallelism TS, in particular:
 *
 * - Element types are the arithmetic types except `bool`.
 * - Scalars are only broadcast implicitly if the conversion does not lose
 *   precision, e.g. `native_simd<float>() * 0.5` does not compile; use
 *   `Scalar<V>(0.5)`.  This also applies to operations of lanes of `double`
 *   vectors with `float` vectors.
 * - Masks with different element types cannot be combined.
 * - A mask cannot be copy-initialized from a `bool` (`M m = true;`); use
 *   `M m(true);`.
 */

namespace Dune {
  namespace Simd {

    namespace StdSimdImpl {

      namespace stdx = std::experimental;

      //! true for `std::experimental::simd_mask` types
      template<class V>
      struct IsMask : std::bool_constant<stdx::is_simd_mask_v<V> > {};

      //! true for `std::experimental::simd` and `simd_mask` types
      template<class V>
      struct IsVector
        : std::bool_constant<stdx::is_simd_v<V> || stdx::is_simd_mask_v<V> >
      {};

      //! true for the element types of `std::experimental::simd`
      template<class T>
      struct IsVectorizable
        : std::bool_constant<std::is_arithmetic<T>::value &&
                             !std::is_same<T, bool>::value> {};

      //! the vector type of a vector or mask type
      template<class V, class = void>
      struct VectorType { using type = V; };

      template<class M>
      struct VectorType<M, std::enable_if_t<IsMask<M>::value> >
      {
        using type = typename M::simd_type;
      };

      //! Operations of lane proxies with `std::experimental::simd` types
      /**
       * Base class of the lane proxy, see `Impl::ValueProxy`.  `Proxy` is
       * the proxy class, `ValueType` its `value_type`.
       *
       * The library's vectors broadcast from anything that converts to their
       * scalar type, including the proxy.  For operations with a vector of
       * another scalar type this can make the library's operators ambiguous,
       * e.g. for `w << lane(l, v)` with `w` of type `simd<int>` and `v` of
       * type `simd<unsigned>`.  These overloads match exactly and do the
       * operation with the scalar, so the proxy behaves like the scalar it
       * represents.
       */
      template<class Proxy, class ValueType>
      struct ProxyOperations
      {
        using value_type = ValueType;

#define DUNE_SIMD_STDSIMD_BINARY(OP)                                    \
        template<class T, class Abi>                                    \
        friend auto operator OP(const stdx::simd<T, Abi> &l, Proxy&& r) \
          -> decltype(l OP std::declval<value_type>())                  \
        {                                                               \
          return l OP value_type(r);                                    \
        }                                                               \
        template<class T, class Abi>                                    \
        friend auto operator OP(Proxy&& l, const stdx::simd<T, Abi> &r) \
          -> decltype(std::declval<value_type>() OP r)                  \
        {                                                               \
          return value_type(l) OP r;                                    \
        }

        DUNE_SIMD_STDSIMD_BINARY(*);
        DUNE_SIMD_STDSIMD_BINARY(/);
        DUNE_SIMD_STDSIMD_BINARY(%);
        DUNE_SIMD_STDSIMD_BINARY(+);
        DUNE_SIMD_STDSIMD_BINARY(-);
        DUNE_SIMD_STDSIMD_BINARY(<<);
        DUNE_SIMD_STDSIMD_BINARY(>>);
        DUNE_SIMD_STDSIMD_BINARY(&);
        DUNE_SIMD_STDSIMD_BINARY(^);
        DUNE_SIMD_STDSIMD_BINARY(|);
        DUNE_SIMD_STDSIMD_BINARY(<);
        DUNE_SIMD_STDSIMD_BINARY(>);
        DUNE_SIMD_STDSIMD_BINARY(<=);
        DUNE_SIMD_STDSIMD_BINARY(>=);
        DUNE_SIMD_STDSIMD_BINARY(==);
        DUNE_SIMD_STDSIMD_BINARY(!=);
#undef DUNE_SIMD_STDSIMD_BINARY

#define DUNE_SIMD_STDSIMD_ASSIGN(OP)                                    \
        template<class T, class Abi>                                    \
        friend auto operator OP(stdx::simd<T, Abi> &l, Proxy&& r)       \
          -> decltype(l OP std::declval<value_type>())                  \
        {                                                               \
          return l OP value_type(r);                                    \
        }

        DUNE_SIMD_STDSIMD_ASSIGN(*=);
        DUNE_SIMD_STDSIMD_ASSIGN(/=);
        DUNE_SIMD_STDSIMD_ASSIGN(%=);
        DUNE_SIMD_STDSIMD_ASSIGN(+=);
        DUNE_SIMD_STDSIMD_ASSIGN(-=);
        DUNE_SIMD_STDSIMD_ASSIGN(&=);
        DUNE_SIMD_STDSIMD_ASSIGN(^=);
        DUNE_SIMD_STDSIMD_ASSIGN(|=);
        DUNE_SIMD_STDSIMD_ASSIGN(<<=);
        DUNE_SIMD_STDSIMD_ASSIGN(>>=);
#undef DUNE_SIMD_STDSIMD_ASSIGN
      };

      //! A reference-like proxy for lanes of `std::experimental::simd` types
      template<class V>
      using Proxy = Impl::ValueProxy<V, ProxyOperations>;

    } // namespace StdSimdImpl

    namespace Overloads {

      /** @name Specialized classes and overloaded functions
       *  @ingroup SIMDStdSimd
       *  @{
       */

      //! should have a member type \c type
      /**
       * Implements Simd::Scalar
       */
      template<class V>
      struct ScalarType<V,
                        std::enable_if_t<StdSimdImpl::IsVector<V>::value> >
      {
        using type = typename V::value_type;
      };

      //! should have a member type \c type
      /**
       * Implements Simd::Rebind
       *
       * This specialization covers
       * - Mask -> bool
       * - Vector -> Scalar<Vector>
       */
      template<class V>
      struct RebindType<Simd::Scalar<V>, V,
                        std::enable_if_t<StdSimdImpl::IsVector<V>::value> >
      {
        using type = V;
      };

      //! should have a member type \c type
      /**
       * Implements Simd::Rebind
       *
       * This specialization covers
       * - Vector -> bool
       */
      template<class V>
      struct RebindType<bool, V,
                        std::enable_if_t<StdSimdImpl::IsVector<V>::value &&
                                         !StdSimdImpl::IsMask<V>::value> >
      {
        using type = typename V::mask_type;
      };

      //! should have a member type \c type
      /**
       * Implements Simd::Rebind
       *
       * This specialization covers
       * - Mask -> Scalar<Mask::simd_type>
       */
      template<class M>
      struct RebindType<Scalar<typename M::simd_type>, M,
                        std::enable_if_t<StdSimdImpl::IsMask<M>::value> >
      {
        using type = typename M::simd_type;
      };

      //! should have a member type \c type
      /**
       * Implements Simd::Rebind
       *
       * This specialization covers
       * - Mask -> vectorizable type except bool, Scalar<Mask::simd_type>
       * - Vector -> vectorizable type except bool, Scalar<Vector>
       *
       * The result is the vector type `std::experimental::rebind_simd_t` of
       * the vector type of `V`, which may have a different ABI.
       */
      template<class S, class V>
      struct RebindType<S, V,
                        std::enable_if_t<
                          StdSimdImpl::IsVector<V>::value &&
                          StdSimdImpl::IsVectorizable<S>::value &&
                          !std::is_same<
                            S, Scalar<typename StdSimdImpl::VectorType<V>::type>
                            >::value> >
      {
        using type = std::experimental::rebind_simd_t<
          S, typename StdSimdImpl::VectorType<V>::type>;
      };

      //! should have a member type \c type
      /**
       * Implements Simd::Rebind
       *
       * This specialization covers
       * - Mask -> non-vectorizable type except bool
       * - Vector -> non-vectorizable type except bool
       */
      template<class S, class V>
      struct RebindType<S, V,
                        std::enable_if_t<
                          StdSimdImpl::IsVector<V>::value &&
                          !StdSimdImpl::IsVectorizable<S>::value &&
                          !std::is_same<S, bool>::value &&
                          !std::is_same<S, Scalar<V> >::value> >
      {
        using type = LoopSIMD<S, Simd::lanes<V>()>;
      };

      //! should be derived from an Dune::index_constant
      /**
       * Implements Simd::lanes()
       */
      template<class V>
      struct LaneCount<V,
                       std::enable_if_t<StdSimdImpl::IsVector<V>::value> >
        : public index_constant<V::size()>
      { };

      //! implements Simd::lane()
      template<class V>
      StdSimdImpl::Proxy<V> lane(ADLTag<5, StdSimdImpl::IsVector<V>::value>,
                                 std::size_t l, V &v)
      {
        return { l, v };
      }

      //! implements Simd::lane()
      template<class V>
      Scalar<V> lane(ADLTag<5, StdSimdImpl::IsVector<V>::value>,
                     std::size_t l, const V &v)
      {
        return v[l];
      }

      //! implements Simd::lane()
      template<class V,
               class = std::enable_if_t<!std::is_reference<V>::value> >
      Scalar<V> lane(ADLTag<5, StdSimdImpl::IsVector<V>::value>,
                     std::size_t l, V &&v)
      {
        return std::as_const(v)[l];
      }

      //! implements Simd::cond()
      template<class V>
      V cond(ADLTag<5, StdSimdImpl::IsVector<V>::value>,
             const Mask<V> &mask, const V &ifTrue, const V &ifFalse)
      {
        V result = ifFalse;
        std::experimental::where(mask, result) = ifTrue;
        return result;
      }

      //! implements binary Simd::max()
      template<class V>
      V max(ADLTag<5, StdSimdImpl::IsVector<V>::value &&
                      !StdSimdImpl::IsMask<V>::value>,
            const V &v1, const V &v2)
      {
        return std::experimental::max(v1, v2);
      }

      //! implements binary Simd::max()
      template<class M>
      M max(ADLTag<5, StdSimdImpl::IsMask<M>::value>,
            const M &m1, const M &m2)
      {
        return m1 || m2;
      }

      //! implements binary Simd::min()
      template<class V>
      V min(ADLTag<5, StdSimdImpl::IsVector<V>::value &&
                      !StdSimdImpl::IsMask<V>::value>,
            const V &v1, const V &v2)
      {
        return std::experimental::min(v1, v2);
      }

      //! implements binary Simd::min()
      template<class M>
      M min(ADLTag<5, StdSimdImpl::IsMask<M>::value>,
            const M &m1, const M &m2)
      {
        return m1 && m2;
      }

      //! implements Simd::anyTrue()
      template<class M>
      bool anyTrue (ADLTag<5, StdSimdImpl::IsMask<M>::value>, const M &mask)
      {
        return std::experimental::any_of(mask);
      }

      //! implements Simd::allTrue()
      template<class M>
      bool allTrue (ADLTag<5, StdSimdImpl::IsMask<M>::value>, const M &mask)
      {
        return std::experimental::all_of(mask);
      }

      // anyFalse() is handled by defaults.hh

      //! implements Simd::allFalse()
      template<class M>
      bool allFalse(ADLTag<5, StdSimdImpl::IsMask<M>::value>, const M &mask)
      {
        return std::experimental::none_of(mask);
      }

      //! implements Simd::maxValue()
      template<class V>
      auto max(ADLTag<5, StdSimdImpl::IsVector<V>::value &&
                         !StdSimdImpl::IsMask<V>::value>,
               const V &v)
      {
        return std::experimental::hmax(v);
      }

      //! implements Simd::maxValue()
      template<class M>
      bool max(ADLTag<5, StdSimdImpl::IsMask<M>::value>, const M &mask)
      {
        return std::experimental::any_of(mask);
      }

      //! implements Simd::minValue()
      template<class V>
      auto min(ADLTag<5, StdSimdImpl::IsVector<V>::value &&
                         !StdSimdImpl::IsMask<V>::value>,
               const V &v)
      {
        return std::experimental::hmin(v);
      }

      //! implements Simd::minValue()
      template<class M>
      bool min(ADLTag<5, StdSimdImpl::IsMask<M>::value>, const M &mask)
      {
        return std::experimental::all_of(mask);
      }

      //! implements Simd::maskAnd()
      template<class S1, class V2>
      auto maskAnd(ADLTag<5, std::is_same<Mask<S1>, bool>::value &&
                             StdSimdImpl::IsVector<V2>::value>,
                   const S1 &s1, const V2 &v2)
      {
        return Simd::Mask<V2>(Simd::mask(s1)) && Simd::mask(v2);
      }

      //! implements Simd::maskAnd()
      template<class V1, class S2>
      auto maskAnd(ADLTag<5, StdSimdImpl::IsVector<V1>::value &&
                             std::is_same<Mask<S2>, bool>::value>,
                   const V1 &v1, const S2 &s2)
      {
        return Simd::mask(v1) && Simd::Mask<V1>(Simd::mask(s2));
      }

      //! implements Simd::maskOr()
      template<class S1, class V2>
      auto maskOr(ADLTag<5, std::is_same<Mask<S1>, bool>::value &&
                            StdSimdImpl::IsVector<V2>::value>,
                  const S1 &s1, const V2 &v2)
      {
        return Simd::Mask<V2>(Simd::mask(s1)) || Simd::mask(v2);
      }

      //! implements Simd::maskOr()
      template<class V1, class S2>
      auto maskOr(ADLTag<5, StdSimdImpl::IsVector<V1>::value &&
                            std::is_same<Mask<S2>, bool>::value>,
                  const V1 &v1, const S2 &s2)
      {
        return Simd::mask(v1) || Simd::Mask<V1>(Simd::mask(s2));
      }

      //! @} group SIMDStdSimd

    } // namespace Overloads

  } // namespace Simd

  /*
   * Specialize IsNumber for std::experimental::simd to be able to use it as a
   * scalar in DenseMatrix etc.
   */
  template<class T, class Abi>
  struct IsNumber<std::experimental::simd<T, Abi> >
    : public std::integral_constant<bool, IsNumber<T>::value> {
  };

} // namespace Dune

#endif // DUNE_HAVE_CXX_EXPERIMENTAL_SIMD

#endif // DUNE_COMMON_SIMD_STDSIMD_HH
