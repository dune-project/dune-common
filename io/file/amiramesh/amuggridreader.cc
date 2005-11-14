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

#include <dune/grid/uggrid/boundaryextractor.hh>
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

    AmiraCallPositionParametrizationForDomain(domain_, triangle_, barCoords, &result[0]);

    return result;
  }

  int domain_;
  int triangle_;
};
#endif // #define HAVE_PSURFACE


// Create the domain from an explicitly given boundary description
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::createDomain(UGGrid<3,3>& grid,
                                                             const std::string& filename,
                                                             std::vector<int>& isBoundaryNode)
{
#ifdef HAVE_PSURFACE
  int point[3] = {-1, -1, -1};

  std::string domainname = filename;
  /* Load data */
  if(AmiraLoadMesh(domainname.c_str(), filename.c_str()) != AMIRA_OK)
    DUNE_THROW(IOError, "Error in AmiraMeshReader<Dune::UGGrid<3,3> >::createDomain:"
               << "Domain file could not be opened!");

  if(AmiraStartEditingDomain(domainname.c_str()) != AMIRA_OK)
    DUNE_THROW(IOError, "Error in AmiraMeshReader<Dune::UGGrid<3,3> >::createDomain:"
               << "StartEditing failed!");

  /* Alle weiteren Anfragen an die Bibliothek beziehen sich jetzt auf das eben
     geladen Gebiet. Maessig elegant, sollte aber gehen */

  int noOfSegments = AmiraGetNoOfSegments();
  if(noOfSegments <= 0)
    DUNE_THROW(IOError, "no segments found");

  int noOfNodes = AmiraGetNoOfNodes();
  if(noOfNodes <= 0)
    DUNE_THROW(IOError, "No nodes found");

  grid.createDomain(noOfNodes, noOfSegments);

  static int boundaryNumber = 0;

  for(int i = 0; i < noOfSegments; i++) {

    // Gets the vertices of a boundary segment
    AmiraGetNodeNumbersOfSegment(point, i);

    std::vector<int> vertices(3);
    vertices[0] = point[0];
    vertices[1] = point[1];
    vertices[2] = point[2];

    grid.insertBoundarySegment(vertices,
                               new PSurfaceBoundarySegment(boundaryNumber,i));

  }
  boundaryNumber++;
  std::cout << noOfSegments << " segments created!" << std::endl;

#endif // #define HAVE_PSURFACE

}


// Create the domain by extracting the boundary of the given grid
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::createDomain(UGGrid<3,3>& grid,
                                                             AmiraMesh* am,
                                                             std::vector<int>& isBoundaryNode)
{
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
    else {
      delete am;
      DUNE_THROW(IOError, "No vertex coordinates found in the file!");
    }
  }

  AmiraMesh::Data* tetrahedronData = am->findData("Tetrahedra", HxINT32, 4, "Nodes");
  int*  elemData         = (int*)tetrahedronData->dataPtr();
  int noOfElem = am->nElements("Tetrahedra");

  // Extract boundary faces
  std::vector<FieldVector<int, 3> > faceList;
  BoundaryExtractor::detectBoundarySegments(elemData, noOfElem, faceList);

  if(faceList.size() == 0)
  {
    delete am;
    DUNE_THROW(IOError, "createTetraDomain: no segments found");
  }

  int nBndSegments = faceList.size();

  dverb << faceList.size() << " boundary segments found!" << std::endl;

  int noOfNodes = am->nElements("Nodes");

  int nBndNodes = BoundaryExtractor::detectBoundaryNodes(faceList, noOfNodes, isBoundaryNode);
  if(nBndNodes <= 0)
    DUNE_THROW(IOError, "createDomain: no boundary nodes found");

  dverb << nBndNodes << " boundary nodes found!" << std::endl;

  grid.createDomain(nBndNodes, faceList.size());

  for(int i = 0; i < nBndSegments; i++) {

    std::vector<FieldVector<double,3> > coordinates(3);
    coordinates[0][0] = am_node_coordinates_float[3*faceList[i][0] + 0];
    coordinates[0][1] = am_node_coordinates_float[3*faceList[i][0] + 1];
    coordinates[0][2] = am_node_coordinates_float[3*faceList[i][0] + 2];
    coordinates[1][0] = am_node_coordinates_float[3*faceList[i][1] + 0];
    coordinates[1][1] = am_node_coordinates_float[3*faceList[i][1] + 1];
    coordinates[1][2] = am_node_coordinates_float[3*faceList[i][1] + 2];
    coordinates[2][0] = am_node_coordinates_float[3*faceList[i][2] + 0];
    coordinates[2][1] = am_node_coordinates_float[3*faceList[i][2] + 1];
    coordinates[2][2] = am_node_coordinates_float[3*faceList[i][2] + 2];

    std::vector<int> vertices(3);
    vertices[0] = isBoundaryNode[faceList[i][0]];
    vertices[1] = isBoundaryNode[faceList[i][1]];
    vertices[2] = isBoundaryNode[faceList[i][2]];

    grid.insertLinearSegment(vertices, coordinates);

  }

  std::cout << nBndSegments << " segments created!" << std::endl;
}


