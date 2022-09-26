// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_SIMD_VC_HH
#define DUNE_COMMON_SIMD_VC_HH

/** @file
 *  @ingroup SIMDVc
 *  @brief SIMD abstractions for Vc
 */

#include <cstddef>
#include <type_traits>
#include <utility>

#include <dune/common/indices.hh>
#include <dune/common/simd/base.hh>
#include <dune/common/simd/defaults.hh> // for anyFalse()
#include <dune/common/simd/loop.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/vc.hh>

/** @defgroup SIMDVc SIMD Abstraction Implementation for Vc
 *  @ingroup SIMDApp
 *
 * This implements the vectorization interface for Vc types, namely
 * `Vc::Vector`, `Vc::Mask`, `Vc::SimdArray` and `Vc::SimdMask`.
 *
 * As an application developer, you need to `#include
 * <dune/common/simd/vc.hh>`.  You need to make sure that `HAVE_VC` is true
 * before doing so:
 *
 * - If your program works both in the presence and the absence of Vc, wrap
 *   the include in `#if HAVE_VC` and `#endif`
 *
 * - If you write a unit test, in your `CMakeLists.txt` use
 *   `dune_add_test(... CMAKE_GUARD Vc_FOUND)`
 *
 * You also need to make sure that the compiler uses the correct flags and
 * that the linker can find the library.  (The compilation flags include one
 * that ensures a name mangling scheme that can distinguish the
 * compiler-intrinsic vector types from non-vector types is used.)
 *
 * - Either use `add_dune_vc_flags(your_application)` in `CMakeLists.txt`,
 *
 * - or use `dune_enable_all_packages()` in your module's toplevel
 *   `CMakeLists.txt`.
 *
 * There should be no need to explicitly call `find_package(Vc)` in your
 * `CMakeLists.txt`, dune-common already does that.  If your module can't live
 * without Vc, you may however want to do something like this in your
 * `cmake/modules/YourModuleMacros.cmake`:
 * \code
 * if(NOT Vc_FOUND)
 *   message(SEND_ERROR "This module requires Vc")
 * endif(NOT Vc_FOUND)
 * \endcode
 *
 * If you just want to compile dune, and have Vc installed to a location where
 * cmake is not looking by default, you need to add that location to
 * `CMAKE_PREFIX_PATH`.  E.g. pass `-DCMAKE_PREFIX_PATH=$VCDIR` to cmake, for
 * instance by including that in the variable `CMAKE_FLAGS` in the options
 * file that you pass to dunecontrol.  `$VCDIR` should be the same directory
 * that you passed in `-DCMAKE_INSTALL_PREFIX=...` to cmake when you compiled
 * Vc, i.e. Vc's main include file should be found under
 * `$VCDIR/include/Vc/Vc`, and the library under `$VCDIR/lib/libVc.a` or
 * similar.
 *
 * @section SIMDVcRestrictions Restrictions
 *
 * During thorough testing of the Vc abstraction implementation it turned out
 * that certain operations were not supported, or were buggy.  This meant that
 * the tests had to be relaxed, and/or some restrictions had to made as to how
 * things must be done through the SIMD abstraction, see @ref
 * simd_abstraction_limit.
 *
 * For future reference, here is a detailed list of things that certain Vc
 * types do or don't support.  `s` denotes a scalar object/expression (i.e. of
 * type `double` or in the case of masks `bool`).  `v` denotes a vector/mask
 * object/expression.  `sv` means that both scalar and vector arguments are
 * accepted.  `V` denotes a vector/mask type.  `@` means any applicable
 * operator that is not otherwise listed.
 *
 * <!-- The following table is in orgtbl format -- If you are using emacs, you
 *      may want to enable the `orgtbl` minor mode.  We substitute `|` with
 *      `¦` when describing or-operators so as to not confuse orgtbl. -->
 * \code
   |                         | Vector       | Vector    | SimdArray  | SimdArray | Masks[4]  |
   |                         | <double> AVX | <int> SSE | <double,4> | <int,4>   |           |
   |-------------------------+--------------+-----------+------------+-----------+-----------|
   | V v(s);                 | y            | y         | y          | y         | y         |
   | V v = s;                | y            | y         | y          | y         | *N*       |
   | V v{s};                 | *N*          | y         | *N*        | *N*       | y         |
   | V v = {s};              | *N*          | y         | *N*        | *N*       | *N*       |
   |-------------------------+--------------+-----------+------------+-----------+-----------|
   | v = s;                  | y            | y         | y          | y         | *N*       |
   | v = {s};                | *N*          | *N*       | *N*        | *N*       | *N*       |
   |-------------------------+--------------+-----------+------------+-----------+-----------|
   | v++; ++v;               | y            | y         | *N*        | *N*       | y(n/a)[2] |
   | v--; --v;               | y            | y         | *N*        | *N*       | n/a       |
   |-------------------------+--------------+-----------+------------+-----------+-----------|
   | +v; -v;                 | y            | y         | y          | y         | *N*       |
   | !v;                     | y            | y         | y          | y         | y         |
   | ~v;                     | n/a          | y         | n/a        | y         | *N*       |
   |-------------------------+--------------+-----------+------------+-----------+-----------|
   | sv @ sv; but see below  | y            | y         | y          | y         | *N*       |
   |-------------------------+--------------+-----------+------------+-----------+-----------|
   | s << v; s >> v;         | n/a          | *N*       | n/a        | *N*       | *N*       |
   |-------------------------+--------------+-----------+------------+-----------+-----------|
   | v == v; v != v;         | y            | y         | y          | y         | *N* [1]   |
   |-------------------------+--------------+-----------+------------+-----------+-----------|
   | v & v; v ^ v; v ¦ v;    | n/a          | y         | n/a        | y         | y         |
   | sv && sv; sv ¦¦ sv;     | y            | y         | *N*        | *N*       | *N*       |
   | v && v; v ¦¦ v;         | y            | y         | *N*        | *N*       | y         |
   |-------------------------+--------------+-----------+------------+-----------+-----------|
   | v @= sv; but see below  | y            | y         | y          | y         | *N*       |
   | v &= v; v ^= v; v ¦= v; | n/a          | y         | n/a        | y         | y         |
   |-------------------------+--------------+-----------+------------+-----------+-----------|
   | v, v;[3]                | *N*          | *N*       | y          | y         | y         |
 * \endcode
 *
 * Notes:
 *
 * - [1] The result of the mask-mask `==` and `!=` operation is a scalar.
 *
 * - [2] `++` (either kind) on bools is deprecated by the standard
 *
 * - [3] contrary to the other operators, the expected result for `(sv1, sv2)`
 *   is exactly `sv2`, no broadcasting applied.
 *
 * - [4] Checked with `Vector<int>::Mask` [SSE] and `SimdArray<int, 4>::Mask`,
 *   which behaved identical
 *
 * Support levels:
 *
 * - `y`: operation generally works; some instances of the operation may not
 *   apply
 *
 * - `*N*`: operation generally does not work; some instances of the operation
 *   may not apply
 *
 * - `n/a`: operation does not apply (i.e. bitwise operations to
 *   floating-point operands, `--` (and in the future possibly `++`) to
 *   boolean operands, assignment operators to scalar left hand sides)
 *
 * Each operation was tested with the full set of combinations of possible
 * `const`/non-`const` lvalue/xvalue arguments.  Each combination of constness
 * and value category was applied to the scalar type and the operation tried
 * in an SFINAE context; combinations that failed here were skipped for vector
 * arguments too.
 */

