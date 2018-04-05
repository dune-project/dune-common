// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

/**
 * @file   pseudofuture.hh
 * @author Nils-Arne Dreier <n.dreier@uni-muenster.de>
 *
 * @brief PseudoFuture for wrapping a value into a Future.
 *
 * @ingroup ParallelCommunication
 */

#ifndef DUNE_COMMON_PARALLEL_PSEUDOFUTURE_HH
#define DUNE_COMMON_PARALLEL_PSEUDOFUTURE_HH

#include <memory>

namespace Dune {
  // @brief Wraps a value into a Future;
  template<class T = void>
  class PseudoFuture {
    std::unique_ptr<T> p_buffer_;
    bool is_valid_;
  public:
    PseudoFuture() :
      p_buffer_(nullptr),
      is_valid_(false)
    {}

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
  public:
    PseudoFuture() = default;

    PseudoFuture(PseudoFuture&&) = default;

    PseudoFuture& operator= (PseudoFuture&& ) = default;

    bool valid() const
    { return false; }

    void wait() const {}

    void get(){}
  };
}
#endif  // DUNE_PARALLEL_FUTURE
