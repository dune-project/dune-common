// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_ARRAY_HH
#define DUNE_ARRAY_HH

/** \file
    \brief Fallback implementation of the std::array class (a static array)
 */

#include <array>

#include <dune/common/streamoperators.hh>
#include <dune/common/std/make_array.hh>

namespace Dune
{
  /** @addtogroup Common

     @{
   */

  // pull in default implementation
  using std::array;

  using Dune::Std::make_array;

  //! Create an array and fill it with copies of the provided value.
  /**
   * \note This method is Dune-specific and not part of any C++ standard.
   */
  template<typename T, std::size_t n>
  std::array<T,n> fill_array(const T& t)
  {
    std::array<T,n> r;
    r.fill(t);
    return r;
  }

  /** @} */

} // end namespace Dune

#endif