namespace Dune {
  namespace Simd {

    namespace VcImpl {

      //! specialized to true for Vc mask types
      template<class V, class SFINAE = void>
      struct IsMask : std::false_type {};

      template<typename T, typename A>
      struct IsMask<Vc::Mask<T, A> > : std::true_type {};

      template<typename T, std::size_t n, typename V, std::size_t m>
      struct IsMask<Vc::SimdMaskArray<T, n, V, m> > : std::true_type {};

      //! specialized to true for Vc vector and mask types
      template<class V, class SFINAE = void>
      struct IsVector : IsMask<V> {};

      template<typename T, typename A>
      struct IsVector<Vc::Vector<T, A> > : std::true_type {};

      template<typename T, std::size_t n, typename V, std::size_t m>
      struct IsVector<Vc::SimdArray<T, n, V, m> > : std::true_type {};

      template<typename T> struct IsVectorizable : std::false_type {};
      template<> struct IsVectorizable<double>        : std::true_type {};
      template<> struct IsVectorizable<float>         : std::true_type {};
      template<> struct IsVectorizable<std::int32_t>  : std::true_type {};
      template<> struct IsVectorizable<std::uint32_t> : std::true_type {};
      template<> struct IsVectorizable<std::int16_t>  : std::true_type {};
      template<> struct IsVectorizable<std::uint16_t> : std::true_type {};

