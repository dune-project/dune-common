// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifndef DUNE_COMMON_DOCUMENTATION_HH
#define DUNE_COMMON_DOCUMENTATION_HH

/** \file
    \brief Documentation related stuff
 */

namespace Dune {

  /**
   * \brief Dummy struct used for documentation purposes
   *
   * This struct can be used for documenting interfaces.  One example would
   * be:
   * \code
   * // Traits class that determines some property for some other type T
   * template<class T>
   * class SomeTraits {
   *   static_assert(Dune::AlwaysFalse<T>::value,
   *                 "Sorry, SomeTraits must be specialized for all types");
   * public:
   *   // The type of some property of T
   *   typedef ImplementationDefined type;
   * };
   * #ifndef DOXYGEN
   * template<>
   * struct SomeTraits<int>
   *   typedef ... type;
   * };
   * // ...
   * #endif // DOXYGEN
   * \endcode
   *
   * \sa implementationDefined
   * \ingroup Common
   */
  struct ImplementationDefined {};

  /**
   * \brief Dummy integral value used for documentation purposes
   *
   * \var Dune::implementationDefined
   * \code
   * #include <dune/common/documentation.hh>
   * \endcode
   *
   * \sa ImplementationDefined
   * \ingroup Common
   */
  enum { implementationDefined };

}


#endif // DUNE_COMMON_DOCUMENTATION_HH
