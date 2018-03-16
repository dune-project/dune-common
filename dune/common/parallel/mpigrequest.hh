// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

/**
 * @file @brief Wrapper class for MPI Generalized Requests
 *
 * @author Nils-Arne Dreier
 * @ingroup ParallelCommunication
 */

#ifndef DUNE_COMMON_PARALLEL_MPIGREQUEST_HH
#define DUNE_COMMON_PARALLEL_MPIGREQUEST_HH

#include "mpihelper.hh"

#include <thread>
#include <mutex>

#if HAVE_MPI

namespace Dune{
#ifndef DOXYGEN
  namespace Impl{
    struct no_op
    {
      void operator()()
      {
        return;
      }
    };
  }
#endif

  /**
   * @brief Wrapper class for Generalized Request
   */
  template<class WFN, class CFN, class FFN = Impl::no_op>
  class MPIGRequest {
    struct Data {
      MPI_Request req_;
      CFN cancel_;
      FFN free_;
      MPIStatus status_;
      std::thread worker_;

      Data(CFN&& cancel, FFN&& free)
        : req_(MPI_REQUEST_NULL)
        , cancel_(cancel)
        , free_(free)
        , status_()
      {}

      void complete(){
        dune_mpi_call(MPI_Grequest_complete, req_);
      }
    };
    Data* data_;
    MPI_Request req_;

    static int query_fn(void * data, MPI_Status* s){
      Data* ptr = static_cast<Data*>(data);
      *s = ptr->status_;
      return MPI_SUCCESS;
    }

    static int free_fn(void * data){
      Data* ptr = static_cast<Data*>(data);
      ptr->worker_.join();
      ptr->free_();
      delete ptr;
      return MPI_SUCCESS;
    }

    static int cancel_fn(void * data, int complete){
      Data* ptr = static_cast<Data*>(data);
      ptr->cancel_(complete);
      ptr->status_.set_cancelled(true);
      return MPI_SUCCESS;
    }

  public:
    MPIGRequest(WFN&& work, CFN&& cancel, FFN&& free = Impl::no_op{})
    {
      data_ = new Data(std::move(cancel), std::move(free));
      dune_mpi_call(MPI_Grequest_start, &query_fn, &free_fn, &cancel_fn,
                    data_,
                    &req_);
      data_->req_ = req_;
      data_->worker_ = std::thread([data{this->data_}, work{std::move(work)}]() mutable{
          try{
            work(data->status_);
            data->complete();
          }catch(MPIError& e){
            data->status_.set_error(e.get_error_code());
          }catch(...){
            data->status_.set_error(MPI_ERR_UNKNOWN);
          }
        });
    }

    operator MPI_Request& (){
      return req_;
    }

    operator MPI_Request* (){
      return &req_;
    }
  };
}

#endif
#endif