      //! A reference-like proxy for elements of random-access vectors.
      /**
       * This is necessary because Vc's lane-access operation return a proxy
       * that cannot constructed by non-Vc code (i.e. code that isn't
       * explicitly declared `friend`).  This means in particular that there
       * is no copy/move constructor, meaning we cannot return such proxies
       * from our own functions, such as `lane()`.  To work around this, we
       * define our own proxy class which internally holds a reference to the
       * vector and a lane index.
       *
       * Note: this should be unnecessary with C++17, as just returning a
       * temporary object should not involve copying it.
       */
      template<class V>
      class Proxy
      {
        static_assert(std::is_same<V, std::decay_t<V> >::value, "Class Proxy "
                      "may only be instantiated with unqualified types");
      public:
        using value_type = typename V::value_type;

      private:
        static_assert(std::is_arithmetic<value_type>::value,
                      "Only arithmetic types are supported");
        V &vec_;
        std::size_t idx_;

      public:
        Proxy(std::size_t idx, V &vec)
          : vec_(vec), idx_(idx)
        { }

        Proxy(const Proxy&) = delete;
        // allow move construction so we can return proxies from functions
        Proxy(Proxy&&) = default;

        operator value_type() const { return vec_[idx_]; }

        // assignment operators
#define DUNE_SIMD_VC_ASSIGNMENT(OP)                              \
        template<class T,                                        \
                 class = decltype(std::declval<value_type&>() OP \
                                  autoCopy(std::declval<T>()) )> \
        Proxy operator OP(T &&o) &&                              \
        {                                                        \
          vec_[idx_] OP autoCopy(std::forward<T>(o));            \
          return { idx_, vec_ };                                 \
        }
        DUNE_SIMD_VC_ASSIGNMENT(=);
        DUNE_SIMD_VC_ASSIGNMENT(*=);
        DUNE_SIMD_VC_ASSIGNMENT(/=);
        DUNE_SIMD_VC_ASSIGNMENT(%=);
        DUNE_SIMD_VC_ASSIGNMENT(+=);
        DUNE_SIMD_VC_ASSIGNMENT(-=);
        DUNE_SIMD_VC_ASSIGNMENT(<<=);
        DUNE_SIMD_VC_ASSIGNMENT(>>=);
        DUNE_SIMD_VC_ASSIGNMENT(&=);
        DUNE_SIMD_VC_ASSIGNMENT(^=);
        DUNE_SIMD_VC_ASSIGNMENT(|=);
#undef DUNE_SIMD_VC_ASSIGNMENT

        // unary (prefix) operators
        template<class T = value_type,
                 class = std::enable_if_t<!std::is_same<T, bool>::value> >
        Proxy operator++() { ++(vec_[idx_]); return *this; }
        template<class T = value_type,
                 class = std::enable_if_t<!std::is_same<T, bool>::value> >
        Proxy operator--() { --(vec_[idx_]); return *this; }

        // postfix operators
        template<class T = value_type,
                 class = std::enable_if_t<!std::is_same<T, bool>::value> >
        value_type operator++(int) { return vec_[idx_]++; }
        template<class T = value_type,
                 class = std::enable_if_t<!std::is_same<T, bool>::value> >
        value_type operator--(int) { return vec_[idx_]--; }


