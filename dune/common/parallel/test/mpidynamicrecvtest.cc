// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpiguard.hh>


int main(int argc, char** argv){
  Dune::MPIHelper& helper = Dune::MPIHelper::instance(argc, argv);

  auto comm = helper.getCommunicator();
  int rank = comm.rank();
  std::cout << "Rank: " << rank << std::endl;
  int size = comm.size();

  if(size > 1){
    auto ptpc = helper.getPointToPointCommunication();
    if(rank == 0){
      std::vector<double> buf(2);
      ptpc.send(buf, 1, 0);
    }
    if(rank == 1){
      auto f = ptpc.irecv(std::vector<double>{}, 0, 0, true);
      f.wait();
      auto vec = f.get();
      auto f2 = ptpc.irecv(std::vector<double>{}, 0, 0, true); // this gets cancelled
    }
  }

  return 0;
}
