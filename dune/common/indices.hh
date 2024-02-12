// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifndef DUNE_COMMON_INDICES_HH
#define DUNE_COMMON_INDICES_HH

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace Dune
{
  /** \addtogroup Common
   *  \{
   */

  /** \brief An index constant with value i
   *
   * An index constant is a simple type alias for an integral_constant.
   * Its main advantages are clarity (it is easier to see that code uses it
   * as an index) and the fact that the integral type is fixed, reducing verbosity
   * and avoiding the problem of maybe trying to overload / specialize using a different
   * integral type.
   */
  template<std::size_t i>
  using index_constant = std::integral_constant<std::size_t, i>;



  /** \brief Namespace with predefined compile time indices for the range [0,19]
   *
   * The predefined index objects in this namespace are `constexpr`, which allows them to
   * be used in situations where a compile time constant is needed, e.g. for a template
   * parameter. Apart from that, `constexpr` implies internal linkage, which helps to avoid
   * ODR problems.
   *
   * The constants implicitly convert to their contained value, so you can for example write
   *
   * \code{.cc}
   * std::array<int,_10> a;
   * // the above line is equivalent to
   * std::array<int,10> b;
   * \endcode
   *
   */
  namespace Indices
  {
    //! Compile time index with value 0.
    inline constexpr index_constant< 0>  _0 = {};

    //! Compile time index with value 1.
    inline constexpr index_constant< 1>  _1 = {};

    //! Compile time index with value 2.
    inline constexpr index_constant< 2>  _2 = {};

    //! Compile time index with value 3.
    inline constexpr index_constant< 3>  _3 = {};

    //! Compile time index with value 4.
    inline constexpr index_constant< 4>  _4 = {};

    //! Compile time index with value 5.
    inline constexpr index_constant< 5>  _5 = {};

    //! Compile time index with value 6.
    inline constexpr index_constant< 6>  _6 = {};

    //! Compile time index with value 7.
    inline constexpr index_constant< 7>  _7 = {};

    //! Compile time index with value 8.
    inline constexpr index_constant< 8>  _8 = {};

    //! Compile time index with value 9.
    inline constexpr index_constant< 9>  _9 = {};

    //! Compile time index with value 10.
    inline constexpr index_constant<10> _10 = {};

    //! Compile time index with value 11.
    inline constexpr index_constant<11> _11 = {};

    //! Compile time index with value 12.
    inline constexpr index_constant<12> _12 = {};

    //! Compile time index with value 13.
    inline constexpr index_constant<13> _13 = {};

    //! Compile time index with value 14.
    inline constexpr index_constant<14> _14 = {};

    //! Compile time index with value 15.
    inline constexpr index_constant<15> _15 = {};

    //! Compile time index with value 16.
    inline constexpr index_constant<16> _16 = {};

    //! Compile time index with value 17.
    inline constexpr index_constant<17> _17 = {};

    //! Compile time index with value 18.
    inline constexpr index_constant<18> _18 = {};

    //! Compile time index with value 19.
    inline constexpr index_constant<19> _19 = {};

  } // namespace Indices

  /**
   * \brief Unpack an std::integer_sequence<I,i...> to std::integral_constant<I,i>...
   *
   * This forward all entries of the given std::integer_sequence
   * as individual std::integral_constant arguments to the given callback.
   *
   * \param f Callback which has to accept unpacked values
   * \param sequence Packed std::integer_sequence of values
   * \returns Result of calling f with unpacked integers.
   */
  template<class F, class I, I... i>
  decltype(auto) constexpr unpackIntegerSequence(F&& f, std::integer_sequence<I, i...> sequence)
  {
    return f(std::integral_constant<I, i>()...);
  }


  namespace Indices { inline namespace Literals
  {
    namespace Impl
    {
      // convert a single character into an unsigned integer
      constexpr unsigned char2digit (const char c)
      {
        if (c >= '0' && c <= '9')
          return unsigned(c) - unsigned('0');
        else {
          throw std::invalid_argument("Character is not a digit.");
          return 0u;
        }
      }

      // convert a sequence of character digits into an unsigned integer
      template <class T, char... digits>
      constexpr T chars2number ()
      {
        const char arr[] = {digits...};
        T result = 0;
        T power  = 1;
        const T base = 10;

        const int N = sizeof...(digits);
        for (int i = 0; i < N; ++i) {
            char c = arr[N - 1 - i];
            result+= char2digit(c) * power;
            power *= base;
        }

        return result;
      }

    } //namespace Impl

    /**
     * \brief Literal to create an index compile-time constant
     *
     * \b Example:
     * `1_ic -> std::integral_constant<std::size_t,1>`
     **/
    template <char... digits>
    constexpr auto operator"" _ic()
    {
      return std::integral_constant<std::size_t, Impl::chars2number<std::size_t,digits...>()>{};
    }

    /**
     * \brief Literal to create an unsigned integer compile-time constant
     *
     * \b Example:
     * `1_uc -> std::integral_constant<unsigned,1>`
     **/
    template <char... digits>
    constexpr auto operator"" _uc()
    {
      return std::integral_constant<unsigned, Impl::chars2number<unsigned,digits...>()>{};
    }

    /**
     * \brief Literal to create a signed integer compile-time constant
     *
     * \b Example:
     * `1_sc -> std::integral_constant<int,1>`
     **/
    template <char... digits>
    constexpr auto operator"" _sc()
    {
      return std::integral_constant<int, Impl::chars2number<int,digits...>()>{};
    }

    /**
     * \brief Negation operator for integral constants.
     *
     * \b Example:
     * `-1_sc -> std::integral_constant<int,-1>`
     **/
    template <class T, T value>
    constexpr auto operator- (std::integral_constant<T,value>)
    {
      return std::integral_constant<std::make_signed_t<T>, -value>{};
    }

  }} //namespace Indices::Literals
} //namespace Dune

#endif // DUNE_COMMON_INDICES_HH
