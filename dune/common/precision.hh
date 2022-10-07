// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_PRECISION_HH
#define DUNE_PRECISION_HH

/** \file
 * \brief Various precision settings for calculations with FieldMatrix and FieldVector
 */

#include <stdlib.h>

namespace Dune {

  /**
      @addtogroup DenseMatVec
      @{
   */

  /**
   * @brief Precisions for calculations with FieldMatrix and FieldVector.
   */
  template <class ctype = double>
  class FMatrixPrecision {
  public:
    //! return threshold to declare matrix singular
    static ctype absolute_limit ()
    {
      return _absolute;
    }

    //! set singular threshold
    static void set_absolute_limit (ctype absthres)
    {
      _absolute = absthres;
    }

  private:
    // just to demonstrate some state information
    static ctype _absolute;
  };

  template <class ctype>
  ctype FMatrixPrecision<ctype>::_absolute = 1E-80;

  /** @} end documentation */

} // end namespace

#endif
