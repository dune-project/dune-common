// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=8 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception OR LGPL-3.0-or-later

#ifndef DUNE_COMMON_HYBRIDMULTIINDEX_HH
#define DUNE_COMMON_HYBRIDMULTIINDEX_HH

#include <cstddef>
#include <cassert>
#include <iostream>
#include <type_traits>

#include <dune/common/typetraits.hh>
#include <dune/common/indices.hh>
#include <dune/common/hybridutilities.hh>

namespace Dune {

  // The Impl namespace collects some free standing functions helper functions
  namespace Impl {

    template<class T>
    constexpr bool isHybridSizeT()
    {
      if constexpr (std::is_same_v<T, std::size_t>)
        return true;
      else
      {
        if constexpr (requires { T::value; })
          return std::is_same_v<T, std::integral_constant<std::size_t, T::value>>;
        else
          return false;
      }
    }

    template<class T>
    constexpr auto castToHybridSizeT(T t)
    {
      if constexpr (Dune::IsIntegralConstant<T>::value)
      {
        using VT = typename T::value_type;
        static_assert(
          std::is_convertible_v<VT,std::size_t> &&
          std::is_integral_v<VT> &&
          T::value >= 0,
          "HybridMultiIndex indices must be convertible to std::size_t or std::integral_constant<std::size_t,v>");
        return std::integral_constant<std::size_t, T::value>{};
      }
      if constexpr (std::is_integral_v<T>)
      {
        static_assert(
          std::is_convertible_v<T,std::size_t> &&
          std::is_integral_v<T>,
          "HybridMultiIndex indices must be convertible to std::size_t or std::integral_constant<std::size_t,v>");
        assert(t >= 0 &&
          "HybridMultiIndex indices must be convertible to std::size_t or std::integral_constant<std::size_t,v>");
        return std::size_t(t);
      }
      if constexpr (not (Dune::IsIntegralConstant<T>::value or std::is_integral_v<T>))
      {
        return std::size_t(0);
      }
    }

  }

  /**
   * \brief A hybrid multi-index class that supports both compile time and run time indices.
   *
   * A `HybridMultiIndex` supports storing a combination of run time and compile time indices.
   * This allows to construct multi-indices that provide sufficient information for accessing
   * nested multi-type containers.
   *
   * \note Internally all indices are stored as std::size_t or
   * std::integral_constant<std::size_t,v>. The latter is the same
   * as Dune::index_constant<v>.
   */
  template<typename... T>
  class HybridMultiIndex
  {

    // make sure that all indices use std::size_t as the underlying number type
    static_assert((... && Impl::isHybridSizeT<T>()),
      "HybridMultiIndex index storage must be std::size_t or std::integral_constant<std::size_t,v>");

  public:

    //! An `index_sequence` for the entries in this `HybridMultiIndex`.
    using index_sequence = std::index_sequence_for<T...>;

    constexpr HybridMultiIndex() = default;

    constexpr HybridMultiIndex(const HybridMultiIndex& tp) = default;

    constexpr HybridMultiIndex(HybridMultiIndex&& tp) = default;

    constexpr HybridMultiIndex& operator=(const HybridMultiIndex& tp) = default;

    constexpr HybridMultiIndex& operator=(HybridMultiIndex&& tp) = default;

    //! Constructor from a `std::tuple`
    explicit constexpr HybridMultiIndex(std::tuple<T...> t)
      : _data(t)
    {}

    //! Constructor from arguments
    template<typename... I>
    requires ((sizeof...(T) > 0 && sizeof...(I) == sizeof...(T))
              and ((std::is_integral_v<I> or Dune::IsIntegralConstant<I>::value) && ...))
    explicit constexpr HybridMultiIndex(I... i)
      : _data(Impl::castToHybridSizeT(i)...) // we assume that all arguments are convertible to the types T...
    {}

    //! Returns an index_sequence for enumerating the components of this HybridMultiIndex.
    [[nodiscard]] constexpr static index_sequence enumerate()
    {
      return {};
    }

    //! Get the size (length) of this multi-index.
    [[nodiscard]] constexpr static std::size_t size()
    {
      return sizeof...(T);
    }

    //! Get the size (length) of this multi-index.
    [[nodiscard]] constexpr static std::size_t max_size()
    {
      return size();
    }

    /**
     * \brief Get the index value at position pos.
     *
     * The get member function is required by the std-tuple-protocol
     * which e.g. enables structured bindings.
     */
    template<std::size_t i>
    requires (sizeof...(T) > i)
    [[nodiscard]] constexpr auto get() const
    {
      return std::get<i>(_data);
    }

