// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_SIMD_INTERFACE_HH
#define DUNE_COMMON_SIMD_INTERFACE_HH

/** @file
 *  @brief User interface of the SIMD abstraction
 *  @ingroup SIMDLib
 *
 * This file provides the user interface functions of the SIMD abstraction
 * layer.
 *
 * This file should never be included by users of the SIMD
 * abstraction. Include <dune/common/simd/simd.hh> instead.
 */

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

#include <dune/common/simd/base.hh>
#include <dune/common/typelist.hh>

namespace Dune {

  //! @brief Namespace for vectorization interface functions used by library
  //!        developers
  /**
   * @ingroup SIMDLib
   */
  namespace Simd {

    /** @addtogroup SIMDLib
     *
     * @{
     *
     * @section understand_simd Understanding SIMD types
     *
     * The (idealized) model of a SIMD type `V` used in this abstraction layer
     * is that they are fixed-length vectors of some scalar type `S`.
     * Operations and operators that take values of type `S` as arguments,
     * except for `operator,()`, should be overloaded to support values of
     * type `V` too.  These operations should apply element-wise.  If the
     * operation takes more than one argument, it should accept arbitrary
     * combinations of `V` and `S`.  The exception is the combination of `S`
     * on the left hand side and `V` on the right hand side of one of the
     * assignment operators, which does not make sense.
     *
     * The result of a boolean operation is a mask type `M`, which is a SIMD
     * type with scalar type `bool` with the same number of elements as `V`.
     * The result of all other operations is again of type `V`, or of some
     * type convertible to `V`.
     *
     * This is very similar to `std::valarray`, with the main difference
     * being that `std::valarray` is dynamic in size, while for this
     * abstraction the size is static.
     *
     * @section SIMDLibPromoWarn Type promotion issues
     *
     * True SIMD types have an issue with type promotion, which means they
     * cannot behave completely analogous to built-in integral types (this is
     * a non-issue with floating point types).  Essentially, operations on
     * true SIMD types cannot promote their arguments, because the promoted
     * types typically require more storage than the original types, meaning
     * an argument that was passed in a single vector register would need
     * multiple vector registers after promotion, which would mean greater
     * register pressure.  Also, there would be conversion operations
     * required, which (at least on x86) is not typically the case for
     * promotions of the built-in types.  Lastly, with larger types the vector
     * units can typically operate on fewer lanes at a time.
     *
     * Omitting integral promotions has in many cases no negative impact,
     * because many programmers do not really expect them anyway.  There are
     * however cases where they matter, and for illustration I want to explain
     * one that crept up during unit testing.
     *
     * Here is a simplified (and somewhat pseudo-code) version of the test.
     * The test checks the result of unary `-` on `Vc::Vector<unsigned short>`
     * by comparing the result of unary `-` when applied to the complete
     * vector to the result of unary `-` when applied to each lane
     * individually.
     * \code
     * Vc::Vector<unsigned short> varg;
     * for(std::size_t l = 0; l < lanes(varg); ++l)
     *   lane(l, varg) = l + 1;
     * auto vresult = -varg;
     * for(std::size_t l = 0; l < lanes(varg); ++l)
     *   assert(lane(l, vresult) == -lane(l, varg));
     * \endcode
     * The test fails in lane 0.  On the left side of the `==`, `lane(0,
     * vresult)` is `(unsigned short)65535`, which is the same as `(unsigned
     * short)-1`, as it should be.  On the right side, `lane(0, varg)` is
     * `(unsigned short)1`.  `-` promotes its argument, so that becomes
     * `(int)1`, and the result of the negation is `(int)-1`.
     *
     * Now the comparison is `(unsigned short)65535 == (int)-1`.  The
     * comparison operator applies the *usual arithmetic conversions* to bring
     * both operands to the same type.  In this case this boils down to
     * converting the left side to `int` via integral promotions and the
     * comparison becomes `(int)65535 == (int)-1`.  The result is of course
     * `false` and the assertion triggers.
     *
     * The only way to thoroughly prevent this kind of problem is to convert
     * the result of any operation back to the expected type.  In the above
     * example, the assertion would need to be written as `assert(lane(l,
     * vresult) == static_cast<unsigned short>(-lane(l, varg)));`.  In
     * practice, this should only be a problem with operations on unsigned
     * types where the result may be "negative".  Most code in Dune will want
     * to operate on floating point types, where this is a non-issue.
     *
     * (Of course, this is also a problem for code that operates on untrusted
     * input, but you should not be doing that with Dune anyway).
     *
     * Still, when writing code using the SIMD abstractions, you should be
     * aware that in the following snippet
     * \code
     * auto var1 = lane(0, -vec);
     * auto var2 = -lane(0, vec);
     * \endcode
     * the exact types of `var1` and `var2` may be somewhat surprising.
     *
     * @section simd_abstraction_limit Limitations of the Abstraction Layer
     *
     * Since the abstraction layer cannot overload operators of SIMD types
     * (that would be meddling with the domain of the library that provides
     * the SIMD types), nor provide its own constructors, there are severe
     * limitations in what the abstraction layer guarantees.  Besides the
     * standard types, the first SIMD library supported is Vc, so that is
     * where most of the limitations stem from; see \ref SIMDVcRestrictions in
     * \ref SIMDVc.
     *
     * The biggest limitations are with masks.  In Vc masks support a very
     * restricted set of operations compared to other SIMD types, so in what
     * follows we will distinguish between masks with a very small set of
     * operations and between vectors with a larger set of operations.
     *
     * Here is a compact table of the limitations as a quick reference,
     * together with suggested workarounds for the constructs that don't work.
     * `s` denotes a scalar object/expression (i.e. of type `double` or in the
     * case of masks `bool`).  `v` denotes a vector/mask object/expression.
     * `sv` means that both scalar and vector arguments are accepted.  `V`
     * denotes a vector/mask type.  `@` means any applicable operator that is
     * not otherwise listed.
     *
     * <!-- The following table is in orgtbl format -- If you are using emacs,
     *      you may want to enable the `orgtbl` minor mode.  We substitute `|`
     *      with `¦` when describing or-operators so as to not confuse
     *      orgtbl. -->
     * \code
       |                         | Vectors | workaround                 | Masks       | workaround       |
       |-------------------------+---------+----------------------------+-------------+------------------|
       | V v(s);                 | y       |                            | y           |                  |
       | V v = s;                | y       | V v(s);                    | *N*         | V v(s);          |
       | V v{s};                 | *N*     | V v(s);                    | y           | V v(s);          |
       | V v = {s};              | *N*     | V v(s);                    | y           | V v(s);          |
       |-------------------------+---------+----------------------------+-------------+------------------|
       | v = s;                  | y       | v = V(s);                  | *N*         | v = V(s);        |
       | v = {s};                | *N*     | v = V(s);                  | *N*         | v = V(s);        |
       |-------------------------+---------+----------------------------+-------------+------------------|
       | v++; ++v;               | *N*     | v += Scalar<V>(1);         | *N*(n/a)[2] | v = V(true);     |
       | v--; --v;               | *N*     | v -= Scalar<V>(1);         | n/a         |                  |
       |-------------------------+---------+----------------------------+-------------+------------------|
       | +v; -v;                 | y       |                            | *N*         | none             |
       | !v;                     | y       |                            | y           |                  |
       | ~v;                     | y       |                            | *N*         | none             |
       |-------------------------+---------+----------------------------+-------------+------------------|
       | sv @ sv; but see below  | y       |                            | *N*         | none             |
       |-------------------------+---------+----------------------------+-------------+------------------|
       | s << v; s >> v;         | *N*     | v << V(s);                 | *N*         | none             |
       |-------------------------+---------+----------------------------+-------------+------------------|
       | v == v; v != v;         | y       |                            | *N* [1]     | !(v ^ v); v ^ v; |
       |-------------------------+---------+----------------------------+-------------+------------------|
       | v & v; v ^ v; v ¦ v;    | y       |                            | y           |                  |
       | v && v; v ¦¦ v;         | *N*     | maskAnd(v,v); maskOr(v,v); | y           |                  |
       |-------------------------+---------+----------------------------+-------------+------------------|
       | v @= sv; but see below  | y       |                            | *N*         | none             |
       | v &= v; v ^= v; v ¦= v; | y       |                            | y           |                  |
       |-------------------------+---------+----------------------------+-------------+------------------|
       | v, v;[3,4]              | *N*     | void(v), v;                | y           |                  |
     * \endcode
     *
     * Notes:
     *
     * - [1] In Vc, mask-mask `==` and `!=` operations exist, but the result
     *   is of type `bool`, i.e. a scalar.
     *
     * - [2] `++` (either kind) on bools is deprecated by the standard.  Our
     *   test suite does not check for it on masks, but it was supported by Vc
     *   masks at some point.
     *
     * - [3] Contrary to the other operators, the expected result for `(sv1,
     *   sv2)` is exactly `sv2`, no broadcasting applied.
     *
     * - [4] Try to avoid the use of `operator,` unless both operands are
     *   built-in types if possible.  Libraries had a tendency to overload
     *   `operator,` to provide for things like container initialization
     *   before C++11, and these overloads may still be present in the library
     *   you are using and replace the default meaning of `operator,`.
     *
     * Support levels:
     *
     * - `y`: operation generally works; some instances of the operation may
     *   not apply
     *
     * - `*N*`: operation generally does not work; some instances of the
     *   operation may not apply
     *
     * - `n/a`: operation does not apply (i.e. bitwise operations to
     *   floating-point operands, `--` (and in the future possibly `++`) to
     *   boolean operands, assignment operators to scalar left hand sides)
     */

