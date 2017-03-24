// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DEBUGALIGN_HH
#define DUNE_DEBUGALIGN_HH

#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>

#include <dune/common/classname.hh>
#include <dune/common/typetraits.hh>

namespace Dune {

  //! type of the handler called by `violatedAlignment()`
  using ViolatedAlignmentHandler =
    std::function<void(const char*, std::size_t, const void*)>;

  //! access the handler called by `violatedAlignment()`
  /**
   * This may be used to obtain the handler for the purpose of calling, or for
   * saving it somwhere to restore it later.  It may also be used to set the
   * handler simply by assigning a new handler.  Setting the handler races
   * with other accesses.
   */
  ViolatedAlignmentHandler &violatedAlignmentHandler();

  //! called when an alignment violation is detected
  /**
   * \p className         Name of the class whose alignment was violated
   * \p expectedAlignment The (over-)alignment that the class expected
   * \p address           The address the class actually found itself at.
   *
   * The main purpose of the function is to serve as a convenient breakpoint
   * for debugging -- which is why we put it in an external compilation unit
   * so it isn't inlined.
   */
  void violatedAlignment(const char *className, std::size_t expectedAlignment,
                         const void *address);

  //! check whether an address conforms to the given alignment
  inline bool isAligned(const void *p, std::size_t align)
  {
    return std::uintptr_t(p) % align == 0;
  }

  //! CRTP base mixin class to check alignment
  template<std::size_t align, class Impl>
  class alignas(align) AlignedBase
  {
    void checkAlignment() const
    {
      auto pimpl = static_cast<const Impl*>(this);
      if(!isAligned(pimpl, align))
        violatedAlignment(className<Impl>().c_str(), align, pimpl);
    }
  public:
    AlignedBase()                    { checkAlignment(); }
    AlignedBase(const AlignedBase &) { checkAlignment(); }
    AlignedBase(AlignedBase &&)      { checkAlignment(); }
    ~AlignedBase()                   { checkAlignment(); }

    AlignedBase& operator=(const AlignedBase &) = default;
    AlignedBase& operator=(AlignedBase &&)      = default;
  };

  //! an alignment large enough to trigger alignment errors
  static constexpr auto debugAlignment = 2*alignof(std::max_align_t);

  template<class T, std::size_t align>
  class AlignedNumber;

  //! align a value to a certain alignment
  template<std::size_t align = debugAlignment, class T>
  AlignedNumber<T, align> aligned(T value) { return { std::move(value) }; }