    //! Get the index value at position pos.
    template<std::size_t i>
    requires (sizeof...(T) > i)
    [[nodiscard]] constexpr auto operator[](Dune::index_constant<i>) const
    {
      return std::get<i>(_data);
    }

    //! Get the index value at position pos.
    [[nodiscard]] constexpr std::size_t operator[](std::size_t pos) const
    {
      std::size_t entry = 0;
      Dune::Hybrid::forEach(enumerate(), [&] (auto i) {
          if (i==pos)
            entry = (*this)[i];
      });
      return entry;
    }

    //! Get the last index value.
    template<std::size_t i>
    requires (sizeof...(T) > i)
    [[deprecated("Method will be removed after Dune 2.11. Use operator[] instead.")]]
    [[nodiscard]] constexpr auto element(Dune::index_constant<i> pos = {}) const
    {
      return std::get<i>(_data);
    }

    //! Get the index value at position pos.
    [[deprecated("Method will be removed after Dune 2.11. Use operator[] instead.")]]
    [[nodiscard]] constexpr std::size_t element(std::size_t pos) const
    {
      std::size_t entry = 0;
      Dune::Hybrid::forEach(enumerate(), [&] (auto i) {
          if (i==pos)
            entry = (*this)[i];
      });
      return entry;
    }

    //! Get the first index value. Only available in non-empty multi-indices.
    template<std::size_t n = sizeof...(T)>
    requires (n > 0 && n == sizeof...(T))
    [[nodiscard]] constexpr auto front() const
    {
      return std::get<0>(_data);
    }

    //! Get the last index value. Only available in non-empty multi-indices.
    template<std::size_t n = sizeof...(T)>
    requires (n > 0 && n == sizeof...(T))
    [[nodiscard]] constexpr auto back() const
    {
      return std::get<n-1>(_data);
    }

  private:

    template<class... Head, class... Other>
    friend constexpr auto join(const HybridMultiIndex<Head...>&, const Other&...);

    std::tuple<T...> _data;

  };

  template<typename... I>
  requires (((std::is_integral_v<I> or Dune::IsIntegralConstant<I>::value) && ...))
  HybridMultiIndex(I... i) -> HybridMultiIndex<decltype(Impl::castToHybridSizeT(i))...>;

  template<typename... I>
  HybridMultiIndex(std::tuple<I...>) -> HybridMultiIndex<I...>;

  //! Returns a copy of the last element of the `HybridMultiIndex`.
  /**
   * As `HybridMultiIndex` instances should not be mutated after their creation, this function
   * returns a copy of the value. As values are either `std::integral_constant` or `std::size_t`, that's
   * just as cheap as returning a reference.
   */
  template<typename... T>
  [[nodiscard]] constexpr auto back(const HybridMultiIndex<T...>& tp)
    -> decltype(tp.back())
  {
    return tp.back();
  }

  //! Returns a copy of the first element of the `HybridMultiIndex`.
  /**
   * As `HybridMultiIndex` instances should not be mutated after their creation, this function
   * returns a copy of the value. As values are either `std::integral_constant` or `std::size_t`, that's
   * just as cheap as returning a reference.
   */
  template<typename... T>
  [[nodiscard]] constexpr auto front(const HybridMultiIndex<T...>& tp)
    -> decltype(tp.front())
  {
    return tp.front();
  }

  //! Appends a run time index to a `HybridMultiIndex`.
  /**
   * This function returns a new `HybridMultiIndex` with the run time index `i` appended.
   */
  template<typename... T>
  [[nodiscard]] constexpr HybridMultiIndex<T...,std::size_t> push_back(const HybridMultiIndex<T...>& tp, std::size_t i)
  {
    return unpackIntegerSequence([&](auto... j){
      return HybridMultiIndex(tp[j] ..., i);
    }, tp.enumerate());
  }

  //! Appends a compile time index to a `HybridMultiIndex`.
  /**
   * This function returns a new `HybridMultiIndex` with the compile time index `i` appended.
   *
   * The value for the new entry can be passed either as a template parameter or as an `index_constant`:
   *
   * \code{.cc}
   * auto tp = HybridMultiIndex(1,2,3,4);
   * using namespace Dune::Indices;
   * // the following two lines are equivalent
   * auto tp_a = push_back<1>(tp);
   * auto tp_b = push_back(tp,_1);
   * \endcode
   *
   */
  template<std::size_t i, typename... T>
  [[nodiscard]] constexpr HybridMultiIndex<T...,index_constant<i>> push_back(const HybridMultiIndex<T...>& tp, index_constant<i> iConstant = {})
  {
    return unpackIntegerSequence([&](auto... j){
      return HybridMultiIndex(tp[j] ..., iConstant);
    }, tp.enumerate());
  }

