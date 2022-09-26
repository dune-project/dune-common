// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef REVERSE_HH
#define REVERSE_HH

#include "buildindexset.hh"

/**
 * @brief Reverse the local indices of an index set.
 *
 * Let the index set have N entries than the index 0 will become N-1,
 * 1 become N-2, ..., and N-1 will become 0.
 * @param indexSet The index set to reverse.
 */
template<typename TG, typename TL, int N>
void reverseLocalIndex(Dune::ParallelIndexSet<TG,TL,N>& indexSet)
{
  // reverse the local indices
  typedef typename Dune::ParallelIndexSet<TG,TL,N>::iterator iterator;

  iterator end = indexSet.end();
  size_t maxLocal = 0;

  // find the maximal local index
  for(iterator index = indexSet.begin(); index != end; ++index) {
    // Get the local index
    LocalIndex& local = index->local();
    maxLocal = std::max(maxLocal, local.local());
  }

  for(iterator index = indexSet.begin(); index != end; ++index) {
    // Get the local index
    LocalIndex& local = index->local();
    local = maxLocal--;
  }

}
#endif
