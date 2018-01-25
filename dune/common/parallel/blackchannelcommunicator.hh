// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

/**
 * @file @brief Extends the ManagedMPIComm with Fault-Tolerant
 features, which only depend on MPI-3.
 @author Nils-Arne Dreier
 @ingroup ParallelCommunication
 */

#ifndef DUNE_COMMON_PARALLEL_BLACKCHANNELCOMMUNICATOR_HH
#define DUNE_COMMON_PARALLEL_BLACKCHANNELCOMMUNICATOR_HH

#if HAVE_MPI

#include <set>
#include <vector>
#include <type_traits>

#include "pointtopointcommunication.hh"
#include "collectivecommunication.hh"
#include "mpifuture.hh"

namespace Dune
{
  class ManagedMPIComm;

  template<class>
  class BlackChannelFuture;

  template<class>
  class BlackChannelProbeFuture;

  template<class>
  struct is_BlackChannelFuture : std::false_type {};
  template<class T>
  struct is_BlackChannelFuture<BlackChannelFuture<T>> : std::true_type {};

  /*! @brief Extends the ManagedMPIComm with a 'Black-Channel' on
      which is listen when waiting for an request. If the request of
      the Black-Channel completes, this Communicator falls into an
      error state similar to the revoke error-state in ULFM and an
      exception is thrown.

      The 'Black-Channel' is implemented by a non-blocking irecv. The
      Black-Channel Future overrides the wait method and waits for
      both the 'Black-Channel' and the actual request. If the
      Black-Channel completed an exception is thrown.
   */
  class BlackChannelCommunicator : public ManagedMPIComm
  {
    template<class>
    friend class BlackChannelFuture;
    template<class>
    friend class BlackChannelProbeFuture;
    template<class>
    friend class MPIProbeFuture;
    template<class, class>
    friend class MPIRecvFuture;

    friend class PointToPointCommunication<BlackChannelCommunicator>;

    ManagedMPIComm bc_comm_;
    std::shared_ptr<Future<int>> bc_req_;
    std::shared_ptr<bool> is_revoked_;

    static void throw_exception() {
      if(!std::uncaught_exception())
        DUNE_THROW(MPIRemoteError, comm_world().rank() << ":\tRemote rank failed!");
    }

  protected:
    /*! @brief recovers the communicator from error-state
     */
    void renew_bc(){
      dverb << "BlackChannelCommunicator::renew_bc()" << std::endl;
      bc_comm_.renew();
      PointToPointCommunication<ManagedMPIComm> ptpc(bc_comm_);
      *bc_req_ = ptpc.irecv(1, MPI_ANY_SOURCE, 666, false);
    }

    // Adds a Black-Channel to a ManagedMPIComm
    BlackChannelCommunicator( const ManagedMPIComm& mc) :
      ManagedMPIComm(mc),
      bc_comm_(mc?mc.dup():mc),
      is_revoked_(std::make_shared<bool>(false))
    {
      if(mc){
        dverb << "BlackChannelCommunicator::BlackChannelCommunicator( const ManagedMPIComm& )" << std::endl;
        PointToPointCommunication<ManagedMPIComm> ptpc(bc_comm_);
        bc_req_ = std::make_shared<Future<int>>(ptpc.irecv(1, MPI_ANY_SOURCE, 666));
      }
    }
  public:
    // Export the Future-type
    template<class T = void>
    using FutureType = BlackChannelFuture<T>;

    template<class T = void>
    using RecvFutureType = MPIRecvFuture<T, BlackChannelFuture<T>>;

    template<class T = void>
    using ProbeFutureType = BlackChannelProbeFuture<T>;

    BlackChannelCommunicator() : BlackChannelCommunicator(ManagedMPIComm())
    {};

    DUNE_DEPRECATED
    BlackChannelCommunicator(MPI_Comm c) : BlackChannelCommunicator(ManagedMPIComm(c))
    {};

