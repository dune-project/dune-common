// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PROMOTIONTRAITS_HH
#define DUNE_PROMOTIONTRAITS_HH

#include <utility>

namespace Dune {
  /**
   * @file
   * @brief  Compute type of the result of an arithmetic operation involving two different number types.
   *
   * @author Matthias Wohlmuth
   */

  /** @addtogroup Common
   *
   * @{
   */

  /** \brief Compute type of the result of an arithmetic operation involving two different number types.
  */
  template <typename T1, typename T2>
  struct PromotionTraits
  {
    typedef decltype(std::declval<T1>()+std::declval<T2>()) PromotedType;
  };

  // Specialization for the case of two equal types
  // One should think that the generic template should handle this case as well.
  // However, the fvectortest.cc unit test fails without it if ENABLE_GMP is set.
  template <typename T1>
  struct PromotionTraits<T1,T1> { typedef T1 PromotedType; };

  /** @} */
} // end namespace


#endif // DUNE_PROMOTIONTRAITS_HH
