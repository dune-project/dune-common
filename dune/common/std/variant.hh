// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_STD_VARIANT_HH
#define DUNE_COMMON_STD_VARIANT_HH
#if __has_include(<variant>)
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
}
}
#else
#include <tuple>
#include <memory>
#include <dune/common/hybridutilities.hh>
#include <dune/common/exceptions.hh>

namespace Dune {
namespace Std {
namespace Impl {

  // indicator value if something's not yet (or not any longer) valid
  constexpr const auto invalidIndex = std::numeric_limits<size_t>::max();

  /* helper constructs to find position of a type T in a pack Ts... */
  template <typename T, typename... Ts>
  struct index_in_pack;

  template <typename T, typename... Ts>
  struct index_in_pack<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

  template <typename T, typename U, typename... Ts>
  struct index_in_pack<T, U, Ts...> : std::integral_constant<std::size_t, 1 + index_in_pack<T, Ts...>::value> {};

  /* end helper constructs to find position of a type T in a pack Ts... */

  template<typename Tp>
  struct Buffer_ : std::aligned_storage<sizeof(Tp)> {
    using Storage = std::aligned_storage_t<sizeof(Tp)>;
    Storage storage_;

    void* addr() {
      return static_cast<void*>(&storage_);
    }

    const void* addr() const {
      return static_cast<const void*>(&storage_);
    }

    Tp* ptr() {
      return static_cast<Tp*>(addr());
    }

    const Tp* ptr() const {
      return static_cast<const Tp*>(addr());
    }
  };

  template<typename Tp, bool isTrivial>
  struct TypeStorage_ { };

  template<typename Tp>
  struct TypeStorage_<Tp, true> {
    TypeStorage_(Tp t) :
      tp_(t) {}

    template<typename... Args>
    TypeStorage_(Args... args) :
      tp_(args...) {}

    auto& get() {
      return tp_;
    }
    const auto& get() const {
      return tp_;
    }

    void reset() {};

    private:
    Tp tp_;
  };

  template<typename Tp>
  struct TypeStorage_<Tp, false> {
    TypeStorage_(Tp t) {
      ::new (&tp_) Tp(t);
    }

    template<typename... Args>
    TypeStorage_(Args... args) {
      ::new (&tp_) Tp(std::forward<Args>(args)...);
    }

    TypeStorage_() = delete;

    auto& get() {
      return *(tp_.ptr());
    }
    const auto& get() const {
      return *(tp_.ptr());
    }

    void reset() {
      // Properly destruct the member:
      tp_.ptr()->~Tp();
      // (the memory itself doesn't need to be free'd. This is done when the Buffer_ member gets destructed)
    }

    private:
    Buffer_<Tp> tp_;
  };

  template<typename... T>
  union variant_union_ {
    // dummy (this should never be called)
    void resetByIndex(size_t) {
      assert(false);
    };
  };

  template<typename Head_, typename... Tail_>
  union variant_union_<Head_, Tail_...> {
    constexpr variant_union_() :
      tail_() {}

    template<typename... Args>
    constexpr variant_union_(std::integral_constant<size_t, 0>, Args&&... args) :
      head_(std::forward<Args...>(args)...) {}

    template<size_t N, typename... Args>
    constexpr variant_union_(std::integral_constant<size_t, N>, Args&&... args) :
      tail_(std::integral_constant<size_t, N-1>(), std::forward<Args...>(args)...) {}

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

    void resetByIndex(size_t indexToReset) {
      if (indexToReset == 0) {
        head_.reset();
        return;
      }
      else {
        tail_.resetByIndex(indexToReset-1);
      }
    }

    template<typename Tp>
    void set(Tp&& obj) {
      using T = std::decay_t<Tp>;
      Dune::Hybrid::ifElse(std::is_same<T, Head_>(),
        [&](auto&& id)    { id(head_)=std::forward<Tp>(obj); },
        [&](auto&& id) { return id(tail_).set(std::forward<Tp>(obj)); }
      );
    }

    constexpr size_t size() const {
      return sizeof...(Tail_)+1;
    }

    private:
    TypeStorage_<Head_, std::is_trivial<Head_>::value> head_;
    variant_union_<Tail_...> tail_;
  };

  template<typename...T>
  struct variant_{

    constexpr variant_() :
      unions_(),
      index_(invalidIndex) {}

    template<typename Tp>
    constexpr variant_(Tp obj) :
      unions_(),
      index_(index_in_pack<Tp, T...>::value)
      {
        unions_.set(std::move(obj));
      }

    variant_(variant_&& other) {
      unions_ = std::move(other.unions_);
      index_ = other.index_;
      other.index_ = invalidIndex;
    }

    variant_(const variant_& other) {
      index_ = other.index_;

      namespace H = Dune::Hybrid;
      H::forEach(H::integralRange(std::integral_constant<size_t, size_>()), [&](auto i) {
            if(i==index_)
              unions_.set(other.template get<i>());
          });
    }

    variant_& operator=(const variant_& other) {
      if(index_ != invalidIndex)
        unions_.resetByIndex(index_);

      index_ = other.index_;

      namespace H = Dune::Hybrid;
      H::forEach(H::integralRange(std::integral_constant<size_t, size_>()), [&](auto i) {
            if(i==index_)
              unions_.set(other.template get<i>());
          });
      return *this;
    }

    variant_& operator=(variant_&& other) {
      unions_ = std::move(other.unions_);
      index_ = other.index_;
      other.index_ = invalidIndex;

      return *this;
    }


    template<typename Tp>
    auto& get() {
      constexpr size_t idx = index_in_pack<Tp, T...>::value;
      if (index_ != idx)
        DUNE_THROW(Dune::Exception, "Bad variant access.");

      return get<idx>();
    }

