// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpicommunication.hh>
#include <dune/common/test/testsuite.hh>

#include <iostream>
int main(int argc, char** argv)
{
  Dune::TestSuite t;

  typedef Dune::MPIHelper Helper;
  Helper& mpi = Helper::instance(argc, argv);

  {
    typedef Helper::MPICommunicator MPIComm;
    Dune::Communication<MPIComm> comm(mpi.getCommunicator());

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
    double size = mpi.size();
    for(int i=0; i<length; ++i)
    {
      t.check( std::abs( values[i] - size ) < 1e-8 );
      t.check( std::abs( val[i]    - size ) < 1e-8 );
    }

    {
      int i = 1;
      const auto sum = comm.sum(i);
      t.check(sum == comm.size())
        << "sum of 1 must be equal to number of processes";
    }
    {
      const int i = 1;
      const auto sum = comm.sum(i);
      t.check(sum == comm.size())
        << "sum of 1 must be equal to number of processes";
    }
  }

  std::cout << "We are at the end!"<<std::endl;

  return t.exit();
}
