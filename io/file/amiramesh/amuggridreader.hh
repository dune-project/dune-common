// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// /////////////////////////////////////////////////////////////////////////
// Specialization of the AmiraMesh reader for UGGrid<3,3> and UGGrid<2,2>
// /////////////////////////////////////////////////////////////////////////

#include <dune/grid/uggrid.hh>
#include <dune/io/file/amirameshreader.hh>

#include <vector>


class AmiraMesh;

namespace Dune {

  template<>
  class AmiraMeshReader<UGGrid<3,3> > {

  public:

    static void read(UGGrid<3,3>& grid,
                     const std::string& filename);

    static void read(UGGrid<3,3>& grid,
                     const std::string& gridFilename,
                     const std::string& domainFilename);

  protected:

    static void CreateDomain(UGGrid<3,3>& grid,
                             const std::string& domainName,
                             const std::string& filename);

    static void CreateDomain(UGGrid<3,3>& grid,
                             const std::string& domainName,
                             AmiraMesh* am);

    static void buildGrid(UGGrid<3,3>& grid, AmiraMesh* am);

    static void createHexaDomain(UGGrid<3,3>& grid, AmiraMesh* am);

    static void detectBoundarySegments(int* elemData,
                                       int noOfElem,
                                       std::vector<FieldVector<int, 4> >& face_list);

    static void detectBoundarySegments(int* elemData,
                                       int noOfElem,
                                       std::vector<FieldVector<int, 3> >& face_list);

    AmiraMeshReader() {}

  };




  /*********************************************************************************/
  /*********************************************************************************/
  /*                                                                               */
  /* The code for reading 2D grids from an AmiraMesh file into a UGGrid object     */
  /*                                                                               */
  /*********************************************************************************/
  /*********************************************************************************/


  template<>
  class AmiraMeshReader<UGGrid<2,2> > {

  public:

    static void read(UGGrid<2,2>& grid,
                     const std::string& filename);

  protected:
    static void CreateDomain(UGGrid<2,2>& grid,
                             const std::string& domainName,
                             const std::string& filename);

    static void detectBoundarySegments(int* elemData,
                                       int numElems,
                                       std::vector<FieldVector<int, 2> >& face_list,
                                       bool containsOnlyTriangles);

  };

}
