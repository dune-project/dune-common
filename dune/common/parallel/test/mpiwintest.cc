// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#define DUNE_MINIMAL_DEBUG_LEVEL 1

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpiguard.hh>
#include <dune/common/parallel/mpiwin.hh>

#include <dune/common/stdstreams.hh>
#include <dune/common/fvector.hh>


int main(int argc, char** argv)
{
#if MPI_VERSION < 3
  return 1;
#else
  Dune::MPIHelper& helper = Dune::MPIHelper::instance(argc, argv);
  if (helper.isFake)
    std::cout << "--- MPI not found ---" << std::endl;
  else
    std::cout << "--- MPI found! ---" << std::endl;

  auto comm = helper.getCommunicator().dup();
  int rank = comm.rank();

  constexpr int vec_len = 2;
  typedef Dune::FieldVector<double, vec_len> VectorType;

  VectorType data(42.0 + rank);
  Dune::MPIWin<decltype(comm)> win(comm, data);
  // test assignment
  Dune::MPIWin<decltype(comm)> win2 = win;
  win2.lock(MPI_LOCK_SHARED, 0);
  auto f = win2.get(0, 0, VectorType{});
  f.wait();
  win2.unlock(0);
  auto data_0 = f.get();
  std::cout << rank << ":\tdata of rank 0: " << data_0 << std::endl;
  if(data_0[0] != 42)
      throw;
#endif
}
