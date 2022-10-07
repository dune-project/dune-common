// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_CONDITIONAL_HH
#define DUNE_COMMON_CONDITIONAL_HH

namespace Dune
{

    /** \brief conditional evaluate

        sometimes call immediate if, evaluates to

        \code
        if (b)
           return v1;
        else
           return v2;
        \endcode

        In contrast to if-then-else the cond function can also be
        evaluated for vector valued SIMD data types, see simd.hh.

        \param b boolean value
        \param v1 value of b==true
        \param v2 value of b==false
    */
    template<typename T1, typename T2>
    const T1 cond(bool b, const T1 & v1, const T2 & v2)
    {
        return (b ? v1 : v2);
    }

} // end namespace Dune

#endif // DUNE_COMMON_CONDITIONAL_HH
