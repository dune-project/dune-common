// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_AMIRAMESHREADER_HH__
#define __DUNE_AMIRAMESHREADER_HH__

#include <string>
#include "../../common/array.hh"

namespace Dune {

  /** @ingroup IO
   * \brief Provides file reading facilities in the AmiraMesh format.
   *
   */
  template<class GRID, class T>
  class AmiraMeshReader {

  public:

    /** \brief The method that does the writing.
     *
     * @param grid The grid objects that is to be written
     * @param sol  Data that should be written along with the grid
     * @param filename The filename
     */
    static void read(GRID& grid,
                     const std::string& filename);


    AmiraMeshReader() {}

  };

}

// Default implementation
template<class GRID, class T>
void Dune::AmiraMeshReader<GRID, T>::read(GRID& grid,
                                          const std::string& filename)
{
  printf("No AmiraMesh reading has been implemented for this grid type!\n");
}


// the amiramesh reader for UGGrid
#ifdef HAVE_UG
#include "amuggridreader.cc"
#endif

#endif
