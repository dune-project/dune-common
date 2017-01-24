#ifndef DUNE_COMMON_SIMD_INTERFACE_HH
#define DUNE_COMMON_SIMD_INTERFACE_HH

/** @file
 *  @brief User interface of the SIMD abstraction
 *  @ingroup SIMDLib
 *
 * This file provides the unser interface functions of the SIMD abstraction
 * layer.
 *
 * This file should never be included by users of the SIMD
 * abstraction. Include <dune/common/simd/simd.hh> instead.
 */

#include <cassert>
#include <cstddef>
#include <utility>

#include <dune/common/simd/base.hh>

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
     * This is very similiar to `std::valarray`, with the main difference
     * being that `std::valarray` is dynamic in size, while for this
     * abstraction the size is static.
     *
     * @section simd_abstraction_limit Limitations of the Abstraction Layer
     *
     * Since the abstraction layer cannot overload operators of SIMD types
     * (that would be meddling with the domain of the library that provides
     * the SIMD types), nor provide it's own constructors, there are severe
     * limitations in what the abstraction layer guarantees.  Besides the
     * standard types, the first SIMD library supported is Vc, so that is
     * where most of the limitations stem from.
     *
     * The biggest limitations are with masks.  In Vc masks support a very
     * restricted set of operations compared to other SIMD types, so in what
     * follows we will distinguish between masks with a very small set of
     * operations and between vectors with a larger set of operations.
     *
     * The limitations for vectors are (`V` is a vector type, `v` is an object
     * of type `V`, and s is an expression of type `Scalar<V>`):
     *
     * - Use `V v(s)` for broadcast construction
     *
     * - Don't use `++` or `--` (either kind), use `+= Scalar<V>(1)` / `-=
     *   Scalar<V>(1)` instead
     *
     * - If you use `<<` or `>>` with scalar arguments, broadcast them
     *   explicitly.
     *
     * - Don't rely on conversion to mask in `&&` and `||`, convert explicitly
     *   instead.  `!!v` should work but may be expensive.  `(v !=
     *   Scalar<V>(0))` should work too, but may be too ugly.  `Mask<V>(v)`
     *   may work, but is untested.
     *
     *   \todo I should probably introduce an interface function mask() for
     *         this job.
     *
     * - If you must use `operator,`, explicitly convert the left argument to
     *   `void`.  That may hide compiler warnings, but, well, you have already
     *   decided that you must use `operator,` despite those warnings.
     *
     * The limitations for masks are (`M` is a mask type, `m` is an object of
     * type `M`, and `b` is an expression of type `bool`):
     *
     * - Use `M m(b)` for broadcast construction
     *
     * - Explicitly broadcast all arguments that may be scalar, e.g. `m @
     *   M(b)`.
     *
     * - This applies for broadcast assignement too!  Use `m = M(b)`
     *
     * - The only operators that you can use are the binary operators `&`,
     *   `^`, `|` and their assignment versions `&=`, `^=`, `|=`, the logical
     *   operators `&&`, `||`, and `!`, and the assignment `=`.  If you must,
     *   you can use the comma operator `,`.
     *
     * - In particular you cannot use comparison operators `==` and `!=`.  If
     *   you can't live without them, instead of `!=` use `^`, and instead of
     *   `==` use `!(m1 ^ m2)`.
     *
     *   \todo I should probably introduce interface functions `eq()` and
     *         `ne()` to make this less arcane.
     *
     * The documentation for the Vc abstraction layer has more details on what
     * is supported there, but we only try to make sure the the above works.
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
     * \tparam V The SIMD (mask or vector) type.  May not include `const`,
     *           `volatile` or reference qualifiers.
     *
     * Not all operations that access the element of a vector return (a
     * reference to) the scalar type -- some may return proxy objects instead.
     * Use `valueCast()` to make sure you are getting a prvalue of the scalar
     * type.
     *
     * Implemented by `Overloads::ScalarType`.
     */
    template<class V>
    using Scalar = typename Overloads::ScalarType<V>::type;

    //! Index vector type of some SIMD type
    /**
     * \tparam V The SIMD (mask or vector) type.  May not include `const`,
     *           `volatile` or reference qualifiers.
     *
     * The index type is a SIMD vector of integers with the same number of
     * lanes as `V`.  The signedness and size of the integers is
     * implementation defined, in particular, it may be as small as `char` --
     * this can make sense if `V` is itself a SIMD vector of `char`.
     *
     * Implemented by `Overloads::IndexType`.
     */
    template<class V>
    using Index = typename Overloads::IndexType<V>::type;

    //! Mask type type of some SIMD type
    /**
     * \tparam V The SIMD (mask or vector) type.  May not include `const`,
     *           `volatile` or reference qualifiers.
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
     * Implemented by `Overloads::MaskType`.
     */
    template<class V>
    using Mask = typename Overloads::MaskType<V>::type;

    //! Number of lanes in a SIMD type
    /**
     * \tparam V The SIMD (mask or vector) type.  May not include `const`,
     *           `volatile` or reference qualifiers.
     *
     * Implemented by `Overloads::LaneCount`.
     */
    template<class V>
    constexpr std::size_t lanes()
    {
      return Overloads::LaneCount<V>::value;
    }

    //! Extract an element of a SIMD type
    /**
     * \param l Number of lane to extract
     * \param v SIMD object to extract from
     *
     * \return If `v` is a non-`const` lvalue, a reference
     *         `Scalar<decay_t<V>>&`, or a proxy object through which the
     *         element of `v` may be modified.  Overwise, `v` is a `const`
     *         lvalue or an rvalue, and the result is a prvalue (a temporary)
     *         of type `Scalar<decay_t<V>>`.
     *
     * Implemented by `Overloads::lane()`.
     */
    template<class V>
    decltype(auto) lane(std::size_t l, V &&v)
    {
      assert(l < lanes<V>());
      return lane(Overloads::ADLTag<6>{}, l, std::forward<V>(v));
    }

    //! Unproxies and returns a temporary
    /**
     * \param t A value of any value category of some scalar obtained from a
     *          SIMD type, or a proxy object denoting an element of a SIMD
     *          type.
     *
     * Many of the interface functions are allowed to return proxy objects as
     * well as references when they are passed lvalues as arguments.  This
     * makes it possible to deduce types of local variables when you want the
     * actual type, not a proxy:
     * \code
     *   auto first_value = valueCast(lane(0, v));
     * \endcode
     *
     * Implemented by `Overloads::valueCast()`.
     */
    template<class T>
    auto valueCast(T &&t)
    {
      return valueCast(Overloads::ADLTag<6>{}, std::forward<T>(t));
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
    template<class V>
    V cond(Mask<V> mask, V ifTrue, V ifFalse)
    {
      return cond(Overloads::ADLTag<6>{}, mask, ifTrue, ifFalse);
    }

    //! Whether any entry is `true`
    /**
     * Implemented by `Overloads::anyTrue()`.
     */
    template<class Mask>
    bool anyTrue(Mask mask)
    {
      return anyTrue(Overloads::ADLTag<6>{}, mask);
    }

    //! Whether all entries are `true`
    /**
     * Implemented by `Overloads::allTrue()`.
     */
    template<class Mask>
    bool allTrue(Mask mask)
    {
      return allTrue(Overloads::ADLTag<6>{}, mask);
    }

    //! Whether any entry is `false`
    /**
     * Implemented by `Overloads::anyFalse()`.
     */
    template<class Mask>
    bool anyFalse(Mask mask)
    {
      return anyFalse(Overloads::ADLTag<6>{}, mask);
    }

    //! Whether all entries are `false`
    /**
     * Implemented by `Overloads::allFalse()`.
     */
    template<class Mask>
    bool allFalse(Mask mask)
    {
      return allFalse(Overloads::ADLTag<6>{}, mask);
    }

    //! @}

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
    std::size_t lanes(V)
    {
      return lanes<V>();
    }

    //! @} group Syntactic Sugar

    //! @} Group SIMDLib

  } // namespace Simd
} // namespace Dune

#endif // DUNE_COMMON_SIMD_INTERFACE_HH
