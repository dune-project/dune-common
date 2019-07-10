// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_STD_VARIANT_HH
#define DUNE_COMMON_STD_VARIANT_HH
#ifdef DUNE_HAVE_CXX_VARIANT
#include <variant>
namespace Dune {
namespace Std {
  using std::variant;
  using std::visit;
  using std::variant_size;
  using std::variant_size_v;
  using std::get;
  using std::get_if;
  using std::holds_alternative;
  using std::monostate;
}
}
#else
#include <tuple>
#include <memory>
#include <dune/common/hybridutilities.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/typelist.hh>
#include <dune/common/rangeutilities.hh>

namespace Dune {
namespace Std {
namespace Impl {

  // indicator value if something's not yet (or not any longer) valid
  constexpr const auto invalidIndex = std::numeric_limits<std::size_t>::max();

  /* helper constructs to find position of a type T in a pack Ts... */
  template <typename T, typename... Ts>
  struct index_in_pack;

  template <typename T, typename... Ts>
  struct index_in_pack<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

  template <typename T, typename U, typename... Ts>
  struct index_in_pack<T, U, Ts...> : std::integral_constant<std::size_t, 1 + index_in_pack<T, Ts...>::value> {};

  /* end helper constructs to find position of a type T in a pack Ts... */

  template<typename T>
  struct TypeStorage {

    using Buffer = std::aligned_storage_t<sizeof(T), alignof(T)>;

    // We only allow construction of empty TypeStorage
    // objects and no asignment.  Actual construction
    // and assignment of stored values is done using
    // special methods.
    TypeStorage() = default;

    TypeStorage(const TypeStorage&) = delete;
    TypeStorage(TypeStorage&&) = delete;
    TypeStorage& operator=(const TypeStorage&) = delete;
    TypeStorage& operator=(TypeStorage&&) = delete;

    void construct(const T& t) {
      ::new (&buffer_) T(t);
    }

    void construct(T&& t) {
      ::new (&buffer_) T(std::move(t));
    }

    void assign(const T& t) {
      this->get() = t;
    }

    void assign(T&& t) {
      this->get() = std::move(t);
    }

    void destruct() {
      reinterpret_cast<T*>(&buffer_)->~T();
    }

    auto& get() {
      return *(reinterpret_cast<T*>(&buffer_));
    }

    const auto& get() const {
      return *(reinterpret_cast<const T*>(&buffer_));
    }

  private:
    Buffer buffer_;
  };



  // A variadic union type providing access by index
  // of member.
  template<typename... T>
  union VariadicUnion;

  // This is the recursion closure dummy.
  // It's methods should never be called.
  template<>
  union VariadicUnion<> {
    template<class Ti>
    void construct(Ti&&) { assert(false); }

    template<class Ti>
    void assign(Ti&&) { assert(false); }

    void destruct(std::size_t) { assert(false); };

  };

  template<typename Head, typename... Tail>
  union VariadicUnion<Head, Tail...>
  {
    // We only allow construction of empty VariadicUnion
    // objects and no asignment.  Actual construction
    // and assignment of stored values is done using
    // special methods.
    constexpr VariadicUnion() = default;

    VariadicUnion(const VariadicUnion& other) = delete;
    VariadicUnion(VariadicUnion&& other) = delete;
    VariadicUnion& operator=(const VariadicUnion& other) = delete;
    VariadicUnion& operator=(VariadicUnion&& other) = delete;

    // Construct stored object
    void construct(const Head& obj) {
      new (&head_) TypeStorage<Head>;
      head_.construct(obj);
    }

    void construct(Head&& obj) {
      new (&head_) TypeStorage<Head>;
      head_.construct(std::move(obj));
    }

    template<class Ti,
      std::enable_if_t<not std::is_same<std::decay_t<Ti>, Head>::value, int> = 0>
    void construct(Ti&& obj) {
      new (&tail_) VariadicUnion<Tail...>;
      tail_.construct(std::forward<Ti>(obj));
    }

