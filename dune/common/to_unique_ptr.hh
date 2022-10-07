// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifndef DUNE_TO_UNIQUE_PTR_HH
#define DUNE_TO_UNIQUE_PTR_HH

#warning to_unique_ptr.hh and ToUniquePtr are deprecated. Use std::unique_ptr or std::shared_ptr instead.

#include <memory>

namespace Dune
{
  /// \brief Alias for `std::unique_ptr` introduced as transition wrapper.
  /// \deprecated
  template <class T>
  using ToUniquePtr [[deprecated]] = std::unique_ptr<T>;

  /// \brief Alias for `std::make_unique` introduced as transition wrapper.
  /// \deprecated
  template <class T, class... Args>
  [[deprecated]] std::unique_ptr<T> makeToUnique (Args&&... args)
  {
    return std::make_unique(std::forward<Args>(args)...);
  }

} // end namespace Dune

#endif // DUNE_TO_UNIQUE_PTR_HH
