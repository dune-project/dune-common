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
    friend class MPI_when_all_future;
    template<class>
    friend class MPI_when_any_future;
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
          dune_mpi_call(MPI_Finalized, &wasFinalized);
          if (!wasFinalized){
            dverb << "free request " << req_ << " (null = " << MPI_REQUEST_NULL << ")" << std::endl;
            dune_mpi_call(MPI_Cancel, &req_);
            dune_mpi_call(MPI_Request_free, &req_);
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

    MPI_Request& mpirequest() {
      return req_;
    }

    const MPI_Request& mpirequest() const{
      return req_;
    }

    virtual bool ready() {
      if(req_ == MPI_REQUEST_NULL)
        return true;
      int flag = -1;
      dune_mpi_call(MPI_Test, &req_, &flag, status_);
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
      dune_mpi_call(MPI_Wait, &req_, status_);
    }
  };

  class FutureNotValidException : public InvalidStateException {};

  template<class U>
  struct unique_ptr_or_nothing : public std::unique_ptr<U>
  {
    using std::unique_ptr<U>::unique_ptr;
    unique_ptr_or_nothing() = default;
    unique_ptr_or_nothing(std::unique_ptr<U>&& u) :
      std::unique_ptr<U>(std::move(u))
    {}
  };

  template<>
  struct unique_ptr_or_nothing<void> {
    friend MPIFuture<void>;
    void release() const {}
    explicit operator bool () const {
      return is_valid_;
    }

    unique_ptr_or_nothing(bool valid = false)
      : is_valid_(valid)
    {}

    // disallow copy
    unique_ptr_or_nothing(const unique_ptr_or_nothing&) = delete;
    unique_ptr_or_nothing& operator= (const unique_ptr_or_nothing&) = delete;
    // Move
    unique_ptr_or_nothing(unique_ptr_or_nothing&& o)
    {
      is_valid_ = o.is_valid_;
      o.is_valid_ = false;
    }
    unique_ptr_or_nothing& operator=(unique_ptr_or_nothing&& o){
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
    friend class MPI_when_all_future;
    template<class>
    friend class MPI_when_any_future;

    unique_ptr_or_nothing<T> p_buffer_; // use a unique_ptr to ensure that the
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
      if (mpirequest() != MPI_REQUEST_NULL && !cancelable_)
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
  class MPI_when_all_future {};

  template<class F>
  class MPI_when_all_future<std::vector<F>>
  {
    std::vector<F> futures_;
    std::vector<MPI_Request> req_;
    std::vector<MPI_Status> status_;
    bool valid_;
  public:
    template<class InputIt>
    MPI_when_all_future(InputIt first, InputIt last)
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
      dune_mpi_call(MPI_Testall, req_.size(), req_.data(), &flag, status_.data());
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
      dune_mpi_call(MPI_Waitall, req_.size(), req_.data(), status_.data());
      for(size_t i = 0; i < futures_.size(); i++){
        if(futures_[i].req_ != MPI_REQUEST_NULL){
          futures_[i].req_ = MPI_REQUEST_NULL;
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
  class MPI_when_all_future<std::tuple<Fs...>>{
    std::array<MPI_Request, sizeof...(Fs)> req_;
    std::array<MPI_Status, sizeof...(Fs)> status_;
    std::tuple<Fs...> futures_;
    bool valid_;

    void refresh_requests(){
      Hybrid::forEach(Std::make_index_sequence<sizeof...(Fs)>{},
                      [&](auto i){
                        req_[i] = std::get<i>(futures_).req_;
                      });
    }
  public:
    MPI_when_all_future(Fs&&... futures)
      : futures_(std::forward<Fs>(futures)...)
      , valid_(true)
    {}

    bool ready(){
      int flag = 0;
      refresh_requests();
      dune_mpi_call(MPI_Testall, sizeof...(Fs), req_.data(), &flag, status_.data());
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
      refresh_requests();
      dune_mpi_call(MPI_Waitall, sizeof...(Fs), req_.data(), status_.data());
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
  class MPI_when_any_future {};

  template<class Sequence>
  struct MPI_when_any_result{
    Sequence futures;
    size_t index;
  };

  template<class F>
  class MPI_when_any_future<std::vector<F>>
  {
    MPI_when_any_result<std::vector<F>> result_;
    std::vector<MPI_Request> req_;
    std::vector<int> indices_;
    std::vector<MPI_Status> status_;
    bool valid_;
  public:
    template<class InputIt>
    MPI_when_any_future(InputIt first, InputIt last)
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
      dune_mpi_call(MPI_Testsome, req_.size(), req_.data(), &outcount, indices_.data(), status_.data());
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
      dune_mpi_call(MPI_Waitsome, req_.size(), req_.data(), &outcount, indices_.data(), status_.data());
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

    MPI_when_any_result<std::vector<F>> get(){
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
  class MPI_when_any_future<std::tuple<Fs...>>{
    std::array<MPI_Request, sizeof...(Fs)> req_;
    std::array<MPI_Status, sizeof...(Fs)> status_;
    std::array<int, sizeof...(Fs)> indices_;
    MPI_when_any_result<std::tuple<Fs...>> result_;
    bool valid_;

    void refresh_requests(){
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
    MPI_when_any_future(Fs&&... futures)
      : result_{{std::forward<Fs>(futures)...}, SIZE_MAX}
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
      refresh_requests();
      dune_mpi_call(MPI_Testsome, sizeof...(Fs), req_.data(), &outcount, indices_.data(), status_.data());
      if(outcount != MPI_UNDEFINED && outcount > 0){
        if(result_.index == SIZE_MAX)
          result_.index = indices_[0];
        for(int i = 0; i < outcount; i++){
          apply(indices_[i], [&](auto& f){
              f.mpirequest() = MPI_REQUEST_NULL;
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
      refresh_requests();
      dune_mpi_call(MPI_Waitsome, sizeof...(Fs), req_.data(), &outcount, indices_.data(), status_.data());
      if(outcount != MPI_UNDEFINED){
        if(result_.index == SIZE_MAX)
          result_.index = indices_[0];
        for(int i = 0; i < outcount; i++){
          apply(indices_[i], [&](auto& f){
              f.mpirequest() = MPI_REQUEST_NULL;
              f.status() = status_[i];
            });
        }
      }
    }

    MPI_when_any_result<std::tuple<Fs...>> get(){
      if(!valid())
        DUNE_THROW(FutureNotValidException, "The requested Future is not valid.");
      wait();
      valid_ = false;
      return std::move(result_);
    }
  };

  template<class F>
  struct is_MPIFuture : std::false_type {};
  template<class T>
  struct is_MPIFuture<MPIFuture<T>> : std::true_type {};

  template<class InputIt>
  std::enable_if_t<std::is_same<MPIFutureBase, typename std::iterator_traits<InputIt>::value_type>::value
                   || is_MPIFuture<typename std::iterator_traits<InputIt>::value_type>::value,
                   MPI_when_any_future<typename std::vector<typename std::iterator_traits<InputIt>::value_type>>>
  when_any(InputIt first, InputIt last){
    typedef typename std::vector<typename std::iterator_traits<InputIt>::value_type> FutureType;
    return MPI_when_any_future<FutureType>(first, last);
  }

  template<class... Fs>
  std::enable_if_t<(... &&
                    (std::is_same<MPIFutureBase, std::decay_t<Fs>>::value
                     || is_MPIFuture<std::decay_t<Fs>>::value)),
                   MPI_when_any_future<typename std::tuple<Fs...>>>
  when_any(Fs&&... futures){
    return MPI_when_any_future<typename std::tuple<Fs...>>(std::forward<Fs>(futures)...);
  }

  template<class InputIt>
  std::enable_if_t<(std::is_same<MPIFutureBase, typename std::iterator_traits<InputIt>::value_type>::value
                    || is_MPIFuture<typename std::iterator_traits<InputIt>::value_type>::value),
                   MPI_when_all_future<typename std::vector<typename std::iterator_traits<InputIt>::value_type>>>
  when_all(InputIt first, InputIt last){
    typedef typename std::vector<typename std::iterator_traits<InputIt>::value_type> FutureType;
    return MPI_when_all_future<FutureType>(first, last);
  }

  template<class... Fs>
  std::enable_if_t<(... &&
                    (std::is_same<MPIFutureBase, std::decay_t<Fs>>::value
                     || is_MPIFuture<std::decay_t<Fs>>::value
                     )
                    ),
                   MPI_when_all_future<typename std::tuple<Fs...>>>
  when_all(Fs&&... futures){
    return MPI_when_all_future<typename std::tuple<Fs...>>(std::forward<Fs>(futures)...);
  }
#endif // HAVE_MPI

}  // namespace Dune
#endif
