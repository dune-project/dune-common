// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_STRINGUTILITY_HH
#define DUNE_COMMON_STRINGUTILITY_HH

/** \file
    \brief Miscellaneous helper stuff
 */

#include <cstddef>
#include <cstring>
#include <algorithm>

namespace Dune {

   /** @addtogroup Common

          @{
   */


  /** \brief Check whether a character container has a given prefix
   *
   * The container must support the begin() and size() methods.
   */
  template<typename C>
  bool hasPrefix(const C& c, const char* prefix) {
    std::size_t len = std::strlen(prefix);
    return c.size() >= len &&
           std::equal(prefix, prefix+len, c.begin());
  }

  /** \brief Check whether a character container has a given suffix
   *
   * The container must support the the begin() and size() methods and the
   * const_iterator member type.
   *
   * \note This is slow for containers which don't have random access iterators.
   *       In the case of containers with bidirectional iterators, this
   *       slowness is unnecessary.
   */
  template<typename C>
  bool hasSuffix(const C& c, const char* suffix) {
    std::size_t len = std::strlen(suffix);
    if(c.size() < len) return false;
    typename C::const_iterator it = c.begin();
    std::advance(it, c.size() - len);
    return std::equal(suffix, suffix+len, it);
  }

  /** @} */

}

#endif // DUNE_COMMON_STRINGUTILITY_HH