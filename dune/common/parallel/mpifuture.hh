// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_PARALLEL_MPIFUTURE_HH
#define DUNE_COMMON_PARALLEL_MPIFUTURE_HH

#include <optional>

#include <dune/common/parallel/communication.hh>
#include <dune/common/parallel/future.hh>
#include <dune/common/parallel/mpidata.hh>

#if HAVE_MPI
namespace Dune{

  namespace impl{
    template<class T>
    struct Buffer{
      Buffer(bool valid){
        if(valid)
          value = std::make_unique<T>();
      }
      template<class V>
      Buffer(V&& t)
        : value(std::make_unique<T>(std::forward<V>(t)))
      {}
      std::unique_ptr<T> value;
      T get(){
        T tmp = std::move(*value);
        value.reset();
        return tmp;
      }
      operator bool () const {
        return (bool)value;
      }
      T& operator *() const{
        return *value;
      }
    };

    template<class T>
    struct Buffer<T&>{
      Buffer(bool valid = false)
      {
        if(valid)
          value = T();
      }
      template<class V>
      Buffer(V&& t)
        : value(std::forward<V>(t))
      {}
      std::optional<std::reference_wrapper<T>> value;
      T& get(){
        T& tmp = *value;
        value.reset();
        return tmp;
      }
      operator bool () const{
        return (bool)value;
      }
      T& operator *() const{
        return *value;
      }
    };

    template<>
    struct Buffer<void>{
      bool valid_;
      Buffer(bool valid = false)
        : valid_(valid)
      {}
      operator bool () const{
        return valid_;
      }
      void get(){}
    };
  }

  /*! \brief Provides a future-like object for MPI communication.  It contains
    the object that will be received and might contain also a sending object,
    which must be hold (keep alive) until the communication has been completed.
   */
  template<class R, class S = void>
  class MPIFuture{
    mutable MPI_Request req_;
    mutable MPI_Status status_;
    impl::Buffer<R> data_;
    impl::Buffer<S> send_data_;
    friend class Communication<MPI_Comm>;
  public:
    MPIFuture(bool valid = false)
      : req_(MPI_REQUEST_NULL)
      , data_(valid)
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
      , data_(std::forward<V>(recv_data))
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
      return data_.get();
    }

    S get_send_data(){
      wait();
      return send_data_.get();
    }

    auto get_mpidata(){
      return getMPIData(*data_);
    }

    auto get_send_mpidata(){
      return getMPIData(*send_data_);
    }
  };

}
#endif
#endif
