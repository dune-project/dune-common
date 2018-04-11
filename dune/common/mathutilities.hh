#ifndef DUNE_ACTIPIPE_COMMON_MATHUTILITIES_HH
#define DUNE_ACTIPIPE_COMMON_MATHUTILITIES_HH

#include <type_traits>


namespace Dune
{

  // factorial
  // ---------

  template<class T,
           std::enable_if_t<std::is_integral<std::decay_t<T>>::value, int> = 0>
  constexpr inline static auto factorial(T&& n) noexcept
    -> std::decay_t< T >
  {
    std::decay_t<T> fac = 1;
    for(std::decay_t<T> k = 0; k < n; ++k)
      fac *= k+1;
    return fac;
  }

  template<class T, T n>
  constexpr inline static auto factorial (std::integral_constant<T, n>) noexcept
  {
    return std::integral_constant<T, factorial(n)>{};
  }

  // binomial
  // --------

  template<class T, class U,
           std::enable_if_t<std::is_same<std::decay_t<T>, std::decay_t<U>>::value, int> = 0,
           std::enable_if_t<std::is_integral<std::decay_t<T>>::value, int> = 0>
  constexpr inline static auto binomial (T&& n, U&& k) noexcept
    -> std::decay_t< T >
  {
    if (2*k > n)
      return binomial(n, n-k);

    if( k < 0)
      return 0;

    std::decay_t<T> bin = 1;
    for(auto i = n-k; i < n; ++i)
      bin *= i+1;
    return bin / factorial(k);
  }

  template< class T, T n, T k>
  constexpr inline static auto binomial (std::integral_constant<T, n>, std::integral_constant<T, k>)
  {
    return std::integral_constant<T, binomial(n, k)>{};
  }

} // namespace Dune

#endif // #ifndef DUNE_ACTIPIPE_COMMON_MATHUTILITIES_HH
