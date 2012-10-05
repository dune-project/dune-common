// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug_allocator.hh"

#include <iostream>
#include <unistd.h>
#include <cstdlib>

const Dune::DebugMemory::AllocationManager::difference_type Dune::DebugMemory::AllocationManager::page_size = getpagesize();

void Dune::DebugMemory::AllocationManager::allocation_error(const char* msg)
{
  std::cerr << "Abort - Memory Corruption: " << msg << std::endl;
  std::abort();
}

Dune::DebugMemory::AllocationManager Dune::DebugMemory::alloc_man;
