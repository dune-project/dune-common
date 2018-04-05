// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/collectivecommunication.hh>
#include <dune/common/parallel/pointtopointcommunication.hh>
#include <dune/common/parallel/mpipack.hh>

int main(int argc, char** argv)
{
  auto& helper = Dune::MPIHelper::instance(argc, argv);
  auto world = helper.getCommunicator();
  if(world.size() < 2)
    return 0;
#if HAVE_MPI
  Dune::PointToPointCommunication<decltype(world)> p2p(world);
  int rank = world.rank();
  Dune::MPIPack<decltype(world)> p(world, 8);
  if(rank == 0){
    p << (int)1;
    p << rank;
    p.pack(std::array<int, 2>{{3, 12}});
    p2p.send(p, 1, 4711);
  }
  if(rank == 1){
    auto status = p2p.mprobe(0, 4711);
    status.recv(p);
    int i,k;
    std::array<int, 2> arr;
    p >> i >> k >> arr;
    std::cout << "Rank 1 received:" << i << ", " << k <<  ", (" << arr[0] << "," << arr[1] << ")" << std::endl;
  }
#endif
  return 0;
}