  //! Prepends a run time index to a `HybridMultiIndex`.
  /**
   * This function returns a new `HybridMultiIndex` with the run time index `i` prepended.
   */
  template<typename... T>
  [[nodiscard]] constexpr HybridMultiIndex<std::size_t,T...> push_front(const HybridMultiIndex<T...>& tp, std::size_t i)
  {
    return unpackIntegerSequence([&](auto... j){
      return HybridMultiIndex(i, tp[j] ...);
    }, tp.enumerate());
  }

  //! Prepends a compile time index to a `HybridMultiIndex`.
  /**
   * This function returns a new `HybridMultiIndex` with the compile time index `i` prepended.
   *
   * The value for the new entry can be passed either as a template parameter or as an `index_constant`:
   *
   * \code{.cc}
   * auto tp = HybridMultiIndex(1,2,3,4);
   * using namespace Dune::Indices;
   * // the following two lines are equivalent
   * auto tp_a = push_front<1>(tp);
   * auto tp_b = push_front(tp,_1);
   * \endcode
   *
   */
  template<std::size_t i, typename... T>
  [[nodiscard]] constexpr HybridMultiIndex<index_constant<i>,T...> push_front(const HybridMultiIndex<T...>& tp, index_constant<i> iConstant = {})
  {
    return unpackIntegerSequence([&](auto... j){
      return HybridMultiIndex(iConstant, tp[j] ...);
    }, tp.enumerate());
  }

  //! Hybrid utility that accumulates to the back of a multi-index
  /**
   * @brief The back of the multi-index will be accumulated and promoted in order to
   * hold the new index:
   *
   * \code{.cc}
   *  accumulate_back(HybridMultiIndex(_0,_2),_2) -> HybridMultiIndex(_0,_4)
   *  accumulate_back(HybridMultiIndex(_0,_2), 2) -> HybridMultiIndex(_0, 4)
   *  accumulate_back(HybridMultiIndex(_0, 2),_2) -> HybridMultiIndex(_0, 4)
   *  accumulate_back(HybridMultiIndex(_0, 2), 2) -> HybridMultiIndex(_0, 4)
   * \endcode
   */
  template<typename I, typename... T>
  requires (sizeof...(T) > 0)
  [[nodiscard]] constexpr auto accumulate_back(const HybridMultiIndex<T...>& tp, I i) {
    using ::Dune::Hybrid::plus;
    return push_back(pop_back(tp), plus(back(tp), i));
  }


  //! Hybrid utility that accumulates to the front of a multi-index
  /**
   * @brief The front of the multi-index will be accumulated and promoted in order to
   * hold the new index:
   *
   * \code{.cc}
   *  accumulate_front(HybridMultiIndex(_0,_2),_2) -> HybridMultiIndex(_2,_2)
   *  accumulate_front(HybridMultiIndex(_0,_2), 2) -> HybridMultiIndex( 2,_2)
   *  accumulate_front(HybridMultiIndex( 0,_2),_2) -> HybridMultiIndex( 2,_2)
   *  accumulate_front(HybridMultiIndex( 0,_2), 2) -> HybridMultiIndex( 2,_2)
   * \endcode
   */
  template<typename I, typename... T>
  requires (sizeof...(T) > 0)
  [[nodiscard]] constexpr auto accumulate_front(const HybridMultiIndex<T...>& tp, I i) {
    using ::Dune::Hybrid::plus;
    return push_front(pop_front(tp), plus(front(tp), i));
  }

  //! Join two hybrid multi-indices into one
  template<class... Head, class... Other>
  [[nodiscard]] constexpr auto join(const HybridMultiIndex<Head...>& head, const Other&... tail) {
    return Dune::HybridMultiIndex{std::tuple_cat(head._data, tail._data...)};
  }

  //! Reverses the order of the elements in the multi-index
  template<class... T>
  [[nodiscard]] constexpr auto reverse(const HybridMultiIndex<T...>& tp) {
    constexpr std::size_t size = sizeof...(T);
    return unpackIntegerSequence([&](auto... i){
      return HybridMultiIndex(tp[index_constant<size-i-1>{}] ...);
    }, std::make_index_sequence<size>{});
  }