    /** @name Basic interface
     *
     * Templates and functions in this group are directly implemented by
     * templates and functions in namespace Overloads.
     *
     * @{
     */

    //! Element type of some SIMD type
    /**
     * \tparam V The SIMD (mask or vector) type.  `const`, `volatile` or
     *           reference qualifiers are automatically ignored.
     *
     * Not all operations that access the element of a vector return (a
     * reference to) the scalar type -- some may return proxy objects instead.
     * Use `autoCopy()` to make sure you are getting a prvalue of the scalar
     * type.
     *
     * Implemented by `Overloads::ScalarType`.
     */
    template<class V>
    using Scalar = typename Overloads::ScalarType<std::decay_t<V> >::type;

    //! Construct SIMD type with different scalar type
    /**
     * \tparam S The new scalar type
     * \tparam V The SIMD (mask or vector) type.
     *
     * The resulting type a SIMD vector of `S` with the same number of lanes
     * as `V`.  `const`, `volatile` or reference qualifiers in `S` and `V` are
     * automatically ignored, and the result will have no such qualifiers.
     *
     * Implementations shall rebind to `LoopSIMD<S, lanes<V>()>` if they can't
     * support a particular rebind natively.
     *
     * Implemented by `Overloads::RebindType`.
     */
    template<class S, class V>
    using Rebind =
      typename Overloads::RebindType<std::decay_t<S>, std::decay_t<V>>::type;

