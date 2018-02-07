// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

/**
 * @file @brief Implements a Future for MPI communication. It wrapps a
 * MPI_Request and provides methods to handle it. It also stores the
 * receive buffer.
 *
 * @author Nils-Arne Dreier
 * @ingroup ParallelCommunication
 */

#ifndef DUNE_COMMON_PARALLEL_MPIFUTURE_HH
#define DUNE_COMMON_PARALLEL_MPIFUTURE_HH

#include <type_traits>
#include <utility>
#include <memory>
#include <dune/common/stdstreams.hh>
#include "span.hh"
#include "mpiexceptions.hh"
#include "mpistatus.hh"

#if HAVE_MPI
#include <mpi.h>

namespace Dune
{

  class ManagedMPIComm;

  template<class C>
  class PointToPointCommunication;

  template<class C>
  class GenericMPICollectiveCommunication;

  template<class C>
  class MPIFile;

  template<class C>
  class MPIWin;

  template<class>
  class MPIFuture;

  class MPIFutureBase {
    friend class ManagedMPIComm;
  protected:
    bool is_valid_;
    MPI_Request req_;
    MPIFutureBase()
      : req_(MPI_REQUEST_NULL)
      , is_valid_(false){
      dverb << "MPIFutureBase::MPIFutureBase()" << std::endl;
    }

    virtual void complete(MPI_Status) {
      req_ = MPI_REQUEST_NULL;
    }

  public:
    MPIFutureBase (const MPI_Request& r)
      : req_(r)
      , is_valid_(true)
    {}
    MPIFutureBase (const MPIFutureBase&) = delete;
    MPIFutureBase (MPIFutureBase&& o)
      : req_(MPI_REQUEST_NULL)
      , is_valid_(false)
    {
      std::swap(req_, o.req_);
      std::swap(is_valid_, o.is_valid_);
    }

    MPIFutureBase& operator=(MPIFutureBase&& o) = default;

    MPI_Request get_req() const{
      return req_;
    }

    bool valid() const {
      return is_valid_;
    }

    virtual bool ready() {
      if(!valid())
        return false;
      if(req_ == MPI_REQUEST_NULL)
        return true;
      int flag = 0;
      MPI_Status s;
      dune_mpi_call(MPI_Test, &req_, &flag, &s);
      if(flag){
        complete(s);
      }
      return flag;
    }
  };

  template<class U>
  struct unique_ptr_or_nothing : public std::unique_ptr<U>
  {
    unique_ptr_or_nothing(std::unique_ptr<U>&& u) :
      std::unique_ptr<U>(std::move(u))
    {}
  };

  template<>
  struct unique_ptr_or_nothing<void> {
    void release() const {}
  };

  /*! @brief Wraps a MPI_Request and provides the interface of a
    std::future. It also holds the respective memory as long as it
    is need by the request.
  */
  template<class T = void>
  class MPIFuture : public MPIFutureBase {
    friend class GenericMPICollectiveCommunication<ManagedMPIComm>;
    friend class PointToPointCommunication<ManagedMPIComm>;
    friend class MPIFile<ManagedMPIComm>;
    friend class MPIWin<ManagedMPIComm>;
    friend class MPIMatchingStatus;

    unique_ptr_or_nothing<T> p_buffer_; // use a unique_ptr to ensure that the
                             // address is preserved during moving
  protected:
    // We need to store whether the request is collective_ since
    // collective_ communications can't be canceled.
    bool collective_;

    std::add_lvalue_reference_t<T> buffer(){
      return *p_buffer_;
    }

  public:
    MPIFuture(){
      dverb << "MPIFuture::MPIFuture()" << std::endl;
    }

    template<typename U = T>
    MPIFuture(const ManagedMPIComm&, bool isCollective, U&& d) :
      MPIFutureBase(MPI_REQUEST_NULL),
      p_buffer_{std::make_unique<T>(std::forward<U>(d))},
      collective_(isCollective) {
      dverb << "MPIFuture::MPIFuture(const ManagedMPIComm&, bool, T&&)" << std::endl;
    }

    MPIFuture(const ManagedMPIComm&, bool isCollective)
      : MPIFutureBase(MPI_REQUEST_NULL)
      , collective_(isCollective) {
      dverb << "MPIFuture::MPIFuture(const ManagedMPIComm&, bool)" << std::endl;
    }

    MPIFuture(const MPIFuture&) = delete;
    MPIFuture(MPIFuture&& other) = default;

    MPIFuture& operator=(MPIFuture&& o) = default;