    // Assign to stored object. This should
    // only be called if it's clear, that
    // the VariadicUnion already stores
    // on object of the passed type.
    void assign(const Head& obj){
      head_.assign(obj);
    }

    void assign(Head&& obj){
      head_.assign(std::move(obj));
    }

    template<class Ti,
      std::enable_if_t<not std::is_same<std::decay_t<Ti>, Head>::value, int> = 0>
    void assign(Ti&& obj){
      tail_.assign(std::forward<Ti>(obj));
    }

    // Destruct stored object. This should only
    // be called with the appropriate index of
    // the stored object.
    void destruct(size_t indexToReset) {
      if (indexToReset == 0) {
        head_.destruct();
        return;
      }
      else
        tail_.destruct(indexToReset-1);
    }

    // Access to stored object
    auto& getByIndex(std::integral_constant<size_t, 0>) {
      return head_.get();
    }

    const auto& getByIndex(std::integral_constant<size_t, 0>) const {
      return head_.get();
    }

    template<size_t N>
    auto& getByIndex(std::integral_constant<size_t, N>) {
      return tail_.getByIndex(std::integral_constant<size_t, N-1>());
    }

    template<size_t N>
    const auto& getByIndex(std::integral_constant<size_t, N>) const {
      return tail_.getByIndex(std::integral_constant<size_t, N-1>());
    }

    constexpr size_t size() const {
      return sizeof...(Tail)+1;
    }

  private:
    TypeStorage<Head> head_;
    VariadicUnion<Tail...> tail_;
  };

  template<typename...T>
  struct variant_{

    // Compute index of Ti in T...
    template<class Ti>
    constexpr static auto typeIndex()
    {
      return index_in_pack<std::decay_t<Ti>, T...>::value;
    }

    // Create static index range for iterating over T...
    constexpr static auto indexRange()
    {
      return Dune::range(Dune::index_constant<size_>());
    }

    constexpr void destructIfValid()
    {
      if (index_ != invalidIndex)
        unions_.destruct(index_);
      index_ = invalidIndex;
    }

    // All methods will only use the default constructor but
    // no other constructors or assignment operators of VariadicUnion.
    // The construction and assignment of stored values is done
    // using special methods.

    // Default constructor.
    // Default construct T_0 if possible, otherwise set to invalid state
    constexpr variant_() :
      index_(invalidIndex)
    {
      using T0 = TypeListEntry_t<0, TypeList<T...>>;
      Dune::Hybrid::ifElse(std::is_default_constructible<T0>(),
        [&](auto&& id) {
          unions_.construct(id(T0{}));
          index_ = 0;
        });
    }

    // Construct from some Ti
    template<typename Ti,
      disableCopyMove<variant_, Ti> = 0>
    constexpr variant_(Ti&& obj) :
      index_(typeIndex<Ti>())
    {
      unions_.construct(std::forward<Ti>(obj));
    }

    // Copy constructor
    variant_(const variant_& other) {
      index_ = other.index_;
      if (index_==invalidIndex)
        return;
      Dune::Hybrid::forEach(indexRange(), [&](auto i) {
        if(i==index_)
          unions_.construct(other.template get<i>());
      });
    }

    // Move constructor
    variant_(variant_&& other) {
      index_ = other.index_;
      if (index_==invalidIndex)
        return;
      Dune::Hybrid::forEach(indexRange(), [&](auto i) {
        if(i==index_)
          unions_.construct(std::move(other.template get<i>()));
      });
      other.destructIfValid();
    }

    // Copy assignment operator
    variant_& operator=(const variant_& other) {
      if(index_ == other.index_) {
        if (index_ != invalidIndex)
          Dune::Hybrid::forEach(indexRange(), [&](auto i) {
            if(i==index_)
              unions_.assign(other.template get<i>());
          });
      }
      else {
        destructIfValid();
        index_ = other.index_;
        if (index_ != invalidIndex)
          Dune::Hybrid::forEach(indexRange(), [&](auto i) {
            if(i==index_)
              unions_.construct(other.template get<i>());
          });
      }
      return *this;
    }

