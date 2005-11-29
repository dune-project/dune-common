// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_AMIRAMESH_UGGRID_READER_HH
#define DUNE_AMIRAMESH_UGGRID_READER_HH

/**
   \file
   \brief Specialization of the AmiraMesh reader for UGGrid<3,3> and UGGrid<2,2>
 */


#include <dune/grid/uggrid.hh>
#include <dune/io/file/amirameshreader.hh>


class AmiraMesh;

namespace Dune {

  /** \brief Read a 3d coarse grid from an AmiraMesh file into a UGGrid
   */
  template<>
  class AmiraMeshReader<UGGrid<3,3> > {

  public:

    /** \brief Read a grid from an AmiraMesh file */
    static void read(UGGrid<3,3>& grid,
                     const std::string& filename);

    /** \brief Read a grid with a parametrized boundary

       UGGrid supports grids where each grid boundary segment carries a
       function describing the true shape of the boundary segment.
       This information will the be considered when refining the grid.

       In
       <em>'Sander, Krause, Automatic Construction of Boundary Parametrizations
       for Geometric Multigrid Solvers, CVS, 2005'</em>,
       the authors describe a
       way to automatically build such boundary descriptions.  Their
       file format can be read by this routine to be used with a UGGrid.

       \attention This is a somewhat experimental feature.  To use it, you
       have to have the psurface library and build Dune with --with-psurface.
       Ask Oliver sander@math.fu-berlin.de for help.

       \param grid The grid to be read into
       \param gridFilename The AmiraMesh file containing the grid
       \param domainFilename The AmiraMesh file containing the boundary description
     */
    static void read(UGGrid<3,3>& grid,
                     const std::string& gridFilename,
                     const std::string& domainFilename);

  protected:

    static void createDomain(UGGrid<3,3>& grid,
                             const std::string& filename);

    static void buildGrid(UGGrid<3,3>& grid,
                          AmiraMesh* am);

  };



  /** \brief Read a 2d coarse grid from an AmiraMesh file into a UGGrid

     \warning UG internally requires that the coarse grid vertices are ordered
     such that the vertex list contains first all boundary vertices and the
     all interior vertices.  If the input data file is not already set up this
     way, the reader will reorder your vertices!
   */
  template<>
  class AmiraMeshReader<UGGrid<2,2> > {

  public:

    /** \brief Read a grid */
    static void read(UGGrid<2,2>& grid,
                     const std::string& filename);

  };

}   // end namespace Dune


#endif
