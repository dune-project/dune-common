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
      bool deleted_;

      Data(CFN&& cancel, FFN&& free)
        : req_(MPI_REQUEST_NULL)
        , cancel_(cancel)
        , free_(free)
        , status_()
        , deleted_(false)
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
      return s->MPI_ERROR;
    }

    static int free_fn(void * data){
      try{
        Data* ptr = static_cast<Data*>(data);
        if(std::this_thread::get_id() == ptr->worker_.get_id())
          ptr->worker_.detach(); // free_fn is called within complete (joining would deadlock)
        else
          ptr->worker_.join();
        ptr->free_();
        delete ptr;
      }catch(std::system_error& e){
        std::cout << "Caught an exception! " << e.what() << std::endl;
      }
      return MPI_SUCCESS;
    }

    static int cancel_fn(void * data, int complete){
      Data* ptr = static_cast<Data*>(data);
      ptr->cancel_(complete);
      int finalized = 0;
      MPI_Finalized( &finalized );
      if(!complete && !finalized)
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
          }catch(MPIError& e){
            data->status_.set_error(e.get_error_code());
          }catch(...){
            data->status_.set_error(MPI_ERR_UNKNOWN);
          }
          data->complete();
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
