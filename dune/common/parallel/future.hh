// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_PARALLEL_FUTURE_HH
#define DUNE_COMMON_PARALLEL_FUTURE_HH

#include <memory>

namespace Dune{

  template<class T>
  class FutureBase{
  public:
    virtual ~FutureBase(){};
    virtual void wait() = 0;
    virtual T get()  = 0;
    virtual bool ready() = 0;
    virtual bool valid() const = 0;
  };

  class InvalidFutureException : public InvalidStateException
  {};

  // Type-erasure class for Future
  template<class T>
  class Future : public FutureBase<T>{
    std::unique_ptr<FutureBase<T>> _future;
  public:
    template<class F>
    Future(F&& f)
      : _future(new F(std::forward<F>(f)))
    {}

    Future(){}
    virtual void wait() override{
      _future->wait();
    }
    virtual T get() override {
      return _future->get();
    }
    virtual bool ready() override {
      return _future->ready();
    }
    virtual bool valid() const override {
      if(_future)
        return _future->valid();
      return false;
    }
  };

  template<class T>
  class PseudoFuture : public FutureBase<T>{
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

    void wait() override{
      if(!valid_)
        DUNE_THROW(InvalidFutureException, "The PseudoFuture is not valid");
    }

    bool ready() override{
      if(!valid_)
        DUNE_THROW(InvalidFutureException, "The PseudoFuture is not valid");
      return true;
    }

    T get() override{
      if(!valid_)
        DUNE_THROW(InvalidFutureException, "The PseudoFuture is not valid");
      valid_ = false;
      return std::forward<T>(data_);
    }

    bool valid() const override{
      return valid_;
    }
  };

  template<>
  class PseudoFuture<void> : FutureBase<void>{
    bool valid_;
  public:
    PseudoFuture(bool valid = false) :
      valid_(valid)
    {}

    void wait() override{
      if(!valid_)
        DUNE_THROW(InvalidFutureException, "The PseudoFuture is not valid");
    }
    bool ready() override{
      if(!valid_)
        DUNE_THROW(InvalidFutureException, "The PseudoFuture is not valid");
      return true;
    }

    void get() override{
      if(!valid_)
        DUNE_THROW(InvalidFutureException, "The PseudoFuture is not valid");
      valid_ = false;
    }

    bool valid() const override{
      return valid_;
    }
  };
}

#endif