    static BlackChannelCommunicator comm_world(){
      static BlackChannelCommunicator mmc_world(ManagedMPIComm::comm_world());
      return mmc_world;
    }

    static BlackChannelCommunicator comm_self(){
      static BlackChannelCommunicator mmc_self(ManagedMPIComm::comm_self());
      return mmc_self;
    }

    BlackChannelCommunicator dup() const
    {
      ManagedMPIComm mmc_duplicate = ManagedMPIComm::dup();
      return mmc_duplicate;
    }

    /**
     * @param color
     * @param key order of ranks in split communicator
     *
     * @return new communicator containing all ranks with same color
     */
    BlackChannelCommunicator split(int color, int key) const
    {
      ManagedMPIComm mmc_split = ManagedMPIComm::split(color, key);
      return mmc_split;
    }

    /** @brief Sends messages on the black-channel to all remote ranks
     * indicating that this communicator has been revoked.
     */
    void revoke()
    {
      if (*is_revoked_)
        return;
      dverb << "BlackChannelCommunicator::revoke()" << std::endl;
      std::vector<MPIFuture<>> send_futures(0);
      send_futures.reserve(size());
      PointToPointCommunication<ManagedMPIComm> ptpc(bc_comm_);
      for (int i = 0; i < size(); ++i)
      {
        if(i != rank())
          send_futures.push_back(ptpc.isend(1, i, 666));
      }
      renew_bc(); // synchronize with others
      *is_revoked_ = true;
    }

    /** @brief Resolves revoked state by duplicating this
     * communicator. This invalids all existing requests (including
     * collective ones).
     */
    void shrink()
    {
      dverb << "BlackChannelCommunicator::shrink()" << std::endl;
      MPI_Comm new_comm;
      dune_mpi_call(MPI_Comm_dup, bc_comm_, &new_comm);
      freeComm(comm_.get());
      *comm_ = new_comm;
      *is_revoked_ = false;
    }
  };


  /*! @brief Adapts the MPIFutures for the Black-Channel communicator
      by overriding the wait and valid methods.
   */
  template<class T = void>
  class BlackChannelFuture
    : public MPIFuture<T>
  {
    friend class BlackChannelCommunicator;
    friend class GenericMPICollectiveCommunication<BlackChannelCommunicator>;
    friend class PointToPointCommunication<BlackChannelCommunicator>;
    friend class MPIFile<BlackChannelCommunicator>;
    friend class MPIWin<BlackChannelCommunicator>;
    friend class MPIMatchingStatus;

  protected:
    BlackChannelCommunicator comm_;
    using MPIFutureBase::req_;
  public:
    BlackChannelFuture() : MPIFuture<T>() {
      dverb << "BlackChannelFuture::BlackChannelFuture()" << std::endl;
    }

    template<typename U = T>
    BlackChannelFuture(const BlackChannelCommunicator& c, bool isCollective, U&& d) :
      MPIFuture<T>(comm_, isCollective, std::forward<U>(d)),
      comm_(c) {
      dverb << "BlackChannelFuture::BlackChannelFuture(const BlackChannelCommunicator&, bool, U&&)" << std::endl;
    }

    BlackChannelFuture(const BlackChannelCommunicator& c, bool isCollective) :
      MPIFuture<T>(comm_, isCollective),
      comm_(c) {
      dverb << "BlackChannelFuture::BlackChannelFuture(const BlackChannelCommunicator&, bool)" << std::endl;
    }

    BlackChannelFuture(BlackChannelFuture&& o) :
      MPIFuture<T>(std::move(o)),
      comm_(std::move(o.comm_))
    {
    }

    BlackChannelFuture& operator= (BlackChannelFuture&& o){
      std::swap(comm_, o.comm_);
      MPIFuture<T>::operator =(std::move(o));
    }

    // We have to wait for both the actual requests and the
    // Black-Channel.
    virtual void wait() override {
      Future<T> f(*this);
      std::vector<int> ind = waitsome(*this->comm_.bc_req_, f);
      if(ind.size() > 1 || ind[0] == 0){
        comm_.renew_bc();
        *comm_.is_revoked_ = true;
        this->comm_.throw_exception();
      }
    }

    virtual bool valid() override{
      if(this->comm_.bc_req_->valid()){
        comm_.renew_bc();
        *comm_.is_revoked_ = true;
        this->comm_.throw_exception();
      }
      return MPIFuture<T>::valid();
    }
  };


