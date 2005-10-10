// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_RAWDATAWRITER_HH
#define DUNE_RAWDATAWRITER_HH

#include <string>

namespace Dune {

  /** @ingroup IO
   * \brief Provides file writing facilities in the raw data format.
   *
   * Use it by calling the static method write().  Its default implementation
   * only yields a warning message.  Actual functionality is provided by
   * specializations of the methods.  So far, the following ones are
   * available:
   * <ul>
   * <li> SimpleGrid<3,3> </li>
   * </ul>
   */
  template<class GRID, class T>
  class RawDataWriter {

  public:

    /** \brief The method that does the writing.
     *
     * @param grid The grid objects that is to be written
     * @param sol  Data that should be written along with the grid
     * @param filename The filename
     */
    static void write(const GRID& grid,
                      const Array<T>& sol,
                      const std::string& filename);


    RawDataWriter() {}

  };

}

// Default implementation
template<class GRID, class T>
void Dune::RawDataWriter<GRID, T>::write(const GRID& grid,
                                         const Array<T>& sol,
                                         const std::string& filename)
{
  printf("No rawdata writing has been implemented for this grid type!\n");
}

#include "rawdata/rdsimplegridwriter.cc"

#endif
