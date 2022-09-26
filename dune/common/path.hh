// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_PATH_HH
#define DUNE_COMMON_PATH_HH

#include <string>

namespace Dune {
  /**
   * @addtogroup Path
   * @{
   */

  /**
   * @file
   * @author Jö Fahlke <jorrit@jorrit.de>
   * @brief Utilities for handling filesystem paths
   */

  //! concatenate two paths
  /**
   * \param base The base path.
   * \param p    The path to concatenate onto base.
   *
   * If p is an absolute path, return p.  Otherwise return the
   * string-concatenation of base and path, possibly with a '/' in between, if
   * necessary.
   *
   * Some examples:
   * <table>
   * <tr><th> base     </th><th> p           </th><th> result      </th></tr>
   * <tr><td> anything </td><td> "/abs/path" </td><td> "/abs/path" </td></tr>
   * <tr><td> "a"      </td><td> "b"         </td><td> "a/b"       </td></tr>
   * <tr><td> "/a"     </td><td> "b"         </td><td> "/a/b"      </td></tr>
   * <tr><td> "a/"     </td><td> "b"         </td><td> "a/b"       </td></tr>
   * <tr><td> "a"      </td><td> "b/"        </td><td> "a/b/"      </td></tr>
   * <tr><td> ".."     </td><td> "b"         </td><td> "../b"      </td></tr>
   * <tr><td> "a"      </td><td> ".."        </td><td> "a/.."      </td></tr>
   * <tr><td> "."      </td><td> "b"         </td><td> "./b"       </td></tr>
   * <tr><td> "a"      </td><td> "."         </td><td> "a/."       </td></tr>
   * <tr><td> ""       </td><td> "b"         </td><td> "b"         </td></tr>
   * <tr><td> "a"      </td><td> ""          </td><td> "a"         </td></tr>
   * <tr><td> ""       </td><td> ""          </td><td> ""          </td></tr>
   * </table>
   *
   * If both base and p are sanitized as per processPath(), and if p does not
   * contain any leading "../", then the result will also be sanitized.
   */
  std::string concatPaths(const std::string& base, const std::string& p);

  //! sanitize a path for further processing
  /**
   * Sanitize the path as far as possible to make further processing easier.
   * The resulting path has the following properties:
   * <ul>
   * <li> The path is a series of components, each followed by a single '/'.
   * <li> An absolute path starts with an empty component followed by a '/',
   *      so its first character will be '/'.  This is the only case where an
   *      empty component can occur.
   * <li> The path does not contain any component ".".  Any such component in
   *      the input is removed.
   * <li> A ".." component may only occur in the following case:  A relative
   *      path may contain a series of ".." in the beginning.  Any other
   *      occurrences of ".." in the input is collapsed with a preceding
   *      component or simply removed if it is at the beginning of an absolute
   *      path.
   * </ul>
   *
   * \note The result is really meant for processing only since it has two
   *       unusual properties: First, any path denoting the current directory
   *       in the input, such as "." will result in an empty path "".  Second,
   *       any non-empty result path will have a trailing '/'.  For other
   *       uses, prettyPath() may be more appropriate.
   *
   * Some examples:
   * <table>
   * <tr><th> p        </th><th> result  </th></tr>
   * <tr><td> ""       </td><td> ""      </td></tr>
   * <tr><td> "."      </td><td> ""      </td></tr>
   * <tr><td> "./"     </td><td> ""      </td></tr>
   * <tr><td> "a/.."   </td><td> ""      </td></tr>
   * <tr><td> ".."     </td><td> "../"   </td></tr>
   * <tr><td> "../a"   </td><td> "../a/" </td></tr>
   * <tr><td> "a"      </td><td> "a/"    </td></tr>
   * <tr><td> "a//"    </td><td> "a/"    </td></tr>
   * <tr><td> "a///b"  </td><td> "a/b/"  </td></tr>
   * <tr><td> "/"      </td><td> "/"     </td></tr>
   * <tr><td> "/."     </td><td> "/"     </td></tr>
   * <tr><td> "/.."    </td><td> "/"     </td></tr>
   * <tr><td> "/a/.."  </td><td> "/"     </td></tr>
   * <tr><td> "/a"     </td><td> "/a/"   </td></tr>
   * <tr><td> "/a/"    </td><td> "/a/"   </td></tr>
   * <tr><td> "/../a/" </td><td> "/a/"   </td></tr>
   * </table>
   */
  std::string processPath(const std::string& p);

