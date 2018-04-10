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
#include <dune/common/hybridutilities.hh>
#include <dune/common/std/utility.hh>
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

  template<class>
  class MPI_when_all_future;

  template<class>
  class MPI_when_any_future;

  class MPIFutureBase {
    template<class>
    friend class MPIWhenAllFuture;
    template<class>
    friend class MPIWhenAnyFuture;
    friend class ManagedMPIComm;
    MPI_Request req_;

  protected:
    MPIStatus status_;
    bool cancelable_;

    MPIFutureBase()
      : req_(MPI_REQUEST_NULL)
      , status_()
      , cancelable_(false){
      dverb << "MPIFutureBase::MPIFutureBase()" << std::endl;
    }

  public:
    MPIFutureBase (const MPI_Request& r, bool cancelable)
      : req_(r)
      , cancelable_(cancelable)
    {}

    ~MPIFutureBase() noexcept(false){
      if (req_ != MPI_REQUEST_NULL){
        if(cancelable_){
          int wasFinalized = 0;
          duneMPICall(MPI_Finalized, &wasFinalized);
          if (!wasFinalized){
            dverb << "free request " << req_ << " (null = " << MPI_REQUEST_NULL << ")" << std::endl;
            duneMPICall(MPI_Cancel, &req_);
            duneMPICall(MPI_Request_free, &req_);
          }
        }
      }
    }


    MPIFutureBase (const MPIFutureBase&) = delete;
    MPIFutureBase& operator=(const MPIFutureBase&) = delete;

    MPIFutureBase (MPIFutureBase&& o)
      : req_(MPI_REQUEST_NULL)
      , cancelable_(false)
    {
      std::swap(req_, o.req_);
      std::swap(status_, o.status_);
      std::swap(cancelable_, o.cancelable_);
    }

    MPIFutureBase& operator=(MPIFutureBase&& o){
      std::swap(req_, o.req_);
      std::swap(status_, o.status_);
      std::swap(cancelable_, o.cancelable_);
      return *this;
    }

    MPI_Request& mpiRequest() {
      return req_;
    }

    const MPI_Request& mpiRequest() const{
      return req_;
    }

    virtual bool ready() {
      if(req_ == MPI_REQUEST_NULL)
        return true;
      int flag = -1;
      duneMPICall(MPI_Test, &req_, &flag, status_);
      return flag;
    }

    virtual const MPIStatus& status() const{
      return status_;
    }

    virtual MPIStatus& status() {
      return status_;
    }

    virtual void wait() {
      dverb << "MPIFutureBase::wait(), req_=" << req_ << std::endl;
      if (req_ == MPI_REQUEST_NULL)
        return;
      duneMPICall(MPI_Wait, &req_, status_);
    }
  };

  class FutureNotValidException : public InvalidStateException {};

  template<class U>
  struct unique_ptrOrNothing : public std::unique_ptr<U>
  {
    using std::unique_ptr<U>::unique_ptr;
    unique_ptrOrNothing() = default;
    unique_ptrOrNothing(std::unique_ptr<U>&& u) :
      std::unique_ptr<U>(std::move(u))
    {}
  };

  template<>
  struct unique_ptrOrNothing<void> {
    friend MPIFuture<void>;
    void release() const {}
    explicit operator bool () const {
      return is_valid_;
    }

    unique_ptrOrNothing(bool valid = false)
      : is_valid_(valid)
    {}

    // disallow copy
    unique_ptrOrNothing(const unique_ptrOrNothing&) = delete;
    unique_ptrOrNothing& operator= (const unique_ptrOrNothing&) = delete;
    // Move
    unique_ptrOrNothing(unique_ptrOrNothing&& o)
    {
      is_valid_ = o.is_valid_;
      o.is_valid_ = false;
    }
    unique_ptrOrNothing& operator=(unique_ptrOrNothing&& o){
      is_valid_ = o.is_valid_;
      o.is_valid_ = false;
      return *this;
    }

  protected:
    bool is_valid_ = true;
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
    template<class>
    friend class MPIWhenAllFuture;
    template<class>
    friend class MPIWhenAnyFuture;

    unique_ptrOrNothing<T> p_buffer_; // use a unique_ptr to ensure that the
    // address is preserved during moving
  protected:
    std::add_lvalue_reference_t<T> buffer(){
      return *p_buffer_;
    }

  public:
    typedef T value_type;

    MPIFuture(){
      dverb << "MPIFuture::MPIFuture()" << std::endl;
    }

    template<typename U = T>
    MPIFuture(const ManagedMPIComm&, bool isCancelable, U&& d) :
      MPIFutureBase(MPI_REQUEST_NULL, isCancelable),
      p_buffer_(std::make_unique<T>(std::forward<U>(d)))
    {
      dverb << "MPIFuture::MPIFuture(const ManagedMPIComm&, bool, T&&)" << std::endl;
    }

    MPIFuture(const ManagedMPIComm&, bool isCancelable)
      : MPIFutureBase(MPI_REQUEST_NULL, isCancelable)
    {
      dverb << "MPIFuture::MPIFuture(const ManagedMPIComm&, bool, T&&)" << std::endl;
    }

    MPIFuture(const MPIFuture&) = delete;
    MPIFuture& operator=(const MPIFuture&) = delete;

    MPIFuture(MPIFuture&& other) = default;
    MPIFuture& operator=(MPIFuture&& o) = default;

    ~MPIFuture() noexcept(false){
      if (mpiRequest() != MPI_REQUEST_NULL && !cancelable_)
        {
          dverb << "cant cancel request, releasing memory!" << std::endl;
          p_buffer_.release();
        }
    }

    bool valid() const{
      return (bool)p_buffer_;
    }

    T get(){
      if(!valid())
        DUNE_THROW(FutureNotValidException, "The requested future is not valid.");
      wait();
      // move buffer to a local unique_ptr s.t. p_buffer_ points to null when get() returns
      T result(std::move(*p_buffer_));
      p_buffer_.reset();
      return std::move(result);
    }
  };

  template<>
  void MPIFuture<void>::get(){
    wait();
    p_buffer_.is_valid_ = false;
  }

  template<>
  MPIFuture<void>::MPIFuture(const ManagedMPIComm&, bool isCancelable)
    : MPIFutureBase(MPI_REQUEST_NULL, isCancelable)
    , p_buffer_(true) // init as valid
  {
    dverb << "MPIFuture::MPIFuture(const ManagedMPIComm&, bool)" << std::endl;
  }

  template<class Sequence>
  class MPIWhenAllFuture {};

  template<class F>
  class MPIWhenAllFuture<std::vector<F>>
  {
    std::vector<F> futures_;
    std::vector<MPI_Request> req_;
    std::vector<MPI_Status> status_;
    bool valid_;
  public:
    template<class InputIt>
    MPIWhenAllFuture(InputIt first, InputIt last)
      : futures_(std::move_iterator<InputIt>(first), std::move_iterator<InputIt>(last))
      , req_(futures_.size())
      , status_(futures_.size())
      , valid_(true)
    {
      std::transform(futures_.begin(), futures_.end(), req_.begin(),
                     [](const auto& f){
                       return f.req_;
                     });
    }

    bool ready(){
      int flag = 0;
      duneMPICall(MPI_Testall, req_.size(), req_.data(), &flag, status_.data());
      if(flag){
        for(int i = 0; i < futures_.size(); i++){
          if(futures_[i].req_ != MPI_REQUEST_NULL){
            futures_[i].req_ = MPI_REQUEST_NULL;
            futures_[i].status_ = status_[i];
          }
        }
      }
      return flag;
    }

    bool valid(){
      return valid_;
    }

    void wait(){
      duneMPICall(MPI_Waitall, req_.size(), req_.data(), status_.data());
      for(size_t i = 0; i < futures_.size(); i++){
        if(futures_[i].mpiRequest() != MPI_REQUEST_NULL){
          futures_[i].mpiRequest() = MPI_REQUEST_NULL;
          futures_[i].status_ = status_[i];
        }
      }
    }

    std::vector<F> get(){
      if(!valid_)
        DUNE_THROW(FutureNotValidException, "The requested future is not valid.");
      wait();
      valid_ = false;
      req_.clear();
      status_.clear();
      return std::move(futures_);
    }
  };

  template<class... Fs>
  class MPIWhenAllFuture<std::tuple<Fs...>>{
    std::array<MPI_Request, sizeof...(Fs)> req_;
    std::array<MPI_Status, sizeof...(Fs)> status_;
    std::tuple<Fs...> futures_;
    bool valid_;

    void refreshRequests(){
      Hybrid::forEach(Std::make_index_sequence<sizeof...(Fs)>{},
                      [&](auto i){
                        req_[i] = std::get<i>(futures_).req_;
                      });
    }
  public:
    MPIWhenAllFuture(Fs&&... futures)
      : futures_(std::forward<Fs>(futures)...)
      , valid_(true)
    {}

    bool ready(){
      int flag = 0;
      refreshRequests();
      duneMPICall(MPI_Testall, sizeof...(Fs), req_.data(), &flag, status_.data());
      if(flag){
        Hybrid::forEach(Std::make_index_sequence<sizeof...(Fs)>{},
                        [&](auto i){
                          if(std::get<i>(futures_).req_ != MPI_REQUEST_NULL){
                            std::get<i>(futures_).req_ = MPI_REQUEST_NULL;
                            std::get<i>(futures_).status_ = status_[i];
                          }
                        });
      }
      return flag;
    }

    bool valid(){
      return valid_;
    }

    void wait(){
      refreshRequests();
      duneMPICall(MPI_Waitall, sizeof...(Fs), req_.data(), status_.data());
      Hybrid::forEach(Std::make_index_sequence<sizeof...(Fs)>{},
                      [&](auto i){
                        if(std::get<i>(futures_).req_ != MPI_REQUEST_NULL){
                          std::get<i>(futures_).req_ = MPI_REQUEST_NULL;
                          std::get<i>(futures_).status_ = status_[i];
                        }
                      });
    }

    std::tuple<Fs...> get(){
      if(!valid_)
        DUNE_THROW(FutureNotValidException, "The requested future is not valid.");
      wait();
      valid_ = false;
      return std::move(futures_);
    }
  };

  template<class Sequence>
  class MPIWhenAnyFuture {};

  template<class Sequence>
  struct MPIWhenAnyResult{
    Sequence futures;
    size_t index;
  };

  template<class F>
  class MPIWhenAnyFuture<std::vector<F>>
  {
    MPIWhenAnyResult<std::vector<F>> result_;
    std::vector<MPI_Request> req_;
    std::vector<int> indices_;
    std::vector<MPI_Status> status_;
    bool valid_;
  public:
    template<class InputIt>
    MPIWhenAnyFuture(InputIt first, InputIt last)
      : result_({{std::move_iterator<InputIt>(first), std::move_iterator<InputIt>(last)}, SIZE_MAX})
      , req_(result_.futures.size())
      , indices_(result_.futures.size())
      , status_(result_.futures.size())
      , valid_(true)
    {
      std::transform(result_.futures.begin(), result_.futures.end(), req_.begin(),
                     [](const auto& f){
                       return f.req_;
                     });
      for(size_t i = 0; i < result_.futures.size(); i++)
        if(result_.futures[i].ready()){
          result_.index = i;
          break;
        }
    }

    bool ready(){
      if(result_.index != SIZE_MAX)
        return true;
      int outcount = -1;
      duneMPICall(MPI_Testsome, req_.size(), req_.data(), &outcount, indices_.data(), status_.data());
      if(outcount != MPI_UNDEFINED && outcount > 0){
        if(result_.index == SIZE_MAX)
              result_.index = indices_[0];
        for(int i = 0; i < outcount; i++){
          if(result_.futures[indices_[i]].req_ != MPI_REQUEST_NULL){
            result_.futures[indices_[i]].req_ = MPI_REQUEST_NULL;
            result_.futures[indices_[i]].status_ = status_[indices_[i]];
          }
        }
      }
      return result_.index != SIZE_MAX;
    }

    bool valid(){
      return valid_;
    }

    void wait(){
      if(result_.index != SIZE_MAX)
        return;
      int outcount = -1;
      duneMPICall(MPI_Waitsome, req_.size(), req_.data(), &outcount, indices_.data(), status_.data());
      if(outcount != MPI_UNDEFINED){
        for(int i = 0; i < outcount; i++){
          if(result_.futures[indices_[i]].req_ != MPI_REQUEST_NULL){
            result_.futures[indices_[i]].req_ = MPI_REQUEST_NULL;
            result_.futures[indices_[i]].status_ = status_[i];
            if(result_.index == SIZE_MAX)
              result_.index = indices_[i];
          }
        }
      }
    }

    MPIWhenAnyResult<std::vector<F>> get(){
      if(!valid_)
        DUNE_THROW(FutureNotValidException, "The requested Future is not valid.");
      wait();
      valid_ = false;
      req_.clear();
      status_.clear();
      return std::move(result_);
    }
  };

  template<class... Fs>
  class MPIWhenAnyFuture<std::tuple<Fs...>>{
    std::array<MPI_Request, sizeof...(Fs)> req_;
    std::array<MPI_Status, sizeof...(Fs)> status_;
    std::array<int, sizeof...(Fs)> indices_;
    MPIWhenAnyResult<std::tuple<Fs...>> result_;
    bool valid_;

    void refreshRequests(){
      Hybrid::forEach(Std::make_index_sequence<sizeof...(Fs)>{},
                      [&](auto i){
                        req_[i] = std::get<i>(result_.futures).req_;
                      });
    }

    template<class F>
    void apply(size_t i, F f){
      Hybrid::forEach(Std::make_index_sequence<sizeof...(Fs)>{},
                      [&](auto j){
                        if(i == (int)j){
                          f(std::get<j>(result_.futures));
                        }
                      });
    }
  public:
    MPIWhenAnyFuture(Fs&&... futures)
      : result_{std::tuple<Fs...>(std::forward<Fs>(futures)...), SIZE_MAX}
      , valid_(true)
    {
      Hybrid::forEach(Std::make_index_sequence<sizeof...(Fs)>{},
                      [&](auto j){
                        if(std::get<j>(result_.futures).ready() && result_.index == SIZE_MAX)
                          result_.index = j;
                      });
    }

    bool ready(){
      if(result_.index != SIZE_MAX)
        return true;
      int outcount = 0;
      refreshRequests();
      duneMPICall(MPI_Testsome, sizeof...(Fs), req_.data(), &outcount, indices_.data(), status_.data());
      if(outcount != MPI_UNDEFINED && outcount > 0){
        if(result_.index == SIZE_MAX)
          result_.index = indices_[0];
        for(int i = 0; i < outcount; i++){
          apply(indices_[i], [&](auto& f){
              f.mpiRequest() = MPI_REQUEST_NULL;
              f.status() = status_[i];
            });
        }
      }
      return result_.index != SIZE_MAX;
    }

    bool valid(){
      return valid_;
    }

    void wait(){
      if(result_.index != SIZE_MAX)
        return;
      int outcount = 0;
      refreshRequests();
      duneMPICall(MPI_Waitsome, sizeof...(Fs), req_.data(), &outcount, indices_.data(), status_.data());
      if(outcount != MPI_UNDEFINED){
        if(result_.index == SIZE_MAX)
          result_.index = indices_[0];
        for(int i = 0; i < outcount; i++){
          apply(indices_[i], [&](auto& f){
              f.mpiRequest() = MPI_REQUEST_NULL;
              f.status() = status_[i];
            });
        }
      }
    }

    MPIWhenAnyResult<std::tuple<Fs...>> get(){
      if(!valid())
        DUNE_THROW(FutureNotValidException, "The requested Future is not valid.");
      wait();
      valid_ = false;
      return std::move(result_);
    }
  };

  template<class F>
  struct isMPIFuture : std::false_type {};
  template<class T>
  struct isMPIFuture<MPIFuture<T>> : std::true_type {};

  template<class InputIt>
  std::enable_if_t<std::is_same<MPIFutureBase, typename std::iterator_traits<InputIt>::value_type>::value
                   || isMPIFuture<typename std::iterator_traits<InputIt>::value_type>::value,
                   MPIWhenAnyFuture<typename std::vector<typename std::iterator_traits<InputIt>::value_type>>>
  when_any(InputIt first, InputIt last){
    typedef typename std::vector<typename std::iterator_traits<InputIt>::value_type> FutureType;
    return MPIWhenAnyFuture<FutureType>(first, last);
  }

  constexpr bool staticAnd(){
    return true;
  }

  template<class... B>
  constexpr bool staticAnd(bool b0, B... bools){
    return b0 && staticAnd(bools...);
  }

  template<class... Fs>
  std::enable_if_t<staticAnd((std::is_same<MPIFutureBase, std::decay_t<Fs>>::value
                     || isMPIFuture<std::decay_t<Fs>>::value)...),
                   MPIWhenAnyFuture<typename std::tuple<Fs...>>>
  when_any(Fs&&... futures){
    return MPIWhenAnyFuture<typename std::tuple<Fs...>>(std::forward<Fs>(futures)...);
  }

  template<class InputIt>
  std::enable_if_t<(std::is_same<MPIFutureBase, typename std::iterator_traits<InputIt>::value_type>::value
                    || isMPIFuture<typename std::iterator_traits<InputIt>::value_type>::value),
                   MPIWhenAllFuture<typename std::vector<typename std::iterator_traits<InputIt>::value_type>>>
  when_all(InputIt first, InputIt last){
    typedef typename std::vector<typename std::iterator_traits<InputIt>::value_type> FutureType;
    return MPIWhenAllFuture<FutureType>(first, last);
  }

  template<class... Fs>
  std::enable_if_t<staticAnd((std::is_same<MPIFutureBase, std::decay_t<Fs>>::value
                     || isMPIFuture<std::decay_t<Fs>>::value
                     )...),
                   MPIWhenAllFuture<typename std::tuple<Fs...>>>
  when_all(Fs&&... futures){
    return MPIWhenAllFuture<typename std::tuple<Fs...>>(std::forward<Fs>(futures)...);
  }
#endif // HAVE_MPI

}  // namespace Dune
#endif