void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::read(Dune::UGGrid<3,3>& grid,
                                                     const std::string& filename,
                                                     const std::string& domainFilename)
{
#ifndef HAVE_PSURFACE
  DUNE_THROW(IOError, "Dune has not been built with support for the "
             << "AmiraMesh-Parametrization library!");
#else
  dverb << "This is the AmiraMesh reader for UGGrid<3,3>!" << std::endl;

  // Officially start grid creation
  grid.createbegin();

  // /////////////////////////////////////////////////////
  // Load the AmiraMesh file
  // /////////////////////////////////////////////////////
  AmiraMesh* am = AmiraMesh::read(filename.c_str());
  std::vector<int> isBoundaryNode;

  if(!am)
    DUNE_THROW(IOError, "Could not open AmiraMesh file " << filename);

  if (am->findData("Hexahedra", HxINT32, 8, "Nodes")) {

    // Load a domain from an AmiraMesh hexagrid file
    std::cout << "Hexahedral grids with a parametrized boundary are not supported!" << std::endl;
    std::cout << "I will therefore ignore the boundary parametrization." << std::endl;
    createHexaDomain(grid, am, isBoundaryNode);

  } else {

    // Load domain from an AmiraMesh hexagrid file
    createDomain(grid, domainFilename, isBoundaryNode);

  }

  // read and build the grid
  buildGrid(grid, am, isBoundaryNode);
#endif // #define HAVE_PSURFACE
}


void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::read(Dune::UGGrid<3,3>& grid,
                                                     const std::string& filename)
{
  dverb << "This is the AmiraMesh reader for UGGrid<3,3>!" << std::endl;

  // Officially start grid creation
  grid.createbegin();

  // /////////////////////////////////////////////////////
  // Load the AmiraMesh file
  // /////////////////////////////////////////////////////
  AmiraMesh* am = AmiraMesh::read(filename.c_str());
  std::vector<int> isBoundaryNode;

  if(!am)
    DUNE_THROW(IOError, "read: Could not open AmiraMesh file " << filename);

  if (am->findData("Hexahedra", HxINT32, 8, "Nodes")) {

    // Create a domain from an AmiraMesh hexagrid file
    createHexaDomain(grid, am, isBoundaryNode);

  } else {

    // Create a domain from an AmiraMesh tetragrid file
    createDomain(grid, am, isBoundaryNode);

  }

  buildGrid(grid, am, isBoundaryNode);

}


