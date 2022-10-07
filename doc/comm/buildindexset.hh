// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef BUILDINDEXSET_HH
#define BUILDINDEXSET_HH


#include <dune/common/parallel/indexset.hh>
#include <dune/common/parallel/plocalindex.hh>

/**
 * @brief Flag for marking the indices.
 */
enum Flag {owner, overlap};

// The type of local index we use
typedef Dune::ParallelLocalIndex<Flag> LocalIndex;

/**
 * @brief Add indices to the example index set.
 * @param indexSet The index set to build.
 */
template<class C, class TG, int N>
void build(C& comm, Dune::ParallelIndexSet<TG,LocalIndex,N>& indexSet)
{




  // The rank of our process
  int rank=comm.rank();

  // Indicate that we add or remove indices.
  indexSet.beginResize();

  if(rank==0) {
    indexSet.add(0, LocalIndex(0,overlap,true));
    indexSet.add(2, LocalIndex(1,owner,true));
    indexSet.add(6, LocalIndex(2,owner,true));
    indexSet.add(3, LocalIndex(3,owner,true));
    indexSet.add(5, LocalIndex(4,owner,true));
  }

  if(rank==1) {
    indexSet.add(0, LocalIndex(0,owner,true));
    indexSet.add(1, LocalIndex(1,owner,true));
    indexSet.add(7, LocalIndex(2,owner,true));
    indexSet.add(5, LocalIndex(3,overlap,true));
    indexSet.add(4, LocalIndex(4,owner,true));
  }

  // Modification is over
  indexSet.endResize();
}
#endif