    // Move assignment operator
    variant_& operator=(variant_&& other) {
      if(index_ == other.index_) {
        if (index_ != invalidIndex)
          Dune::Hybrid::forEach(indexRange(), [&](auto i) {
            if(i==index_)
              unions_.assign(std::move(other.template get<i>()));
          });
      }
      else {
        destructIfValid();
        index_ = other.index_;
        if (index_ != invalidIndex)
          Dune::Hybrid::forEach(indexRange(), [&](auto i) {
            if(i==index_)
              unions_.construct(std::move(other.template get<i>()));
          });
      }
      other.destructIfValid();
      return *this;
    }

    // Assignment from some Ti
    template<typename Ti,
      disableCopyMove<variant_, Ti> = 0>
    constexpr variant_& operator=(Ti&& obj) {
      constexpr auto newIndex = typeIndex<Ti>();
      if (index_ == newIndex)
        unions_.assign(std::forward<Ti>(obj));
      else
      {
        destructIfValid();
        index_ = newIndex;
        unions_.construct(std::forward<Ti>(obj));
      }
      return *this;
    }

    template<typename Tp>
    auto& get() {
      constexpr auto idx = typeIndex<Tp>();
      if (index_ != idx)
        DUNE_THROW(Dune::Exception, "Bad variant access.");

      return get<idx>();
    }

    template<typename Tp>
    const auto& get() const {
      constexpr auto idx = typeIndex<Tp>();
      if (index_ != idx)
        DUNE_THROW(Dune::Exception, "Bad variant access.");

      return get<idx>();
    }

    template<typename Tp>
    Tp* get_if() {
      if (not holds_alternative<Tp>())
        return (Tp*) nullptr;
      else
        return &(get<Tp>());
    }

    template<typename Tp>
    const Tp* get_if() const {
      if (not holds_alternative<Tp>())
        return (Tp*) nullptr;
      else
        return &(get<Tp>());
    }

    template<std::size_t N>
    auto* get_if() {
      using Tp = std::decay_t<decltype(get<N>())>;
      if (not holds_alternative<N>())
        return (Tp*) nullptr;
      else
        return &(get<Tp>());
    }

    template<std::size_t N>
    const auto* get_if() const {
      using Tp = std::decay_t<decltype(get<N>())>;
      if (not holds_alternative<N>())
        return (Tp*) nullptr;
      else
        return &(get<Tp>());
    }

    template<std::size_t N>
    auto& get() {
      if (index_ != N || index_ == invalidIndex)
        DUNE_THROW(Dune::Exception, "Bad variant access.");
      return unions_.template getByIndex(std::integral_constant<std::size_t, N>());
    }
    template<std::size_t N>
    const auto& get() const {
      if (index_ != N || index_ == invalidIndex)
        DUNE_THROW(Dune::Exception, "Bad variant access.");
      return unions_.template getByIndex(std::integral_constant<std::size_t, N>());
    }

    constexpr std::size_t index() const noexcept {
      return index_;
    }

    constexpr auto size() const {
      return sizeof...(T);
    }

    ~variant_() {
      destructIfValid();
    }

    /** \brief Apply visitor to the active variant.
     *
     * visit assumes that the result of
     * func(T) has the same type for all types T
     * in this variant.
     */
    template<typename F>
    decltype(auto) visit(F&& func) {
      auto dummyElseBranch = [&]() -> decltype(auto) { return func(this->get<0>());};
      auto indices = std::make_index_sequence<size_>{};
      return Hybrid::switchCases(indices, index(), [&](auto staticIndex) -> decltype(auto) {
        return func(this->template get<decltype(staticIndex)::value>());
      }, dummyElseBranch);
    }