    template<typename Tp>
    const auto& get() const {
      constexpr size_t idx = index_in_pack<Tp, T...>::value;
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

    template<size_t N>
    auto* get_if() {
      using Tp = std::decay_t<decltype(get<N>())>;
      if (not holds_alternative<N>())
        return (Tp*) nullptr;
      else
        return &(get<Tp>());
    }

    template<size_t N>
    const auto* get_if() const {
      using Tp = std::decay_t<decltype(get<N>())>;
      if (not holds_alternative<N>())
        return (Tp*) nullptr;
      else
        return &(get<Tp>());
    }

    template<size_t N>
    auto& get() {
      if (index_ != N || index_ == invalidIndex)
        DUNE_THROW(Dune::Exception, "Bad variant access.");
      return unions_.template getByIndex(std::integral_constant<size_t, N>());
    }
    template<size_t N>
    const auto& get() const {
      if (index_ != N || index_ == invalidIndex)
        DUNE_THROW(Dune::Exception, "Bad variant access.");
      return unions_.template getByIndex(std::integral_constant<size_t, N>());
    }

    template<typename Tp>
    constexpr Tp& operator=(Tp obj) {
      constexpr auto index = index_in_pack<Tp, T...>::value;
      // before setting a new object into the buffer, we have to destruct the old element
      if(not (index_ == index or index_ == invalidIndex)) {
        unions_.resetByIndex(index_);
      }
      unions_.set(std::move(obj));
      index_=index;
      return unions_.getByIndex(std::integral_constant<size_t,index>());
    }

    constexpr std::size_t index() const noexcept {
      return index_;
    }

    constexpr auto size() const {
      return sizeof...(T);
    }

    ~variant_() {
      if (index_ != invalidIndex) {
        unions_.resetByIndex(index_);
      }
    }

    /* \brief Apply visitor to the active variant.
     *
     * visit assumes that the result of
     * func(T) has the same type for all types T
     * in this variant.
     */
    template<typename F>
    auto visit(F&& func) {
      using namespace Dune::Hybrid;

      using Result = decltype(func(unions_.getByIndex(std::integral_constant<size_t, 0>())));

      return ifElse(std::is_same<Result, void>(), [&, this](auto id) {
          constexpr auto tsize = size_;
          Dune::Hybrid::forEach(Dune::Hybrid::integralRange(std::integral_constant<size_t, tsize>()), [&](auto i) {
            if (i==this->index_)
              func(id(unions_).getByIndex(std::integral_constant<size_t, i>()));
            });
          return;},
        [&func,this](auto id) {
          constexpr auto tsize = size_;

          auto result = std::unique_ptr<Result>();

          Dune::Hybrid::forEach(Dune::Hybrid::integralRange(std::integral_constant<size_t, tsize>()), [&, this](auto i) {
            if (i==this->index_)
              result = std::make_unique<Result>(func(id(this->unions_).getByIndex(std::integral_constant<size_t, i>())));
          });
      return *result;
       });
    }

    template<typename F>
    auto visit(F&& func) const {
      using namespace Dune::Hybrid;

      using Result = decltype(func(unions_.getByIndex(std::integral_constant<size_t, 0>())));

      return ifElse(std::is_same<Result, void>(), [&, this](auto id) {
          constexpr auto tsize = size_;
          Dune::Hybrid::forEach(Dune::Hybrid::integralRange(std::integral_constant<size_t, tsize>()), [&](auto i) {
            if (i==this->index_)
              func(id(unions_).getByIndex(std::integral_constant<size_t, i>()));
            });
          return;},
        [&func,this](auto id) {
          constexpr auto tsize = size_;

          auto result = std::unique_ptr<Result>();

          Dune::Hybrid::forEach(Dune::Hybrid::integralRange(std::integral_constant<size_t, tsize>()), [&, this](auto i) {
            if (i==this->index_)
              result = std::make_unique<Result>(func(id(this->unions_).getByIndex(std::integral_constant<size_t, i>())));
          });
      return *result;
       });
    }

    /** \brief Check if a given type is the one that is currently active in the variant. */
    template<typename Tp>
    constexpr bool holds_alternative() const {
      // I have no idea how this could be really constexpr, but for STL-conformity,
      // I'll leave the modifier there.
      return (index_in_pack<Tp, T...>::value == index_);
    }

    /** \brief Check if a given type is the one that is currently active in the variant. */
    template<size_t N>
    constexpr bool holds_alternative() const {
      // I have no idea how this could be really constexpr, but for STL-conformity,
      // I'll leave the modifier there.
      return (N == index_);
    }

    private:
    variant_union_<T...> unions_;
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
  auto visit(F&& visitor, variant<T...>& var) {
    return var.visit(std::forward<F>(visitor));
  }

  template<typename F, typename... T>
  auto visit(F&& visitor, const variant<T...>& var) {
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
  const auto* get_if(const variant<T...>& var) {
    return var.template get_if<Tp>();
  }

  template<typename Tp, typename ...T>
  auto* get_if(variant<T...>& var) {
    return var.template get_if<Tp>();
  }

  template<size_t N, typename ...T>
  const auto* get_if(const variant<T...>& var) {
    return var.template get_if<N>();
  }

  template<size_t N, typename ...T>
  auto* get_if(variant<T...>& var) {
    return var.template get_if<N>();
  }

  template<typename Tp, typename ...T>
  constexpr bool holds_alternative(const variant<T...>& var) {
    return var.template holds_alternative<Tp>();
  }

  template <typename... T>
  constexpr auto variant_size_v(const variant<T...>&) {
    return std::integral_constant<std::size_t,sizeof...(T)>::value;
  }

} // end namespace Std
} // end namespace Dune
#endif