        // swap on proxies swaps the proxied vector entries.  As such, it
        // applies to rvalues of proxies too, not just lvalues
        friend void swap(const Proxy &a, const Proxy &b) {
          // don't use swap() ourselves -- not supported by Vc 1.3.0 (but is
          // supported by Vc 1.3.2)
          value_type tmp = std::move(a.vec_[a.idx_]);
          a.vec_[a.idx_] = std::move(b.vec_[b.idx_]);
          b.vec_[b.idx_] = std::move(tmp);
        }
        friend void swap(value_type &a, const Proxy &b) {
          // don't use swap() ourselves -- not supported by Vc 1.3.0 (but is
          // supported by Vc 1.3.2)
          value_type tmp = std::move(a);
          a = std::move(b.vec_[b.idx_]);
          b.vec_[b.idx_] = std::move(tmp);
        }
        friend void swap(const Proxy &a, value_type &b) {
          // don't use swap() ourselves -- not supported by Vc 1.3.0 (but is
          // supported by Vc 1.3.2)
          value_type tmp = std::move(a.vec_[a.idx_]);
          a.vec_[a.idx_] = std::move(b);
          b = std::move(tmp);
        }

        // binary operators
        //
        // Normally, these are provided by the conversion operator in
        // combination with C++'s builtin binary operators.  Other classes
        // that need to provide the binary operators themselves should either
        // 1. deduce the "foreign" operand type independently, i.e. use
        //      template<class... Args, class Foreign>
        //      auto operator@(MyClass<Args...>, Foreign);
        //    or
        // 2. not deduce anything from the foreign argument, i.e.
        //      template<class... Args>
        //      auto operator@(MyClass<Args...>,
        //                     typename MyClass<Args...>::value_type);
        //    or
        //      template<class T, class... Args>
        //      struct MyClass {
        //        auto operator@(T);
        //      }
        //    or
        //      template<class T, class... Args>
        //      struct MyClass {
        //        friend auto operator@(MyClass, T);
        //      }
        //
        // This allows either for an exact match (in the case of option 1.) or
        // for conversions to be applied to the foreign argument (options 2.).
        // In contrast, allowing some of the template parameters being deduced
        // from the self argument also being deduced from the foreign argument
        // will likely lead to ambiguous deduction when the foreign argument is
        // a proxy:
        //   template<class T, class... Args>
        //   auto operator@(MyClass<T, Args...>, T);
        // One class that suffers from this problem is std::complex.
        //
        // Note that option 1. is a bit dangerous, as the foreign argument is
        // catch-all.  This seems tempting in the case of a proxy class, as
        // the operator could just be forwarded to the proxied object with the
        // foreign argument unchanged, immediately creating interoperability
        // with arbitrary foreign classes.  However, if the foreign class also
        // choses option 1., this will result in ambiguous overloads, and there
        // is no clear guide to decide which class should provide the overload
        // and which should not.
        //
        // Fortunately, deferring to the conversion and the built-in operators
        // mostly works in the case of this proxy class, because only built-in
        // types can be proxied anyway.  Unfortunately, the Vc vectors and
        // arrays suffer from a slightly different problem.  They chose option
        // 1., but they can't just accept the argument type they are given,
        // since they need to somehow implement the operation in terms of
        // intrinsics.  So they check the argument whether it is one of the
        // expected types, and remove the operator from the overload set if it
        // isn't via SFINAE.  Of course, this proxy class is not one of the
        // expected types, even though it would convert to them...
        //
        // So what we have to do here, unfortunately, is to provide operators
        // for the Vc types explicitly, and hope that there won't be some Vc
        // version that gets the operators right, thus creating ambiguous
        // overloads.  Well, if guess it will be #ifdef time if it comes to
        // that.
#define DUNE_SIMD_VC_BINARY(OP)                                         \
        template<class T, class Abi>                                    \
        friend auto operator OP(const Vc::Vector<T, Abi> &l, Proxy&& r) \
          -> decltype(l OP std::declval<value_type>())                  \
        {                                                               \
          return l OP value_type(r);                                    \
        }                                                               \
        template<class T, class Abi>                                    \
        auto operator OP(const Vc::Vector<T, Abi> &r) &&                \
          -> decltype(std::declval<value_type>() OP r)                  \
        {                                                               \
          return value_type(*this) OP r;                                \
        }                                                               \
        template<class T, std::size_t n, class Vec, std::size_t m>      \
        friend auto                                                     \
        operator OP(const Vc::SimdArray<T, n, Vec, m> &l, Proxy&& r)    \
          -> decltype(l OP std::declval<value_type>())                  \
        {                                                               \
          return l OP value_type(r);                                    \
        }                                                               \
        template<class T, std::size_t n, class Vec, std::size_t m>      \
        auto operator OP(const Vc::SimdArray<T, n, Vec, m> &r) &&       \
          -> decltype(std::declval<value_type>() OP r)                  \
        {                                                               \
          return value_type(*this) OP r;                                \
        }

