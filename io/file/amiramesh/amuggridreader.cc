// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// /////////////////////////////////////////////////////////////////////////
// Specialization of the AmiraMesh reader for UGGrid<3,3> and UGGrid<2,2>
// /////////////////////////////////////////////////////////////////////////

#include <dune/config.h>
#include <dune/io/file/amiramesh/amuggridreader.hh>
#include <dune/grid/uggrid.hh>
#include <dune/grid/common/boundarysegment.hh>
#include <dune/common/stdstreams.hh>

#include <amiramesh/AmiraMesh.h>

#include <vector>
#include <algorithm>

#if defined HAVE_PSURFACE
#include <psurface.h>
#endif


// //////////////////////////////////////////////////
// //////////////////////////////////////////////////
#ifdef HAVE_PSURFACE
class PSurfaceBoundarySegment : public Dune::BoundarySegment<3>
{
public:
  PSurfaceBoundarySegment(int domain, int triangle)
    : domain_(domain),
      triangle_(triangle)
  {}

  virtual Dune::FieldVector<double, 3> operator()(const Dune::FieldVector<double,2>& local) const {

    Dune::FieldVector<double, 3> result;

    // Transform local to barycentric coordinates
    double barCoords[2];
    const double A[4] = {-1, 1, 0, -1};
    const double b[2] = {1, 0};

    // barCoords = A*param + b;
    barCoords[0] = A[0]*local[0] + A[2]*local[1];
    barCoords[1] = A[1]*local[0] + A[3]*local[1];

    barCoords[0] += b[0];
    barCoords[1] += b[1];

    psurface::CallPositionParametrizationForDomain(domain_, triangle_, barCoords, &result[0]);

    return result;
  }

  int domain_;
  int triangle_;
};
#endif // #define HAVE_PSURFACE


// Create the domain from an explicitly given boundary description
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::createDomain(UGGrid<3,3>& grid,
                                                             const std::string& filename)
{
#ifdef HAVE_PSURFACE
  int point[3] = {-1, -1, -1};

  std::string domainname = filename;
  /* Load data */
  if (psurface::LoadMesh(domainname.c_str(), filename.c_str()) != psurface::OK)
    DUNE_THROW(IOError, "Error in AmiraMeshReader<Dune::UGGrid<3,3> >::createDomain:"
               << "Domain file could not be opened!");

  if (psurface::StartEditingDomain(domainname.c_str()) != psurface::OK)
    DUNE_THROW(IOError, "Error in AmiraMeshReader<Dune::UGGrid<3,3> >::createDomain:"
               << "StartEditing failed!");

  // All further queries to the psurface library refer to the most recently
  // loaded parametrization.

  int noOfSegments = psurface::GetNoOfSegments();
  if(noOfSegments <= 0)
    DUNE_THROW(IOError, "no segments found");

  int noOfNodes = psurface::GetNoOfNodes();
  if(noOfNodes <= 0)
    DUNE_THROW(IOError, "No nodes found");

  static int boundaryNumber = 0;

  for(int i = 0; i < noOfSegments; i++) {

    // Gets the vertices of a boundary segment
    psurface::GetNodeNumbersOfSegment(point, i);

    std::vector<unsigned int> vertices(3);
    vertices[0] = point[0];
    vertices[1] = point[1];
    vertices[2] = point[2];

    grid.insertBoundarySegment(vertices,
                               new PSurfaceBoundarySegment(boundaryNumber,i));

  }
  boundaryNumber++;
  std::cout << noOfSegments << " segments from psurface file " << filename
            << " created!" << std::endl;

#endif // #define HAVE_PSURFACE

}


