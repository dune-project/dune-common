// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/futurebatch.hh>

int main(int argc, char** argv){
  Dune::MPIHelper& helper = Dune::MPIHelper::instance(argc, argv);
  auto comm = helper.getCollectiveCommunication();

  Dune::FutureBatch<std::vector<int>> batch; // accumulate 3 Futures
  auto f1 = batch.batch([](int t){return t+1;});
  auto f2 = batch.batch();
  auto f3 = batch.batch();

  std::vector<int> vec = {comm.rank(), comm.size(), 42};
  batch.start(comm.template iallreduce<std::plus<int>>(std::move(vec)));

  int v1 = f1.get();
  int v2 = f2.get();
  int v3 = f3.get();
  if(v1-1 != comm.size()*(comm.size()-1)/2)
    DUNE_THROW(Dune::Exception, "Result 1 mismatch");
  if(v2 != comm.size()*comm.size())
    DUNE_THROW(Dune::Exception, "Result 2 mismatch");
  if(v3 != 42*comm.size())
    DUNE_THROW(Dune::Exception, "Result 3 mismatch");
  if(comm.rank() == 0){
  std::cout << "1: " << v1 << std::endl
            << "2: " << v2 << std::endl
            << "3: " << v3 << std::endl;
  }
}
