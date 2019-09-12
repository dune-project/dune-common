// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include <iostream>
#include <vector>
#include <numeric>

#include <config.h>

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/communicationpattern.hh>
#include <dune/common/parallel/patterncommunicator.hh>
#include <dune/common/exceptions.hh>

using namespace Dune;

int main(int argc, char** argv){
  auto& helper = MPIHelper::instance(argc, argv);
  int rank = helper.rank();
  int size = helper.size();

  using A=CommunicationAttributes;
  // setup pattern
  CommunicationPattern<> ringPattern(rank);
  ringPattern[(rank+1)%size] = // right neighbor
    {{2, A::overlap, A::owner},
     {3, A::copy, A::owner}};
  // use insert to cover the case where size==1 or size==2
  // (then both neighbors are the same)
  auto insert_it = ringPattern[(rank-1+size)%size].begin();
  if(size==2 && rank==1)
    insert_it = ringPattern[(rank-1+size)%size].end();
  ringPattern[(rank-1+size)%size].insert( // left neighbor
    insert_it,
    {{0, A::owner, A::overlap},
     {1, A::owner, A::copy}
    });

  std::cout << ringPattern << std::endl;

  typedef Dune::MPIPatternCommunicator<Dune::CommunicationPattern<>> Comm;

  {
    Comm communicator(ringPattern, helper.getCommunicator());
    std::vector<double> data(4);
    std::iota(data.begin(), data.end(), rank);

    // communicate one double
    communicator.exchange([&](auto& buf, auto& idx){
                            buf.write(data[idx]);
                          },
                          [&](auto& buf, auto& idx){
                            buf.read(data[idx]);
                          });

    // communicate two double
    communicator.exchange([&](auto& buf, auto& idx){
                            buf.write(data[idx]);
                            buf.write(data[idx]+1);},
                          [&](auto& buf, auto& idx){
                            double x, y;
                            buf.read(x);
                            buf.read(y);
                            data[idx] = x+y;
                          });

    // make consistent:
    communicator.exchange([&](auto& buf, auto& idx){
                           if(idx.localAttribute()==A::owner)
                             buf.write(data[idx]);
                         },
                         [&](auto& buf, auto& idx){
                           if(idx.remoteAttribute()==A::owner)
                             buf.read(data[idx]);
                         });

    // check consistency:
    communicator.exchange([&](auto& buf, auto& idx){
                            buf.write(data[idx]);
                          },
                          [&](auto& buf, auto& idx){
                            double temp;
                            buf.read(temp);
                            if(temp != data[idx])
                              DUNE_THROW(Exception, "data is not consistent");
                          });
  }
  return 0;
}
