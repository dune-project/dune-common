// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_STRINGUTILITY_HH
#define DUNE_COMMON_STRINGUTILITY_HH

/** \file
    \brief Miscellaneous helper stuff
 */

#include <cstddef>
#include <cstring>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <memory>
#include <string>
#include <new>

#include <dune/common/exceptions.hh>


namespace Dune {

   /**
    * @addtogroup StringUtilities
    *
    * @{
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
   * The container must support the begin() and size() methods and the
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

  /**
   * \brief Format values according to printf format string
   *
   * \param s The format string to be used
   * \param args The valued to be formatted
   *
   * This is a wrapper to std::snprintf that provides
   * overflow save printf functionality. For up to 1000
   * characters a static buffer is used. If this is not sufficient
   * a dynamic buffer of appropriate size is allocated.
   */
  template<class... T>
  static std::string formatString(const std::string& s, const T&... args)
  {
    static const int bufferSize=1000;
    char buffer[bufferSize];

    // try to format with static buffer
    int r = std::snprintf(buffer, bufferSize, s.c_str(), args...);

    // negative return values correspond to errors
    if (r<0)
      DUNE_THROW(Dune::Exception,"Could not convert format string using given arguments.");

    // if buffer was large enough return result as string
    if (r<bufferSize)
      return std::string(buffer);

    // if buffer was to small allocate a larger buffer using
    // the predicted size hint (+1 for the terminating 0-byte).
    int dynamicBufferSize = r+1;

    std::unique_ptr<char[]> dynamicBuffer;
    try {
      dynamicBuffer = std::make_unique<char[]>(dynamicBufferSize);
    }
    catch (const std::bad_alloc&) {
      DUNE_THROW(Dune::Exception,"Could allocate large enough dynamic buffer in formatString.");
    }

    // convert and check for errors again
    r = std::snprintf(dynamicBuffer.get(), dynamicBufferSize, s.c_str(), args...);
    if (r<0)
      DUNE_THROW(Dune::Exception,"Could not convert format string using given arguments.");

    // the new buffer should always be large enough
    assert(r<dynamicBufferSize);

    return std::string(dynamicBuffer.get());
  }
  /** @} */

}

#endif // DUNE_COMMON_STRINGUTILITY_HH
