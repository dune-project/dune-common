// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

/**
 * @file @brief Executes MPI code. Some places in the code are marked
 for possible throwing exceptions. All combinations of throwing
 exceptions are iterated.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

// Setting debug output level
// #define DUNE_MINIMAL_DEBUG_LEVEL 1
// name         level         usage
// Dune::dwarn  1 2 3 4
// Dune::dinfo  1 2 3         implementation details
// Dune::dverb  1 2           destructors and contructors
// Dune::dvverb 1

#include <cmath>

#include <unistd.h>
#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpiguard.hh>
#include <dune/common/parallel/pointtopointcommunication.hh>
#include <dune/common/stdstreams.hh>

#define THROW_PLACE                                                     \
  do{                                                                   \
    int count = __COUNTER__;                                            \
    if((throw_case & 1 << count) &&                                     \
       (throw_rank & 1 << c.rank())){                                   \
      std::cout << c.rank() << ":\tthrowing at place " << count << std::endl; \
      DUNE_THROW(Dune::Exception, "Exception from place " << count);    \
    }                                                                   \
  }while(false)

template<class C>
int checkmpiguard(C& c, const int& throw_case, int throw_rank){
  std::cout << "============== Check MPIGuard ==============" << std::endl;
  Dune::MPIGuard guard(c);
  THROW_PLACE;  // 0
  // P2P
  std::cout << "Check P2P --------------------" << std::endl;
  if (c.size() > 1){
    Dune::PointToPointCommunication<C> ptpc(c);
    if (c.rank() == 0){
      THROW_PLACE;  // 1
      auto f = ptpc.isend(42, 1, 4711);
      f.get();
    }
    else if(c.rank() == 1)
    {
      THROW_PLACE;  // 2
      auto v = std::make_unique<int>(3);
      auto f = ptpc.template irecv<std::unique_ptr<int>>(std::move(v), 0, 4711);
      f.get();
    }
    THROW_PLACE;  // 3
  }
  std::cout << "Check Collectives --------------------" << std::endl;
#if MPI_VERSION >= 3
  Dune::CollectiveCommunication<C> cc(c);
  cc.ibarrier().wait();
  THROW_PLACE;  // 4
  int j = c.rank();
  auto f2 = cc.template iallreduce<Dune::Max<int>>(j);
  THROW_PLACE;  // 5
  f2.wait();
  THROW_PLACE;  // 6
#endif
  return 1;
}

std::string DecimalToBinaryString(int a)
{
    uint b = (uint)a;
    std::string binary = "";
    uint mask = 0x80000000u;
    while (mask > 0)
    {
        binary += ((b & mask) == 0) ? '0' : '1';
        mask >>= 1;
    }
    return binary;
}

int main(int argc, char** argv){
  Dune::MPIHelper& helper = Dune::MPIHelper::instance(argc, argv, MPI_THREAD_SINGLE);
  if (helper.isFake)
    std::cout << "--- MPI not found ---" << std::endl;
  else
    std::cout << "--- MPI found! ---" << std::endl;

  auto comm = Dune::MPIHelper::getCommunicator();

  int rank = comm.rank();

  for (int c = 16; c < std::pow(2, __COUNTER__); c++){
    for (int r = 1; r < std::pow(2, comm.size()); ++r){
      std::cout << "case " << c <<": " << DecimalToBinaryString(c) << "\t throwing rank: " << DecimalToBinaryString(r) << std::endl;
      try{
        checkmpiguard(comm, c, r);
      }catch(Dune::Exception& e){
        std::cout << rank << ":\tcaught an exception!" << std::endl;
        std::cout << e.what() << std::endl;
        comm.shrink();
      }
    }
  }

  return 0;
}
