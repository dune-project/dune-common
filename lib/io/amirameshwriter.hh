// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_AMIRAMESHWRITER_HH__
#define __DUNE_AMIRAMESHWRITER_HH__

#include <string>
#include "../../common/array.hh"

namespace Dune {

  /** @ingroup IO
   * \brief Provides file writing facilities in the AmiraMesh format.
   *
   * Use it by calling the static method write().  Its default implementation
   * only yields a warning message.  Actual functionality is provided by
   * specializations of the methods.  So far, the following ones are
   * available:
   * <ul>
   * <li> SimpleGrid<3,3>, with <code>double</code> data </li>
   * </ul>
   */
  template<class GRID, class T>
  class AmiraMeshWriter {

  public:

    /** \brief The method that does the writing.
     *
     * @param grid The grid objects that is to be written
     * @param sol  Data that should be written along with the grid
     * @param filename The filename
     */
    /** \todo The grid argument should be const */
    static void write(GRID& grid,
                      const Array<T>& sol,
                      const std::string& filename);

    AmiraMeshWriter() {}

  };

}

// The default implementation
#include "amirameshwriter.cc"

// the amiramesh writer for SimpleGrid
#include "amsimplegridwriter.cc"

#endif