void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::read(Dune::UGGrid<3,3>& grid,
                                                     const std::string& filename,
                                                     const std::string& domainFilename)
{
#ifndef HAVE_PSURFACE
  DUNE_THROW(IOError, "Dune has not been built with support for the "
             << " psurface library!");
#else
  dverb << "This is the AmiraMesh reader for UGGrid<3,3>!" << std::endl;

  // Officially start grid creation
  grid.createBegin();

  // /////////////////////////////////////////////////////
  // Load the AmiraMesh file
  // /////////////////////////////////////////////////////
  AmiraMesh* am = AmiraMesh::read(filename.c_str());

  if(!am)
    DUNE_THROW(IOError, "Could not open AmiraMesh file " << filename);

  if (am->findData("Hexahedra", HxINT32, 8, "Nodes")) {

    // Load a domain from an AmiraMesh hexagrid file
    std::cout << "Hexahedral grids with a parametrized boundary are not supported!" << std::endl;
    std::cout << "I will therefore ignore the boundary parametrization." << std::endl;

  } else {

    // Load domain from an AmiraMesh tetragrid file
    createDomain(grid, domainFilename);

  }

  // read and build the grid
  buildGrid(grid, am);
#endif // #define HAVE_PSURFACE
}


void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::read(Dune::UGGrid<3,3>& grid,
                                                     const std::string& filename)
{
  dverb << "This is the AmiraMesh reader for UGGrid<3,3>!" << std::endl;

  // Officially start grid creation
  grid.createBegin();

  // /////////////////////////////////////////////////////
  // Load the AmiraMesh file
  // /////////////////////////////////////////////////////
  AmiraMesh* am = AmiraMesh::read(filename.c_str());

  if(!am)
    DUNE_THROW(IOError, "read: Could not open AmiraMesh file " << filename);

  buildGrid(grid, am);

}