void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::buildGrid(UGGrid<3,3>& grid,
                                                          AmiraMesh* am,
                                                          std::vector<int>& isBoundaryNode)
{
  bool isTetraGrid = am->findData("Tetrahedra", HxINT32, 4, "Nodes");

  // call configureCommand and newCommand
  grid.makeNewUGMultigrid();

  // If we are in a parallel setting and we are _not_ the master
  // process we can exit here.
#ifdef ModelP
  if (PPIF::me!=0) {
    delete(am);
    grid.createend();
    return;
  }
#endif

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

  int*  elemData         = (int*)elementData->dataPtr();

  /*
     All Boundary nodes are assumed to be inserted already.
     We just have to insert the inner nodes and the elements
   */
  int maxBndNodeID = grid.numNodesOnBoundary_-1;

  dverb << "Already " << maxBndNodeID+1 << " nodes existing" << std::endl;

  int noOfNodes = am->nElements("Nodes");

  std::cout << "AmiraMesh has " << noOfNodes << " total nodes." << std::endl;

  int noOfElem = (isTetraGrid)
                 ? am->nElements("Tetrahedra")
                 : am->nElements("Hexahedra");

  // ///////////////////////////////////////////////////////////////////////////////
  //   The isBoundaryNode array contains information about which of all nodes is
  //   on the grid boundary.  If we have read a parametrized domain description,
  //   though, the array is empty.  To be able to proceed without to much hassle
  //   we fill the array now.
  // ///////////////////////////////////////////////////////////////////////////////
  if (isBoundaryNode.size()==0) {
    int i=0;
    isBoundaryNode.resize(noOfNodes);
    for (; i<=maxBndNodeID; i++)
      isBoundaryNode[i] = i;
    for (; i<noOfNodes; i++)
      isBoundaryNode[i] = -1;
  }

  // //////////////////////////////////////
  //   Insert interior nodes
  // //////////////////////////////////////
  assert(am_node_coordinates_float || am_node_coordinates_double);
  for(int i=0; i < noOfNodes; i++) {

    if (isBoundaryNode[i] != -1)
      continue;

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

    isBoundaryNode[i] = ++maxBndNodeID;

  }



  /* all inner nodes are inserted , now we insert the elements */
  int noOfCreatedElem = 0;

  for(int i=0; i < noOfElem; i++) {

    const int* thisElem = elemData + (i* ((isTetraGrid) ? 4 : 8));

    if (isTetraGrid) {

      int numberOfCorners = 4;
      std::vector<unsigned int> cornerIDs(numberOfCorners);

      for (int j=0; j<numberOfCorners; j++)
        cornerIDs[j] = isBoundaryNode[elemData[numberOfCorners*i+j]-1];

      grid.insertElement(simplex, cornerIDs);

    } else {

      if (thisElem[2]==thisElem[3]
          && thisElem[4]==thisElem[5]
          && thisElem[5]==thisElem[6]
          && thisElem[6]==thisElem[7]) {

        // Tetrahedron
        std::vector<unsigned int> cornerIDs(4);

        cornerIDs[0] = isBoundaryNode[thisElem[0]-1];
        cornerIDs[1] = isBoundaryNode[thisElem[1]-1];
        cornerIDs[2] = isBoundaryNode[thisElem[2]-1];
        cornerIDs[3] = isBoundaryNode[thisElem[4]-1];

        grid.insertElement(simplex, cornerIDs);

      }else if (thisElem[4]==thisElem[5] && thisElem[5]==thisElem[6]
                && thisElem[6]==thisElem[7]) {

        // Pyramid
        std::vector<unsigned int> cornerIDs(5);

        cornerIDs[0] = isBoundaryNode[thisElem[0]-1];
        cornerIDs[1] = isBoundaryNode[thisElem[1]-1];
        cornerIDs[2] = isBoundaryNode[thisElem[2]-1];
        cornerIDs[3] = isBoundaryNode[thisElem[3]-1];
        cornerIDs[4] = isBoundaryNode[thisElem[4]-1];

        grid.insertElement(pyramid, cornerIDs);

      } else if (thisElem[1]==thisElem[2] && thisElem[5]==thisElem[6]) {

        // Prism
        std::vector<unsigned int> cornerIDs(6);

        cornerIDs[0] = isBoundaryNode[thisElem[0]-1];
        cornerIDs[1] = isBoundaryNode[thisElem[1]-1];
        cornerIDs[2] = isBoundaryNode[thisElem[3]-1];
        cornerIDs[3] = isBoundaryNode[thisElem[4]-1];
        cornerIDs[4] = isBoundaryNode[thisElem[5]-1];
        cornerIDs[5] = isBoundaryNode[thisElem[7]-1];

        grid.insertElement(prism, cornerIDs);

      } else if (thisElem[2]==thisElem[3] && thisElem[6]==thisElem[7]) {

        std::vector<unsigned int> cornerIDs(6);

        cornerIDs[0] = isBoundaryNode[thisElem[0]-1];
        cornerIDs[1] = isBoundaryNode[thisElem[1]-1];
        cornerIDs[2] = isBoundaryNode[thisElem[2]-1];
        cornerIDs[3] = isBoundaryNode[thisElem[4]-1];
        cornerIDs[4] = isBoundaryNode[thisElem[5]-1];
        cornerIDs[5] = isBoundaryNode[thisElem[6]-1];

        grid.insertElement(prism, cornerIDs);

      } else {

        int numberOfCorners = 8;
        std::vector<unsigned int> cornerIDs(numberOfCorners);

        cornerIDs[0] = isBoundaryNode[elemData[numberOfCorners*i+0]-1];
        cornerIDs[1] = isBoundaryNode[elemData[numberOfCorners*i+1]-1];
        cornerIDs[2] = isBoundaryNode[elemData[numberOfCorners*i+3]-1];
        cornerIDs[3] = isBoundaryNode[elemData[numberOfCorners*i+2]-1];
        cornerIDs[4] = isBoundaryNode[elemData[numberOfCorners*i+4]-1];
        cornerIDs[5] = isBoundaryNode[elemData[numberOfCorners*i+5]-1];
        cornerIDs[6] = isBoundaryNode[elemData[numberOfCorners*i+7]-1];
        cornerIDs[7] = isBoundaryNode[elemData[numberOfCorners*i+6]-1];

        grid.insertElement(cube, cornerIDs);

      }

    }

    noOfCreatedElem++;

  }

  if(noOfElem != noOfCreatedElem)
    DUNE_THROW(IOError, "Inserting element failed");

  std::cout << "AmiraMesh reader: " << noOfCreatedElem << " elements created.\n";
  delete am;

  grid.createend();

}