        DUNE_SIMD_VC_BINARY(*);
        DUNE_SIMD_VC_BINARY(/);
        DUNE_SIMD_VC_BINARY(%);
        DUNE_SIMD_VC_BINARY(+);
        DUNE_SIMD_VC_BINARY(-);
        DUNE_SIMD_VC_BINARY(<<);
        DUNE_SIMD_VC_BINARY(>>);
        DUNE_SIMD_VC_BINARY(&);
        DUNE_SIMD_VC_BINARY(^);
        DUNE_SIMD_VC_BINARY(|);
        DUNE_SIMD_VC_BINARY(<);
        DUNE_SIMD_VC_BINARY(>);
        DUNE_SIMD_VC_BINARY(<=);
        DUNE_SIMD_VC_BINARY(>=);
        DUNE_SIMD_VC_BINARY(==);
        DUNE_SIMD_VC_BINARY(!=);
#undef DUNE_SIMD_VC_BINARY

        // this is needed to implement broadcast construction from proxy as
        // the unadorned assignment operator cannot be a non-member
        template<class T, class Abi,
                 class = std::enable_if_t<std::is_convertible<value_type,
                                                              T>::value> >
        operator Vc::Vector<T, Abi>() &&
        {
          return value_type(*this);
        }
        template<class T, std::size_t n, class Vec, std::size_t m,
                 class = std::enable_if_t<std::is_convertible<value_type,
                                                              T>::value> >
        operator Vc::SimdArray<T, n, Vec, m>() &&
        {
          return value_type(*this);
        }

#define DUNE_SIMD_VC_ASSIGN(OP)                                         \
        template<class T, class Abi>                                    \
        friend auto operator OP(Vc::Vector<T, Abi> &l, Proxy&& r)       \
          -> decltype(l OP std::declval<value_type>())                  \
        {                                                               \
          return l OP value_type(r);                                    \
        }

        DUNE_SIMD_VC_ASSIGN(*=);
        DUNE_SIMD_VC_ASSIGN(/=);
        DUNE_SIMD_VC_ASSIGN(%=);
        DUNE_SIMD_VC_ASSIGN(+=);
        DUNE_SIMD_VC_ASSIGN(-=);
        DUNE_SIMD_VC_ASSIGN(&=);
        DUNE_SIMD_VC_ASSIGN(^=);
        DUNE_SIMD_VC_ASSIGN(|=);
        // The shift assignment would not be needed for Vc::Vector since it
        // has overloads for `int` rhs and the proxy can convert to that --
        // except that there is also overloads for Vector, and because of the
        // conversion operator needed to support unadorned assignments, the
        // proxy can convert to that, too.
        DUNE_SIMD_VC_ASSIGN(<<=);
        DUNE_SIMD_VC_ASSIGN(>>=);
#undef DUNE_SIMD_VC_ASSIGN
      };

    } // namespace VcImpl

    namespace Overloads {

      /** @name Specialized classes and overloaded functions
       *  @ingroup SIMDVc
       *  @{
       */

      //! should have a member type \c type
      /**
       * Implements Simd::Scalar
       */
      template<class V>
      struct ScalarType<V, std::enable_if_t<VcImpl::IsVector<V>::value> >
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
                        std::enable_if_t<VcImpl::IsVector<V>::value> >
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
      struct RebindType<bool, V, std::enable_if_t<VcImpl::IsVector<V>::value &&
                                                  !VcImpl::IsMask<V>::value>>
      {
        using type = typename V::mask_type;
      };

      //! should have a member type \c type
      /**
       * Implements Simd::Rebind
       *
       * This specialization covers
       * - Mask -> Scalar<Mask::Vector>
       */
      template<class M>
      struct RebindType<Scalar<typename M::Vector>, M,
                        std::enable_if_t<VcImpl::IsMask<M>::value>>
      {
        using type = typename M::Vector;
      };