void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::buildGrid(UGGrid<3,3>& grid,
                                                          AmiraMesh* am)
{
  bool isTetraGrid = am->findData("Tetrahedra", HxINT32, 4, "Nodes");

  float* am_node_coordinates_float = NULL;
  double* am_node_coordinates_double = NULL;

  // get the different data fields
  AmiraMesh::Data* am_coordinateData =  am->findData("Nodes", HxFLOAT, 3, "Coordinates");
  if (am_coordinateData)
    am_node_coordinates_float = (float*) am_coordinateData->dataPtr();
  else {
    am_coordinateData =  am->findData("Nodes", HxDOUBLE, 3, "Coordinates");
    if (am_coordinateData)
      am_node_coordinates_double = (double*) am_coordinateData->dataPtr();
    else
      DUNE_THROW(IOError, "No vertex coordinates found in the file!");

  }


  AmiraMesh::Data* elementData = (isTetraGrid)
                                 ? am->findData("Tetrahedra", HxINT32, 4, "Nodes")
                                 : am->findData("Hexahedra", HxINT32, 8, "Nodes");

  int*  elemData = (int*)elementData->dataPtr();

  int noOfNodes = am->nElements("Nodes");

  std::cout << "AmiraMesh has " << noOfNodes << " total nodes." << std::endl;

  int noOfElem = (isTetraGrid)
                 ? am->nElements("Tetrahedra")
                 : am->nElements("Hexahedra");

  // //////////////////////////////////////
  //   Insert interior nodes
  // //////////////////////////////////////
  assert(am_node_coordinates_float || am_node_coordinates_double);
  for(int i=0; i < noOfNodes; i++) {

    FieldVector<double,3> nodePos;

    if (am_node_coordinates_float) {
      nodePos[0] = am_node_coordinates_float[3*i];
      nodePos[1] = am_node_coordinates_float[3*i+1];
      nodePos[2] = am_node_coordinates_float[3*i+2];
    } else {
      nodePos[0] = am_node_coordinates_double[3*i];
      nodePos[1] = am_node_coordinates_double[3*i+1];
      nodePos[2] = am_node_coordinates_double[3*i+2];
    }

    grid.insertVertex(nodePos);

  }

  /* all inner nodes are inserted , now we insert the elements */
  int noOfCreatedElem = 0;

  for(int i=0; i < noOfElem; i++) {

    const int* thisElem = elemData + (i* ((isTetraGrid) ? 4 : 8));

    if (isTetraGrid) {

      int numberOfCorners = 4;
      std::vector<unsigned int> cornerIDs(numberOfCorners);

      for (int j=0; j<numberOfCorners; j++)
        cornerIDs[j] = elemData[numberOfCorners*i+j]-1;

      grid.insertElement(GeometryType(GeometryType::simplex,3), cornerIDs);

    } else {

      if (thisElem[2]==thisElem[3]
          && thisElem[4]==thisElem[5]
          && thisElem[5]==thisElem[6]
          && thisElem[6]==thisElem[7]) {

        // Tetrahedron
        std::vector<unsigned int> cornerIDs(4);

        cornerIDs[0] = thisElem[0]-1;
        cornerIDs[1] = thisElem[1]-1;
        cornerIDs[2] = thisElem[2]-1;
        cornerIDs[3] = thisElem[4]-1;

        grid.insertElement(GeometryType(GeometryType::simplex,3), cornerIDs);

      }else if (thisElem[4]==thisElem[5] && thisElem[5]==thisElem[6]
                && thisElem[6]==thisElem[7]) {

        // Pyramid
        std::vector<unsigned int> cornerIDs(5);

        cornerIDs[0] = thisElem[0]-1;
        cornerIDs[1] = thisElem[1]-1;
        cornerIDs[2] = thisElem[2]-1;
        cornerIDs[3] = thisElem[3]-1;
        cornerIDs[4] = thisElem[4]-1;

        grid.insertElement(GeometryType(GeometryType::pyramid,3), cornerIDs);

      } else if (thisElem[1]==thisElem[2] && thisElem[5]==thisElem[6]) {

        // Prism
        std::vector<unsigned int> cornerIDs(6);

        cornerIDs[0] = thisElem[0]-1;
        cornerIDs[1] = thisElem[1]-1;
        cornerIDs[2] = thisElem[3]-1;
        cornerIDs[3] = thisElem[4]-1;
        cornerIDs[4] = thisElem[5]-1;
        cornerIDs[5] = thisElem[7]-1;

        grid.insertElement(GeometryType(GeometryType::prism,3), cornerIDs);

      } else if (thisElem[2]==thisElem[3] && thisElem[6]==thisElem[7]) {

        std::vector<unsigned int> cornerIDs(6);

        cornerIDs[0] = thisElem[0]-1;
        cornerIDs[1] = thisElem[1]-1;
        cornerIDs[2] = thisElem[2]-1;
        cornerIDs[3] = thisElem[4]-1;
        cornerIDs[4] = thisElem[5]-1;
        cornerIDs[5] = thisElem[6]-1;

        grid.insertElement(GeometryType(GeometryType::prism,3), cornerIDs);

      } else {

        int numberOfCorners = 8;
        std::vector<unsigned int> cornerIDs(numberOfCorners);

        cornerIDs[0] = elemData[numberOfCorners*i+0]-1;
        cornerIDs[1] = elemData[numberOfCorners*i+1]-1;
        cornerIDs[2] = elemData[numberOfCorners*i+3]-1;
        cornerIDs[3] = elemData[numberOfCorners*i+2]-1;
        cornerIDs[4] = elemData[numberOfCorners*i+4]-1;
        cornerIDs[5] = elemData[numberOfCorners*i+5]-1;
        cornerIDs[6] = elemData[numberOfCorners*i+7]-1;
        cornerIDs[7] = elemData[numberOfCorners*i+6]-1;

        grid.insertElement(GeometryType(GeometryType::cube,3), cornerIDs);

      }

    }

    noOfCreatedElem++;

  }

  if(noOfElem != noOfCreatedElem)
    DUNE_THROW(IOError, "Inserting element failed");

  std::cout << "AmiraMesh reader: " << noOfCreatedElem << " elements created.\n";
  delete am;

  grid.createEnd();

}

/*********************************************************************************/
/*********************************************************************************/
/*                                                                               */
/* The code for reading 2D grids from an AmiraMesh file into a UGGrid object     */
/*                                                                               */
/*********************************************************************************/
/*********************************************************************************/


