// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_AMIRAMESH_WRITER_HH
#define DUNE_AMIRAMESH_WRITER_HH

#include <string>
#include <dune/istl/bvector.hh>

namespace Dune {

  /** @ingroup IO
   * \brief Provides file writing facilities in the AmiraMesh format.
   *
   */
  template<class GridType>
  class AmiraMeshWriter {

  public:

    /** \brief Writes a grid in AmiraMesh format
     *
     * @param grid The grid objects that is to be written
     * @param filename The filename
     */
    static void writeGrid(const GridType& grid,
                          const std::string& filename);

    /** \brief Writes a discrete function in AmiraMesh format
     *
     * @param f Function that should be written
     * @param filename The filename
     */
    template <class DiscFuncType>
    static void writeFunction(const DiscFuncType& f,
                              const std::string& filename);

    /** \brief Writes an ISTL block vector in AmiraMesh format */
    static void writeBlockVector(const GridType& grid,
                                 const Dune::BlockVector<Dune::FieldVector<double, 3> >& f,
                                 const std::string& filename);
    AmiraMeshWriter() {}

  };

}

// The default implementation
#include "amiramesh/amirameshwriter.cc"

// the amiramesh writer for SGrid
//#include "amiramesh/amsgridwriter.cc"

#endif