  template<class T>
  class BlackChannelProbeFuture : public BlackChannelFuture<T>
  {
    int source_, tag_;
    MPIMatchingStatus status_;

    void irecv(){
      Span<T> span(this->buffer());
      if(Span<T>::dynamic_size){
        span.resize(status_.get_count(span.mpi_type()));
      }
      dune_mpi_call(MPI_Imrecv, span.ptr(), span.size(),
                    span.mpi_type(), &status_.message_, &this->req_);
    }

  protected:
    virtual void complete(MPI_Status s) override {
      BlackChannelFuture<T>::complete(s);
      status_ = s;
      // resize if possible
      if (Span<T>::dynamic_size){
        Span<T> span(this->buffer());
        span.resize(status_.get_count(span.mpi_type()));
      }
    }

  public:
    template<typename C = BlackChannelCommunicator, typename U = T>
    BlackChannelProbeFuture(const C& c, bool isCollective, U&& d, int rank, int tag)
      : BlackChannelFuture<T>(c, isCollective, std::forward<U>(d))
      , source_(rank)
      , tag_(tag)
      , status_()
    {}

    BlackChannelProbeFuture(BlackChannelProbeFuture&& o) :
      BlackChannelFuture<T>(std::move(o)),
      status_()
    {
      std::swap(source_, o.source_);
      std::swap(tag_, o.tag_);
      std::swap(status_, o.status_);
    }

    virtual bool valid() override{
      if(this->comm_.bc_req_->valid()){
        this->comm_.renew_bc();
        *this->comm_.is_revoked_ = true;
        this->comm_.throw_exception();
      }
      if(status_.is_empty() &&
         this->status_.message_ == MPI_MESSAGE_NULL &&
         this->req_ == MPI_REQUEST_NULL){
        int flag;
        dune_mpi_call(MPI_Improbe, source_, tag_, this->comm_, &flag, &status_.message_,
                      status_);
        if(flag){
          irecv();
        }else
          return false;
      }
      return MPIFuture<T>::valid();
    }

    // we do not need to reimplement wait() because BlackChannelFuture::wait() rely on valid()

    const MPIStatus& status() const{
      return status_;
    }
  };

  // Specialize blocking communications to use non-blocking
  // communication. That is needed by the BlackChannelCommunicator

  //! @copydoc PointToPointCommunication<ManagedMPIComm>::send
  template<>
  template<typename T, Communication_Mode m>
  void PointToPointCommunication<BlackChannelCommunicator>::send(const T& data, int rank, int tag)
  {
    isend<T, m>(data, rank, tag).wait();
  }

  //! @copydoc PointToPointCommunication<ManagedMPIComm>::recv
  template<>
  template<typename T>
  MPIStatus PointToPointCommunication<BlackChannelCommunicator>::recv(T& data, int rank, int tag)
  {
    do{ // busy wait check black-channel and probe alternating
      if(communicator.bc_req_->valid())
        BlackChannelCommunicator::throw_exception();
      MPIMatchingStatus s = improbe(rank, tag);
      if(s.has_message()){
        s.recv(data);
        return s;
      }
    }while(true);
  }

  // same for collective communications
  template<>
  inline void GenericMPICollectiveCommunication<BlackChannelCommunicator>::barrier() const
  {
    ibarrier().wait();
  }