    template<typename F>
    decltype(auto) visit(F&& func) const {
      auto dummyElseBranch = [&]() -> decltype(auto) { return func(this->get<0>());};
      auto indices = std::make_index_sequence<size_>{};
      return Hybrid::switchCases(indices, index(), [&](auto staticIndex) -> decltype(auto) {
        return func(this->template get<decltype(staticIndex)::value>());
      }, dummyElseBranch);
    }

    /** \brief Check if a given type is the one that is currently active in the variant. */
    template<typename Tp>
    constexpr bool holds_alternative() const {
      // I have no idea how this could be really constexpr, but for STL-conformity,
      // I'll leave the modifier there.
      return (typeIndex<Tp>() == index_);
    }

    /** \brief Check if a given type is the one that is currently active in the variant. */
    template<std::size_t N>
    constexpr bool holds_alternative() const {
      // I have no idea how this could be really constexpr, but for STL-conformity,
      // I'll leave the modifier there.
      return (N == index_);
    }

    private:
    VariadicUnion<T...> unions_;
    std::size_t index_;
    constexpr static auto size_ = sizeof...(T);
  };

} // end namespace Impl

  /** \brief Incomplete re-implementation of C++17's std::variant. */
  template<typename ...T>
  using variant = Impl::variant_<T...>;

  template<size_t N, typename... T>
  auto& get(variant<T...>& var) {
    return var.template get<N>();
  }

  template<size_t N, typename... T>
  const auto& get(const variant<T...>& var) {
    return var.template get<N>();
  }

  template<typename F, typename... T>
  decltype(auto) visit(F&& visitor, variant<T...>& var) {
    return var.visit(std::forward<F>(visitor));
  }

  template<typename F, typename... T>
  decltype(auto) visit(F&& visitor, const variant<T...>& var) {
    return var.visit(std::forward<F>(visitor));
  }

  template<typename Tp, typename ...T>
  auto& get(variant<T...>& var) {
    return var.template get<Tp>();
  }

  template<typename Tp, typename ...T>
  const auto& get(const variant<T...>& var) {
    return var.template get<Tp>();
  }

  template<typename Tp, typename ...T>
  const auto* get_if(const variant<T...>* var) {
    if (var == nullptr)
      return (const Tp*) nullptr;
    return var->template get_if<Tp>();
  }

  template<typename Tp, typename ...T>
  auto* get_if(variant<T...>* var) {
    if (var == nullptr)
      return (Tp*) nullptr;
    return var->template get_if<Tp>();
  }

  template<size_t N, typename ...T>
  const auto* get_if(const variant<T...>* var) {
    using Tp = std::decay_t<decltype(var->template get<N>())>;
    if (var == nullptr)
      return (const Tp*) nullptr;
    return var->template get_if<N>();
  }

  template<size_t N, typename ...T>
  auto* get_if(variant<T...>* var) {
    using Tp = std::decay_t<decltype(var->template get<N>())>;
    if (var == nullptr)
      return (Tp*) nullptr;
    return var->template get_if<N>();
  }

  template<typename Tp, typename ...T>
  constexpr bool holds_alternative(const variant<T...>& var) {
    return var.template holds_alternative<Tp>();
  }

  template <typename T>
  struct variant_size {};

  template <typename... T>
  struct variant_size<variant<T...>>
  : std::integral_constant<std::size_t, sizeof...(T)> { };

  // this cannot be inline (as it is in the STL) as this would need C++17
  template <typename T>
  constexpr std::size_t variant_size_v = variant_size<T>::value;

  /**
   * \brief Trial default constructible class
   *
   * This can be used to make Std::variant default-constructible.
   */
  struct monostate {};

  constexpr bool operator<(monostate, monostate) noexcept { return false; }
  constexpr bool operator>(monostate, monostate) noexcept { return false; }
  constexpr bool operator<=(monostate, monostate) noexcept { return true; }
  constexpr bool operator>=(monostate, monostate) noexcept { return true; }
  constexpr bool operator==(monostate, monostate) noexcept { return true; }
  constexpr bool operator!=(monostate, monostate) noexcept { return false; }


} // end namespace Std
} // end namespace Dune
#endif
#endif