  //! aligned wrappers for arithmetic types
  template<class T, std::size_t align = debugAlignment>
  class AlignedNumber
    : public AlignedBase<align, AlignedNumber<T, align> >
  {
    T value_;

  public:
    AlignedNumber() = default;
    AlignedNumber(T value) : value_(std::move(value)) {}
    template<class U, std::size_t uAlign,
             class = std::enable_if_t<(align >= uAlign) &&
                                      std::is_convertible<U, T>::value> >
    AlignedNumber(const AlignedNumber<U, uAlign> &o) : value_(U(o)) {}

    template<class U,
             class = std::enable_if_t<std::is_convertible<T, U>::value> >
    explicit operator U() const { return value_; }

    // The trick with `template<class U = T, class = void_t<expr(U)> >` is
    // needed because at least g++-4.9 seems to evaluates a default argument
    // in `template<class = void_t<expr(T))> >` as soon as possible and will
    // error out if `expr(T)` is invalid.  E.g. for `expr(T)` =
    // `decltype(--std::declval<T&>())`, instantiating `AlignedNumber<bool>`
    // will result in an unrecoverable error (`--` cannot be applied to a
    // `bool`).

    // Increment, decrement
    template<class U = T, class = void_t<decltype(++std::declval<U&>())> >
    AlignedNumber &operator++() { ++value_; return *this; }

    template<class U = T, class = void_t<decltype(--std::declval<U&>())> >
    AlignedNumber &operator--() { --value_; return *this; }

    template<class U = T, class = void_t<decltype(std::declval<U&>()++)> >
    decltype(auto) operator++(int) { return aligned<align>(value_++); }

    template<class U = T, class = void_t<decltype(std::declval<U&>()--)> >
    decltype(auto) operator--(int) { return aligned<align>(value_--); }

    // unary operators
    template<class U = T, class = void_t<decltype(+std::declval<const U&>())> >
    decltype(auto) operator+() const { return aligned<align>(+value_); }

    template<class U = T, class = void_t<decltype(-std::declval<const U&>())> >
    decltype(auto) operator-() const { return aligned<align>(-value_); }

    template<class U = T, class = void_t<decltype(~std::declval<const U&>())> >
    decltype(auto) operator~() const { return aligned<align>(~value_); }

    template<class U = T, class = void_t<decltype(!std::declval<const U&>())> >
    decltype(auto) operator!() const { return aligned<align>(!value_); }

    // assignment operators
#define DUNE_ASSIGN_OP(OP)                                              \
    template<class U, std::size_t uAlign,                               \
             class = std::enable_if_t<                                  \
               ( uAlign <= align &&                                     \
                 sizeof(std::declval<T&>() OP std::declval<U>()) )> >   \
    AlignedNumber &operator OP(const AlignedNumber<U, uAlign> &u)       \
    {                                                                   \
      value_ OP U(u);                                                   \
      return *this;                                                     \
    }                                                                   \
                                                                        \
    template<class U,                                                   \
             class = void_t<decltype(std::declval<T&>() OP              \
                                     std::declval<U>())> >              \
    AlignedNumber &operator OP(const U &u)                              \
    {                                                                   \
      value_ OP u;                                                      \
      return *this;                                                     \
    }                                                                   \
                                                                        \
    static_assert(true, "Require semicolon to unconfuse editors")

    DUNE_ASSIGN_OP(+=);
    DUNE_ASSIGN_OP(-=);

    DUNE_ASSIGN_OP(*=);
    DUNE_ASSIGN_OP(/=);
    DUNE_ASSIGN_OP(%=);

    DUNE_ASSIGN_OP(^=);
    DUNE_ASSIGN_OP(&=);
    DUNE_ASSIGN_OP(|=);

    DUNE_ASSIGN_OP(<<=);
    DUNE_ASSIGN_OP(>>=);

#undef DUNE_ASSIGN_OP
  };

  // binary operators
#define DUNE_BINARY_OP(OP)                                              \
  template<class T, std::size_t tAlign, class U, std::size_t uAlign,    \
           class = void_t<decltype(std::declval<T>() OP std::declval<U>())> > \
  decltype(auto)                                                        \
  operator OP(const AlignedNumber<T, tAlign> &t,                        \
              const AlignedNumber<U, uAlign> &u)                        \
  {                                                                     \
    /* can't use std::max(); not constexpr */                           \
    return aligned<(tAlign > uAlign ? tAlign : uAlign)>(T(t) OP U(u));  \
  }                                                                     \
                                                                        \
  template<class T, class U, std::size_t uAlign,                        \
           class = void_t<decltype(std::declval<T>() OP std::declval<U>())> > \
  decltype(auto)                                                        \
  operator OP(const T &t, const AlignedNumber<U, uAlign> &u)            \
  {                                                                     \
    return aligned<uAlign>(t OP U(u));                                  \
  }                                                                     \
                                                                        \
  template<class T, std::size_t tAlign, class U,                        \
           class = void_t<decltype(std::declval<T>() OP std::declval<U>())> > \
  decltype(auto)                                                        \
  operator OP(const AlignedNumber<T, tAlign> &t, const U &u)            \
  {                                                                     \
    return aligned<tAlign>(T(t) OP u);                                  \
  }                                                                     \
                                                                        \
  static_assert(true, "Require semicolon to unconfuse editors")

  DUNE_BINARY_OP(+);
  DUNE_BINARY_OP(-);

  DUNE_BINARY_OP(*);
  DUNE_BINARY_OP(/);
  DUNE_BINARY_OP(%);

  DUNE_BINARY_OP(^);
  DUNE_BINARY_OP(&);
  DUNE_BINARY_OP(|);

  DUNE_BINARY_OP(<<);
  DUNE_BINARY_OP(>>);

  DUNE_BINARY_OP(==);
  DUNE_BINARY_OP(!=);
  DUNE_BINARY_OP(<);
  DUNE_BINARY_OP(>);
  DUNE_BINARY_OP(<=);
  DUNE_BINARY_OP(>=);

  DUNE_BINARY_OP(&&);
  DUNE_BINARY_OP(||);

#undef DUNE_BINARY_OP

} // namespace Dune

#endif // DUNE_DEBUGALIGN_HH