    //! @} group Basic interface

    /** @name Syntactic Sugar
     *
     * Templates and functions in this group provide syntactic sugar, they are
     * implemented using the functionality from @ref SimdInterfaceBase, and
     * are not customizable by implementations.
     *
     * @{
     */

    //! Mask type type of some SIMD type
    /**
     * \tparam V The SIMD (mask or vector) type.  `const`, `volatile` or
     *           reference qualifiers are automatically ignored.
     *
     * The mask type is kind of a SIMD vector of `bool` with the same number
     * of lanes as `V`.  It results from comparison operations between values
     * of type `V`.  It is only "kind of" a SIMD vector, because the
     * guaranteed supported operations are extremely limited.  At the moment
     * only the logical operators `&&`, `||` and `!` and the "bitwise"
     * operators `&`, `^` and `|` between masks are supported, and even with
     * those operators you cannot rely on automatic broadcasting of `bool`
     * values.
     *
     * \note In particular, masks do not support comparison.  As a workaround
     *       you can use `^` instead of `!=` and `!(m1 ^ m2)` instead of `m1
     *       == m2`.  (The reason why comparison is not supported is because
     *       in Vc `==` and `!=` between masks yield a single `bool` result
     *       and not a mask.)
     *
     * This is an alias for `Rebind<bool, V>`.
     */
    template<class V>
    using Mask = Rebind<bool, V>;

