#ifndef DUNE_COMMON_STD_CMATH_HH
#define DUNE_COMMON_STD_CMATH_HH

#include <cmath>
#include <utility>

#if HAVE_QUADMATH
#include <quadmath.h>
#endif

namespace Dune
{
  namespace Std
  {
    using std::abs;
    using std::min;
    using std::max;
    using std::sqrt;
    using std::pow;


#if HAVE_QUADMATH
#if defined(__STRICT_ANSI__) || !defined(_GLIBCXX_USE_FLOAT128) || defined(__CUDACC__)
    // libstdc++ defines abs for __float128 if some flags are not set
    inline constexpr __float128 abs(__float128 x) { return x < 0 ? -x : x; }
#endif

    inline constexpr __float128 const& min(__float128 const& x, __float128 const& y) { return x < y ? x : y; }
    inline constexpr __float128 const& max(__float128 const& x, __float128 const& y) { return y < x ? x : y; }
    inline __float128 sqrt(__float128 const& x) { return sqrtq(x); }
    inline __float128 pow(__float128 const& x, __float128 const& p) { return powq(x,p); }

    inline __float128 abs(std::complex<__float128> x)
    {
      return sqrtq(x.real() * x.real() + x.imag() * x.imag());
    }
#endif

  } // namespace Std
} // namespace Dune

#endif // #ifndef DUNE_COMMON_STD_CMATH_HH
