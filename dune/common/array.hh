// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_ARRAY_HH
#define DUNE_ARRAY_HH

#ifndef DUNE_COMMON_ARRAY_HH_DISABLE_DEPRECATION_WARNING

#warning This header is deprecated.
#warning Instead of Dune::array, use std::array from <array>.
#warning Instead of Dune::make_array(), use Dune::Std::make_array() from <dune/common/std/make_array.hh>.
#warning Instead of Dune::fill_array(), use Dune::filledArray() from <dune/common/filledarray.hh>.

#endif // DUNE_DISABLE_ARRAY_HH_DEPRECATION_WARNING

/** \file
    \brief Fallback implementation of the std::array class (a static array)
 */

#include <array>

#include <dune/common/deprecated.hh>
#include <dune/common/streamoperators.hh>
#include <dune/common/std/make_array.hh>

namespace Dune
{
  /** @addtogroup Common

     @{
   */

  // pull in default implementation
  // deprecation does not work:
  // DUNE_DEPRECATED_MSG("Use std::array from <array>")
  using std::array;

  // deprecation does not work:
  // DUNE_DEPRECATED_MSG("Use Dune::Std::make_array from <dune/common/std/make_array.hh>")
  using Dune::Std::make_array;

  //! Create an array and fill it with copies of the provided value.
  /**
   * \note This method is Dune-specific and not part of any C++ standard.
   *
   * \deprecated Use Dune::filledArray() from <dune/common/filledarray.hh>.
   *
   * \ingroup CxxUtilities
   */
  template<typename T, std::size_t n>
  DUNE_DEPRECATED_MSG("Use Dune::filledArray() from <dune/common/filledarray.hh>")
  std::array<T,n> fill_array(const T& t)
  {
    std::array<T,n> r;
    r.fill(t);
    return r;
  }

  /** @} */

} // end namespace Dune

#endif
