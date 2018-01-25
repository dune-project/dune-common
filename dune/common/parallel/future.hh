// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

/**
 * @file   future.hh
 * @author Nils-Arne Dreier <n.deier@uni-muenster.de>
 *
 * @brief Type-erasure class for objects satisfying the
 * Future-concept.
 *
 * @ingroup ParallelCommunication
 */


#ifndef DUNE_COMMON_PARALLEL_FUTURE_HH
#define DUNE_COMMON_PARALLEL_FUTURE_HH

#include <memory>
#include <type_traits>
#include <dune/common/concept.hh>
#include "mpiexceptions.hh"
#include "span.hh"

namespace Dune {
#ifndef DOXYGEN
  namespace impl {
    template<class T>
    class FutureBase
    {
    public:
      virtual bool valid() = 0;
      virtual void wait() = 0;
      virtual T get() = 0;
      virtual int source() const = 0;
      virtual int count() const = 0;
    };

    template<class F, class T, typename = void>
    class FutureHolder : public FutureBase<T>
    {
    protected:
      F obj_;

    public:
      FutureHolder(F&& f) :
        obj_(std::forward<F>(f)) {}

      bool valid() override
      {
        return obj_.valid();
      }

      void wait() override{
        obj_.wait();
      }

      T get() override{
        return obj_.get();
      }

      int source() const override {
        return -1;
      }

      int count() const override {
        return 0;
      }
    };

    struct HasStatus
    {
      template<class T>
      auto require(const T& t) -> decltype(
                                           t.status()
                                           );
    };

    template<class F, class T>
    class FutureHolder<F, T, std::enable_if_t<models<HasStatus, F>()>>
      : public FutureHolder<F, T, bool>
    {
      using FutureHolder<F, T, bool>::FutureHolder;
      int source() const override {
        return this->obj_.status().get_source();
      }

      int count() const override {
        return this->obj_.status().get_count(Span<T>::mpi_type());
      }
    };
  }
#endif

  // type-erasure class for generic Future-types
  template<class T = void>
  class Future
  {
  protected:
    std::unique_ptr<impl::FutureBase<T>> ptr_;

  public:
    template<class F>
    Future(F&& f) :
      ptr_(std::make_unique<impl::FutureHolder<F, T>>(std::forward<F>(f)))
    {
    }

    Future() = default;

    /** @brief Checks whether the 'Future<T>' is valid. I.e. the
     *  communication is complete.
     */
    bool valid()
    {
      return ptr_->valid();
    }

    /** @brief Waits for the completion of the communication.
     */
    void wait()
    {
      ptr_->wait();
    }

    /** @brief Returns the received data.
     */
    T get() {
      return ptr_->get();
    }
  };

  // extention of Future<T> for receiving communications.
  template<class T = void>
  class RecvFuture : public Future<T> {
  public:
    using Future<T>::Future;

    /** @brief Returns the source of the communication.
     */
    int source() const {
      return this->ptr_->source();
    }

    /** @brief Returns the size of the data that was actually
        received (might be unequal to the size of the receive buffer).
     */
    int count() const {
      return this->ptr_->count();
    }
  };

  // @brief Fall-back implementation for non-blocking communication if
  // MPI is not available.
  template<class T = void>
  class PseudoFuture {
    std::unique_ptr<T> p_buffer_;
    bool is_valid_;

    T& buffer(){
      return *p_buffer_;
    }
  public:
    PseudoFuture(T&& d) :
      p_buffer_(std::make_unique<std::decay_t<T>>(std::move(d))),
      is_valid_(true)
    {}
    PseudoFuture(PseudoFuture&&) = default;
    PseudoFuture& operator= (PseudoFuture&& ) = default;

    bool valid() const
    { return is_valid_;}
    void wait() const {}

    T get(){
      wait();
      is_valid_ = false;
      return std::move(*p_buffer_);
    }
  };

  template<>
  class PseudoFuture<void> {
    bool is_valid_;
  public:
    PseudoFuture() :
      is_valid_(true)
    {}
    PseudoFuture(PseudoFuture&&) = default;
    PseudoFuture& operator= (PseudoFuture&& ) = default;

    bool valid() const
    { return is_valid_;}

    void wait() const {}

    void get(){
      wait();
      is_valid_ = false;
    }
  };

  template<class T = void>
  class RecvPseudoFuture : public PseudoFuture<T> {
  public:
    using PseudoFuture<T>::PseudoFuture;

    int source() const {
      return 0;
    }

    int count() const {
      return -1;
    }
  };

      // The following wait... methods are implemented by a busy wait,
    // because not all Futures are providing a MPI_Requests,
    // s.t. MPI_Wait... can be used (MPIProbeFuture).

    /**@brief Waits for some futures to finish. If multiple requests
     * are ready for competition. This function will complete all
     * ready requests and returns their indices.
     *
     * @throw MPIError
     */
    template<class... Ts>
    static std::vector<int> waitsome(Future<Ts>&... futures) {
      std::array<bool, sizeof...(futures)> is_valid;
      do{
        is_valid = {{futures.valid()...}};
      }while(std::all_of(is_valid.begin(), is_valid.end(), [](bool b){return !b;}));
      std::vector<int> indices;
      for(size_t i = 0; i < is_valid.size(); i++){
        if(is_valid[i])
          indices.push_back(i);
      }
      return indices;
    }

    /** @brief Waits for any future to finish and return its index.
     *
     * @throw MPIError
     */
    template<class... Ts>
    static int waitany(Future<Ts>&... futures) {
      std::array<bool, sizeof...(futures)> is_valid;
      do{
        is_valid = {futures.valid()...};
      }while(std::any_of(is_valid.begin(), is_valid.end(), [](bool b){return b;}));
      return std::find(is_valid.begin(), is_valid.end(), true) - is_valid.begin();
    }

    /** @brief Waits for all futures to finish.
     *
     * @throw MPIError
     */
    template<class... Ts>
    static void waitall(Future<Ts>&... futures) {
      int x[] = {(futures.wait(), 0)...};
      (void)x;
    }

    /// @copydoc waitsome
    template<class C>
    static std::vector<int> waitsome(C& futures)
    {
      std::vector<int> ind(0);
      do{
        for(size_t i = 0; i < futures.size(); i++){
          if(futures[i].valid())
            ind.push_back(i);
        }
      }while(ind.size()==0);
      return ind;
    }

    /// @copydoc waitany
    template<class C>
    static int waitany(C& futures)
    {
      for(size_t i = 0; true; i = (i+1)%futures.size()){
        if(futures[i].valid())
          return i;
      }
    }

    /// @copydoc waitall
    template<class C>
    static void waitall(C& futures)
    {
      bool not_all_ready;
      do{
        not_all_ready = false;
        for(auto f : futures){
          if(!f.valid())
            not_all_ready = true;
        }
      }while(not_all_ready);
    }
}

#endif  // DUNE_PARALLEL_FUTURE
