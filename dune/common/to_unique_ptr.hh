// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_TO_UNIQUE_PTR_HH
#define DUNE_TO_UNIQUE_PTR_HH

#include <memory>

namespace Dune
{
  /// \brief An owning pointer wrapper that can be assigned to (smart) pointers. Cannot be copied.
  /// Transfers ownership by cast to any (smart) pointer type. Releases the stored pointer on transfer.
  /// NOTE: This is an intermediate solution to switch to std::unique_ptr in later releases smoothly.
  /**
   * Example of usage:
   * ```
   * ToUniquePtr<int> f() { return new int(1); }
   * auto g() { return makeToUnique<int>(2); }
   *
   * int* p1 = f(); // p1 gets ownership, must delete explicitly
   * delete p1;
   *
   * std::unique_ptr<int> p2 = f();
   * std::shared_ptr<int> p3 = f();
   *
   * auto p4 = f(); // ToUniquePtr has itself pointer semantic
   * std::cout << *p4 << '\n';
   *
   * std::unique_ptr<int> p5( g() );
   * ```
   **/
  template <class T>
  class ToUniquePtr
      : public std::unique_ptr<T>
  {
    using Super = std::unique_ptr<T>;

  public:
    // Member types:
    //@{

    using pointer = typename Super::pointer;

    //@}


  public:
    // Constructors:
    //@{

    /// Constructor, stores the pointer.
    ToUniquePtr(pointer ptr = pointer()) noexcept
      : Super(ptr)
    {}

    /// Constructor, creates a `nullptr`
    ToUniquePtr(std::nullptr_t) noexcept
      : Super(nullptr)
    {}

    //@}


  public:
    // Conversion operators:
    //@{

    /// Convert to underlying pointer, releases the stored pointer.
    /// \deprecated Cast to raw pointer is deprecated. Use std::unique_ptr or std::shared_ptr instead.
    ///             Will be removed after Dune 2.8
    [[deprecated("Cast to raw pointer is deprecated. Use std::unique_ptr or std::shared_ptr instead.")]]
    operator pointer() noexcept { return Super::release(); }

    /// Convert to unique_ptr, invalidates the stored pointer
    operator std::unique_ptr<T>() noexcept { return std::move(static_cast<Super&>(*this)); }

    /// Convert to shared_ptr, invalidates the stored pointer
    operator std::shared_ptr<T>() noexcept { return std::move(static_cast<Super&>(*this)); }

    /// Checks whether *this owns an object
    explicit operator bool() noexcept { return bool(static_cast<Super&>(*this)); }

    /// Checks whether *this owns an object
    explicit operator bool() const noexcept { return bool(static_cast<Super const&>(*this)); }

    //@}
  };


  /// Constructs an object of type T and wraps it in a ToUniquePtr, \relates ToUniquePtr
  template <class T, class... Args>
  ToUniquePtr<T> makeToUnique(Args&&... args)
  {
    return {new T(std::forward<Args>(args)...)};
  }

} // end namespace Dune

#endif // DUNE_TO_UNIQUE_PTR_HH
