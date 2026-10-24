// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_SIMD_VALUEPROXY_HH
#define DUNE_COMMON_SIMD_VALUEPROXY_HH

/** @file
 *  @brief A proxy for lanes of SIMD types with `operator[]`
 *
 * This file is meant for implementers of SIMD abstractions; it should not be
 * needed by users of the abstraction.
 */

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include <dune/common/typetraits.hh>

namespace Dune {
  namespace Simd {
    namespace Impl {

      //! Default for `ValueProxy`: no additional operations
      template<class Proxy, class ValueType>
      struct NoExtraOperations {};

      //! A reference-like proxy for a lane of a SIMD type with `operator[]`
      /**
       * SIMD libraries usually provide access to the lanes of a vector `v`
       * via `v[i]`.  For a mutable vector, the result is a library-specific
       * reference object, which can typically neither be copied nor moved,
       * and which may only accept assignments that do not lose precision.
       * Such objects cannot be returned from `Simd::lane()`.  This proxy
       * holds a reference to the vector and the lane index instead, and
       * behaves like a `Scalar<V>&`:
       *
       * - It converts to `value_type`, and can be assigned from anything a
       *   `value_type&` can be assigned from (also with compound
       *   assignments, `++` and `--`).  The operation is done on a copy of the
       *   lane, which is then written back with `v[i] = x`.
       * - In binary operations with a vector of type `V` or with another
       *   proxy, it behaves like a `value_type`.
       * - It is move-only.
       *
       * Operations with vectors of other scalar types (mixed precision) are
       * left to the SIMD library and to `ExtraOperations`, which is a base
       * class of the proxy that a library abstraction can use to add
       * operators for its own vector types.
       *
       * `ExtraOperations` is a base class that receives the proxy type as a
       * template parameter (CRTP), and not a class derived from the proxy,
       * so that `lane()` returns exactly the type that all operators expect:
       * - Its friend operators take `ValueProxy&&`, an exact match.  With a
       *   class derived from `ValueProxy`, the operators of `ValueProxy`
       *   would need a derived-to-base conversion, and could then lose
       *   overload resolution against catch-all operator templates of the
       *   SIMD library.
       * - The assignment operators return `ValueProxy`; in a derived class
       *   they would return the base class and lose the extra operators.
       * - There is one proxy class for all libraries, with one set of
       *   constructors and one `AutonomousValueType` specialization.
       *
       * Usage in an abstraction implementation:
       * \code
       *   template<class V>
       *   Impl::ValueProxy<V> lane(ADLTag<5, IsMyVector<V>::value>,
       *                            std::size_t l, V &v)
       *   {
       *     return { l, v };
       *   }
       * \endcode
       *
       * \tparam V                The SIMD type.  `V::value_type` must be the
       *                          scalar type, and `V` must support `v[i]` for
       *                          mutable and const objects.
       * \tparam ExtraOperations  A class template, used as base class
       *                          `ExtraOperations<ValueProxy, value_type>`.
       *                          It may provide friend operators and
       *                          conversion operators for library-specific
       *                          vector types, e.g. for mixed precision.
       */
      template<class V,
               template<class, class> class ExtraOperations = NoExtraOperations>
      class ValueProxy
        : public ExtraOperations<ValueProxy<V, ExtraOperations>,
                                 typename V::value_type>
      {
        static_assert(std::is_same<V, std::decay_t<V> >::value, "Class "
                      "ValueProxy may only be instantiated with unqualified "
                      "types");
        static_assert(IsIndexable<V&>::value && IsIndexable<const V&>::value,
                      "Class ValueProxy requires a SIMD type with "
                      "operator[]");
      public:
        using value_type = typename V::value_type;

      private:
        static_assert(std::is_arithmetic<value_type>::value,
                      "Only arithmetic types are supported");
        V &vec_;
        std::size_t idx_;

        value_type get() const { return std::as_const(vec_)[idx_]; }
        void set(const value_type &x) const { vec_[idx_] = x; }

      public:
        ValueProxy(std::size_t idx, V &vec)
          : vec_(vec), idx_(idx)
        { }

        ValueProxy(const ValueProxy&) = delete;
        // allow move construction so we can return proxies from functions
        ValueProxy(ValueProxy&&) = default;

        operator value_type() const { return get(); }

        // assignment operators
#define DUNE_SIMD_VALUEPROXY_ASSIGNMENT(OP)                      \
        template<class T,                                        \
                 class = decltype(std::declval<value_type&>() OP \
                                  autoCopy(std::declval<T>()) )> \
        ValueProxy operator OP(T &&o) &&                         \
        {                                                        \
          value_type x = get();                                  \
          x OP autoCopy(std::forward<T>(o));                     \
          set(x);                                                \
          return { idx_, vec_ };                                 \
        }
        DUNE_SIMD_VALUEPROXY_ASSIGNMENT(=);
        DUNE_SIMD_VALUEPROXY_ASSIGNMENT(*=);
        DUNE_SIMD_VALUEPROXY_ASSIGNMENT(/=);
        DUNE_SIMD_VALUEPROXY_ASSIGNMENT(%=);
        DUNE_SIMD_VALUEPROXY_ASSIGNMENT(+=);
        DUNE_SIMD_VALUEPROXY_ASSIGNMENT(-=);
        DUNE_SIMD_VALUEPROXY_ASSIGNMENT(<<=);
        DUNE_SIMD_VALUEPROXY_ASSIGNMENT(>>=);
        DUNE_SIMD_VALUEPROXY_ASSIGNMENT(&=);
        DUNE_SIMD_VALUEPROXY_ASSIGNMENT(^=);
        DUNE_SIMD_VALUEPROXY_ASSIGNMENT(|=);
#undef DUNE_SIMD_VALUEPROXY_ASSIGNMENT

        // unary (prefix) operators
        template<class T = value_type,
                 class = std::enable_if_t<!std::is_same<T, bool>::value> >
        ValueProxy operator++()
        {
          value_type x = get();
          set(++x);
          return { idx_, vec_ };
        }
        template<class T = value_type,
                 class = std::enable_if_t<!std::is_same<T, bool>::value> >
        ValueProxy operator--()
        {
          value_type x = get();
          set(--x);
          return { idx_, vec_ };
        }

        // postfix operators
        template<class T = value_type,
                 class = std::enable_if_t<!std::is_same<T, bool>::value> >
        value_type operator++(int)
        {
          value_type x = get();
          value_type result = x++;
          set(x);
          return result;
        }
        template<class T = value_type,
                 class = std::enable_if_t<!std::is_same<T, bool>::value> >
        value_type operator--(int)
        {
          value_type x = get();
          value_type result = x--;
          set(x);
          return result;
        }

        // swap on proxies swaps the proxied vector entries.  As such, it
        // applies to rvalues of proxies too, not just lvalues
        friend void swap(const ValueProxy &a, const ValueProxy &b) {
          value_type tmp = a.get();
          a.set(b.get());
          b.set(tmp);
        }
        friend void swap(value_type &a, const ValueProxy &b) {
          value_type tmp = std::move(a);
          a = b.get();
          b.set(tmp);
        }
        friend void swap(const ValueProxy &a, value_type &b) {
          value_type tmp = a.get();
          a.set(std::move(b));
          b = std::move(tmp);
        }

        // binary operators with the vector type V (or types derived from
        // it), between proxies, and with arithmetic scalars
        //
        // The scalar operand is `value_type`, so the proxy behaves like the
        // scalar it represents.  The vector operand is deduced and
        // constrained with `std::derived_from`, so neither operand is
        // converted implicitly; otherwise e.g. `proxy == scalar` would become
        // ambiguous with the built-in comparison.  Libraries
        // with catch-all operator templates for their own types (e.g. Vc's
        // `operator@(Vector<T, Abi>, U&&)`) need more specialized overloads
        // in `ExtraOperations`.  The overloads with arithmetic scalars are
        // needed for libraries whose vectors broadcast from anything that
        // converts to their scalar type, including this proxy (e.g.
        // std::experimental::simd): without them, `proxy << c` with a
        // `char c` would be ambiguous between the built-in operator and the
        // library's vector-scalar operator.
#define DUNE_SIMD_VALUEPROXY_BINARY(OP)                                 \
        template<std::derived_from<V> W>                                \
        friend auto operator OP(const W &l, ValueProxy &&r)             \
          -> decltype(l OP std::declval<value_type>())                  \
        {                                                               \
          return l OP r.get();                                          \
        }                                                               \
        template<std::derived_from<V> W>                                \
        friend auto operator OP(ValueProxy &&l, const W &r)             \
          -> decltype(std::declval<value_type>() OP r)                  \
        {                                                               \
          return l.get() OP r;                                          \
        }                                                               \
        template<class D = void>                                        \
        friend auto operator OP(ValueProxy &&l, ValueProxy &&r)         \
          -> decltype(std::declval<std::conditional_t<true, value_type, D> >() \
                      OP std::declval<value_type>())                    \
        {                                                               \
          return l.get() OP r.get();                                    \
        }                                                               \
        template<class S,                                               \
                 std::enable_if_t<std::is_arithmetic<S>::value, int> = 0> \
        friend auto operator OP(const S &l, ValueProxy &&r)             \
          -> decltype(l OP std::declval<value_type>())                  \
        {                                                               \
          return l OP r.get();                                          \
        }                                                               \
        template<class S,                                               \
                 std::enable_if_t<std::is_arithmetic<S>::value, int> = 0> \
        friend auto operator OP(ValueProxy &&l, const S &r)             \
          -> decltype(std::declval<value_type>() OP r)                  \
        {                                                               \
          return l.get() OP r;                                          \
        }

        DUNE_SIMD_VALUEPROXY_BINARY(*);
        DUNE_SIMD_VALUEPROXY_BINARY(/);
        DUNE_SIMD_VALUEPROXY_BINARY(%);
        DUNE_SIMD_VALUEPROXY_BINARY(+);
        DUNE_SIMD_VALUEPROXY_BINARY(-);
        DUNE_SIMD_VALUEPROXY_BINARY(<<);
        DUNE_SIMD_VALUEPROXY_BINARY(>>);
        DUNE_SIMD_VALUEPROXY_BINARY(&);
        DUNE_SIMD_VALUEPROXY_BINARY(^);
        DUNE_SIMD_VALUEPROXY_BINARY(|);
        DUNE_SIMD_VALUEPROXY_BINARY(<);
        DUNE_SIMD_VALUEPROXY_BINARY(>);
        DUNE_SIMD_VALUEPROXY_BINARY(<=);
        DUNE_SIMD_VALUEPROXY_BINARY(>=);
        DUNE_SIMD_VALUEPROXY_BINARY(==);
        DUNE_SIMD_VALUEPROXY_BINARY(!=);
#undef DUNE_SIMD_VALUEPROXY_BINARY

#define DUNE_SIMD_VALUEPROXY_ASSIGN(OP)                                 \
        template<std::derived_from<V> W>                                \
        friend auto operator OP(W &l, ValueProxy &&r)                   \
          -> decltype(l OP std::declval<value_type>())                  \
        {                                                               \
          return l OP r.get();                                          \
        }

        DUNE_SIMD_VALUEPROXY_ASSIGN(*=);
        DUNE_SIMD_VALUEPROXY_ASSIGN(/=);
        DUNE_SIMD_VALUEPROXY_ASSIGN(%=);
        DUNE_SIMD_VALUEPROXY_ASSIGN(+=);
        DUNE_SIMD_VALUEPROXY_ASSIGN(-=);
        DUNE_SIMD_VALUEPROXY_ASSIGN(&=);
        DUNE_SIMD_VALUEPROXY_ASSIGN(^=);
        DUNE_SIMD_VALUEPROXY_ASSIGN(|=);
        DUNE_SIMD_VALUEPROXY_ASSIGN(<<=);
        DUNE_SIMD_VALUEPROXY_ASSIGN(>>=);
#undef DUNE_SIMD_VALUEPROXY_ASSIGN
      };

    } // namespace Impl
  } // namespace Simd

  //! Specialization of AutonomousValue for lane proxies
  template<class V, template<class, class> class ExtraOperations>
  struct AutonomousValueType<Simd::Impl::ValueProxy<V, ExtraOperations> > :
    AutonomousValueType<typename V::value_type> {};

} // namespace Dune

#endif // DUNE_COMMON_SIMD_VALUEPROXY_HH