  template<>
  template<typename T>
  DUNE_DEPRECATED
  inline int GenericMPICollectiveCommunication<BlackChannelCommunicator>::broadcast (T* inout, int len, int root) const{
    assert(len > 0);
    auto f = ibroadcast(Span<T*>{inout, (size_t)len}, root);
    f.wait();
    return 0;
  }

  template<>
  template<typename T>
  inline void GenericMPICollectiveCommunication<BlackChannelCommunicator>::broadcast (T& inout, int root) const{
    auto f = ibroadcast(std::move(inout), root);
    inout = f.get();
  }

  template<>
  template<typename T, typename S>
  DUNE_DEPRECATED
  inline int GenericMPICollectiveCommunication<BlackChannelCommunicator>::gather (T* in, S* out, int len, int root) const{
    if(root == rank())
      igather(Span<T*>{in, size()*len}, Span<S*>{out, len}, root).wait();
    else
      igather(Span<T*>{in, 0}, Span<S*>{out, len}, root).wait();
    return 0;
  }

  template<>
  template<typename T, typename S>
  inline void GenericMPICollectiveCommunication<BlackChannelCommunicator>::gather (const T& in, S& out, int root) const{
    auto f = igather(in, std::move(out), root);
    if(rank() == root)
      out = f.get();
    else
      f.wait();
  }

  template<>
  template<typename T, typename S>
  DUNE_DEPRECATED
  inline int GenericMPICollectiveCommunication<BlackChannelCommunicator>::scatter (T* send, S* recv, int len, int root) const
  {
    if(root == rank())
      iscatter(Span<T*>{send, size()*len}, Span<S*>{recv, len}, root).wait();
    else
      iscatter(Span<T*>{send, 0}, Span<S*>{recv, len}, root).wait();
    return 0;
  }

  template<>
  template<typename T, typename S>
  inline void GenericMPICollectiveCommunication<BlackChannelCommunicator>::scatter (const T& send, S& recv, int root) const
  {
    auto f = iscatter(send, std::move(recv), root);
    recv = f.get();
  }

  template<>
  template<typename T, typename T1>
  DUNE_DEPRECATED
  inline int GenericMPICollectiveCommunication<BlackChannelCommunicator>::allgather(T* sbuf, int count, T1* rbuf) const
  {
    iallgather(Span<T*>{sbuf, count}, Span<T*>{rbuf, size()*count}).wait();
    return 0;
  }

  template<>
  template<typename T, typename T1>
  inline void GenericMPICollectiveCommunication<BlackChannelCommunicator>::allgather(const T& sbuf, T1& rbuf) const
  {
    auto f = iallgather(sbuf, std::move(rbuf));
    rbuf = f.get();
  }

  template<>
  template<typename BinaryFunction, typename T>
  DUNE_DEPRECATED
  inline int GenericMPICollectiveCommunication<BlackChannelCommunicator>::allreduce(T* inout, int len) const
  {
    iallreduce<BinaryFunction>(Span<T*>{inout, len}).wait();
    return 0;
  }

  template<>
  template<typename BinaryFunction, typename T>
  inline void GenericMPICollectiveCommunication<BlackChannelCommunicator>::allreduce(const T& in, T& out) const
  {
    auto f = iallreduce<BinaryFunction>(in, std::move(out));
    out = f.get();
  }

  template<>
  template<typename BinaryFunction, typename T>
  inline void GenericMPICollectiveCommunication<BlackChannelCommunicator>::allreduce(T& inout) const
  {
    auto f = iallreduce<BinaryFunction>(std::move(inout));
    inout = f.get();
  }

  template<>
  class CollectiveCommunication<BlackChannelCommunicator> :
    public GenericMPICollectiveCommunication<BlackChannelCommunicator> {
  public:
    CollectiveCommunication(const BlackChannelCommunicator& comm = BlackChannelCommunicator::comm_world()) :
      GenericMPICollectiveCommunication(comm) {}
  };
}
#endif

#endif