    ~MPIFuture() noexcept(false){
      dverb << "MPIFuture::~MPIFuture()" << std::endl;
      if (req_ != MPI_REQUEST_NULL){
        if(collective_)
        {
          dverb << "cant cancel collective_ request, releasing memory!" << std::endl;
          p_buffer_.release();
        } else {
          int wasFinalized = 0;
          dune_mpi_call(MPI_Finalized, &wasFinalized);
          if (!wasFinalized){
            dverb << "free request " << req_ << " (null = " << MPI_REQUEST_NULL << ")" << std::endl;
            dune_mpi_call(MPI_Cancel, &req_);
            dune_mpi_call(MPI_Request_free, &req_);
          }
        }
      }
    }

    virtual void wait() {
      dverb << "MPIFuture::wait(), req_=" << req_ << std::endl;
      if (req_ == MPI_REQUEST_NULL)  // already done
        return;
      MPI_Status s;
      dune_mpi_call(MPI_Wait, &req_, &s);
      complete(s);
    }

    T get(){
      wait();
      is_valid_ = false;
      return std::move(*p_buffer_);
    }
  };

  template<>
  void MPIFuture<void>::get(){
    is_valid_ = false;
    wait();
  }

  // Extend the Future with an MPI Status for MPI_Irecv
  template<class T, class B = MPIFuture<T>>
  class MPIRecvFuture : public B {
  protected:
    MPIStatus status_;

  public:
    MPIRecvFuture(){
      dverb << "MPIFuture::MPIFuture()" << std::endl;
    }

    template<typename C = ManagedMPIComm, typename U = T>
    MPIRecvFuture(const C& c, bool isCollective, U&& d) :
      B(c, isCollective, std::forward<U>(d))
    {
      dverb << "MPIFuture::MPIFuture(const ManagedMPIComm&, bool, T&&)" << std::endl;
    }

    template<typename C = ManagedMPIComm, typename U = T>
    MPIRecvFuture(const C& c, bool isCollective)
      : B(c, isCollective)
    {
      dverb << "MPIFuture::MPIFuture(const ManagedMPIComm&, bool)" << std::endl;
    }

    MPIRecvFuture(const MPIRecvFuture&) = delete;
    MPIRecvFuture(MPIRecvFuture&& other) = default;

    virtual void complete(MPI_Status s) override {
      B::complete(s);
      status_ = s;
      // resize if possible
      if (Span<T>::dynamic_size){
        Span<T> span(this->buffer());
        span.resize(status_.get_count(span.mpi_type()));
      }
    }

    const MPIStatus& status() const{
      return status_;
    }
  };

  template<class T>
  class MPIProbeFuture : public MPIFuture<T>
  {
    int source_, tag_;
    MPI_Comm comm_;

    void irecv(){
      Span<T> span(this->buffer());
      if(Span<T>::dynamic_size){
        span.resize(status_.get_count(span.mpi_type()));
      }
      dune_mpi_call(MPI_Imrecv, span.ptr(), span.size(),
                    span.mpi_type(), &status_.message_, &this->req_);
    }

  protected:
    MPIMatchingStatus status_;
    virtual void complete(MPI_Status s) override {
      MPIFuture<T>::complete(s);
      status_ = s;
      // resize if possible
      if (Span<T>::dynamic_size){
        Span<T> span(this->buffer());
        span.resize(status_.get_count(span.mpi_type()));
      }
    }

  public:
    template<typename C = ManagedMPIComm, typename U = T>
    MPIProbeFuture(const C& c, bool isCollective, U&& d, int rank, int tag)
      : MPIFuture<T>(c, isCollective, std::forward<U>(d))
      , source_(rank)
      , tag_(tag)
      , comm_(c)
      , status_()
    {}

    MPIProbeFuture(MPIProbeFuture&& o)
      : MPIFuture<T>(std::move(o))
      , comm_(MPI_COMM_NULL)
      , status_()
    {
      std::swap(source_, o.source_);
      std::swap(tag_, o.tag_);
      std::swap(status_, o.status_);
      std::swap(comm_, o.comm_);
    }

    virtual bool ready() override{
      if(status_.is_empty() &&
         status_.message_ == MPI_MESSAGE_NULL &&
         this->req_ == MPI_REQUEST_NULL){
        int flag;
        dune_mpi_call(MPI_Improbe, source_, tag_, comm_, &flag, &status_.message_,
                      status_);
        if(flag){
          irecv();
        }else
          return false;
      }
      return MPIFuture<T>::ready();
    }

    virtual void wait() override {
      if(status_.is_empty() &&
         status_.message_ == MPI_MESSAGE_NULL && this->req_ == MPI_REQUEST_NULL){
        dune_mpi_call(MPI_Mprobe, source_, tag_, comm_, &status_.message_,
                      status_);
      }
      if(status_.message_ != MPI_MESSAGE_NULL)
        irecv();
      MPIFuture<T>::wait();
    }

    const MPIStatus& status() const{
      return status_;
    }
  };

#endif // HAVE_MPI

}  // namespace Dune
#endif
