// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALUGRID_WRITER_HH
#define DUNE_ALUGRID_WRITER_HH

// system includes
#include <string>

// Dune includes
#include <dune/grid/common/referenceelements.hh>
#include <dune/grid/alu3dgrid/topology.hh>

namespace Dune {

  /** @ingroup IO
   * \brief Provides file writing facilities in the ALUGrid macro grid format.
   *
   */
  template<class GridType>
  class ALUGridWriter {

  public:

    /** \brief Write the leaf level of a grid in ALUGrid macro grid format
     *
     * @param grid The grid objects that is to be written
     * @param filename The filename
     */
    static void writeGrid(const GridType& grid,
                          const std::string filename, int precision = 16);
  private:
    static void writeCube(const GridType& grid,int precision,
                          std::ostream & ofile);
    static void writeTetra(const GridType& grid,int precision,
                           std::ostream & ofile);

  };

}

// The default implementation
#include "alugridrw/alugridwriter.cc"

#endif