    //! @} group Syntactic Sugar

    /** @name Basic interface
     *  @{
     */

    //! Number of lanes in a SIMD type
    /**
     * \tparam V The SIMD (mask or vector) type.  `const`, `volatile`
     *           or reference qualifiers are automatically ignored.
     *
     * Implemented by `Overloads::LaneCount`.
     */
    template<class V>
    constexpr std::size_t lanes()
    {
      return Overloads::LaneCount<std::decay_t<V>>::value;
    }

    //! Extract an element of a SIMD type
    /**
     * \param l Number of lane to extract
     * \param v SIMD object to extract from
     *
     * \return If `v` is a non-`const` lvalue, a reference
     *         `Scalar<decay_t<V>>&`, or a proxy object through which the
     *         element of `v` may be modified.  Otherwise, `v` is a `const`
     *         lvalue or an rvalue, and the result is a prvalue (a temporary)
     *         of type `Scalar<decay_t<V>>`.
     *
     * Implemented by `Overloads::lane()`.
     */
    template<class V>
    decltype(auto) lane(std::size_t l, V &&v)
    {
      assert(l < lanes<V>());
      return lane(Overloads::ADLTag<7>{}, l, std::forward<V>(v));
    }

    //! Cast an expression from one implementation to another
    /**
     * Implemented by `Overloads::implCast()`
     *
     * Requires the scalar type and the number of lanes to match exactly.
     *
     * This is particularly useful for masks, which often know the type they
     * were derived from.  This can become a problem when doing a conditional
     * operation e.g. on some floating point vector type, but with a mask
     * derived from some index vector type.
     *
     * \note One of the few functions that explicitly take a template
     *       argument (`V` in this case).
     */
    template<class V, class U>
    constexpr V implCast(U &&u)
    {
      static_assert(std::is_same<Scalar<V>, Scalar<U> >::value,
                    "Scalar types must match exactly in implCast");
      static_assert(lanes<V>() == lanes<U>(),
                    "Number of lanes must match in implCast");
      return implCast(Overloads::ADLTag<7>{}, MetaType<std::decay_t<V> >{},
                      std::forward<U>(u));
    }

    //! Broadcast a scalar to a vector explicitly
    /**
     * Implemented by `Overloads::broadcast()`
     *
     * This is useful because the syntax for broadcasting can vary wildly
     * between implementations.
     *
     * \note One of the few functions that explicitly take a template
     *       argument (`V` in this case).
     */
    template<class V, class S>
    constexpr V broadcast(S s)
    {
      return broadcast(Overloads::ADLTag<7>{}, MetaType<std::decay_t<V> >{},
                       std::move(s));
    }

    //! Like the ?: operator
    /**
     * Equivalent to
     * \code
     *   V result;
     *   for(std::size_t l = 0; l < lanes(mask); ++l)
     *     lane(l, result) =
     *       ( lane(l, mask) ? lane(l, ifTrue) : lane(l ifFalse) );
     *   return result;
     * \endcode
     *
     * Implemented by `Overloads::cond()`.
     */
    template<class M, class V>
    V cond(M &&mask, const V &ifTrue, const V &ifFalse)
    {
      return cond(Overloads::ADLTag<7>{},
                  implCast<Mask<V> >(std::forward<M>(mask)), ifTrue, ifFalse);
    }

