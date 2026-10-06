// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_ASNUMBER_HH
#define DUNE_COMMON_ASNUMBER_HH

#include <dune/common/typetraits.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/fvector.hh>


namespace Dune {

namespace Impl {

  /**
      @addtogroup DenseMatVec
      @{
   */

  /**
   * \brief Forward given value as number
   *
   * This overload is selected if the argument
   * is a number type and forwards the argument.
   */
  template<class T>
  requires (Dune::IsNumber<T>::value)
  const T& asNumber(const T& t)
  {
    return t;
  }

  /**
   * \brief Forward given value as number
   *
   * This overload returns the single entry
   * of a `FieldVector<T,1>`.
   */
  template<class T>
  const T& asNumber(const Dune::FieldVector<T, 1>& t)
  {
    return t[0];
  }

  /**
   * \brief Forward given value as number
   *
   * This overload returns the single entry
   * of a `FieldMatrix<T,1,1>`.
   */
  template<class T>
  const T& asNumber(const Dune::FieldMatrix<T, 1, 1>& t)
  {
    return t[0][0];
  }

  /** @} end documentation */

} // end namespace Impl

} // end namespace Dune

#endif // DUNE_COMMON_ASNUMBER_HH
