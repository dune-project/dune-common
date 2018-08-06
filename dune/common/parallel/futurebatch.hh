// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_PARALLEL_FUTUREBATCH_HH
#define DUNE_COMMON_PARALLEL_FUTUREBATCH_HH

#include <memory>
#include "future.hh"

namespace Dune {

  class BatchNotStarted : public ParallelError {};
  template<class V> class BatchedFuture;

  namespace impl {
    template<class V>
    class BatchData{
      template<class W>
      friend class BatchedFuture;
      Future<V> _future;
      // could be optimized when V is reference type
      V _vector;
    public:
      BatchData(Future<V>&& f = {})
        : _future(std::move(f))
      {}

      bool started(){
        return _future.valid() || _vector.size() > 0;
      }
      void wait(){
        if(_future.valid())
          _vector = _future.get();
      }

      bool ready(){
        return _future.ready();
      }
    };
  }

  template<class V>
  class BatchedFuture : public FutureBase<typename std::decay_t<V>::value_type> {
    typedef typename std::decay_t<V>::value_type value_type;
    size_t index_;
    std::shared_ptr<impl::BatchData<V>> data_ptr_;
    std::function<value_type (value_type)> post_clb_;
    bool valid_;
  public:
    BatchedFuture() = default;

    BatchedFuture(size_t index,
                  std::shared_ptr<impl::BatchData<V>> data,
                  std::function<value_type(value_type)> post_callback = {} /* empty function */)
      : index_(index)
      , data_ptr_(data)
      , post_clb_(post_callback)
      , valid_(true)
    { }

    bool valid() const {
      return valid_;
    }

    bool ready() {
      if(data_ptr_ && !data_ptr_->started())
        DUNE_THROW(BatchNotStarted, "The FutureBatch is not started yet! Call start() before waiting for a BatchedFuture.");
      return data_ptr_ && data_ptr_->ready();
    }

    void wait() {
      if(data_ptr_ && !data_ptr_->started())
        DUNE_THROW(BatchNotStarted, "The FutureBatch is not started yet! Call start() before waiting for a BatchedFuture.");
      data_ptr_->wait();
    }

    value_type get() {
      wait();
      value_type t = data_ptr_->_vector[index_];
      data_ptr_.reset();
      if(post_clb_)
        return post_clb_(std::move(t));
      else
        return std::move(t);
    }
  };

  template<class V>
  class FutureBatch {
    size_t counter_;
    std::shared_ptr<impl::BatchData<V>> data_ptr_;
  public:
    typedef typename std::decay_t<V>::value_type value_type;
    FutureBatch()
      : counter_(0)
      , data_ptr_(std::make_shared<impl::BatchData<V>>())
    {}

    BatchedFuture<V> batch(std::function<value_type(value_type)> postprocessing = {}){
      return BatchedFuture<V>(counter_++, data_ptr_, postprocessing);
    }

    template<class F>
    void start(F&& f){
      *data_ptr_ = impl::BatchData<V>(std::move(f));
      data_ptr_ = std::make_shared<impl::BatchData<V>>();
      counter_ = 0;
    }

    size_t size() const{
      return counter_;
    }
  };

}

#endif
