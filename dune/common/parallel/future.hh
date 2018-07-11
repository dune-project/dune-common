// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_PARALLEL_FUTURE_HH
#define DUNE_COMMON_PARALLEL_FUTURE_HH

namespace Dune{

  template<class T>
  class Future{
  public:
    virtual ~Future(){};
    virtual void wait() = 0;
    virtual T get()  = 0;
    virtual bool ready() = 0;
    virtual bool valid() const = 0;
  };

  class InvalidFutureException : public InvalidStateException
  {};

  template<class T>
  class PseudoFuture : Future<T>{
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
  class PseudoFuture<void> : Future<void>{
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