/** \todo Extend this such that it also reads double vertex positions */
void Dune::AmiraMeshReader<Dune::UGGrid<2,2> >::read(Dune::UGGrid<2,2>& grid,
                                                     const std::string& filename)
{
  dverb << "Loading 2D Amira mesh " << filename << std::endl;

  // Officially start grid creation
  grid.createBegin();

  // /////////////////////////////////////////////////////
  // Load the AmiraMesh file
  // /////////////////////////////////////////////////////
  std::auto_ptr<AmiraMesh> am(AmiraMesh::read(filename.c_str()));

  if(am.operator->() == NULL)
    DUNE_THROW(IOError, "2d AmiraMesh reader: File '" << filename << "' could not be read!");

  // Determine whether grid contains only triangles
  bool containsOnlyTriangles = am->findData("Triangles", HxINT32, 3, "Nodes");

  // get the different data fields
  AmiraMesh::Data* am_coordinateData =  am->findData("Nodes", HxFLOAT, 2, "Coordinates");
  if (!am_coordinateData)
    DUNE_THROW(IOError, "2D AmiraMesh loader: field 'Nodes' not found!");

  float* am_node_coordinates = (float*) am_coordinateData->dataPtr();

  // Get the element list
  int*  elemData = 0;

  if (containsOnlyTriangles) {
    AmiraMesh::Data* triangleData = am->findData("Triangles", HxINT32, 3, "Nodes");
    if (triangleData)
      elemData = (int*)triangleData->dataPtr();
    else
      DUNE_THROW(IOError, "2D AmiraMesh loader: field 'Triangles' not found!");

  } else {
    AmiraMesh::Data* elementData = am->findData("Quadrilaterals", HxINT32, 4, "Nodes");
    if (elementData) {
      elemData = (int*)elementData->dataPtr();
    } else
      DUNE_THROW(IOError, "2D AmiraMesh loader: field 'Quadrilaterals' not found!");
  }


  int noOfNodes = am->nElements("Nodes");
  int noOfElem  = (containsOnlyTriangles) ? am->nElements("Triangles") : am->nElements("Quadrilaterals");

  std::cout << "AmiraMesh contains " << noOfNodes << " nodes and "
            << noOfElem << " elements\n";

  // Insert interior nodes
  for(int i=0; i < noOfNodes; i++) {

    FieldVector<double,2> nodePos;
    nodePos[0] = am_node_coordinates[2*i];
    nodePos[1] = am_node_coordinates[2*i+1];

    grid.insertVertex(nodePos);

  }

  int noOfCreatedElem = 0;
  for (int i=0; i < noOfElem; i++) {

    if (containsOnlyTriangles) {

      std::vector<unsigned int> cornerIDs(3);

      /* only triangles */
      cornerIDs[0] = elemData[3*i]-1;
      cornerIDs[1] = elemData[3*i+1]-1;
      cornerIDs[2] = elemData[3*i+2]-1;

      grid.insertElement(GeometryType(GeometryType::simplex,2), cornerIDs);

    } else {

      if (elemData[4*i+2]==elemData[4*i+3]) {
        // Triangle within a quadrilateral grid file
        std::vector<unsigned int> cornerIDs(3);

        /* a triangle in a quadrilateral file */
        cornerIDs[0] = elemData[4*i]-1;
        cornerIDs[1] = elemData[4*i+1]-1;
        cornerIDs[2] = elemData[4*i+2]-1;

        grid.insertElement(GeometryType(GeometryType::simplex,2), cornerIDs);

      } else {

        std::vector<unsigned int> cornerIDs(4);

        /* a true quadrilateral */
        cornerIDs[0] = elemData[4*i]-1;
        cornerIDs[1] = elemData[4*i+1]-1;
        cornerIDs[2] = elemData[4*i+3]-1;
        cornerIDs[3] = elemData[4*i+2]-1;

        grid.insertElement(GeometryType(GeometryType::cube,2), cornerIDs);

      }

    }

    noOfCreatedElem++;

  }

  std::cout << "amiraloadmesh: " << noOfCreatedElem << " elements created" << std::endl;

  grid.createEnd();

}
