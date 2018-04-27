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
 * that ensures a name mangling scheme that can distiguish the
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

      //! A reference-like proxy for elements of random-access vectors.
      /**
       * This is necessary because Vc's lane-access operation return a proxy
       * that cannot constructed by non-Vc code (i.e. code that isn't
       * explicitly declared `friend`).  This means in particular that there
       * is no copy/move constructor, meaning we cannot return such proxies
       * from our own functions, such as `lane()`.  To work around this, we
       * define our own proxy class which internally holds a reference to the
       * vector and a lane index.
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
                      "Only artihmetic types are supported");
        V &vec_;
        std::size_t idx_;

      public:
        Proxy(std::size_t idx, V &vec)
          : vec_(vec), idx_(idx)
        { }

        operator value_type() const { return vec_[idx_]; }

        // postfix operators

        template<class T = value_type,
                 class = std::enable_if_t<!std::is_same<T, bool>::value> >
        value_type operator++(int) { return vec_[idx_]++; }
        template<class T = value_type,
                 class = std::enable_if_t<!std::is_same<T, bool>::value> >
        value_type operator--(int) { return vec_[idx_]--; }

        // unary (prefix) operators
        template<class T = value_type,
                 class = std::enable_if_t<!std::is_same<T, bool>::value> >
        Proxy &operator++() { ++(vec_[idx_]); return *this; }
        template<class T = value_type,
                 class = std::enable_if_t<!std::is_same<T, bool>::value> >
        Proxy &operator--() { --(vec_[idx_]); return *this; }
        decltype(auto) operator!() const { return !(vec_[idx_]); }
        decltype(auto) operator+() const { return +(vec_[idx_]); }
        decltype(auto) operator-() const { return -(vec_[idx_]); }
        template<class T = value_type,
                 class = std::enable_if_t<std::is_integral<T>::value> >
        decltype(auto) operator~() const { return ~(vec_[idx_]); }

        // binary operators
#define DUNE_SIMD_VC_BINARY_OP(OP)                                      \
        template<class T>                                               \
        auto operator OP(T &&o) const                                   \
          -> decltype(vec_[idx_] OP valueCast(std::forward<T>(o)))      \
        {                                                               \
          return vec_[idx_] OP valueCast(std::forward<T>(o));           \
        }
        DUNE_SIMD_VC_BINARY_OP(*);
        DUNE_SIMD_VC_BINARY_OP(/);
        DUNE_SIMD_VC_BINARY_OP(%);

        DUNE_SIMD_VC_BINARY_OP(+);
        DUNE_SIMD_VC_BINARY_OP(-);

        DUNE_SIMD_VC_BINARY_OP(<<);
        DUNE_SIMD_VC_BINARY_OP(>>);

        DUNE_SIMD_VC_BINARY_OP(<);
        DUNE_SIMD_VC_BINARY_OP(>);
        DUNE_SIMD_VC_BINARY_OP(<=);
        DUNE_SIMD_VC_BINARY_OP(>=);

        DUNE_SIMD_VC_BINARY_OP(==);
        DUNE_SIMD_VC_BINARY_OP(!=);

        DUNE_SIMD_VC_BINARY_OP(&);
        DUNE_SIMD_VC_BINARY_OP(^);
        DUNE_SIMD_VC_BINARY_OP(|);

        DUNE_SIMD_VC_BINARY_OP(&&);
        DUNE_SIMD_VC_BINARY_OP(||);
#undef DUNE_SIMD_VC_BINARY_OP

#define DUNE_SIMD_VC_ASSIGNMENT(OP)                             \
        template<class T>                                       \
        auto operator OP(T &&o)                                 \
          -> std::enable_if_t<AlwaysTrue<decltype(              \
                   vec_[idx_] OP valueCast(std::forward<T>(o))  \
                 )>::value, Proxy&>                             \
        {                                                       \
          vec_[idx_] OP valueCast(std::forward<T>(o));          \
          return *this;                                         \
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

        // swap on proxies swaps the proxied vector entries.  As such, it
        // applies to rvalues of proxies too, not just lvalues
        template<class V1, class V2>
        friend void swap(Proxy<V1> p1, Proxy<V2> p2);

        template<class T>
        friend void swap(Proxy p1, T& s2)
        {
          // don't use swap() ourselves -- not supported by Vc 1.3.0 (but is
          // supported by Vc 1.3.2)
          T tmp = p1.vec_[p1.idx_];
          p1.vec_[p1.idx_] = s2;
          s2 = tmp;
        }

        template<class T>
        friend void swap(T& s1, Proxy p2)
        {
          T tmp = s1;
          s1 = p2.vec_[p2.idx_];
          p2.vec_[p2.idx_] = tmp;
        }
      };

      template<class V1, class V2>
      void swap(Proxy<V1> p1, Proxy<V2> p2)
      {
        typename V1::value_type tmp = p1.vec_[p1.idx_];
        p1.vec_[p1.idx_] = p2.vec_[p2.idx_];
        p2.vec_[p2.idx_] = tmp;
      }

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
       * Implements Simd::Index
       */
      template<class V>
      struct IndexType<V, std::enable_if_t<VcImpl::IsVector<V>::value &&
                                           !VcImpl::IsMask<V>::value> >
      {
        using type = typename V::IndexType;
      };

      //! should have a member type \c type
      /**
       * Implements Simd::Index
       */
      template<class V>
      struct IndexType<V, std::enable_if_t<VcImpl::IsVector<V>::value &&
                                           VcImpl::IsMask<V>::value> >
      {
        using type = typename V::Vector::IndexType;
      };

      //! should have a member type \c type
      /**
       * Implements Simd::Mask
       */
      template<class V>
      struct MaskType<V, std::enable_if_t<VcImpl::IsVector<V>::value &&
                                          !VcImpl::IsMask<V>::value> >
      {
        using type = typename V::mask_type;
      };

      //! should have a member type \c type
      /**
       * Implements Simd::Mask
       */
      template<class V>
      struct MaskType<V, std::enable_if_t<VcImpl::IsVector<V>::value &&
                                          VcImpl::IsMask<V>::value> >
      {
        using type = V;
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

      //! implements Simd::valueCast()
      template<class V>
      Scalar<V> valueCast(ADLTag<5>, VcImpl::Proxy<V> p)
      {
        return p;
      }

      //! implements Simd::cond()
      template<class V>
      V cond(ADLTag<5, VcImpl::IsVector<V>::value &&
                       !VcImpl::IsMask<V>::value>,
             Mask<V> mask, V ifTrue, V ifFalse)
      {
        return Vc::iif(mask, ifTrue, ifFalse);
      }

      //! implements Simd::cond()
      /*
       * Kludge because iif seems to be unimplemented for masks
       */
      template<class V>
      V cond(ADLTag<5, VcImpl::IsMask<V>::value>,
             V mask, V ifTrue, V ifFalse)
      {
        return (mask && ifTrue) || (!mask && ifFalse);
      }

      //! implements Simd::anyTrue()
      template<class M>
      bool anyTrue (ADLTag<5, VcImpl::IsMask<M>::value>, M mask)
      {
        return Vc::any_of(mask);
      }

      //! implements Simd::allTrue()
      template<class M>
      bool allTrue (ADLTag<5, VcImpl::IsMask<M>::value>, M mask)
      {
        return Vc::all_of(mask);
      }

      // nothing like anyFalse() in Vc, so let defaults.hh handle it

      //! implements Simd::allFalse()
      template<class M>
      bool allFalse(ADLTag<5, VcImpl::IsMask<M>::value>, M mask)
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
      bool max(ADLTag<5, VcImpl::IsMask<M>::value>, M mask)
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
      bool min(ADLTag<5, VcImpl::IsMask<M>::value>, M mask)
      {
        return !Vc::any_of(!mask);
      }

      //! @} group SIMDVc

    } // namespace Overloads
  } // namespace Simd

  /*
   * Specialize IsNumber for Vc::SimdArray and Vc::Vector to be able to use
   * it as a scalar in DenseMatrix etc.
   */
  template <typename T, std::size_t N>
  struct IsNumber<Vc::SimdArray<T, N>>
    : public std::integral_constant<bool, IsNumber<T>::value> {
  };

  template <typename T, typename Abi>
  struct IsNumber<Vc::Vector<T, Abi>>
    : public std::integral_constant<bool, IsNumber<T>::value> {
  };

} // namespace Dune

#endif // DUNE_COMMON_SIMD_VC_HH
