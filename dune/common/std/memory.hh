// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set ts=8 sw=2 et sts=2:
#ifndef DUNE_COMMON_STD_MEMORY_HH
#define DUNE_COMMON_STD_MEMORY_HH

#warning dune/common/std/memory.hh is deprecated; use std::make_unique from <memory> instead

#include <memory>

namespace Dune
{

  namespace Std
  {

    using std::make_unique;

  } // namespace Std

} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_MEMORY_HH
