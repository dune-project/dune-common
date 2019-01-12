// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_PARALLEL_FUTURE_HH
#define DUNE_COMMON_PARALLEL_FUTURE_HH

#include <memory>

namespace Dune{

  class InvalidFutureException : public InvalidStateException
  {};

  // Type-erasure class for Future
  template<class T>
  class Future{
    // Future interface:
    class FutureBase{
    public:
      virtual ~FutureBase() = default;
      virtual void wait() = 0;
      virtual bool ready() const = 0;
      virtual bool valid() const = 0;
      virtual T get() = 0;
    };

    // model class
    template<class F>
    class FutureModel
      : public FutureBase
    {
      F _future;
    public:
      FutureModel(F&& f)
        : _future(std::move(f))
      {}

      virtual void wait() override
      {
        _future.wait();
      }

      virtual bool ready() const override
      {
        return _future.ready();
      }

      virtual bool valid() const override
      {
        return _future.valid();
      }

      virtual T get() override{
        return _future.get();
      }
    };

    std::unique_ptr<FutureBase> _future;
  public:
    template<class F>
    Future(F&& f)
      : _future(new FutureModel<F>(std::forward<F>(f)))
    {}

    Future() = default;

    void wait(){
      _future->wait();
    }
    T get() {
      return _future->get();
    }
    bool ready() const {
      return _future->ready();
    }
    bool valid() const {
      if(_future)
        return _future->valid();
      return false;
    }
  };

  template<class T>
  class PseudoFuture{
    bool valid_;
    T data_;
  public:
    PseudoFuture() :
      valid_(false)
    {}

    template<class U>
    PseudoFuture(U&& u) :
      valid_(true),
      data_(std::forward<U>(u))
    {}

    void wait() {
      if(!valid_)
        DUNE_THROW(InvalidFutureException, "The PseudoFuture is not valid");
    }

    bool ready() const {
      if(!valid_)
        DUNE_THROW(InvalidFutureException, "The PseudoFuture is not valid");
      return true;
    }

    T get() {
      if(!valid_)
        DUNE_THROW(InvalidFutureException, "The PseudoFuture is not valid");
      valid_ = false;
      return std::forward<T>(data_);
    }

    bool valid() const {
      return valid_;
    }
  };

  template<>
  class PseudoFuture<void>{
    bool valid_;
  public:
    PseudoFuture(bool valid = false) :
      valid_(valid)
    {}

    void wait(){
      if(!valid_)
        DUNE_THROW(InvalidFutureException, "The PseudoFuture is not valid");
    }
    bool ready() const{
      if(!valid_)
        DUNE_THROW(InvalidFutureException, "The PseudoFuture is not valid");
      return true;
    }

    void get(){
      if(!valid_)
        DUNE_THROW(InvalidFutureException, "The PseudoFuture is not valid");
      valid_ = false;
    }

    bool valid() const{
      return valid_;
    }
  };
}

#endif
