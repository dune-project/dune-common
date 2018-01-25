// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

// #define DUNE_MINIMAL_DEBUG_LEVEL 1

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpiguard.hh>

int main(int argc, char** argv)
{
  Dune::MPIHelper & mpihelper = Dune::MPIHelper::instance(argc, argv);
  auto world = mpihelper.getCommunicator();
  auto cc = mpihelper.getCollectiveCommunication();
  if (mpihelper.rank() == 0)
    std::cout << "---- default constructor" << std::endl;
  try
  {
    // at the end of this block the guard is destroyed and possible exceptions are communicated
    {
      Dune::MPIGuard guard;
      if (mpihelper.rank() > 0)
        DUNE_THROW(Dune::Exception, "Fakeproblem on process " << mpihelper.rank());
      guard.finalize();
    }
  }
  catch (Dune::Exception & e)
  {
    std::cout << "Error (rank " << mpihelper.rank() << "): "
              << e.what() << std::endl;
    world.shrink();
  }

  cc.barrier();
  if (mpihelper.rank() == 0)
    std::cout << "---- guard(MPI_COMM_WORLD)" << std::endl;
  try
  {
#if HAVE_MPI
    // at the end of this block the guard is destroyed and possible exceptions are communicated
    {
      Dune::MPIGuard guard(world);
      if (mpihelper.rank() > 0)
        DUNE_THROW(Dune::Exception, "Fakeproblem on process " << mpihelper.rank());
      guard.finalize();
    }
#else
    std::cout << "Info: no mpi used\n";
#endif
  }
  catch (Dune::Exception & e)
  {
    std::cout << "Error (rank " << mpihelper.rank() << "): "
              << e.what() << std::endl;
    world.shrink();
  }

  cc.barrier();
  if (mpihelper.rank() == 0)
    std::cout << "---- guard(MPIHelper)" << std::endl;
  try
  {
    // at the end of this block the guard is destroyed and possible exceptions are communicated
    {
      Dune::MPIGuard guard(mpihelper);
      if (mpihelper.rank() > 0)
        DUNE_THROW(Dune::Exception, "Fakeproblem on process " << mpihelper.rank());
      guard.finalize();
    }
  }
  catch (Dune::Exception & e)
  {
    std::cout << "Error (rank " << mpihelper.rank() << "): "
              << e.what() << std::endl;
    mpihelper.getCommunicator().shrink();
  }


  mpihelper.getCollectiveCommunication().barrier();
  if (mpihelper.rank() == 0)
    std::cout << "---- manual error" << std::endl;
  try
  {
    // at the end of this block the guard is destroyed and possible exceptions are communicated
    {
      try{
        Dune::MPIGuard guard;
        guard.finalize(mpihelper.rank() > 0);
      } catch(std::exception& e){
        std::cout << e.what() << std::endl;
        mpihelper.getCommunicator().shrink();
      }
    }
  }
  catch (Dune::Exception & e)
  {
    std::cout << "Error (rank " << mpihelper.rank() << "): "
              << e.what() << std::endl;
    mpihelper.getCommunicator().shrink();
  }

#if DUNE_ENABLE_ULFM
  cc.barrier();
  if (mpihelper.rank() == 0)
    std::cout << "---- deadlock test" << std::endl;
  try
  {
    // at the end of this block the guard is destroyed and possible exceptions are communicated
    {
      Dune::MPIGuard guard;
      if (mpihelper.rank() > 0)
        DUNE_THROW(Dune::Exception, "Fakeproblem on process " << mpihelper.rank());
      cc.barrier();
    }
  }
  catch (Dune::Exception & e)
  {
    std::cout << "Error (rank " << mpihelper.rank() << "): "
              << e.what() << std::endl;
    mpihelper.getCommunicator().shrink();
  }
#endif

  mpihelper.getCollectiveCommunication().barrier();
  if (mpihelper.rank() == 0)
    std::cout << "---- done" << std::endl;
  return 0;
}
