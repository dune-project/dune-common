// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>
#include <iostream>

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/communicationpattern.hh>
#include <dune/common/parallel/indexset.hh>
#include <dune/common/parallel/remoteindices.hh>
#include <dune/common/enumset.hh>

using namespace Dune;


int main(int argc, char** argv){
  auto& mpihelper = MPIHelper::instance(argc, argv);
  using A=CommunicationAttributes;
  CommunicationPattern<> pattern(0,
                                 {// send pattern:
                                  {3, {{1, A::owner, A::overlap},
                                       {4, A::owner, A::copy},
                                       {7, A::overlap, A::owner}}},
                                  {1, {{4, A::overlap, A::owner},
                                       {3, A::owner, A::overlap},
                                       {1, A::owner, A::copy}}}
                                 });

  // add manually
  pattern[3].push_back({6, A::owner, A::overlap});

  pattern.strip();
  std::cout << pattern << std::endl;

  // test to convert from RemoteIndices
#if HAVE_MPI
  mpihelper.getCommunication().barrier();
  typedef ParallelIndexSet<int, ParallelLocalIndex<A>> PIS;
  PIS pis;
  int rank = mpihelper.rank();
  if(rank == 0)
    std::cout << "RemoteIndices to CommunicationPattern test" << std::endl;
  int size = mpihelper.size();
  int N = 10;
  int overlap = 3;
  pis.beginResize();
  for(int li = 0; li < N+2*overlap; ++li){
    int gi = (li+rank*N-overlap+size*N)%(size*N);
    A flag;
    if(li<overlap || li >= N+overlap)
      flag = A::overlap;
    else
      flag = A::owner;
    bool isPublic = li < 2*overlap || li > N-overlap;
    pis.add(gi,
            ParallelLocalIndex<A>(li, flag, isPublic));
  }
  pis.endResize();
  RemoteIndices<PIS> ri(pis, pis, mpihelper.getCommunication(), {}, true);
  ri.rebuild<true>();
  auto remoteIndicesPattern = convertRemoteIndicesToCommunicationPattern(ri);

  std::cout << remoteIndicesPattern << std::endl;
#endif
  return 0;
}