  //! check whether the given path indicates that it is a directory
  /**
   * In particular the following kinds of paths indicate a directory:
   * <ul>
   * <li> The empty path (denotes the current directory),
   * <li> any path with a trailing '/',
   * <li> any path whose last component is "." or "..".
   * </ul>
   */
  bool pathIndicatesDirectory(const std::string& p);

  //! pretty print path
  /**
   * \param p           Path to pretty-print.
   * \param isDirectory Whether to append a '/' to make clear this is a
   *                    directory.
   *
   * Pretty print the path.  This removes any duplicate '/' and any
   * superfluous occurrences of ".." and ".".  The resulting path will have a
   * trailing '/' if it is the root path or if isDirectory is true.  It will
   * however not have a trailing '/' if it is otherwise clear that it is a
   * directory -- i.e. if its last component is "." or "..".
   *
   * Some examples:
   * <table>
   * <tr><th> p        </th><th> isDirectory </th><th> result  </th></tr>
   * <tr><td> ""       </td><td> anything    </td><td> "."     </td></tr>
   * <tr><td> "."      </td><td> anything    </td><td> "."     </td></tr>
   * <tr><td> "./"     </td><td> anything    </td><td> "."     </td></tr>
   * <tr><td> "a/.."   </td><td> anything    </td><td> "."     </td></tr>
   * <tr><td> ".."     </td><td> anything    </td><td> ".."    </td></tr>
   * <tr><td> "../a"   </td><td> true        </td><td> "../a/" </td></tr>
   * <tr><td> "../a"   </td><td> false       </td><td> "../a"  </td></tr>
   * <tr><td> "a"      </td><td> true        </td><td> "a/"    </td></tr>
   * <tr><td> "a"      </td><td> false       </td><td> "a"     </td></tr>
   * <tr><td> "a//"    </td><td> true        </td><td> "a/"    </td></tr>
   * <tr><td> "a//"    </td><td> false       </td><td> "a"     </td></tr>
   * <tr><td> "a///b"  </td><td> true        </td><td> "a/b/"  </td></tr>
   * <tr><td> "a///b"  </td><td> false       </td><td> "a/b"   </td></tr>
   * <tr><td> "/"      </td><td> anything    </td><td> "/"     </td></tr>
   * <tr><td> "/."     </td><td> anything    </td><td> "/"     </td></tr>
   * <tr><td> "/.."    </td><td> anything    </td><td> "/"     </td></tr>
   * <tr><td> "/a/.."  </td><td> anything    </td><td> "/"     </td></tr>
   * <tr><td> "/a"     </td><td> true        </td><td> "/a/"   </td></tr>
   * <tr><td> "/a"     </td><td> false       </td><td> "/a"    </td></tr>
   * <tr><td> "/a/"    </td><td> true        </td><td> "/a/"   </td></tr>
   * <tr><td> "/a/"    </td><td> false       </td><td> "/a"    </td></tr>
   * <tr><td> "/../a/" </td><td> true        </td><td> "/a/"   </td></tr>
   * <tr><td> "/../a/" </td><td> false       </td><td> "/a"    </td></tr>
   * </table>
   */
  std::string prettyPath(const std::string& p, bool isDirectory);

  //! pretty print path
  /**
   * \param p Path to pretty-print.
   *
   * This is like prettyPath(const std::string& p, bool isDirectory) with
   * isDirectory automatically determined using pathIndicatesDirectory(p).
   */
  std::string prettyPath(const std::string& p);

  //! compute a relative path between two paths
  /**
   * \param newbase Base path for the resulting relative path.
   * \param p       Path re sulting path should resolve to, when taken
   *                reltively to newbase.
   *
   * Compute a relative path from newbase to p.  newbase is assumed to be a
   * directory.  p and newbase should either both be absolute, or both be
   * relative.  In the latter case they are assumed to both be relative to
   * the same unspecified directory.  The has the form of something sanitized
   * by processPath().
   *
   * \throw NotImplemented The condition that newbase and p must both be
   *                       relative or both be absolute does not hold.
   * \throw NotImplemented After sanitization newbase has more leading ".."
   *                       components than p.
   */
  std::string relativePath(const std::string& newbase, const std::string& p);

  /** @} group Path */
}

#endif // DUNE_COMMON_PATH_HH
