// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTA_WRITER_HH
#define DUNE_ALBERTA_WRITER_HH

#include <string>

namespace Dune {

  /** @ingroup IO
   * \brief Provides file writing facilities in the Alberta grid format.
   *
   */
  template<class GridType>
  class AlbertaWriter {

  public:

    /** \brief Write the leaf level of a grid in Alberta format
     *
     * @param grid The grid objects that is to be written
     * @param filename The filename
     */
    static void writeGrid(const GridType& grid,
                          const std::string& filename);

  };

}

// The default implementation
#include "alberta/albertawriter.cc"

#endif