      //! should have a member type \c type
      /**
       * Implements Simd::Rebind
       *
       * This specialization covers
       * - Mask -> Vc-vectorizable type except bool, Scalar<Mask::Vector>
       */
      template<class S, class M>
      struct RebindType<S, M,
                        std::enable_if_t<
                          VcImpl::IsMask<M>::value &&
                          VcImpl::IsVectorizable<S>::value &&
                          !std::is_same<S, Scalar<typename M::Vector> >::value
                          > >
      {
        using type = Vc::SimdArray<S, Simd::lanes<M>()>;
      };

      //! should have a member type \c type
      /**
       * Implements Simd::Rebind
       *
       * This specialization covers
       * - Vector -> Vc-vectorizable type except bool, Scalar<Vector>
       */
      template<class S, class V>
      struct RebindType<S, V,
                        std::enable_if_t<VcImpl::IsVector<V>::value &&
                                         !VcImpl::IsMask<V>::value &&
                                         VcImpl::IsVectorizable<S>::value &&
                                         !std::is_same<S, Scalar<V> >::value> >
      {
        using type = Vc::SimdArray<S, Simd::lanes<V>()>;
      };

      //! should have a member type \c type
      /**
       * Implements Simd::Rebind
       *
       * This specialization covers
       * - Mask -> non-Vc-vectorizable type except bool
       * - Vector -> non-Vc-vectorizable type except bool
       */
      template<class S, class V>
      struct RebindType<S, V,
                        std::enable_if_t<VcImpl::IsVector<V>::value &&
                                         !VcImpl::IsVectorizable<S>::value &&
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
      struct LaneCount<V, std::enable_if_t<VcImpl::IsVector<V>::value> >
        : public index_constant<V::size()>
      { };

      //! implements Simd::lane()
      template<class V>
      VcImpl::Proxy<V> lane(ADLTag<5, VcImpl::IsVector<V>::value>,
                            std::size_t l, V &v)
      {
        return { l, v };
      }

      //! implements Simd::lane()
      template<class V>
      Scalar<V> lane(ADLTag<5, VcImpl::IsVector<V>::value>,
                     std::size_t l, const V &v)
      {
        return v[l];
      }

      //! implements Simd::lane()
      /*
       * The hack with the SFINAE is necessary, because if I use just
       * Scalar<V> as the return type, the compiler still errors out if V is
       * an lvalue-reference T&.  You'd think he'd notice that he can't
       * instantiate this declaration for this template parameter, and would
       * simply remove it from the overload set, but no...
       */
      template<class V,
               class = std::enable_if_t<!std::is_reference<V>::value> >
      Scalar<V> lane(ADLTag<5, VcImpl::IsVector<V>::value>,
                     std::size_t l, V &&v)
      {
        return std::forward<V>(v)[l];
      }

      //! implements Simd::cond()
      template<class V>
      V cond(ADLTag<5, VcImpl::IsVector<V>::value &&
                       !VcImpl::IsMask<V>::value>,
             const Mask<V> &mask, const V &ifTrue, const V &ifFalse)
      {
        return Vc::iif(mask, ifTrue, ifFalse);
      }

      //! implements Simd::cond()
      /*
       * Kludge because iif seems to be unimplemented for masks
       */
      template<class V>
      V cond(ADLTag<5, VcImpl::IsMask<V>::value>,
             const V &mask, const V &ifTrue, const V &ifFalse)
      {
        return (mask && ifTrue) || (!mask && ifFalse);
      }

      //! implements binary Simd::max()
      template<class V>
      auto max(ADLTag<5, VcImpl::IsVector<V>::value &&
                         !VcImpl::IsMask<V>::value>,
               const V &v1, const V &v2)
      {
        return Simd::cond(v1 < v2, v2, v1);
      }

      //! implements binary Simd::max()
      template<class M>
      auto max(ADLTag<5, VcImpl::IsMask<M>::value>,
               const M &m1, const M &m2)
      {
        return m1 || m2;
      }

      //! implements binary Simd::min()
      template<class V>
      auto min(ADLTag<5, VcImpl::IsVector<V>::value &&
                         !VcImpl::IsMask<V>::value>,
               const V &v1, const V &v2)
      {
        return Simd::cond(v1 < v2, v1, v2);
      }

