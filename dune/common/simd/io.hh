// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_SIMD_IO_HH
#define DUNE_COMMON_SIMD_IO_HH

/** @file
 *  @brief IO interface of the SIMD abstraction
 *  @ingroup SIMDLib
 *
 * This file provides IO interface functions of the SIMD abstraction layer.
 *
 * This file is intended for direct inclusion by header making use of the IO
 * interface.
 */

#include <ios>
#include <type_traits>

#include <dune/common/rangeutilities.hh>
#include <dune/common/simd/simd.hh>
#include <dune/common/typetraits.hh>

namespace Dune {

  namespace SimdImpl {

    template<class T>
    class Inserter {
      T value_;

    public:
      Inserter(const T &value) : value_(value) {}

      template<class Stream,
               class = std::enable_if_t<std::is_base_of<std::ios_base,
                                                        Stream>::value> >
      friend Stream& operator<<(Stream &out, const Inserter &ins)
      {
        const char *sep = "<";
        for(auto l : range(Simd::lanes(ins.value_)))
        {
          out << sep << autoCopy(Simd::lane(l, ins.value_));
          sep = ", ";
        }
        out << '>';
        return out;
      }
    };

    template<class V, class = std::enable_if_t<Simd::lanes<V>() != 1> >
    Inserter<V> io(const V &v)
    {
      return { v };
    }

    template<class V, class = std::enable_if_t<Simd::lanes<V>() == 1> >
    Simd::Scalar<V> io(const V &v)
    {
      return Simd::lane(0, v);
    }

  }

  namespace Simd {

    /** @addtogroup SIMDLib
     *
     * @{
     *
     */

    /** @name IO interface
     *
     * Templates and functions in this group provide syntactic sugar for IO.
     * They are implemented using the functionality from @ref
     * SimdInterfaceBase, and are not customizable by implementations.
     *
     * @{
     */

    //! construct a stream inserter
    /**
     * \tparam V The SIMD (mask or vector) type.
     *
     * Construct an object that can be inserted into an output stream.
     * Insertion prints the vector values separated by a comma and a space,
     * and surrounded by angular brackets.
     */
    template<class V>
    auto vio(const V &v)
    {
      return SimdImpl::Inserter<V>{ v };
    }

    //! construct a stream inserter
    /**
     * \tparam V The SIMD (mask or vector) type.
     *
     * Construct an object that can be inserted into an output stream.  For
     * one-lane vectors, behaves the same as scalar insertion.  For multi-lane
     * vectors, behaves as the inserter returned by `vio()`: insertion prints
     * the vector values separated by a comma and a space, and surrounded by
     * angular brackets.
     */
    template<class V>
    auto io(const V &v)
    {
      return SimdImpl::io(v);
    }

    //! @} group IO interface

    //! @} Group SIMDLib

  } // namespace Simd
} // namespace Dune

#endif // DUNE_COMMON_SIMD_IO_HH
