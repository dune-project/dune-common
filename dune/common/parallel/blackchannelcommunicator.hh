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
    friend class BlackChannel_when_all_future;
    template<class>
    friend class BlackChannel_when_any_future;

    friend class PointToPointCommunication<BlackChannelCommunicator>;

    ManagedMPIComm bc_comm_;
    std::shared_ptr<MPIFuture<int>> bc_req_;
    std::shared_ptr<bool> is_revoked_;

    void throw_exception() {
      *is_revoked_ = true;
      DUNE_THROW(MPIRemoteError, comm_world().rank() << ":\tRemote rank failed!");
    }

  protected:
    // Adds a Black-Channel to a ManagedMPIComm
    BlackChannelCommunicator( const ManagedMPIComm& mc) :
      ManagedMPIComm(mc),
      bc_comm_(mc?mc.dup():mc),
      is_revoked_(std::make_shared<bool>(false))
    {
      if(mc){
        dverb << "BlackChannelCommunicator::BlackChannelCommunicator( const ManagedMPIComm& )" << std::endl;
        PointToPointCommunication<ManagedMPIComm> ptpc(bc_comm_);
        bc_req_ = std::make_shared<MPIFuture<int>>(ptpc.irecv(1, MPI_ANY_SOURCE, 666));
      }
    }
  public:
    // Export the Future-type
    template<class T = void>
    using FutureType = BlackChannelFuture<T>;

    BlackChannelCommunicator() : BlackChannelCommunicator(ManagedMPIComm())
    {};

    DUNE_DEPRECATED
    BlackChannelCommunicator(MPI_Comm c)
      : BlackChannelCommunicator(ManagedMPIComm(c))
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
      send_futures.reserve(size()-1);
      PointToPointCommunication<ManagedMPIComm> ptpc(bc_comm_);
      for (int i = 0; i < size(); ++i)
      {
        if(i != rank())
          send_futures.push_back(ptpc.isend(1, i, 666));
      }
      *is_revoked_ = true;
      when_all(send_futures.begin(), send_futures.end()).wait();
    }

    /** @brief
     * Perforns a allreduce with AND operator on the success flag on the black-channel communicator.
     */
    bool agree(bool success)
    {
      CollectiveCommunication<ManagedMPIComm> cc(bc_comm_);
      cc.allreduce<std::logical_and<bool>>(success);
      return success;
    }

    /** @brief Resolves revoked state by duplicating this
     * communicator. This invalids all existing requests (including
     * collective ones).
     */
    void shrink()
    {
      bc_comm_.renew();
      PointToPointCommunication<ManagedMPIComm> ptpc(bc_comm_);
      *bc_req_ = ptpc.irecv(1, MPI_ANY_SOURCE, 666, false);
      dverb << "BlackChannelCommunicator::shrink()" << "(oldComm = " << *comm_;
      MPI_Comm new_comm;
      dune_mpi_call(MPI_Comm_dup, bc_comm_, &new_comm);
      dverb << "| newComm = " << new_comm << ")" << std::endl;
      freeComm(comm_.get());
      *comm_ = new_comm;
      *is_revoked_ = false;
    }
  };


  /*! @brief Adapts the MPIFutures for the Black-Channel communicator
      by overriding the wait and ready methods.
   */
  template<class T = void>
  class BlackChannelFuture : protected MPIFuture<T>
  {
    friend class BlackChannelCommunicator;
    friend class GenericMPICollectiveCommunication<BlackChannelCommunicator>;
    friend class PointToPointCommunication<BlackChannelCommunicator>;
    friend class MPIFile<BlackChannelCommunicator>;
    friend class MPIWin<BlackChannelCommunicator>;
    friend class MPIMatchingStatus;
    template<class>
    friend class MPI_when_all_future;
    template<class>
    friend class BlackChannel_when_all_future;
    template<class>
    friend class MPI_when_any_future;
    template<class>
    friend class BlackChannel_when_any_future;

  protected:
    BlackChannelCommunicator comm_;
    auto& getBlackChannel(){
      return *(comm_.bc_req_);
    }

  public:
    BlackChannelFuture()
      : MPIFuture<T>()
    {
      dverb << "BlackChannelFuture::BlackChannelFuture()" << std::endl;
    }

    template<typename U = T>
    BlackChannelFuture(const BlackChannelCommunicator& c, bool isCollective, U&& d) :
      MPIFuture<T>(comm_, isCollective, std::forward<U>(d)),
      comm_(c)
    {
      dverb << "BlackChannelFuture::BlackChannelFuture(const BlackChannelCommunicator&, bool, U&&)" << std::endl;
    }

    BlackChannelFuture(const BlackChannelCommunicator& c, bool isCollective) :
      MPIFuture<T>(comm_, isCollective),
      comm_(c)
    {
      dverb << "BlackChannelFuture::BlackChannelFuture(const BlackChannelCommunicator&, bool)" << std::endl;
    }

    BlackChannelFuture(const BlackChannelFuture&) = delete;
    BlackChannelFuture& operator= (const BlackChannelFuture&) = delete;
    BlackChannelFuture(BlackChannelFuture&& o) = default;
    BlackChannelFuture& operator= (BlackChannelFuture&& o) = default;

    // We have to wait for both the actual requests or the
    // Black-Channel.
    virtual void wait() override {
      auto f = when_any(getBlackChannel(), static_cast<MPIFuture<T>&>(*this));
      auto result = f.get();
      if(result.index == 0){
        comm_.throw_exception();
      }
    }

    using MPIFuture<T>::get;
    using MPIFuture<T>::valid;
    using MPIFuture<T>::mpirequest;
    using MPIFuture<T>::status;

    virtual bool ready() override{
      if(getBlackChannel().ready()){
        comm_.throw_exception();
      }
      return MPIFuture<T>::ready();
    }
  };

  template<class Sequence>
  class BlackChannel_when_all_future;

  template<class T>
  class BlackChannel_when_all_future<std::vector<BlackChannelFuture<T>>>
  {
    std::vector<BlackChannelFuture<T>> futures_;
    bool valid_;
  public:
    template<class InputIt>
    BlackChannel_when_all_future(InputIt first, InputIt last)
      : futures_(std::make_move_iterator(first), std::make_move_iterator(last))
      , valid_(true)
    {
    }

    bool ready(){
      for(auto& f : futures_){
        if(!f.ready())
          return false;
      }
      return true;
    }

    bool valid(){
      return valid_;
    }

    void wait(){
      for(auto& f : futures_){
        f.wait();
      }
    }

    std::vector<BlackChannelFuture<T>> get(){
      wait();
      valid_ = false;
      return std::move(futures_);
    }
  };

  template<class... Fs>
  class BlackChannel_when_all_future<std::tuple<Fs...>>{
    std::tuple<Fs...> futures_;
    bool valid_;
  public:
    BlackChannel_when_all_future(Fs&&... futures)
      : futures_(std::forward<Fs>(futures)...)
      , valid_(true)
    {}

    bool ready(){
      bool ready = true;
      Hybrid::forEach(futures_,
                      [&](auto& f){
                        if(f.getBlackChannel().ready()){
                          f.comm_.throw_exception();
                        }
                        if(!f.ready())
                          ready = false;
                      });
      return ready;
    }

    bool valid(){
      return valid_;
    }

    void wait(){
      Hybrid::forEach(futures_,
                      [&](auto& f){
                        f.wait();
                      });
    }

    std::tuple<Fs...> get(){
      wait();
      valid_ = false;
      return std::move(futures_);
    }
  };

  template<class>
  struct is_BlackChannelFuture : std::false_type {};
  template<class T>
  struct is_BlackChannelFuture<BlackChannelFuture<T>> : std::true_type {};

  template<class InputIt>
  std::enable_if_t<is_BlackChannelFuture<typename std::iterator_traits<InputIt>::value_type>::value,
                 BlackChannel_when_all_future<typename std::vector<typename std::iterator_traits<InputIt>::value_type>>>
  when_all(InputIt first, InputIt last){
    typedef typename std::vector<typename std::iterator_traits<InputIt>::value_type> FutureType;
    return BlackChannel_when_all_future<FutureType>(first, last);
  }

  template<class... Ts>
  BlackChannel_when_all_future<typename std::tuple<BlackChannelFuture<Ts>...>>
  when_all(BlackChannelFuture<Ts>&&... futures){
    return BlackChannel_when_all_future<typename std::tuple<BlackChannelFuture<Ts>...>>(std::move(futures)...);
  }

  template<class... Ts>
  BlackChannel_when_all_future<typename std::tuple<BlackChannelFuture<Ts>&...>>
  when_all(BlackChannelFuture<Ts>&... futures){
    return BlackChannel_when_all_future<typename std::tuple<BlackChannelFuture<Ts>&...>>(futures...);
  }


  template<class Sequence>
  class BlackChannel_when_any_future;

  template<class T>
  class BlackChannel_when_any_future<std::vector<BlackChannelFuture<T>>>{
    MPI_when_any_result<std::vector<BlackChannelFuture<T>>> result_;
    std::vector<MPI_Request> req_;
    MPI_Status status_;
    bool valid_;
    bool ready_;
  public:
    template<class InputIt>
    BlackChannel_when_any_future(InputIt first, InputIt last)
      : result_({{std::move_iterator<InputIt>(first), std::move_iterator<InputIt>(last)}, SIZE_MAX})
      , valid_(true)
    {
    }

    bool ready(){
      if(result_.index != SIZE_MAX)
        return true;
      for(size_t i = 0; i < result_.futures.size(); i++){
        if(result_.futures[i].ready()){
          result_.index = i;
          return true;
        }
      }
      return false;
    }

    bool valid(){
      return valid_;
    }

    void wait(){
      if(result_.index != SIZE_MAX)
        return;
      std::vector<MPIFutureBase> vec;
      for(auto& f : result_.futures){
        vec.push_back(std::move(static_cast<MPIFutureBase&>(f)));
      }
      std::vector<size_t> indices;
      for(size_t i = 0; i < result_.futures.size(); i++){
        auto& f = result_.futures[i];
        if(f.getBlackChannel().mpirequest() != MPI_REQUEST_NULL){
          vec.push_back(std::move(static_cast<MPIFutureBase&>(f.getBlackChannel())));
          indices.push_back(i);
        }
      }
      auto any_result = when_any(vec.begin(), vec.end()).get();
      result_.index = any_result.index;
      for(size_t i = 0; i < result_.futures.size(); i++){
        static_cast<MPIFutureBase&>(result_.futures[i]) = std::move(any_result.futures[i]);
      }
      for(size_t i = 0; i < indices.size(); i++){
        static_cast<MPIFutureBase&>(result_.futures[indices[i]].getBlackChannel())
          = std::move(any_result.futures[i + result_.futures.size()]);
      }
      if(any_result.index > result_.futures.size()){
        result_.futures[indices[any_result.index - result_.futures.size()]].comm_.throw_exception();
      }
    }

    MPI_when_any_result<std::vector<BlackChannelFuture<T>>> get(){
      wait();
      valid_ = false;
      return std::move(result_);
    }
  };

  template<class... Fs>
  class BlackChannel_when_any_future<std::tuple<Fs...>>{
    std::vector<MPI_Request> req_;
    MPI_when_any_result<std::tuple<Fs...>> result_;
    MPI_Status status_;
    bool valid_;
    bool ready_;
  public:
    BlackChannel_when_any_future(Fs&&... futures)
      : result_{std::tuple<Fs...>(std::forward<Fs>(futures)...), SIZE_MAX}
      , valid_(true)
    {
    }

    bool ready(){
      bool ready = false;
      Hybrid::forEach(result_.futures,
                      [&](auto& f){
                        if(f.ready())
                          ready = true;
                      });
      return ready;
    }

    bool valid(){
      return valid_;
    }

    void wait(){
      if(result_.index != SIZE_MAX)
        return;
      std::vector<MPIFutureBase> vec;
      Hybrid::forEach(result_.futures,
                      [&](auto& f){
                        vec.push_back(std::move(static_cast<MPIFutureBase&>(f)));
                      });
      std::vector<size_t> indices;
      Hybrid::forEach(Std::make_index_sequence<sizeof...(Fs)>{},
                      [&](auto i){
                        auto& f = std::get<i>(result_.futures);
                        if(f.getBlackChannel().mpirequest() != MPI_REQUEST_NULL){
                          vec.push_back(std::move(static_cast<MPIFutureBase&>(f.getBlackChannel())));
                          indices.push_back((size_t)i);
                        }
                      });
      auto any_result = when_any(vec.begin(), vec.end()).get();
      Hybrid::forEach(Std::make_index_sequence<sizeof...(Fs)>{},
                      [&](auto i){
                        static_cast<MPIFutureBase&>(std::get<i>(result_.futures)) = std::move(any_result.futures[i]);
                      });
      int c = 0;
      Hybrid::forEach(Std::make_index_sequence<sizeof...(Fs)>{},
                      [&](auto i){
                        if(i == indices[c]){
                          static_cast<MPIFutureBase&>(std::get<i>(result_.futures).getBlackChannel())
                            = std::move(any_result.futures[c + sizeof...(Fs)]);
                          c++;
                        }
                      });
      if(any_result.index > sizeof...(Fs)){
        Hybrid::forEach(Std::make_index_sequence<sizeof...(Fs)>{},
                        [&](auto i){
                          if(i == any_result.index -sizeof...(Fs)){
                            std::get<i>(result_.futures).comm_.throw_exception();
                          }
                        });
      }
    }

    MPI_when_any_result<std::tuple<Fs...>> get(){
      wait();
      valid_ = false;
      return std::move(result_);
    }
  };

  template<class InputIt>
  std::enable_if_t<is_BlackChannelFuture<typename std::iterator_traits<InputIt>::value_type>::value,
                 BlackChannel_when_any_future<typename std::vector<typename std::iterator_traits<InputIt>::value_type>>>
  when_any(InputIt first, InputIt last){
    typedef typename std::vector<typename std::iterator_traits<InputIt>::value_type> FutureType;
    return BlackChannel_when_any_future<FutureType>(first, last);
  }

  template<class... Ts>
  BlackChannel_when_any_future<typename std::tuple<BlackChannelFuture<Ts>...>>
  when_any(BlackChannelFuture<Ts>&&... futures){
    return BlackChannel_when_any_future<typename std::tuple<BlackChannelFuture<Ts>...>>(std::move(futures)...);
  }

  template<class... Ts>
  BlackChannel_when_any_future<typename std::tuple<BlackChannelFuture<Ts>&...>>
  when_any(BlackChannelFuture<Ts>&... futures){
    return BlackChannel_when_any_future<typename std::tuple<BlackChannelFuture<Ts>&...>>(futures...);
  }

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
  MPIStatus PointToPointCommunication<BlackChannelCommunicator>::recv(T& data, int rank, int tag, bool resize)
  {
    auto f = irecv(std::move(data), rank, tag, resize);
    data = f.get();
    return f.status();
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
