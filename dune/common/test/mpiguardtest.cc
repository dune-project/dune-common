// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpiguard.hh>

int main(int argc, char** argv)
{
  Dune::MPIHelper & mpihelper = Dune::MPIHelper::instance(argc, argv);

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
  }

  mpihelper.getCollectiveCommunication().barrier();
  if (mpihelper.rank() == 0)
    std::cout << "---- guard(MPI_COMM_WORLD)" << std::endl;
  try
  {
#if HAVE_MPI
    // at the end of this block the guard is destroyed and possible exceptions are communicated
    {
      Dune::MPIGuard guard(MPI_COMM_WORLD);
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
  }

  mpihelper.getCollectiveCommunication().barrier();
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
  }


  mpihelper.getCollectiveCommunication().barrier();
  if (mpihelper.rank() == 0)
    std::cout << "---- manual error" << std::endl;
  try
  {
    // at the end of this block the guard is destroyed and possible exceptions are communicated
    {
      Dune::MPIGuard guard;
      guard.finalize(mpihelper.rank() > 0);
    }
  }
  catch (Dune::Exception & e)
  {
    std::cout << "Error (rank " << mpihelper.rank() << "): "
              << e.what() << std::endl;
  }

#if HAVE_ULFM_REVOKE
  mpihelper.getCollectiveCommunication().barrier();
  if (mpihelper.rank() == 0)
    std::cout << "---- guard(MPIHelper) (ULFM, interrupt barrier)" << std::endl;
  try
  {
    // at the end of this block the guard is destroyed and possible exceptions are communicated
    {
      Dune::MPIGuard guard(mpihelper);
      if (mpihelper.rank() > 0)
        DUNE_THROW(Dune::Exception, "Fakeproblem on process " << mpihelper.rank());
      mpihelper.getCollectiveCommunication().barrier();
    }
  }
  catch (Dune::Exception & e)
  {
    std::cout << "Error (rank " << mpihelper.rank() << "): "
              << e.what() << std::endl;
  }
#else
  if(mpihelper.rank() == 0)
    std::cout << "Info: No ULFM available" << std::endl;
#endif

  mpihelper.getCollectiveCommunication().barrier();

  if (mpihelper.rank() == 0)
    std::cout << "---- test getFailedRanks() --- only even ranks fail" << std::endl;
  try{
    {
      Dune::MPIGuard guard;
      if(mpihelper.rank()%2 == 0)
        DUNE_THROW(Dune::Exception, "Fakeproblem on process" << mpihelper.rank());
      guard.finalize();
    }
  }catch(Dune::Exception& e){
    auto failed = Dune::MPIGuard::getFailedRanks(e, Dune::CollectiveCommunication<MPI_Comm>());
    if(mpihelper.rank() == 0)
      std::cout << "failed ranks are: ";
    for(int r : failed){
      if(mpihelper.rank() == 0)
        std::cout << r << ", ";
      if(r%2 != 0)
        return 1;
    }
    if(mpihelper.rank() == 0)
      std::cout << std::endl;
  }
  mpihelper.getCollectiveCommunication().barrier();
  if (mpihelper.rank() == 0)
    std::cout << "---- done" << std::endl;
  return 0;
}
