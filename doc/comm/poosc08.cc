// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <iostream>
#include <dune/common/exceptions.hh> // We use exceptions
#include <dune/common/parallel/mpihelper.hh> // An initializer of MPI
#include <dune/common/parallel/indexset.hh>
#include <dune/common/parallel/plocalindex.hh>

enum Flags { owner, ghost };

struct Bla
{

  /** @brief The local index. */
  size_t localIndex_;

  /** @brief An attribute for the index. */
  char attribute_;

  /** @brief True if the index is also known to other processors. */
  bool public_;

  /**
   * @brief The state of the index.
   *
   * Has to be one of LocalIndexState!
   * @see LocalIndexState.
   */
  char state_;
};


template<typename T1, typename T2>
void buildBlockedIndexSet(T1& indexset, int N, const T2& comm)
{
  int rank=comm.rank();
  int size=comm.size();
  int localsize=N/size;
  int bigger=N%size;
  int start, end;
  if(rank<bigger) {
    start=rank*(localsize+1);
    end=start+(localsize+1);
  }else{
    start=bigger*(localsize+1)+(rank-bigger)*localsize;
    end=start+localsize;
  }

  indexset.beginResize();
  int index=0;
  int gindex=start;
  typedef typename T1::LocalIndex LocalIndex;

  std::cout<<sizeof(LocalIndex)<<" "<<sizeof(Bla)<<std::endl;

  if(start>0)
    indexset.add(gindex-1,LocalIndex(index++,ghost));

  for(int i=start; i<end; i++,index++,gindex++)
    indexset.add(gindex,LocalIndex(index,owner,true));

  if(end<N)
    indexset.add(gindex,LocalIndex(index,ghost,true));
}
int main(int argc, char** argv)
{
  int n=100;
  try{
    using namespace Dune;

    //Maybe initialize Mpi
    MPIHelper& helper = MPIHelper::instance(argc, argv);
    std::cout << "Hello World! This is poosc08." << std::endl;
    if(Dune::MPIHelper::isFake)
      std::cout<< "This is a sequential program." << std::endl;
    else{
      typedef ParallelIndexSet<int,ParallelLocalIndex<Flags> > IndexSet;
      IndexSet blockedSet;
      buildBlockedIndexSet(blockedSet, n, helper.getCommunication());
    }
    return 0;
  }
  catch (Dune::Exception &e) {
    std::cerr << "Dune reported error: " << e << std::endl;
  }
  catch (...) {
    std::cerr << "Unknown exception thrown!" << std::endl;
  }
}
