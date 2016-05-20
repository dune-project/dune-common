// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "debugallocator.hh"

#include <iostream>
#include <unistd.h>
#include <cstdlib>

namespace Dune
{
  namespace DebugMemory
  {
    // system constant for page size
    const std::ptrdiff_t page_size = sysconf(_SC_PAGESIZE);

    // implement member functions
    void AllocationManager::allocation_error(const char* msg)
    {
      std::cerr << "Abort - Memory Corruption: " << msg << std::endl;
      std::abort();
    }

    // global instance of AllocationManager
    AllocationManager alloc_man;

  }   // end namespace DebugMemory
} // end namespace Dune
