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

enum Flags {
  owner, overlap
};


int main(int argc, char** argv){
  auto& mpihelper = MPIHelper::instance(argc, argv);

  CommunicationPattern<> pattern(0,
                                       {// send pattern:
                                        {3, {1,4,7}},
                                        {1, {4,3,1}},
                                        {42, {}}
    },
                                        {// receive pattern:
                                         {6, {1,2,3}},
                                         {4711, {4,7,1}},
                                         {3, {}}
                                        }
    );

  // add manually
  pattern.sendPattern()[3].push_back(6);
  pattern.recvPattern()[666].push_back(0);

  pattern.strip();
  std::cout << pattern << std::endl;

  // test to convert from RemoteIndices
  #if HAVE_MPI
  mpihelper.getCommunication().barrier();
  typedef ParallelIndexSet<int, ParallelLocalIndex<Flags>> PIS;
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
    Flags flag;
    if(li<overlap || li >= N+overlap)
      flag = Flags::overlap;
    else
      flag = Flags::owner;
    bool isPublic = li < 2*overlap || li > N-overlap;
    pis.add(gi,
            ParallelLocalIndex<Flags>(li, flag, isPublic));
  }
  pis.endResize();
  RemoteIndices<PIS> ri(pis, pis, mpihelper.getCommunication());
  ri.rebuild<true>();
  CommunicationPattern<> remoteIndicesPattern = convertRemoteIndicesToCommunicationPattern(ri,
                                              EnumItem<Flags, Flags::owner>{},
                                              AllSet<Flags>{});

  std::cout << remoteIndicesPattern << std::endl;
  #endif
  return 0;
}
