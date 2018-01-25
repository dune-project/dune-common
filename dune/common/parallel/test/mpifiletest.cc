// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpiguard.hh>
#include <dune/common/parallel/mpifile.hh>

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

  auto comm = helper.getCommunicator();
  int rank = comm.rank();
  const char* filename = "mpi_test_file.out";
  try{
    constexpr int vec_len = 2;
    typedef Dune::FieldVector<double, vec_len> VectorType;
    VectorType vec(42.0 + rank);
    {
      Dune::MPIGuard guard(comm);
      Dune::MPIFile<decltype(comm)> file(comm, filename);
      // test assignment
      Dune::MPIFile<decltype(comm)> file2 = file;
      std::cout << rank << ":\tloaded file " << filename << std::endl;
      std::cout << rank << ":\tfilesize: " << file2.get_size() << std::endl;

      bool a = file2.get_atomicity();
      std::cout << rank << ":\tatomicity is set " << (a?"on":"off") << std::endl;
      file2.seek(sizeof(vec)*rank, Dune::MPIFile<decltype(comm)>::Whence::set);
      std::cout << comm.rank() << ":\twrite " << vec << std::endl;
      auto f = file2.iwrite(vec);
      f.wait();
    }
    // switch to other communicator
    comm = helper.getCommunicator().dup();
    {
      Dune::MPIGuard guard(comm);
      Dune::MPIFile<decltype(comm)> file(comm, filename);
      std::cout << rank << ":\tloaded file " << filename << std::endl;
      std::cout << rank << ":\tfilesize: " << file.get_size() << std::endl;

      file.seek(sizeof(vec)*rank, Dune::MPIFile<decltype(comm)>::Whence::set);
      auto f = file.iread(VectorType{});
      auto vec2 = f.get();
      std::cout << comm.rank() << ":\tread " << vec2 << std::endl;
      if (vec != vec2)
        DUNE_THROW(Dune::Exception, "Read wrong value");
    }
    if(rank == 0)
      Dune::delete_file(filename);
  }catch(std::exception& e){
    std::cout << e.what() << std::endl;
    return 1;
  }
  return 0;
#endif
}
