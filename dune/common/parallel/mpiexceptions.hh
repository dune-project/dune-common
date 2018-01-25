// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_PARALLEL_MPIEXCEPTIONS_HH
#define DUNE_COMMON_PARALLEL_MPIEXCEPTIONS_HH

#if HAVE_MPI

#include <sstream>

#include <mpi.h>

#include <dune/common/exceptions.hh>

namespace Dune{

  struct MPIError : public ParallelError{
    explicit MPIError(int error_code) :
      error_code_(error_code)
    {
    }

    int get_error_code() const{
      return error_code_;
    }

    int get_error_class() const{
      int error_class;
      MPI_Error_class( error_code_, &error_class);
      return error_class;
    }
  private:
    int error_code_;
  };

  struct MPIRemoteError : public ParallelError{};

  template<class F, class... Args>
  void dune_mpi_call(const F& mpi_fun, Args&&... args){
    int result = mpi_fun(std::forward<Args>(args)...);
    if(result != MPI_SUCCESS){
      char buf[MPI_MAX_ERROR_STRING];
      int len;
      MPI_Error_string(result, buf, &len);
      int rank;
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      std::ostringstream oss;
      oss << "MPI Error on rank " << rank << ": "
          << std::string(buf, len);
      MPIError ex(result);
      ex.message(oss.str());
      throw ex;
    }
  }

}  // end namespace Dune
#endif

#endif