/*****************************************************************/
/* Read the UGGrid from an AmiraMesh Hexagrid file               */
/*****************************************************************/


void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::createHexaDomain(UGGrid<3,3>& grid,
                                                                 AmiraMesh* am,
                                                                 std::vector<int>& isBoundaryNode)
{
  const int DIMWORLD = 3;

  // get the different data fields
  float* am_node_coordinates_float = NULL;
  double* am_node_coordinates_double = NULL;

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

  AmiraMesh::Data* hexahedronData = am->findData("Hexahedra", HxINT32, 8, "Nodes");
  int*  elemData         = (int*)hexahedronData->dataPtr();
  int noOfElem = am->nElements("Hexahedra");

  // Extract boundary faces
  std::vector<FieldVector<int, 4> > faceList;
  BoundaryExtractor::detectBoundarySegments(elemData, noOfElem, faceList);

  if(faceList.size() == 0)
    DUNE_THROW(IOError, "CreateHexaDomain: no boundary segments extracted");

  // Count the number of triangular and quadrilateral boundary segments
  int numTriangles = 0;
  int numQuads     = 0;
  for (unsigned int i=0; i<faceList.size(); i++) {
    if (faceList[i][3]==-1)
      numTriangles++;
    else
      numQuads++;
  }

  dverb << faceList.size() << " boundary segments found!" << std::endl;

  int noOfNodes = am->nElements("Nodes");

  int nBndNodes = BoundaryExtractor::detectBoundaryNodes(faceList, noOfNodes, isBoundaryNode);
  if(nBndNodes <= 0)
    DUNE_THROW(IOError, "createHexaDomain: no nodes found");

  dverb << nBndNodes << " boundary nodes found!" << std::endl;

  grid.createDomain(nBndNodes, faceList.size());

  for(int i = 0; i < numTriangles+numQuads; i++) {

    if (faceList[i][3]!=-1) {

      // change around ordering
      std::vector<int> vertices(4);
      vertices[0] = isBoundaryNode[faceList[i][3]];
      vertices[1] = isBoundaryNode[faceList[i][2]];
      vertices[2] = isBoundaryNode[faceList[i][1]];
      vertices[3] = isBoundaryNode[faceList[i][0]];

      std::vector<FieldVector<double,3> > coordinates(4);

      for (int j=0; j<4; j++)
        for (int k=0; k<3; k++)
          coordinates[j][k] = am_node_coordinates_float[DIMWORLD*faceList[i][3-j] + k];

      grid.insertLinearSegment(vertices, coordinates);

    } else {

      std::vector<int> vertices(3);
      vertices[0] = isBoundaryNode[faceList[i][0]];
      vertices[1] = isBoundaryNode[faceList[i][1]];
      vertices[2] = isBoundaryNode[faceList[i][2]];

      std::vector<FieldVector<double,3> > coordinates(3);

      for (int j=0; j<3; j++)
        for (int k=0; k<3; k++)
          coordinates[j][k] = am_node_coordinates_float[DIMWORLD*faceList[i][j] + k];

      grid.insertLinearSegment(vertices, coordinates);

    }

  }

  std::cout << numTriangles << " triangular and " << numQuads << " quadrilateral segments created!" << std::endl;

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
  grid.createbegin();

  // /////////////////////////////////////////////////////
  // Load the AmiraMesh file
  // /////////////////////////////////////////////////////
  /** \todo Use an auto_ptr here */
  AmiraMesh* am = AmiraMesh::read(filename.c_str());

  if(!am)
    DUNE_THROW(IOError, "2d AmiraMesh reader: File '" << filename << "' could not be read!");

  // ///////////////////////////////////////
  // Extract domain from the grid file
  // ///////////////////////////////////////

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
      elemData         = (int*)triangleData->dataPtr();
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

  // Extract boundary segments
  std::vector<FieldVector<int, 2> > boundary_segments;
  BoundaryExtractor::detectBoundarySegments(elemData, noOfElem, boundary_segments, containsOnlyTriangles);
  if (boundary_segments.size() == 0) {
    delete am;
    DUNE_THROW(IOError, "2d AmiraMesh reader: couldn't extract any boundary segments!");
  }

  int noOfBSegments = boundary_segments.size();

  dverb << noOfBSegments << " Boundary segments found!" << std::endl;

  // extract boundary nodes
  std::vector<int> isBoundaryNode;
  BoundaryExtractor::detectBoundaryNodes(boundary_segments, noOfNodes, isBoundaryNode);
  if (isBoundaryNode.size() == 0) {
    delete am;
    DUNE_THROW(IOError, "2d AmiraMesh reader: couldn't extract any boundary nodes!");
  }

  int noOfBNodes = 0;
  for (int i=0; i<noOfNodes; i++) {
    if (isBoundaryNode[i] != -1)
      noOfBNodes++;
  }

  dverb << noOfBNodes << " boundary nodes found!" << std::endl;

  grid.createDomain(noOfBNodes, noOfBSegments);

  for(int i=0; i<noOfBSegments; i++) {

    const FieldVector<int, 2>& thisEdge = boundary_segments[i];

    std::vector<FieldVector<double,2> > coordinates(2);
    coordinates[0][0] = am_node_coordinates[2*thisEdge[0]];
    coordinates[0][1] = am_node_coordinates[2*thisEdge[0]+1];
    coordinates[1][0] = am_node_coordinates[2*thisEdge[1]];
    coordinates[1][1] = am_node_coordinates[2*thisEdge[1]+1];

    std::vector<int> vertices(2);
    vertices[0] = isBoundaryNode[thisEdge[0]];
    vertices[1] = isBoundaryNode[thisEdge[1]];

    grid.insertLinearSegment(vertices, coordinates);

  }

  // call configureCommand and newCommand
  grid.makeNewUGMultigrid();

  // If we are in a parallel setting and we are _not_ the master
  // process we can exit here.
#ifdef ModelP
  if (PPIF::me!=0) {
    delete(am);
    grid.createend();
    return;
  }
#endif

  /*
     All Boundary nodes are  assumed to be inserted already.
     We just have to insert the inner nodes and the elements
   */
  int maxBndNodeID = grid.numNodesOnBoundary_-1;

  std::cout << "AmiraMesh has " << noOfNodes << " nodes." << std::endl;

  // Insert interior nodes
  for(int i=0; i < noOfNodes; i++) {

    if (isBoundaryNode[i] != -1)
      continue;

    FieldVector<double,2> nodePos;
    nodePos[0] = am_node_coordinates[2*i];
    nodePos[1] = am_node_coordinates[2*i+1];

    grid.insertVertex(nodePos);

    isBoundaryNode[i] = ++maxBndNodeID;

  }

  int noOfCreatedElem = 0;
  for (int i=0; i < noOfElem; i++) {

    if (containsOnlyTriangles) {

      std::vector<unsigned int> cornerIDs(3);

      /* only triangles */
      cornerIDs[0] = isBoundaryNode[elemData[3*i]-1];
      cornerIDs[1] = isBoundaryNode[elemData[3*i+1]-1];
      cornerIDs[2] = isBoundaryNode[elemData[3*i+2]-1];

      grid.insertElement(simplex, cornerIDs);

    } else {

      if (elemData[4*i+2]==elemData[4*i+3]) {
        // Triangle within a quadrilateral grid file
        std::vector<unsigned int> cornerIDs(3);

        /* only quadrilaterals */
        cornerIDs[0] = isBoundaryNode[elemData[4*i]-1];
        cornerIDs[1] = isBoundaryNode[elemData[4*i+1]-1];
        cornerIDs[2] = isBoundaryNode[elemData[4*i+2]-1];

        grid.insertElement(simplex, cornerIDs);

      } else {

        std::vector<unsigned int> cornerIDs(4);

        /* only quadrilaterals */
        cornerIDs[0] = isBoundaryNode[elemData[4*i]-1];
        cornerIDs[1] = isBoundaryNode[elemData[4*i+1]-1];
        cornerIDs[2] = isBoundaryNode[elemData[4*i+3]-1];
        cornerIDs[3] = isBoundaryNode[elemData[4*i+2]-1];

        grid.insertElement(cube, cornerIDs);

      }

    }

    noOfCreatedElem++;

  }

  std::cout << "amiraloadmesh: " << noOfCreatedElem << " elements created" << std::endl;

  delete am;

  grid.createend();

}
