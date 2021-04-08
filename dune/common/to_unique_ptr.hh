// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

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
