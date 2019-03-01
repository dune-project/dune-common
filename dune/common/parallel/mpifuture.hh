// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_PARALLEL_MPIFUTURE_HH
#define DUNE_COMMON_PARALLEL_MPIFUTURE_HH

#include <dune/common/std/optional.hh>
#include <dune/common/parallel/future.hh>
#include <dune/common/parallel/mpidata.hh>

#if HAVE_MPI
namespace Dune{

  namespace impl {
    template<class T>
    struct WRAP_IF_REF{
      typedef T type;
    };
    template<class T>
    struct WRAP_IF_REF<T&>{
      typedef std::reference_wrapper<std::remove_reference_t<T>> type;
    };
    template<>
    struct WRAP_IF_REF<void>{
      typedef struct{} type;
    };
  }

  /*! \brief Provides a future-like object for MPI communication.  It contains
    the object that will be received and might contain also a sending object,
    which must be hold (keep alive) until the communication has been completed.
   */
  template<class R, class S = void>
  class MPIFuture{
    template<class T>
    friend class when_all_MPIFuture;

    template<class T>
    friend class when_any_MPIFuture;
    mutable MPI_Request req_;
    mutable MPI_Status status_;
    Std::optional<typename impl::WRAP_IF_REF<R>::type> data_;
    Std::optional<typename impl::WRAP_IF_REF<S>::type> send_data_;
    friend class Communication<MPI_Comm>;
  public:
    MPIFuture(bool valid = false)
      : req_(MPI_REQUEST_NULL)
      , data_(valid?Std::optional<typename impl::WRAP_IF_REF<R>::type>(typename impl::WRAP_IF_REF<R>::type()):Std::nullopt)
    {}

    // Hide this constructor if R or S is void
    template<class V = R, class U = S>
    MPIFuture(V&& recv_data, U&& send_data, typename std::enable_if_t<!std::is_void<V>::value && !std::is_void<U>::value>* = 0) :
      req_(MPI_REQUEST_NULL)
      , data_(std::forward<R>(recv_data))
      , send_data_(std::forward<S>(send_data))
    {}

    // hide this constructor if R is void
    template<class V = R>
    MPIFuture(V&& recv_data, typename std::enable_if_t<!std::is_void<V>::value>* = 0)
      : req_(MPI_REQUEST_NULL)
      , data_(std::forward<R>(recv_data))
    {}

    ~MPIFuture() {
      if(req_ != MPI_REQUEST_NULL){
        try{ // might fail when it is a collective communication
          MPI_Cancel(&req_);
          MPI_Request_free(&req_);
        }catch(...){
        }
      }
    }

    MPIFuture(MPIFuture&& f)
      : req_(MPI_REQUEST_NULL)
      , data_(std::move(f.data_))
      , send_data_(std::move(f.send_data_))
    {
      std::swap(req_, f.req_);
      std::swap(status_, f.status_);
    }

    MPIFuture& operator=(MPIFuture&& f){
      std::swap(req_, f.req_);
      std::swap(status_, f.status_);
      std::swap(data_, f.data_);
      std::swap(send_data_, f.send_data_);
      return *this;
    }

    bool valid() const{
      return (bool)data_;
    }

    void wait(){
      if(!valid())
        DUNE_THROW(InvalidFutureException, "The MPIFuture is not valid!");
      MPI_Wait(&req_, &status_);
    }

    bool ready() const{
      int flag = -1;
      MPI_Test(&req_, &flag, &status_);
      return flag;
    }

    R get() {
      wait();
      Std::optional<typename impl::WRAP_IF_REF<R>::type> tmp;
      std::swap(tmp, data_);
      return (R)tmp.value();
    }

    S get_send_data(){
      wait();
      S tmp(std::move(send_data_.value()));
      send_data_.reset();
      return tmp;
    }

    auto get_mpidata(){
      return getMPIData<R>(*data_);
    }

    auto get_send_mpidata(){
      return getMPIData<S>(*send_data_);
    }
  };
}
#endif
#endif
