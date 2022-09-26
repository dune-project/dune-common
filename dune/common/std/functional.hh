// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set ts=8 sw=2 et sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STD_FUNCTIONAL_HH
#define DUNE_COMMON_STD_FUNCTIONAL_HH

#include <functional>

namespace Dune
{

  namespace Std
  {

    /**
     * @brief   A function object type whose operator() returns its argument unchanged
     * @note    Equivalent to: `return std::forward(t);`
     * @warning When passing `r-values`, the result must be, at most, used for direct
     *          consumption in an outer function call
     */
#if DUNE_HAVE_CXX_STD_IDENTITY
    using std::identity;
#else //DUNE_HAVE_CXX_STD_IDENTITY
    struct identity {
      template<class T>
      constexpr T&& operator()(T&& t ) const noexcept {return std::forward<T>(t);}
    };
#endif
  } // namespace Std

} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_FUNCTIONAL_HH
