// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_PARALLEL_MPIFUTURE_HH
#define DUNE_COMMON_PARALLEL_MPIFUTURE_HH

#include "future.hh"
#include "mpidata.hh"

#if HAVE_MPI
namespace Dune{

  template<class R, class S = void>
  class MPIFuture : public Future<R>{
    MPI_Request req_;
    MPI_Status status_;
    std::unique_ptr<MPIData<R>> data_;
    std::unique_ptr<MPIData<S>> send_data_;
    bool valid_;
    friend class CollectiveCommunication<MPI_Comm>;
  public:
    MPIFuture(bool valid = false) :
      valid_(valid)
    {}

    template<class V = R, class U = S>
    MPIFuture(V&& recv_data, U&& send_data, typename std::enable_if_t<!std::is_void<V>::value && !std::is_void<U>::value>* = 0) :
      req_(MPI_REQUEST_NULL)
      , data_(std::make_unique<MPIData<R>>(std::forward<R>(recv_data)))
      , send_data_(std::make_unique<MPIData<S>>(std::forward<S>(send_data)))
      , valid_(true)
    {}

    template<class V = R>
    MPIFuture(V&& recv_data, typename std::enable_if_t<!std::is_void<V>::value>* = 0) :
      req_(MPI_REQUEST_NULL)
      , data_(std::make_unique<MPIData<R>>(std::forward<R>(recv_data)))
      , valid_(true)
    {}

    virtual ~MPIFuture() override {}
    MPIFuture(MPIFuture&&) = default;
    MPIFuture& operator=(MPIFuture&&) = default;

    bool valid() const override{
      return valid_;
    }

    void wait() override{
      if(!valid_)
        DUNE_THROW(InvalidFutureException, "The MPIFuture is not valid!");
      MPI_Wait(&req_, &status_);
    }

    bool ready() override{
      int flag = -1;
      MPI_Test(&req_, &flag, &status_);
      return flag;
    }

    R get() override{
      if(!valid_)
        DUNE_THROW(InvalidFutureException, "The MPIFuture is not valid!");
      wait();
      valid_ = false;
      return data_->get();
    }

    S get_send_data(){
      wait();
      return send_data_.get();
    }
  };
}
#endif
#endif
