// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_PARALLEL_FUTURE_HH
#define DUNE_COMMON_PARALLEL_FUTURE_HH

#include <memory>
#include <dune/common/exceptions.hh>

namespace Dune{

  /*! \brief This exception is thrown when `ready()`, `wait()` or `get()` is
    called on an invalid future. A future is valid until `get()` is called and
    if it is not default-constructed and it was not moved from.
  */
  class InvalidFutureException : public InvalidStateException
  {};

  // forward declaration
  template<class T>
  class PseudoFuture;

  /*! \brief Type-erasure for future-like objects. A future-like object is a
    object satisfying the interface of FutureBase.
  */
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
        : _future(std::forward<F>(f))
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
        return (T)_future.get();
      }
    };

    std::unique_ptr<FutureBase> _future;
  public:
    template<class F>
    Future(F&& f)
      : _future(std::make_unique<FutureModel<F>>(std::forward<F>(f)))
    {}

    template<class U, std::enable_if_t<std::is_same<U,T>::value && !std::is_same<T,void>::value>>
    Future(U&& data)
      : _future(std::make_unique<FutureModel<PseudoFuture<T>>>(PseudoFuture<T>(std::forward<U>(data))))
    {}

    Future() = default;

    /*! \brief wait until the future is ready
      \throws InvalidFutureException
     */
    void wait(){
      _future->wait();
    }

    /*! \brief Waits until the future is ready and returns the resulting value
      \returns The contained value
      \throws InvalidFutureException
     */
    T get() {
      return _future->get();
    }

    /*! \brief
      \returns true is the future is ready, otherwise false
      \throws InvalidFutureException
    */
    bool ready() const {
      return _future->ready();
    }

    /*! \brief Checks whether the future is valid. I.e. `get()' was not called
      on that future and when it was not default-constructed and not moved
      from.
      \returns true is the future is valid, otherwise false
    */
    bool valid() const {
      if(_future)
        return _future->valid();
      return false;
    }
  };

  /*! \brief A wrapper-class for a object which is ready immediately.
   */
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
