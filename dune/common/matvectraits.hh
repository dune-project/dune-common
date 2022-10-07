// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_MATVECTRAITS_HH
#define DUNE_MATVECTRAITS_HH

/** \file
 * \brief Documentation of the traits classes you need to write for each implementation of DenseVector or DenseMatrix
 */

namespace Dune {

  /**
     @addtogroup DenseMatVec
     \brief Type Traits to retrieve types associated with an implementation of Dune::DenseVector or Dune::DenseMatrix

     you have to specialize this class for every implementation of DenseVector or DenseMatrix.

     \code
     //! export the type of the derived class (e.g. FieldVector<K,SIZE>)
     typedef ... derived_type;
     //! export the type of the stored values
     typedef ... value_type;
     //! export the type representing the size information
     typedef ... size_type;
     \endcode

   */
  template<class T>
  struct DenseMatVecTraits {};

} // end namespace Dune

#endif // DUNE_FTRAITS_HH
