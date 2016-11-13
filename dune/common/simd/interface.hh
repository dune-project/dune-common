#ifndef DUNE_COMMON_SIMD_INTERFACE_HH
#define DUNE_COMMON_SIMD_INTERFACE_HH

/** @file
 *  @brief User interface of the SIMD abstraction
 *
 * This file provides the unser interface functions of the SIMD abstraction
 * layer.
 *
 * This file should never be included by users of the SIMD
 * abstraction. Include <dune/common/simd/simd.hh> instead.
 */

#include <cstddef>
#include <utility>

#include <dune/common/simd/base.hh>

namespace Dune {
  //! @addtogroup SIMD
  //! @{

  namespace Simd {

    /** @defgroup SimdInterfaceBase Basic Simd Interface
     *
     * Templates and functions in this group are directly implemented by
     * templates and functions in namespace Overloads.
     *
     * @{
     */

    //! member type of some vectorized type
    template<class V>
    using Scalar = typename Overloads::ScalarType<V>::type;

    //! index vector type of some vectorized type
    /**
     * \note Typically (but not necessarily), \c
     *       sizeof(Scalar<Index<T>>)==sizeof(Scalar<T>).  This implies that
     *       the range of possible indices may be _very_ limited, e.g. if \c
     *       Scalar<T> is \c char.
     */
    template<class V>
    using Index = typename Overloads::IndexType<V>::type;

    //! mask vector type of some vectorized type
    template<class V>
    using Mask = typename Overloads::MaskType<V>::type;

    //! number of lanes in type T
    template<class V>
    constexpr std::size_t lanes()
    {
      return Overloads::LaneCount<V>::value;
    }

    //! extract lane \c l from \c v
    /**
     * If \c v is an lvalue, the return value is either an lvalue too, or a
     * proxy.  In either case its there is no requirement for its type to be
     * \c Scalar<T>& (and it usually wont be with mask types).
     */
    template<class V>
    decltype(auto) lane(std::size_t l, V &&v)
    {
      return lane(Overloads::ADLTag<6>{}, l, std::forward<V>(v));
    }

    //! unproxies and returns a temporary
    /**
     * Many of the interface functions are allowed to return proxy objects or
     * references when they are passed lvalues as arguments.  This makes it
     * possible to deduce types of local variables when you want the actual
     * type, not a proxy:
     * \code
     *   auto first_value = valueCast(lane(0, vec));
     * \endcode
     */
    template<class T>
    auto valueCast(T &&t)
    {
      return valueCast(Overloads::ADLTag<6>{}, std::forward<T>(t));
    }

    //! like the ?: operator
    template<class V>
    V cond(Mask<V> mask, V ifTrue, V ifFalse)
    {
      return cond(Overloads::ADLTag<6>{}, mask, ifTrue, ifFalse);
    }

    //! whether any entry in \c mask is true
    template<class Mask>
    bool anyTrue(Mask mask)
    {
      return anyTrue(Overloads::ADLTag<6>{}, mask);
    }

    //! whether all entries in \c mask are true
    template<class Mask>
    bool allTrue(Mask mask)
    {
      return allTrue(Overloads::ADLTag<6>{}, mask);
    }

    //! whether any entry in \c mask is false
    template<class Mask>
    bool anyFalse(Mask mask)
    {
      return anyFalse(Overloads::ADLTag<6>{}, mask);
    }

    //! whether all entries in \c mask are false
    template<class Mask>
    bool allFalse(Mask mask)
    {
      return allFalse(Overloads::ADLTag<6>{}, mask);
    }

    //! @} group SimdInterfaceBase

    /** @defgroup SimdInterfaceSugar Extended Simd Interface
     *
     * Templates and functions in this group provide syntactic sugar, they are
     * implemented using the functionality from @ref SimdInterfaceBase, and
     * are not customizable by implementations.
     *
     * @{
     */

    //! number of lanes in v
    /**
     * This actually ignores the value of v and forwards to \c lanes<T>().  It
     * is often more convenient to use.
     */
    template<class T>
    std::size_t lanes(T v)
    {
      return lanes<T>();
    }

    //! @} group SimdInterfaceSugar

  } // namespace Simd

  //! @} Group SIMD
} // namespace Dune

#endif // DUNE_COMMON_SIMD_INTERFACE_HH
