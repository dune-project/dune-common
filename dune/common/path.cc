// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <algorithm>
#include <iterator>
#include <string>

#include <dune/common/exceptions.hh>
#include <dune/common/stringutility.hh>
#include <dune/common/path.hh>

namespace Dune {
  /**
   * @addtogroup Path Filesystem Paths
   * @ingroup Common
   * @{
   */

  /**
   * @file
   * @author Jö Fahlke <jorrit@jorrit.de>
   * @brief Utilities for handling filesystem paths
   */

  //! concatenate two paths
  std::string concatPaths(const std::string& base, const std::string& p) {
    if(p == "") return base;
    if(p[0] == '/') return p;
    if(base == "") return p;
    if(hasSuffix(base, "/")) return base+p;
    else return base+'/'+p;
  }

  //! sanitize a path for further processing
  std::string processPath(const std::string& p) {
    std::string result = p;
    std::string::size_type src, dst;

    // append a '/' to non-empty paths
    if(result != "") result += '/';

    // each path component now has a trailing '/'

    // collapse any occurrence of multiple '/' to a single '/'
    dst = src = 0;
    while(src < result.size()) {
      result[dst] = result[src];
      ++src;
      if(result[dst] == '/')
        while(src < result.size() && result[src] == '/')
          ++src;
      ++dst;
    }
    result.resize(dst);

    // the path is now free of multiple '/' in a row

    // collapse any occurrence of "/./" to "/"
    dst = src = 0;
    while(src < result.size()) {
      result[dst] = result[src];
      ++src;
      if(result[dst] == '/')
        while(src+1 < result.size() && result[src] == '.' &&
              result[src+1] == '/')
          src+=2;
      ++dst;
    }
    result.resize(dst);

    // there may be at most one leading "./".  If so, remove it
    if(hasPrefix(result, "./")) result.erase(0, 2);

    // the path is now free of "."-components

    // remove any "<component>/../" pairs
    src = 0;
    while(true) {
      src = result.find("/../", src);
      if(src == std::string::npos)
        break;
      for(dst = src; dst > 0 && result[dst-1] != '/'; --dst) ;
      if(result.substr(dst, src-dst) == "..") {
        // don't remove "../../"
        src += 3;
        continue;
      }
      if(dst == src)
        // special case: "<component>" is the empty component.  This means we
        // found a leading "/../" in an absolute path, remove "/.."
        result.erase(0, 3);
      else {
        // remove "<component>/../".
        result.erase(dst, src-dst+4);
        src = dst;
        // try to back up one character so we are at a '/' instead of at the
        // beginning of a component
        if(src > 0) --src;
      }
    }

    // absolute paths are now free of ".." components, and relative paths
    // contain only leading ".." components

    return result;
  }

  //! check whether the given path indicates that it is a directory
  bool pathIndicatesDirectory(const std::string& p) {
    if(p == "") return true;
    if(p == ".") return true;
    if(p == "..") return true;
    if(hasSuffix(p, "/")) return true;
    if(hasSuffix(p, "/.")) return true;
    if(hasSuffix(p, "/..")) return true;
    else return false;
  }

  //! pretty print path
  std::string prettyPath(const std::string& p, bool isDirectory) {
    std::string result = processPath(p);
    // current directory
    if(result == "") return ".";
    // root directory
    if(result == "/") return result;

    // remove the trailing '/' for now
    result.resize(result.size()-1);

    // if the result ends in "..", we don't need to append '/' to make clear
    // it's a directory
    if(result == ".." || hasSuffix(result, "/.."))
      return result;

    // if it's a directory, tuck the '/' back on
    if(isDirectory) result += '/';

    return result;
  }

  //! pretty print path
  std::string prettyPath(const std::string& p) {
    return prettyPath(p, pathIndicatesDirectory(p));
  }

  //! compute a relative path between two paths
  std::string relativePath(const std::string& newbase, const std::string& p)
  {
    bool absbase = hasPrefix(newbase, "/");
    bool absp    = hasPrefix(p, "/");
    if(absbase != absp)
      DUNE_THROW(NotImplemented, "relativePath: paths must be either both "
                 "relative or both absolute: newbase=\"" << newbase << "\" "
                 "p=\"" << p << "\"");

    std::string mybase = processPath(newbase);
    std::string myp =    processPath(p);

    // remove as many matching leading components as possible
    // determine prefix length
    std::string::size_type preflen = 0;
    while(preflen < mybase.size() && preflen < myp.size() &&
          mybase[preflen] == myp[preflen])
      ++preflen;
    // backup to the beginning of the component
    while(preflen > 0 && myp[preflen-1] != '/')
      --preflen;
    mybase.erase(0, preflen);
    myp.erase(0,preflen);

    // if mybase contains leading ".." components, we're screwed
    if(hasPrefix(mybase, "../"))
      DUNE_THROW(NotImplemented, "relativePath: newbase has too many leading "
                 "\"..\" components: newbase=\"" << newbase << "\" "
                 "p=\"" << p << "\"");

    // count the number of components in mybase
    typedef std::iterator_traits<std::string::iterator>::difference_type
    count_t;
    count_t count = std::count(mybase.begin(), mybase.end(), '/');

    std::string result;
    // prefix with that many leading components
    for(count_t i = 0; i < count; ++i)
      result += "../";
    // append what is left of p
    result += myp;

    return result;
  }

  /** @} group Path */
}
