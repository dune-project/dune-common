// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_STD_APPLY_HH
#define DUNE_COMMON_STD_APPLY_HH

#include <tuple>

namespace Dune
{
  namespace Std
  {

    /// Invoke the Callable object f with a tuple of arguments.
    /// \deprecated Use `std::apply` directly.
    using std::apply;

  } // namespace Std
} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_APPLY_HH
