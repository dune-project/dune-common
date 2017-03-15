// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_TO_UNIQUE_PTR_HH
#define DUNE_TO_UNIQUE_PTR_HH

#include <memory>

#include <dune/common/deprecated.hh>

namespace Dune
{
  /// \brief An owning pointer wrapper that can be assigned to (smart) pointers. Can not be copyed.
  /// Transfers ownership by cast to any (smart) pointer type. Releases the stored pointer on transfer.
  /// NOTE: This is an intermediate solution to switch to std::unique_ptr in later releases smoothly.
  /**
   * Example of usage:
   * ```
   * to_unique_ptr<int> f() { return new int(1); }
   *
   * int* p1 = f(); // p1 gets ownership, must delete explicitly
   * delete p1;
   *
   * std::unique_ptr<int> p2 = f();
   * std::shared_ptr<int> p3 = f();
   *
   * auto p4 = f(); // to_unique_ptr has itself pointer semantic
   * std::cout << *p4 << '\n';
   *
   * std::unique_ptr<int> p5( f() ); // ERROR: ambiguous constructor call
   * ```
   **/
  template <class T, class D = std::default_delete<T>>
  class to_unique_ptr
      : public std::unique_ptr<T,D>
  {
    using Super = std::unique_ptr<T,D>;

  public: // Member types:
    //@{

    using pointer = typename Super::pointer;
    using element_type = typename Super::element_type;
    using deleter_type = typename Super::deleter_type;

    //@}


  public: // Constructors:
    //@{

    /// \brief Constructor, stores the pointer.
    to_unique_ptr(pointer ptr = pointer()) noexcept
      : Super(ptr)
    {}

    to_unique_ptr(std::nullptr_t) noexcept
      : Super(nullptr)
    {}

    //@}


  public: // Conversion operators:
    //@{

    /// Cast to underlying pointer, invalidates the stored pointer. NOTE: deprecated
    DUNE_DEPRECATED_MSG("Cast to raw pointer is deprecated. Use std::unique_ptr or std::shared_ptr instead.")
    operator pointer() noexcept { return Super::release(); }

    /// Convert the raw pointer to unique_ptr, invalidates the stored pointer
    operator std::unique_ptr<T,D>() noexcept { return std::move(static_cast<Super&>(*this)); }

    /// Convert the raw pointer to shared_ptr, invalidates the stored pointer
    operator std::shared_ptr<T>() noexcept { return std::move(static_cast<Super&>(*this)); }

    /// Checks whether *this owns an object
    explicit operator bool()       noexcept { return bool(static_cast<Super&>(*this)); }

    /// Checks whether *this owns an object
    explicit operator bool() const noexcept { return bool(static_cast<Super const&>(*this)); }

    //@}
  };


  /// Constructs an object of type T and wraps it in a to_unique_ptr, \relates to_unique_ptr
  template <class T, class... Args>
  to_unique_ptr<T> make_to_unique(Args&&... args)
  {
    return to_unique_ptr<T>(new T(std::forward<Args>(args)...));
  }

} // end namespace Dune

#endif // DUNE_TO_UNIQUE_PTR_HH