    //! Like the ?: operator
    /**
     * Overload for plain bool masks, accepting any simd type
     *
     * Implemented by `Overloads::cond()`.
     */
    template<class V>
    V cond(bool mask, const V &ifTrue, const V &ifFalse)
    {
      return mask ? ifTrue : ifFalse;
    }

    //! The binary maximum value over two simd objects
    /**
     * Implemented by `Overloads::max()`.
     */
    template<class V>
    auto max(const V &v1, const V &v2)
    {
      return max(Overloads::ADLTag<7>{}, v1, v2);
    }

    //! The binary minimum value over two simd objects
    /**
     * Implemented by `Overloads::min()`.
     */
    template<class V>
    auto min(const V &v1, const V &v2)
    {
      return min(Overloads::ADLTag<7>{}, v1, v2);
    }

    //! Whether any entry is `true`
    /**
     * Implemented by `Overloads::anyTrue()`.
     */
    template<class Mask>
    bool anyTrue(const Mask &mask)
    {
      return anyTrue(Overloads::ADLTag<7>{}, mask);
    }

    //! Whether all entries are `true`
    /**
     * Implemented by `Overloads::allTrue()`.
     */
    template<class Mask>
    bool allTrue(const Mask &mask)
    {
      return allTrue(Overloads::ADLTag<7>{}, mask);
    }

    //! Whether any entry is `false`
    /**
     * Implemented by `Overloads::anyFalse()`.
     */
    template<class Mask>
    bool anyFalse(const Mask &mask)
    {
      return anyFalse(Overloads::ADLTag<7>{}, mask);
    }

    //! Whether all entries are `false`
    /**
     * Implemented by `Overloads::allFalse()`.
     */
    template<class Mask>
    bool allFalse(const Mask &mask)
    {
      return allFalse(Overloads::ADLTag<7>{}, mask);
    }

    //! The horizontal maximum value over all lanes
    /**
     * Implemented by `Overloads::max()`.
     */
    template<class V>
    Scalar<V> max(const V &v)
    {
      return max(Overloads::ADLTag<7>{}, v);
    }

    //! The horizontal minimum value over all lanes
    /**
     * Implemented by `Overloads::min()`.
     */
    template<class V>
    Scalar<V> min(const V &v)
    {
      return min(Overloads::ADLTag<7>{}, v);
    }

    //! Convert to mask, analogue of bool(s) for scalars
    /**
     * Implemented by `Overloads::mask()`.
     */
    template<class V>
    auto mask(const V &v)
    {
      return mask(Overloads::ADLTag<7>{}, v);
    }

    //! Logic or of masks
    /**
     * Implemented by `Overloads::maskOr()`.
     */
    template<class V1, class V2>
    auto maskOr(const V1 &v1, const V2 &v2)
    {
      return maskOr(Overloads::ADLTag<7>{}, v1, v2);
    }

    //! Logic and of masks
    /**
     * Implemented by `Overloads::maskAnd()`.
     */
    template<class V1, class V2>
    auto maskAnd(const V1 &v1, const V2 &v2)
    {
      return maskAnd(Overloads::ADLTag<7>{}, v1, v2);
    }

    //! @} group Basic interface

    /** @name Syntactic Sugar
     *
     * Templates and functions in this group provide syntactic sugar, they are
     * implemented using the functionality from @ref SimdInterfaceBase, and
     * are not customizable by implementations.
     *
     * @{
     */

    //! Number of lanes in a SIMD type
    /**
     * \tparam V The SIMD (mask or vector) type.
     *
     * The value of the parameter is ignored; the call is simply forwarded to
     * `lanes<V>()`.
     */
    template<class V>
    std::size_t lanes(const V &)
    {
      return lanes<V>();
    }

    //! @} group Syntactic Sugar

    //! @} Group SIMDLib

  } // namespace Simd
} // namespace Dune

#endif // DUNE_COMMON_SIMD_INTERFACE_HH
