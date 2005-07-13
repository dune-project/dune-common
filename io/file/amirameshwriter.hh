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

    enum {dim = GridType::dimension};

    typedef typename GridType::template Codim<0>::LevelIterator ElementIterator;
    typedef typename GridType::template Codim<dim>::LevelIterator VertexIterator;
    typedef typename GridType::template Codim<0>::LeafIterator LeafIterator;

  public:

    /** \brief Write the leaf level of a grid in AmiraMesh format
     *
     * @param grid The grid objects that is to be written
     * @param filename The filename
     */
    static void writeGrid(const GridType& grid,
                          const std::string& filename);

    /** \brief Write one level of a grid in AmiraMesh format
     *
     * @param grid The grid objects that is to be written
     * @param filename The filename
     * @param level The level to be written
     */
    static void writeGrid(const GridType& grid,
                          const std::string& filename,
                          int level);

    /** \brief Writes a discrete function in AmiraMesh format
     *
     * @param f Function that should be written
     * @param filename The filename
     */
    template <class DiscFuncType>
    static void writeFunction(const DiscFuncType& f,
                              const std::string& filename);

    /** \brief Writes an ISTL block vector in AmiraMesh format */
    template <class VectorType>
    static void writeBlockVector(const GridType& grid,
                                 const VectorType& f,
                                 const std::string& filename);
    AmiraMeshWriter() {}

  };

}

// The default implementation
#include "amiramesh/amirameshwriter.cc"

// the amiramesh writer for SGrid
#include "amiramesh/amsgridwriter.cc"

#endif
