#ifndef DUNE_COMMON_PARALLEL_SERIAL_HH
#define DUNE_COMMON_PARALLEL_SERIAL_HH

#include <dune/common/parallel/collectivecommunication.hh>
#include <dune/common/parallel/mpihelper.hh>

namespace Dune {

template<typename F, typename Communicator = MPIHelper::MPICommunicator>
void serial(
  F&& f,
  const CollectiveCommunication<Communicator>& comm = MPIHelper::getCollectiveCommunication())
{
  const int rank = comm.rank();
  const int size = comm.size();

  for (int i = 0; i < size; ++i) {
    if (i == rank)
      f(rank, size);
    comm.barrier();
  }
}

} /* namespace Dune */

#endif
