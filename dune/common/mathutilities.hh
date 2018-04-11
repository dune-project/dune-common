#ifndef DUNE_ACTIPIPE_COMMON_MATHUTILITIES_HH
#define DUNE_ACTIPIPE_COMMON_MATHUTILITIES_HH

#include <type_traits>

#include <dune/common/rangeutilities.hh>


namespace Dune
{

  // factorial
  // ---------

  namespace Impl
  {

    template<class T>
    constexpr inline static auto _factorial (std::integral_constant<T, 0>, PriorityTag<1>) noexcept
    {
      return std::integral_constant<T, 1 >{};
    }

    template<class T, T n>
    constexpr inline static auto _factorial (std::integral_constant<T, n>, PriorityTag<0>) noexcept
    {
      return std::integral_constant<T, n*decltype(_factorial(std::integral_constant<T, n-1>{}, PriorityTag<42>{}))::value >{};
    }

  } // namespace Impl


  template<class T, T n>
  constexpr inline static auto factorial (std::integral_constant<T, n>) noexcept
  {
    return Impl::_factorial(std::integral_constant<T, n>{}, PriorityTag<42>{});
  }

  template<class T,
           std::enable_if_t<std::is_integral<std::decay_t<T>>::value, int> = 0>
  constexpr inline static auto factorial(T&& n) noexcept
    -> std::decay_t< T >
  {
    std::decay_t< T > fac = 1;
    for(auto&& k : range(n))
      fac *= k+1;
    return fac;
  }


  // binomial
  // --------

  namespace Impl
  {

    template<class T, T n, T k, std::enable_if_t<(k>n), int> = 0>
    constexpr inline static auto _binomial (std::integral_constant<T, n>, std::integral_constant<T, k>, PriorityTag<3>) noexcept
    {
      return std::integral_constant<T, 0>{};
    }

    template<class T, T n>
    constexpr inline static auto _binomial (std::integral_constant<T, n>, std::integral_constant<T, 0>, PriorityTag<2>) noexcept
    {
      return std::integral_constant<T, 1>{};
    }

    template<class T, T n>
    constexpr inline static auto _binomial (std::integral_constant<T, n>, std::integral_constant<T, 1>, PriorityTag<2>) noexcept
    {
      return std::integral_constant<T, n>{};
    }

    template<class T, T n, T k, std::enable_if_t<(2*k>n), int> = 0>
    constexpr inline static auto _binomial (std::integral_constant<T, n>, std::integral_constant<T, k>, PriorityTag<1>) noexcept
    {
      return _binomial(std::integral_constant<T, n>{}, std::integral_constant<T, n-k>{}, PriorityTag<42>{});
    }

    template<class T, T n, T k>
    constexpr inline static auto _binomial (std::integral_constant<T, n>, std::integral_constant<T, k>, PriorityTag<0>) noexcept
    {
      return std::integral_constant<T, decltype( _binomial(std::integral_constant<T, n-1>{}, std::integral_constant<T, k-1>{}, PriorityTag<42>{}))::value * n / k>{};
    }

  } // namespace Impl


  template< class T, T n, T k>
  constexpr inline static auto binomial (std::integral_constant<T, n>, std::integral_constant<T, k>)
  {
    return Impl::_binomial(std::integral_constant<T, n>{}, std::integral_constant<T, k>{}, PriorityTag<42>{});
  }

  template<class T, class U,
           std::enable_if_t<std::is_same<std::decay_t<T>, std::decay_t<U>>::value, int> = 0,
           std::enable_if_t<std::is_integral<std::decay_t<T>>::value, int> = 0>
  constexpr inline static auto binomial (T&& n, U&& k) noexcept
    -> std::decay_t< T >
  {
    if ( 2*k > n )
      return binomial( n, n-k );

    if( k < 0)
      return 0;

    std::decay_t< T > bin = 0;
    for( auto&& i : range( n-k, n ) )
      bin *= i+1;
    return bin / factorial( k );
  }

} // namespace Dune

#endif // #ifndef DUNE_ACTIPIPE_COMMON_MATHUTILITIES_HH
