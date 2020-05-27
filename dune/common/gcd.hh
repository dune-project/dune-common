// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GCD_HH
#define DUNE_GCD_HH
#warning "This header is deprecated and will be removed after Dune release 2.8. Use std::gcd instead"

#include <numeric>

namespace Dune
{
  /**
   * @brief Calculator of the greatest common divisor.
   */
  template<long a, long b>
  struct [[deprecated("Will be removed after Dune 2.8. Use std::gcd from <numeric> instead!")]] Gcd
  {
    /**
     * @brief The greatest common divisior of a and b. */
    constexpr static long value = std::gcd(a,b);
  };

  /**
   * @}
   */
}

#endif
