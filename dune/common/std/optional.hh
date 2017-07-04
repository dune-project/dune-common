// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_STD_OPTIONAL_HH
#define DUNE_COMMON_STD_OPTIONAL_HH

#include <utility>

#include <dune/common/typeutilities.hh>

namespace Dune {

namespace Std  {

  /**
   * \brief A wrapper that can either contain an object of T or be empty
   *
   * \ingroup Utility
   *
   * \tparam T Type of wrapped objects
   */
  template<class T>
  class Optional
  {
  public:

    //! Default constructor
    Optional() :
      p_(nullptr)
    {}

    //! Construct internal T from given argument
    template<class TT, disableCopyMove<Optional, TT> = 0>
    Optional(TT&& t) :
      p_(nullptr)
    {
      emplace(std::forward<TT>(t));
    }

    //! Move constructor
    Optional(Optional&& other)
    {
      if (other)
        p_ = new (buffer_) T(std::move(other.value()));
      else
        p_ = nullptr;
    }

    //! Copy constructor
    Optional(const Optional& other)
    {
      if (other)
        p_ = new (buffer_) T(other.value());
      else
        p_ = nullptr;
    }

    //! Destructor
    ~Optional()
    {
      if (operator bool())
        p_->~T();
    }

    /**
     * \brief Assignment
     *
     * If internal T exists, this does an assignement
     * from argument, otherwise a construction.
     */
    template<class TT, disableCopyMove<Optional, TT> = 0 >
    Optional& operator=(TT&& t)
    {
      if (operator bool())
        *p_ = std::forward<T>(t);
      else
        p_ = new (buffer_) T(std::forward<T>(t));
      return *this;
    }

    /**
     * \brief Copy assignment from optional
     */
    Optional& operator=(const Optional& other)
    {
      if (other)
        *this = other.value();
      else if (operator bool())
      {
        p_->~T();
        p_ = nullptr;
      }
      return *this;
    }

    /**
     * \brief Move assignment from optional
     */
    Optional& operator=(Optional&& other)
    {
      if (other)
        *this = std::move(other.value());
      else if (operator bool())
      {
        p_->~T();
        p_ = nullptr;
      }
      return *this;
    }

    //! Check if *this is not emtpy
    explicit operator bool() const
    {
      return p_;
    }

    //! Get reference to internal T
    const T& value() const
    {
      return *p_;
    }

    //! Get mutable reference to internal T
    T& value()
    {
      return *p_;
    }

    //! Construct internal T from given arguments
    template< class... Args >
    void emplace(Args&&... args)
    {
      if (operator bool())
        p_->~T();
      p_ = new (buffer_) T(std::forward<Args>(args)...);
    }

    //! Destruct internal T leaving *this in empty state
    void release()
    {
      if (operator bool())
      {
        p_->~T();
        p_ = nullptr;
      }
    }

  private:

    alignas(T) char buffer_[sizeof(T)];
    T* p_;
  };

} // namespace Std

} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_OPTIONAL_HH
