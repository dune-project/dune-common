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

int main(int argc, char** argv){
  auto& helper = Dune::MPIHelper::instance(argc, argv);
  int rank = helper.rank();
  int size = helper.size();

  // setup pattern
  Dune::CommunicationPattern<> ring_pattern(rank,
                                            { // send pattern:
                                             {(rank+1)%size, {0,1,2}}
    },
    { // recv pattern
     {(rank+size-1)%size, {0,1,2}}
    });

  std::cout << ring_pattern << std::endl;

  typedef Dune::MPIPatternCommunicator<Dune::CommunicationPattern<>> Comm;

  {
    Comm communicator(ring_pattern,helper.getCommunicator(), 8);
    std::vector<double> data(3);
    std::iota(data.begin(), data.end(), rank);

    communicator.exchange([&](auto& buf, auto& idx){ buf.write(data[idx]); },
                          [&](auto& buf, auto& idx){ buf.read(data[idx]); });
    if(data[0] != (rank+1)%size)
      DUNE_THROW(Dune::Exception, "wrong data after communication");
    std::cout << "Rank " << rank << " data:" << std::endl;
    for(double& d : data){
      std::cout << d << " " << std::endl;
    }

    helper.getCommunication().barrier();

    communicator.exchange(data, data, std::plus<double>{});
    if(data[0] != (rank+1)%size + (rank+2)%size)
      DUNE_THROW(Dune::Exception, "wrong data after communication");
    std::cout << "Rank " << rank << " data:" << std::endl;
    for(double& d : data){
      std::cout << d << " " << std::endl;
    }
  }


  // test variable buffer size
  {
    Comm communicator(ring_pattern, helper.getCommunicator());
    std::vector<double> data(3);
    std::iota(data.begin(), data.end(), rank);

    // communicate one double
    communicator.exchange([&](auto& buf, auto& idx){ buf.write(data[idx]); },
                          [&](auto& buf, auto& idx){ buf.read(data[idx]); });
    if(data[0] != (rank+1)%size)
      DUNE_THROW(Dune::Exception, "wrong data after communication");

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
    if(data[0] != (rank+2)%size+(rank+2)%size+1)
      DUNE_THROW(Dune::Exception, "wrong data after communication");
    std::cout << "Rank " << rank << " data:" << std::endl;
    for(double& d : data){
      std::cout << d << " " << std::endl;
    }
  }
  return 0;
}