  //! Removes first index on a `HybridMultiIndex`.
  /**
   * This function returns a new `HybridMultiIndex` without the first index.
   */
  template <class... T>
  requires (sizeof...(T) > 0)
  [[nodiscard]] constexpr auto pop_front(const HybridMultiIndex<T...>& tp)
  {
    return unpackIntegerSequence([&](auto... i){
      return HybridMultiIndex{std::make_tuple(tp[Dune::index_constant<i+1>{}]...)};
    }, std::make_index_sequence<(sizeof...(T) - 1)>{});
  }

  //! Removes last index on a `HybridMultiIndex`.
  /**
   * This function returns a new `HybridMultiIndex` without the last index.
   */
  template <class... T>
  requires (sizeof...(T) > 0)
  [[nodiscard]] constexpr auto pop_back(const HybridMultiIndex<T...>& tp)
  {
    return unpackIntegerSequence([&](auto... i){
      return HybridMultiIndex{std::make_tuple(tp[i]...)};
    }, std::make_index_sequence<(sizeof...(T) - 1)>{});
  }

  //! Compare two `HybridMultiIndex`s for value equality
  /**
   * The function returns true if both hybrid multi-indices are of the same length
   * and all entries have the same value.
   *
   * Note, it might be that the values are represented with different types.
   * To check for same value and same type, use a combination of `std::is_same`
   * and this comparison operator.
   **/
  template <class... S, class... T>
  [[nodiscard]] constexpr bool operator==(
    const HybridMultiIndex<S...>& lhs,
    const HybridMultiIndex<T...>& rhs)
  {
    if constexpr (sizeof...(S) == sizeof...(T)) {
      if constexpr ((Dune::IsInteroperable<S,T>::value &&...)) {
        return unpackIntegerSequence([&](auto... i){
          return ((lhs[i] == rhs[i]) &&...);
        }, lhs.enumerate());
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  //! Overload for purely static `HybridMultiIndex`s.
  /**
   * The function returns `std::true_type` if the values of the passed
   * multi-indices are equal. Otherwise returns `std::false_type`. Note, this
   * overload is chosen for purely static multi-indices only.
   **/
  template <class S, S... lhs, class T, T... rhs>
  [[nodiscard]] constexpr auto operator==(
    const HybridMultiIndex<std::integral_constant<S,lhs>...>&,
    const HybridMultiIndex<std::integral_constant<T,rhs>...>&)
  {
    // If we directly put the expression into std::bool_constant,
    // gcc-10 deduced the return type of this method as `HybridMultiIndex<>.
    constexpr auto result = (HybridMultiIndex(lhs...) == HybridMultiIndex(rhs...));
    return std::bool_constant<result>{};
  }


  //! Compare two `HybridMultiIndex`s for inequality
  template <class... S, class... T>
  [[nodiscard]] constexpr auto operator!=(
    const HybridMultiIndex<S...>& lhs,
    const HybridMultiIndex<T...>& rhs)
  {
    return !(lhs == rhs);
  }

  //! Compare two static `HybridMultiIndex`s for inequality
  template <class S, S... lhs, class T, T... rhs>
  [[nodiscard]] constexpr auto operator!=(
    const HybridMultiIndex<std::integral_constant<S,lhs>...>&,
    const HybridMultiIndex<std::integral_constant<T,rhs>...>&)
  {
    // If we directly put the expression into std::bool_constant,
    // gcc-10 deduced the return type of this method as `HybridMultiIndex<>.
    constexpr auto result = (HybridMultiIndex(lhs...) != HybridMultiIndex(rhs...));
    return std::bool_constant<result>{};
  }

  //! Dumps a `HybridMultiIndex` to a stream.
  template<typename... T>
  std::ostream& operator<<(std::ostream& os, const HybridMultiIndex<T...>& tp)
  {
    os << "HybridMultiIndex< ";
    Dune::Hybrid::forEach(tp, [&] (auto tp_i) {
      os << tp_i << " ";
    });
    os << ">";
    return os;
  }

} //namespace Dune



// Implement the tuple-protocol for HybridMultiIndex
namespace std {

  template<typename... T>
  struct tuple_size<Dune::HybridMultiIndex<T...>> : public std::integral_constant<std::size_t,sizeof...(T)> {};

  template <size_t i, typename... T>
  struct tuple_element<i, Dune::HybridMultiIndex<T...> >
  {
    using type = std::tuple_element_t<i, std::tuple<T...> >;
  };

}


#endif // DUNE_COMMON_HYBRIDMULTIINDEX_HH