      //! implements binary Simd::min()
      template<class M>
      auto min(ADLTag<5, VcImpl::IsMask<M>::value>,
               const M &m1, const M &m2)
      {
        return m1 && m2;
      }

      //! implements Simd::anyTrue()
      template<class M>
      bool anyTrue (ADLTag<5, VcImpl::IsMask<M>::value>, const M &mask)
      {
        return Vc::any_of(mask);
      }

      //! implements Simd::allTrue()
      template<class M>
      bool allTrue (ADLTag<5, VcImpl::IsMask<M>::value>, const M &mask)
      {
        return Vc::all_of(mask);
      }

      // nothing like anyFalse() in Vc, so let defaults.hh handle it

      //! implements Simd::allFalse()
      template<class M>
      bool allFalse(ADLTag<5, VcImpl::IsMask<M>::value>, const M &mask)
      {
        return Vc::none_of(mask);
      }

      //! implements Simd::maxValue()
      template<class V>
      auto max(ADLTag<5, VcImpl::IsVector<V>::value &&
                         !VcImpl::IsMask<V>::value>,
               const V &v)
      {
        return v.max();
      }

      //! implements Simd::maxValue()
      template<class M>
      bool max(ADLTag<5, VcImpl::IsMask<M>::value>, const M &mask)
      {
        return Vc::any_of(mask);
      }

      //! implements Simd::minValue()
      template<class V>
      auto min(ADLTag<5, VcImpl::IsVector<V>::value &&
                         !VcImpl::IsMask<V>::value>,
               const V &v)
      {
        return v.min();
      }

      //! implements Simd::minValue()
      template<class M>
      bool min(ADLTag<5, VcImpl::IsMask<M>::value>, const M &mask)
      {
        return !Vc::any_of(!mask);
      }

      //! implements Simd::maskAnd()
      template<class S1, class V2>
      auto maskAnd(ADLTag<5, std::is_same<Mask<S1>, bool>::value &&
                             VcImpl::IsVector<V2>::value>,
                   const S1 &s1, const V2 &v2)
      {
        return Simd::Mask<V2>(Simd::mask(s1)) && Simd::mask(v2);
      }

      //! implements Simd::maskAnd()
      template<class V1, class S2>
      auto maskAnd(ADLTag<5, VcImpl::IsVector<V1>::value &&
                             std::is_same<Mask<S2>, bool>::value>,
                   const V1 &v1, const S2 &s2)
      {
        return Simd::mask(v1) && Simd::Mask<V1>(Simd::mask(s2));
      }

      //! implements Simd::maskOr()
      template<class S1, class V2>
      auto maskOr(ADLTag<5, std::is_same<Mask<S1>, bool>::value &&
                            VcImpl::IsVector<V2>::value>,
                   const S1 &s1, const V2 &v2)
      {
        return Simd::Mask<V2>(Simd::mask(s1)) || Simd::mask(v2);
      }

      //! implements Simd::maskOr()
      template<class V1, class S2>
      auto maskOr(ADLTag<5, VcImpl::IsVector<V1>::value &&
                            std::is_same<Mask<S2>, bool>::value>,
                   const V1 &v1, const S2 &s2)
      {
        return Simd::mask(v1) || Simd::Mask<V1>(Simd::mask(s2));
      }

      //! @} group SIMDVc

    } // namespace Overloads

  } // namespace Simd

  /*
   * Specialize IsNumber for Vc::SimdArray and Vc::Vector to be able to use
   * it as a scalar in DenseMatrix etc.
   */
  template <typename T, std::size_t N, class V, size_t Wt>
  struct IsNumber<Vc::SimdArray<T, N, V, Wt>>
    : public std::integral_constant<bool, IsNumber<T>::value> {
  };

  template <typename T, typename Abi>
  struct IsNumber<Vc::Vector<T, Abi>>
    : public std::integral_constant<bool, IsNumber<T>::value> {
  };

  //! Specialization of AutonomousValue for Vc proxies
  template<class V>
  struct AutonomousValueType<Simd::VcImpl::Proxy<V> > :
    AutonomousValueType<typename Simd::VcImpl::Proxy<V>::value_type> {};

} // namespace Dune

#endif // DUNE_COMMON_SIMD_VC_HH
