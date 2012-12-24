// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/parallel/mpihelper.hh>

#if HAVE_MPI
#include <dune/common/parallel/mpicollectivecommunication.hh>
#endif

#include <iostream>
int main(int argc, char** argv)
{
  typedef Dune::MPIHelper Helper;
  Helper& mpi = Helper::instance(argc, argv);

#if HAVE_MPI
  {
    typedef Helper::MPICommunicator MPIComm;
    Dune::CollectiveCommunication<MPIComm> comm(mpi.getCommunicator());

    enum { length = 5 };
    double values[5];
    for(int i=0; i<length; ++i) values[i] = 1.0;

    double * commBuff = ((double *) &values[0]);
    // calculate global sum
    comm.sum( commBuff , length );

    double val[length];
    for(int i=0; i<length; ++i) val[i] = 1.0;
    // calculate global sum by calling sum for each component
    for(int i=0; i<length; ++i)
    {
      // this method works
      val[i] = comm.sum( val[i] );
    }

    // result from above should be size of job
#ifndef NDEBUG
    double sum = mpi.size();
#endif
    for(int i=0; i<length; ++i)
    {
      assert( std::abs( values[i] - sum ) < 1e-8 );
      assert( std::abs( val[i]    - sum ) < 1e-8 );
    }
  }

  std::cout << "We are at the end!"<<std::endl;
#else
  std::cout << "WARNING: test Dune::CollectiveCommunication<MPI_Comm> disabled because MPI not available! " << std::endl;
  return 77;
#endif
  return 0;
}
