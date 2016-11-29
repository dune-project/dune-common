#ifndef DUNE_COMMON_SIMD_VC_HH
#define DUNE_COMMON_SIMD_VC_HH

/** @file
 *  @brief SIMD abstractions for Vc
 *
 *
 * Here is a list of things that certain types do not support:
 *
 * - Vc::Vector<double> (Vc_1::Vector<double, Vc_1::VectorAbi::Avx>)
 *
 *   - Broadcast-assignment from a braced scalar (v = {s};)
 *
 *   - Broadcast-construction from a braced scalar (`V v{s};` or `V v = {s};`)
 *
 *   - "operator,": `(v1, v2)` constructs some kind of tuple, but the tests
 *     assume that they can access the result of "operator," through lane().
 *     That use of operator, is deprecated, but disable the check anyway so we
 *     can continue.  `(s1, v2)` and `(v1, s2)` seem to work, though.
 *
 * - Vc::Vector<int> (Vc_1::Vector<int, Vc_1::VectorAbi::Sse>)
 *
 *   - Broadcast-assignment from a braced scalar (`v = {s};`)
 *
 *   - << and >> if the left side is a scalar
 *
 *   - "operator,": `(v1, v2)` constructs some kind of tuple, but the tests
 *     assume that they can access the result of "operator," through lane().
 *     That use of operator, is deprecated, but disable the check anyway so we
 *     can continue.  `(s1, v2)` and `(v1, s2)` seem to work, though.
 *
 * - Vc::SimdArray<double, 4>
 *
 *   - Broadcast-assignment from a braced scalar (`v = {s};`)
 *
 *   - Broadcast-construction from a braced scalar (`V v{s};` or `V v = {s};`)
 *
 *   - Postfix/prefix -- and ++.
 *
 *   - operator&& and operator||
 *
 * - Vc::SimdArray<int, 4>
 *
 *   - Broadcast-assignment from a braced scalar (`v = {s};`)
 *
 *   - Broadcast-construction from a braced scalar (`V v{s};` or `V v = {s};`)
 *
 *   - Postfix/prefix -- and ++.
 *
 *   - << and >> if the left side is a scalar
 *
 *   - operator&& and operator||
 *
 * - Vc::Vector<int>::Mask
 *
 *   - Broadcast-assignment (`m = b;` or `m = {b};`)
 *
 *   - Implicit broadcast construction (`M m = b;` or `M m = {b};`)
 *
 *   - prefix +, -, ~
 *
 *   - infix *, /, %, +, -, <<, >>, <, >, <=, >= (all combinations)
 *
 *   - infix ==, != (scalar-vector and vector-scalar don't exist, and the
 *     result vector-vector is a scalar)
 *
 *   - infix &, ^, |, &&, || (scalar-vector and vector-scalar don't exist)
 *
 *   - infix =, &=, ^=, |= (vector-scalar missing, scalar-vector would be
 *     meaningless anyway)
 *
 *   - infix *=, /=, %=, +=, -=, <<=, >>= (vector-vector and vector-scalar,
 *     scalar-vector would not make sense)
 *
 * - Vc::SimdArray<int, 4>::Mask
 *   (Vc_1::SimdMaskArray<int, 4ul, Vc_1::Vector<int, Vc_1::VectorAbi::Sse>, 4ul>)
 *
 *   - Broadcast-assignment (`m = b;` or `m = {b};`)
 *
 *   - Implicit broadcast construction (`M m = b;` or `M m = {b};`)
 *
 *   - prefix +, -, ~
 *
 *   - infix *, /, %, +, -, <<, >>, <, >, <=, >= (all combinations)
 *
 *   - infix ==, != (scalar-vector and vector-scalar don't exist, and the
 *     result vector-vector is a scalar)
 *
 *   - infix &, ^, |, &&, || (scalar-vector and vector-scalar don't exist)
 *
 *   - infix =, &=, ^=, |= (vector-scalar missing, scalar-vector would be
 *     meaningless anyway)
 *
 *   - infix *=, /=, %=, +=, -=, <<=, >>= (vector-vector and vector-scalar,
 *     scalar-vector would not make sense)
 *
 */

#include <cstddef>
#include <type_traits>
#include <utility>

#include <Vc/Vc>

#include <dune/common/indices.hh>
#include <dune/common/simd/base.hh>
#include <dune/common/simd/defaults.hh> // for anyFalse()

namespace Dune {
  namespace Simd {

    namespace VcImpl {

      template<class V, class SFINAE = void>
      struct IsMask : std::false_type {};

      template<class T>
      struct IsMask<T, std::enable_if_t<
                           !std::is_same<T, std::decay_t<T> >::value> >
        : IsMask<std::decay_t<T> > {};

      template<typename T, typename A>
      struct IsMask<Vc::Mask<T, A> > : std::true_type {};

      template<typename T, std::size_t n, typename V, std::size_t m>
      struct IsMask<Vc::SimdMaskArray<T, n, V, m> > : std::true_type {};

      template<class V, class SFINAE = void>
      struct IsVector : IsMask<V> {};

      template<class T>
      struct IsVector<T, std::enable_if_t<
                       !std::is_same<T, std::decay_t<T> >::value> >
        : IsVector<std::decay_t<T> > {};

      template<typename T, typename A>
      struct IsVector<Vc::Vector<T, A> > : std::true_type {};

      template<typename T, std::size_t n, typename V, std::size_t m>
      struct IsVector<Vc::SimdArray<T, n, V, m> > : std::true_type {};

      //! A reference-like proxy for elements of random-access vectors.
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
      };
    } // namespace VcImpl

    namespace Overloads {

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
        : public index_constant<std::decay_t<V>::size()>
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

    } // namespace Overloads
  } // namespace Simd
} // namespace Dune

#endif // DUNE_COMMON_SIMD_VC_HH
