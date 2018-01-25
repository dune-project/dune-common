// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <numeric>

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpiguard.hh>
#include <dune/common/parallel/neighborindexmappingbuilder.hh>

int main(int argc, char** argv)
{
  Dune::MPIHelper& helper = Dune::MPIHelper::instance(argc, argv);

  if (helper.isFake)
    std::cout << "--- MPI not found ---" << std::endl;
  else
    std::cout << "--- MPI found! ---" << std::endl;

  auto comm = helper.getCommunicator();
  int rank = comm.rank();
  int size = comm.rank();
  int N = 10;
  int M = 3;
  std::vector<int> gi(N+2*M);
  std::iota(gi.begin(), gi.end(), rank*N-M);
  Dune::NeighborIndexMappingBuilder<decltype(comm)> nimb(comm);
  auto map1 = nimb.buildNeighborIndexMapping(gi);
  std::map<int, std::vector<int>> map2;
  if(size > 1){
    if(rank == 0){
      map2 = nimb.buildNeighborIndexMapping(gi, {rank+1});
      for(int i = 0; i < 2*M; i++)
        map2[rank+1][i] = N-M+i;
    }else if(rank == size-1){
      map2 = nimb.buildNeighborIndexMapping(gi, {rank-1});
      for(int i = 0; i < 2*M; i++)
        map2[rank-1][i] = (rank-1)*N-M+i;
    }else{
      map2 = nimb.buildNeighborIndexMapping(gi, {rank-1, rank+1});
      for(int i = 0; i < 2*M; i++)
        map2[rank-1][i] = rank*N-M+i;
      for(int i = 0; i < 2*M; i++)
        map2[rank+1][i] = (rank+1)*N-M+i;
    }
  }
  if(map1 != map2)
    DUNE_THROW( Dune::Exception, "Different result for neighbor and non-neighbor method");
  return 0;
}
