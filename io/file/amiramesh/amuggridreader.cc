// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// /////////////////////////////////////////////////////////////////////////
// Specialization of the AmiraMesh reader for UGGrid<3,3> and UGGrid<2,2>
// /////////////////////////////////////////////////////////////////////////

#include <dune/config.h>
#include <dune/io/file/amiramesh/amuggridreader.hh>
#include <dune/grid/uggrid.hh>
#include <dune/common/stdstreams.hh>

#include <amiramesh/AmiraMesh.h>

#include <vector>
#include <algorithm>

#if defined HAVE_PSURFACE && defined _3
#include <parametrization/AmiraParamAccess.h>
#endif


#ifdef _3
// //////////////////////////////////////////////////
// //////////////////////////////////////////////////
#ifdef HAVE_PSURFACE
static int SegmentDescriptionByAmira(void *data, double *param, double *result)
{

  int domainNum = ((int*)data)[0];
  int triNum    = ((int*)data)[1];

  double barCoords[2];
  double A[4] = {-1, 1, 0, -1};
  double b[2] = {1, 0};

  // barCoords = A*param + b;
  barCoords[0] = A[0]*param[0] + A[2]*param[1];
  barCoords[1] = A[1]*param[0] + A[3]*param[1];

  barCoords[0] += b[0];
  barCoords[1] += b[1];

  AmiraCallPositionParametrizationForDomain(domainNum, triNum, barCoords, result);

  return 0;
}
#endif // #define HAVE_PSURFACE

/** This method implements a linear function in order to be able to
 *  work with straight line boundaries.
 *  We interpret data as a DOUBLE* to the world coordinates of the
 *  three endpoints.
 *
 * \todo This should actually be replaced by using LinearSegments
 * instead of BoundarySegments.  But LinearSegments are buggy in UG.
 */
static int linearSegmentDescription3d(void *data, double *param, double *result)
{
  Dune::FieldVector<double, 3> a,b,c,d;
  a[0] = ((double*)data)[0];
  a[1] = ((double*)data)[1];
  a[2] = ((double*)data)[2];
  b[0] = ((double*)data)[3];
  b[1] = ((double*)data)[4];
  b[2] = ((double*)data)[5];
  c[0] = ((double*)data)[6];
  c[1] = ((double*)data)[7];
  c[2] = ((double*)data)[8];
  d[0] = ((double*)data)[9];
  d[1] = ((double*)data)[10];
  d[2] = ((double*)data)[11];

  // linear interpolation
  for (int i=0; i<3; i++)
    result[i] = a[i] + param[0]*(b[i]-a[i]) + param[1]*(d[i]-a[i])
                + param[0]*param[1]*(a[i]+c[i]-b[i]-d[i]);

  return 0;
}

// The same thing for triangles
static int linearSegmentDescription3d_tri(void *data, double *param, double *result)
{
  Dune::FieldVector<double, 3> a,b,c;
  a[0] = ((double*)data)[0];
  a[1] = ((double*)data)[1];
  a[2] = ((double*)data)[2];
  b[0] = ((double*)data)[3];
  b[1] = ((double*)data)[4];
  b[2] = ((double*)data)[5];
  c[0] = ((double*)data)[6];
  c[1] = ((double*)data)[7];
  c[2] = ((double*)data)[8];

  // linear interpolation
  for (int i=0; i<3; i++)
    result[i] = a[i] + param[0]*(b[i]-a[i]) + param[1]*(c[i]-b[i]);

  return 0;
}

/** \todo This is quadratic --> very slow */
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::detectBoundarySegments(int* elemData,
                                                                       int numTetras,
                                                                       std::vector<FieldVector<int, 3> >& faceList)
{
  int i, j;

  static const int idx[][3] = {
    {3,2,0},{1,2,3},{1,3,0},{2,1,0}
  };

  faceList.resize(0);

  for (i=0; i<numTetras; i++) {

    for (int k=0; k<4; k++) {
      FieldVector<int, 3> v;
      v[0] = elemData[4*i+idx[k][0]];
      v[1] = elemData[4*i+idx[k][1]];
      v[2] = elemData[4*i+idx[k][2]];


      // Check if new face exists already in the list
      // (then it is no boundary face)
      for (j=0; j<(int)faceList.size(); j++) {

        const FieldVector<int, 3>& o = faceList[j];
        if ( (v[0]==o[0] && v[1]==o[1] && v[2]==o[2]) ||
             (v[0]==o[0] && v[1]==o[2] && v[2]==o[1]) ||
             (v[0]==o[1] && v[1]==o[0] && v[2]==o[2]) ||
             (v[0]==o[1] && v[1]==o[2] && v[2]==o[0]) ||
             (v[0]==o[2] && v[1]==o[0] && v[2]==o[1]) ||
             (v[0]==o[2] && v[1]==o[1] && v[2]==o[0]) ) {

          break;
        }

      }

      if (j<(int)faceList.size()) {
        // face has been found
        faceList[j] = faceList.back();
        faceList.pop_back();

      } else {

        // Insert k-th face of i-th tetrahedron into face list
        faceList.push_back(v);

      }
    }
  }

  // Switch from AmiraMesh numbering (1,2,3,...) to internal numbering (0,1,2,...)
  for (i=0; i<(int)faceList.size(); i++)
    for (j=0; j<3; j++)
      faceList[i][j]--;

}
#endif // #ifdef _3

template<int NUM_VERTICES>
static int detectBoundaryNodes(const std::vector< Dune::FieldVector<int, NUM_VERTICES> >& faceList,
                               int noOfNodes,
                               std::vector<int>& isBoundaryNode)
{
  isBoundaryNode.resize(noOfNodes);

  int UGNodeIdxCounter = 0;

  for (int i=0; i<noOfNodes; i++)
    isBoundaryNode[i] = -1;

  for (unsigned int i=0; i<faceList.size(); i++) {

    for (int j=0; j<NUM_VERTICES; j++)
      if (faceList[i][j]!=-1 && isBoundaryNode[faceList[i][j]] == -1)
        isBoundaryNode[faceList[i][j]] = 1;

  }

  for (unsigned int i=0; i<isBoundaryNode.size(); i++)
    if (isBoundaryNode[i] != -1)
      isBoundaryNode[i] = UGNodeIdxCounter++;

  return UGNodeIdxCounter;
}

#ifdef _3

// Create the domain from an explicitly given boundary description
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::createDomain(UGGrid<3,3>& grid,
                                                             const std::string& domainName,
                                                             const std::string& filename,
                                                             std::vector<int>& isBoundaryNode)
{
#ifdef HAVE_PSURFACE
  const int CORNERS_OF_BND_SEG = 4;
  int point[CORNERS_OF_BND_SEG] = {-1, -1, -1, -1};
  double alpha[2], beta[2];

  /* Load data */
  if(AmiraLoadMesh(domainName.c_str(), filename.c_str()) != AMIRA_OK)
    DUNE_THROW(IOError, "Error in AmiraMeshReader<Dune::UGGrid<3,3> >::createDomain:"
               << "Domain file could not be opened!");

  if(AmiraStartEditingDomain(domainName.c_str()) != AMIRA_OK)
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

  /* Wir brauchen eine Kugel, die das Gebiet komplett  enthaelt.  Diese Information
     wird für die UG-Graphik benoetigt, die Werte sind hier also komplett egal. */
  double radius = 1;
  double MidPoint[3] = {0, 0, 0};

  /* Jetzt geht es an's eingemachte. Zuerst wird ein neues gebiet konstruiert und
     in der internen UG Datenstruktur eingetragen */

  UG3d::domain* newDomain = (UG3d::domain*) UG3d::CreateDomain(domainName.c_str(),
                                                               MidPoint, radius,
                                                               noOfSegments, noOfNodes,
                                                               false );


  if (!newDomain)
    DUNE_THROW(IOError, "Could not create UG domain data structure!");

  /* Alle weiteren Aufrufe von 'CreateBoundarySegment' beziehen sich jetzt auf das eben
     erzeugte Gebiet */

  /*
     Liste der Nummern der Randsegmente herstellen, wird als user data an das jeweilige
     Segment weitergereicht
   */

  grid.extra_boundary_data_ =  ::malloc(noOfSegments*2*sizeof(int));
  if (grid.extra_boundary_data_ == NULL)
    DUNE_THROW(IOError, "Could not allocate extra_boundary_data");

  static int boundaryNumber = 0;

  for(int i = 0; i < noOfSegments; i++) {

    char segmentName[200];
    int left, right;

    // Gets the vertices of a boundary segment
    AmiraGetNodeNumbersOfSegment(point, i);

    if(sprintf(segmentName, "AmiraSegment %d", i) < 0)
      DUNE_THROW(IOError, "sprintf returned error code");

    /* left = innerRegion, right = outerRegion */
    AmiraGetLeftAndRightSideOfSegment(&left, &right, i);

    // The segment-describing methods gets the number of the boundary
    // and the number of the boundary segment in that boundary
    ((int*)grid.extra_boundary_data_)[2*i] = boundaryNumber;
    ((int*)grid.extra_boundary_data_)[2*i+1] = i;

    /* map Amira Material ID's to UG material ID's */
    left++;
    right++;

    alpha[0] = alpha[1] = 0;
    beta[0]  = beta[1]  = 1;

    if (UG3d::CreateBoundarySegment(segmentName,
                                    left,             /*id of left subdomain */
                                    right,            /*id of right subdomain*/
                                    i,                /*id of segment*/
                                    UG3d::NON_PERIODIC,
                                    1,              // only needed for UG graphics
                                    point,
                                    alpha, beta,
                                    SegmentDescriptionByAmira,
                                    ((int*)grid.extra_boundary_data_)+2*i
                                    )==NULL)
      DUNE_THROW(IOError, "UG3d::CreateBoundarySegment returned error code!");

  }
  boundaryNumber++;
  std::cout << noOfSegments << " segments created!" << std::endl;

#endif // #define HAVE_PSURFACE

}


// Create the domain by extracting the boundary of the given grid
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::createDomain(UGGrid<3,3>& grid,
                                                             const std::string& domainName,
                                                             AmiraMesh* am,
                                                             std::vector<int>& isBoundaryNode)
{
  const int CORNERS_OF_BND_SEG = 4;

  int point[CORNERS_OF_BND_SEG] = {-1, -1, -1, -1};
  double midPoint[3] = {0,0,0};
  const double radius = 1;

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
  detectBoundarySegments(elemData, noOfElem, faceList);

  if(faceList.size() == 0)
  {
    delete am;
    DUNE_THROW(IOError, "createTetraDomain: no segments found");
  }

  int nBndSegments = faceList.size();

  dverb << faceList.size() << " boundary segments found!" << std::endl;

  int noOfNodes = am->nElements("Nodes");

  int nBndNodes = detectBoundaryNodes(faceList, noOfNodes, isBoundaryNode);
  if(nBndNodes <= 0)
    DUNE_THROW(IOError, "createDomain: no boundary nodes found");

  dverb << nBndNodes << " boundary nodes found!" << std::endl;

  /* Zuerst wird ein neues gebiet konstruiert und
     in der internen UG Datenstruktur eingetragen */
  if (!UG3d::CreateDomain(domainName.c_str(),
                          midPoint, radius,
                          faceList.size(), nBndNodes,
                          false)) {
    delete am;
    DUNE_THROW(IOError, "Could not create UG domain data structure!");
  }

  /* Alle weiteren Aufrufe von 'CreateBoundarySegment' beziehen sich jetzt auf das eben
     erzeugte Gebiet */


  // The vertex coordinates get handed over as user data to the
  // respective boundary segment.
  grid.extra_boundary_data_ = ::malloc(nBndSegments*3*3*sizeof(double));

  if(grid.extra_boundary_data_ == NULL)
    DUNE_THROW(IOError, "createDomain: couldn't allocate extra_boundary_data");

  for(int i = 0; i < nBndSegments; i++) {

    char segmentName[20];

    // bordering subdomains
    int left = 1;
    int right = 2;

    point[0] = isBoundaryNode[faceList[i][0]];
    point[1] = isBoundaryNode[faceList[i][1]];
    point[2] = isBoundaryNode[faceList[i][2]];

    if(sprintf(segmentName, "Segment %d", i) < 0)
      DUNE_THROW(IOError, "createDomain: sprintf returned error value");

    /* left = innerRegion, right = outerRegion */
    ((double*)grid.extra_boundary_data_)[9*i+0] = am_node_coordinates_float[3*faceList[i][0] + 0];
    ((double*)grid.extra_boundary_data_)[9*i+1] = am_node_coordinates_float[3*faceList[i][0] + 1];
    ((double*)grid.extra_boundary_data_)[9*i+2] = am_node_coordinates_float[3*faceList[i][0] + 2];
    ((double*)grid.extra_boundary_data_)[9*i+3] = am_node_coordinates_float[3*faceList[i][1] + 0];
    ((double*)grid.extra_boundary_data_)[9*i+4] = am_node_coordinates_float[3*faceList[i][1] + 1];
    ((double*)grid.extra_boundary_data_)[9*i+5] = am_node_coordinates_float[3*faceList[i][1] + 2];
    ((double*)grid.extra_boundary_data_)[9*i+6] = am_node_coordinates_float[3*faceList[i][2] + 0];
    ((double*)grid.extra_boundary_data_)[9*i+7] = am_node_coordinates_float[3*faceList[i][2] + 1];
    ((double*)grid.extra_boundary_data_)[9*i+8] = am_node_coordinates_float[3*faceList[i][2] + 2];

#if 1
    double alpha[2] = {0, 0};
    double beta[2]  = {1, 1};

    if (UG3d::CreateBoundarySegment(segmentName,
                                    left,             /*id of left subdomain */
                                    right,            /*id of right subdomain*/
                                    i,                /*id of segment*/
                                    UG3d::NON_PERIODIC,
                                    1,              // resolution, needed for UG graphics
                                    point,
                                    alpha, beta,
                                    linearSegmentDescription3d_tri,
                                    ((double*)grid.extra_boundary_data_)+9*i
                                    )==NULL)
      DUNE_THROW(IOError, "Error calling CreateBoundarySegment");;

#else
    // It would be a lot smarter to use this way of describing
    // boundary segments.  But as of yet, UG crashes when using
    // linear segments.
    double paramCoords[3][2] = {{0,0}, {1,0}, {0,1}};
    if (UG3d::CreateLinearSegment(segmentName,
                                  left,             /*id of left subdomain */
                                  right,            /*id of right subdomain*/
                                  i,                /*id of segment*/
                                  4,                // Number of corners
                                  point,
                                  paramCoords
                                  )==NULL)
      DUNE_THROW(IOError, "Error calling CreateLinearSegment");
#endif
  }

  std::cout << nBndSegments << " segments created!" << std::endl;
}


/** \todo Clear grid before reading! */
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::read(Dune::UGGrid<3,3>& grid,
                                                     const std::string& filename,
                                                     const std::string& domainFilename)
{
#ifndef HAVE_PSURFACE
  DUNE_THROW(IOError, "Dune has not been built with support for the "
             << "AmiraMesh-Parametrization library!");
#else
  dverb << "This is the AmiraMesh reader for UGGrid<3,3>!" << std::endl;

  // /////////////////////////////////////////////////////
  // Load the AmiraMesh file
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

    // Construct a domain name from the multigrid name
    std::string domainName = grid.name() + "_Domain";

    //loaddomain $file @PARA_FILE $name @DOMAIN
    createDomain(grid, domainName, domainFilename, isBoundaryNode);

  }

  // read and build the grid
  buildGrid(grid, am, isBoundaryNode);
#endif // #define HAVE_PSURFACE
}


/** \todo Clear grid before reading! */
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::read(Dune::UGGrid<3,3>& grid,
                                                     const std::string& filename)
{
  dverb << "This is the AmiraMesh reader for UGGrid<3,3>!" << std::endl;

  // /////////////////////////////////////////////////////
  // Load the AmiraMesh file
  AmiraMesh* am = AmiraMesh::read(filename.c_str());
  std::vector<int> isBoundaryNode;

  if(!am)
    DUNE_THROW(IOError, "read: Could not open AmiraMesh file " << filename);

  if (am->findData("Hexahedra", HxINT32, 8, "Nodes")) {

    // Create a domain from an AmiraMesh hexagrid file
    createHexaDomain(grid, am, isBoundaryNode);

  } else {

    // Construct a domain name from the multigrid name
    std::string domainName = grid.name() + "_Domain";

    //loaddomain $file @PARA_FILE $name @DOMAIN
    createDomain(grid, domainName, am, isBoundaryNode);

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

  const int DIM = 3;

  int i;
  double nodePos[DIM];
  UG3d::NODE* theNode;

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
     All Boundary nodes are  assumed to be inserted already.
     We just have to insert the inner nodes and the elements
   */
  int maxBndNodeID = -1;
  for (theNode=UG_NS<3>::FirstNode(grid.multigrid_->grids[0]); theNode!=NULL; theNode=theNode->succ)
    maxBndNodeID = std::max(theNode->id, maxBndNodeID);

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
    isBoundaryNode.resize(noOfNodes);
    for (i=0; i<=maxBndNodeID; i++)
      isBoundaryNode[i] = i;
    for (; i<noOfNodes; i++)
      isBoundaryNode[i] = -1;
  }

  // //////////////////////////////////////
  //   Insert interior nodes
  // //////////////////////////////////////
  for(i = 0; i < noOfNodes; i++) {

    if (isBoundaryNode[i] != -1)
      continue;

    assert(am_node_coordinates_float || am_node_coordinates_double);

    if (am_node_coordinates_float) {
      nodePos[0] = am_node_coordinates_float[3*i];
      nodePos[1] = am_node_coordinates_float[3*i+1];
      nodePos[2] = am_node_coordinates_float[3*i+2];
    } else {
      nodePos[0] = am_node_coordinates_double[3*i];
      nodePos[1] = am_node_coordinates_double[3*i+1];
      nodePos[2] = am_node_coordinates_double[3*i+2];
    }

    if (UG3d::InsertInnerNode(grid.multigrid_->grids[0], nodePos) == NULL)
      DUNE_THROW(IOError, "inserting an inner node failed");

    isBoundaryNode[i] = ++maxBndNodeID;

  }



  /* all inner nodes are inserted , now we insert the elements */
  int noOfCreatedElem = 0;

  for(i=0; i < noOfElem; i++) {

    const int* thisElem = elemData + (i* ((isTetraGrid) ? 4 : 8));

    if (isTetraGrid) {
      int numberOfCorners = 4;
      int cornerIDs[numberOfCorners];

      for (int j=0; j<numberOfCorners; j++)
        cornerIDs[j] = isBoundaryNode[elemData[numberOfCorners*i+j]-1];

      if (UG3d::InsertElementFromIDs(grid.multigrid_->grids[0], numberOfCorners, cornerIDs, NULL) == NULL)
        DUNE_THROW(IOError, "Inserting element failed");

    } else {

      // Prism
      if (thisElem[1]==thisElem[2] && thisElem[5]==thisElem[6]) {

        int cornerIDs[6] = {isBoundaryNode[thisElem[0]-1],
                            isBoundaryNode[thisElem[1]-1],
                            isBoundaryNode[thisElem[3]-1],
                            isBoundaryNode[thisElem[4]-1],
                            isBoundaryNode[thisElem[5]-1],
                            isBoundaryNode[thisElem[7]-1]};

        if (UG3d::InsertElementFromIDs(grid.multigrid_->grids[0], 6, cornerIDs, NULL) == NULL)
          DUNE_THROW(IOError, "Inserting element failed");

      } else if (thisElem[2]==thisElem[3] && thisElem[6]==thisElem[7]) {

        int cornerIDs[6] = {isBoundaryNode[thisElem[0]-1],
                            isBoundaryNode[thisElem[1]-1],
                            isBoundaryNode[thisElem[2]-1],
                            isBoundaryNode[thisElem[4]-1],
                            isBoundaryNode[thisElem[5]-1],
                            isBoundaryNode[thisElem[6]-1]};

        if (UG3d::InsertElementFromIDs(grid.multigrid_->grids[0], 6, cornerIDs, NULL) == NULL)
          DUNE_THROW(IOError, "Inserting element failed");

      } else {

        int numberOfCorners = 8;
        int cornerIDs[numberOfCorners];

        for (int j=0; j<numberOfCorners; j++)
          cornerIDs[j] = isBoundaryNode[elemData[numberOfCorners*i+j]-1];

        if (UG3d::InsertElementFromIDs(grid.multigrid_->grids[0], numberOfCorners, cornerIDs, NULL) == NULL)
          DUNE_THROW(IOError, "Inserting element failed");

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

/** \todo This is quadratic --> very slow */
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::detectBoundarySegments(int* elemData,
                                                                       int numHexas,
                                                                       std::vector<FieldVector<int, 4> >& faceList)
{
  static const int idx[][4] = {
    {0,4,5,1},{1,5,6,2},{2,6,7,3},{3,7,4,0},{4,7,6,5},{1,2,3,0}
  };

  faceList.resize(0);

  for (int i=0; i<numHexas; i++) {

    for (int k=0; k<6; k++) {
      FieldVector<int, 4> v;
      v[0] = elemData[8*i+idx[k][0]] - 1;
      v[1] = elemData[8*i+idx[k][1]] - 1;
      v[2] = elemData[8*i+idx[k][2]] - 1;
      v[3] = elemData[8*i+idx[k][3]] - 1;

      // Don't do anything if the faces is degenerated to a line
      if ((v[0]==v[1] && v[2]==v[3]) ||
          (v[1]==v[2] && v[3]==v[0]) ||
          (v[0]==v[1] && v[1]==v[2]) ||
          (v[1]==v[2] && v[2]==v[3]) ||
          (v[2]==v[3] && v[3]==v[0]) ||
          (v[3]==v[0] && v[0]==v[1]))
        continue;

      // Check whether the faces is degenerated to a triangle

      // Check if new face exists already in the list
      // (then it is no boundary face)
      int j;
      for (j=0; j<(int)faceList.size(); j++) {

        const FieldVector<int, 4>& o = faceList[j];
        if ( (v[0]==o[0] && v[1]==o[1] && v[2]==o[2] && v[3]==o[3]) ||
             (v[0]==o[0] && v[1]==o[1] && v[2]==o[3] && v[3]==o[2]) ||
             (v[0]==o[0] && v[1]==o[2] && v[2]==o[1] && v[3]==o[3]) ||
             (v[0]==o[0] && v[1]==o[2] && v[2]==o[3] && v[3]==o[1]) ||
             (v[0]==o[0] && v[1]==o[3] && v[2]==o[1] && v[3]==o[2]) ||
             (v[0]==o[0] && v[1]==o[3] && v[2]==o[2] && v[3]==o[1]) ||

             (v[0]==o[1] && v[1]==o[0] && v[2]==o[2] && v[3]==o[3]) ||
             (v[0]==o[1] && v[1]==o[0] && v[2]==o[3] && v[3]==o[2]) ||
             (v[0]==o[1] && v[1]==o[2] && v[2]==o[0] && v[3]==o[3]) ||
             (v[0]==o[1] && v[1]==o[2] && v[2]==o[3] && v[3]==o[0]) ||
             (v[0]==o[1] && v[1]==o[3] && v[2]==o[0] && v[3]==o[2]) ||
             (v[0]==o[1] && v[1]==o[3] && v[2]==o[2] && v[3]==o[0]) ||

             (v[0]==o[2] && v[1]==o[0] && v[2]==o[1] && v[3]==o[3]) ||
             (v[0]==o[2] && v[1]==o[0] && v[2]==o[3] && v[3]==o[1]) ||
             (v[0]==o[2] && v[1]==o[1] && v[2]==o[0] && v[3]==o[3]) ||
             (v[0]==o[2] && v[1]==o[1] && v[2]==o[3] && v[3]==o[0]) ||
             (v[0]==o[2] && v[1]==o[3] && v[2]==o[0] && v[3]==o[1]) ||
             (v[0]==o[2] && v[1]==o[3] && v[2]==o[1] && v[3]==o[0]) ||

             (v[0]==o[3] && v[1]==o[0] && v[2]==o[1] && v[3]==o[2]) ||
             (v[0]==o[3] && v[1]==o[0] && v[2]==o[2] && v[3]==o[1]) ||
             (v[0]==o[3] && v[1]==o[1] && v[2]==o[0] && v[3]==o[2]) ||
             (v[0]==o[3] && v[1]==o[1] && v[2]==o[2] && v[3]==o[0]) ||
             (v[0]==o[3] && v[1]==o[2] && v[2]==o[0] && v[3]==o[1]) ||
             (v[0]==o[3] && v[1]==o[2] && v[2]==o[1] && v[3]==o[0]) )

          break;
      }



      if (j<(int)faceList.size()) {
        // face has been found
        faceList[j] = faceList.back();
        faceList.pop_back();

      } else {
        // Insert k-th face of i-th hexahedron into face list
        faceList.push_back(v);
      }
    }
  }

  // Rearranging faceList entries that represent triangles
  // They can be recognized by containing an index twice
  for (unsigned int i=0; i<faceList.size(); i++) {

    FieldVector<int,4>& f = faceList[i];

    if (f[0]==f[1]) {
      f[1] = f[2];
      f[2] = f[3];
      f[3] = -1;
    } else if (f[1]==f[2]) {
      f[2] = f[3];
      f[3] = -1;
    } else if (f[2]==f[3]) {
      f[3] = -1;
    } else if (f[0]==f[3]) {
      f[0] = f[1];
      f[1] = f[2];
      f[2] = f[3];
      f[3] = -1;
    } else if (f[0]==f[2] || f[1]==f[3])
      DUNE_THROW(IOError, "Impossible case in detectBoundarySegments");

  }

}


void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::createHexaDomain(UGGrid<3,3>& grid,
                                                                 AmiraMesh* am,
                                                                 std::vector<int>& isBoundaryNode)
{
  const int DIMWORLD = 3;

  int point[4] = {-1, -1, -1, -1};
  double midPoint[3] = {0,0,0};
  double radius = 1;

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
  detectBoundarySegments(elemData, noOfElem, faceList);

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

  int nBndNodes = detectBoundaryNodes(faceList, noOfNodes, isBoundaryNode);
  if(nBndNodes <= 0)
    DUNE_THROW(IOError, "createHexaDomain: no nodes found");

  dverb << nBndNodes << " boundary nodes found!" << std::endl;


  /* Zuerst wird ein neues gebiet konstruiert und
     in der internen UG Datenstruktur eingetragen */

  // Construct a domain name from the multigrid name
  std::string domainName = grid.name() + "_Domain";

  UG3d::domain* newDomain = (UG3d::domain*)UG3d::CreateDomain(domainName.c_str(),
                                                              midPoint, radius,
                                                              faceList.size(), nBndNodes,
                                                              false);

  if (!newDomain)
    DUNE_THROW(IOError, "createHexaDomain: UG3d::CreateDomain returned NULL");

  /* Alle weiteren Aufrufe von 'CreateBoundarySegment' beziehen sich jetzt auf das eben
     erzeugte Gebiet */

  /*
     Die Koordinaten der Eckknoten werden als user data an das jeweilige
     Segment weitergereicht.
   */
  grid.extra_boundary_data_ = ::malloc((3*numTriangles + 4*numQuads)*DIMWORLD*sizeof(double));

  if(grid.extra_boundary_data_ == NULL)
    DUNE_THROW(IOError, "createHexaDomain: couldn't allocate extra_boundary_data");

  double* boundaryCoords = (double*)grid.extra_boundary_data_;

  for(int i = 0; i < numTriangles+numQuads; i++) {

    char segmentName[20];

    if(sprintf(segmentName, "Segment %d", i) < 0)
      DUNE_THROW(IOError, "createHexaDomain: sprintf returned error value");

    // bordering subdomains
    int left = 1;
    int right = 2;

    if (faceList[i][3]!=-1) {

      // change around ordering
      point[0] = isBoundaryNode[faceList[i][3]];
      point[1] = isBoundaryNode[faceList[i][2]];
      point[2] = isBoundaryNode[faceList[i][1]];
      point[3] = isBoundaryNode[faceList[i][0]];

      for (int j=0; j<4; j++)
        for (int k=0; k<3; k++)
          boundaryCoords[3*j+k] = am_node_coordinates_float[DIMWORLD*faceList[i][3-j] + k];

      double alpha[2] = {0, 0};
      double beta[2]  = {1, 1};

      if (UG3d::CreateBoundarySegment(segmentName,
                                      left,               /*id of left subdomain */
                                      right,              /*id of right subdomain*/
                                      i,                  /*id of segment*/
                                      UG3d::NON_PERIODIC,
                                      1,                // resolution, used by the UG graphics system
                                      point,
                                      alpha, beta,
                                      linearSegmentDescription3d,
                                      boundaryCoords
                                      )==NULL)
        DUNE_THROW(IOError, "createHexaDomain: UG3d::CreateBoundarySegment returned NULL");

      boundaryCoords += 12;

    } else {

      point[0] = isBoundaryNode[faceList[i][0]];
      point[1] = isBoundaryNode[faceList[i][1]];
      point[2] = isBoundaryNode[faceList[i][2]];
      point[3] = -1;

      /* left = innerRegion, right = outerRegion */
      for (int j=0; j<3; j++)
        for (int k=0; k<3; k++)
          boundaryCoords[3*j+k]  = am_node_coordinates_float[DIMWORLD*faceList[i][j] + k];

      double alpha[2] = {0, 0};
      double beta[2]  = {1, 1};

      if (UG3d::CreateBoundarySegment(segmentName,
                                      left,               /*id of left subdomain */
                                      right,              /*id of right subdomain*/
                                      i,                  /*id of segment*/
                                      UG3d::NON_PERIODIC,
                                      1,                // resolution, used by the UG graphics system
                                      point,
                                      alpha, beta,
                                      linearSegmentDescription3d_tri,
                                      boundaryCoords
                                      )==NULL)
        DUNE_THROW(IOError, "createHexaDomain: UG3d::CreateBoundarySegment returned NULL");

      boundaryCoords += 9;
    }

  }

  std::cout << numTriangles << " triangular and " << numQuads << " quadrilateral segments created!" << std::endl;

}

#endif // #ifdef _3

/*********************************************************************************/
/*********************************************************************************/
/*                                                                               */
/* The code for reading 2D grids from an AmiraMesh file into a UGGrid object     */
/*                                                                               */
/*********************************************************************************/
/*********************************************************************************/

#ifdef _2

/** This method implements a linear function in order to be able to
 *  work with straight line boundaries.
 *  We interpret data as a DOUBLE* to the world coordinates of the
 *  two endpoints.
 */
static int linearSegmentDescription2d(void *data, double *param, double *result)
{
  double a[2], b[2];
  a[0] = ((double*)data)[0];
  a[1] = ((double*)data)[1];
  b[0] = ((double*)data)[2];
  b[1] = ((double*)data)[3];

  // linear interpolation
  result[0] = a[0] + (*param)*(b[0]-a[0]);
  result[1] = a[1] + (*param)*(b[1]-a[1]);

  return 0;
}

/** \todo This is quadratic --> very slow */
void Dune::AmiraMeshReader<Dune::UGGrid<2,2> >::detectBoundarySegments(int* elemData,
                                                                       int numElems,
                                                                       std::vector<FieldVector<int, 2> >& faceList,
                                                                       bool containsOnlyTriangles)
{
  int i, j;

  static const int triIdx[][2] = {
    {0,1},{1,2},{2,0}
  };

  static const int quadIdx[][2] = {
    {0,1},{1,2},{2,3},{3,0}
  };

  int verticesPerElement = (containsOnlyTriangles) ? 3 : 4;

  faceList.resize(0);

  for (i=0; i<numElems; i++) {

    for (int k=0; k<verticesPerElement; k++) {
      FieldVector<int, 2> v;
      if (containsOnlyTriangles) {
        v[0] = elemData[verticesPerElement*i+triIdx[k][0]];
        v[1] = elemData[verticesPerElement*i+triIdx[k][1]];
      } else {
        v[0] = elemData[verticesPerElement*i+quadIdx[k][0]];
        v[1] = elemData[verticesPerElement*i+quadIdx[k][1]];
      }
      if (v[0]==v[1])
        continue;

      // Check if new face exists already in the list
      // (then it is no boundary face
      for (j=0; j<(int)faceList.size(); j++) {

        const FieldVector<int, 2>& o = faceList[j];
        if ( (v[0]==o[0] && v[1]==o[1]) ||
             (v[0]==o[1] && v[1]==o[0]) ) {
          break;
        }

      }

      if (j<(int)faceList.size()) {
        // face has been found
        faceList[j] = faceList.back();
        faceList.pop_back();

      } else {

        // Insert k-th face of i-th element into face list
        faceList.push_back(v);

      }
    }
  }

  // Switch from AmiraMesh numbering (1,2,3,...) to internal numbering (0,1,2,...)
  for (i=0; i<(int)faceList.size(); i++)
    for (j=0; j<2; j++)
      faceList[i][j]--;

}



/****************************************************************************/
/****************************************************************************/
/*                             domain definitions                           */
/****************************************************************************/
/****************************************************************************/

void Dune::AmiraMeshReader<Dune::UGGrid<2,2> >::createDomain(UGGrid<2,2>& grid,
                                                             const std::string& domainName,
                                                             const std::string& filename)
{

  int i;

  std::cout << "Loading 2D Amira domain " << filename << std::endl;

  ///////////////////////////////////////////////////////
  // Load the AmiraMesh file
  AmiraMesh* am = AmiraMesh::read(filename.c_str());

  if(!am)
    DUNE_THROW(IOError, "AmiraMeshReader<UGGrid<2,2> >::createDomain: Can't open input file");

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
      DUNE_THROW(IOError, "2D AmiraMesh loader: field 'Triangles(3)' not found!");

  } else {
    AmiraMesh::Data* elementData = am->findData("Triangles", HxINT32, 4, "Nodes");
    if (elementData) {
      elemData = (int*)elementData->dataPtr();
    } else
      DUNE_THROW(IOError, "2D AmiraMesh loader: field 'Triangles(4)' not found!");
  }


  int noOfNodes = am->nElements("Nodes");
  int noOfElem  = am->nElements("Triangles");

  std::cout << "AmiraMesh contains " << noOfNodes << " nodes and "
            << noOfElem << " elements\n";

  // Extract boundary segments
  std::vector<FieldVector<int, 2> > boundary_segments;
  detectBoundarySegments(elemData, noOfElem, boundary_segments, containsOnlyTriangles);
  if (boundary_segments.size() == 0) {
    delete am;
    DUNE_THROW(IOError, "2d AmiraMesh reader: couldn't extract any boundary segments!");
  }

  int noOfBSegments = boundary_segments.size();

  dverb << noOfBSegments << " Boundary segments found!" << std::endl;

  // extract boundary nodes
  std::vector<int> isBoundaryNode;
  detectBoundaryNodes(boundary_segments, noOfNodes, isBoundaryNode);
  if (isBoundaryNode.size() == 0) {
    delete am;
    DUNE_THROW(IOError, "2d AmiraMesh reader: couldn't extract any boundary nodes!");
  }

  int noOfBNodes = 0;
  for (i=0; i<noOfNodes; i++) {
    if (isBoundaryNode[i] != -1)
      noOfBNodes++;
  }

  dverb << noOfBNodes << " boundary nodes found!" << std::endl;


  /* Jetzt geht es an's eingemachte. Zuerst wird ein neues gebiet konstruiert und
     in der internen UG Datenstruktur eingetragen */

  double MidPoint[2] = {0, 0};
  double radius = 100;

  if (UG2d::CreateDomain(domainName.c_str(), MidPoint, radius,
                         noOfBSegments, noOfBNodes, false ) == NULL) {
    delete am;
    DUNE_THROW(IOError, "2d AmiraMesh reader: calling UG2d::CreateDomain failed!");
  }

  /* Alle weiteren Aufrufe von 'CreateBoundarySegment' beziehen sich jetzt auf das eben
     erzeugte Gebiet */

  // alpha und beta spannen den  Parameterbereich auf
  double alpha = 0.0;
  double beta  = 1.0;

  /* Koordinate der Endpunkte der Randsegmente herstellen, wird als user data an das jeweilige
     Segment weitergereicht, um eine Parametrisierungsfunktion definieren zu können. */
  grid.extra_boundary_data_ = ::malloc(4*noOfBSegments*sizeof(double));

  if(grid.extra_boundary_data_ == NULL) {
    delete am;
    DUNE_THROW(IOError, "2d AmiraMesh reader: malloc for extra_boundary_data_ failed!");
  }

  for(i = 0; i < noOfBSegments; i++)
  {
    char segmentName[20];

    // Every boundary segment has a name
    if(sprintf(segmentName, "BS %d", i) < 0) {
      delete am;
      DUNE_THROW(IOError, "2d AmiraMesh reader: sprintf returned error code!");
    }

    /* left = innerRegion, right = outerRegion */
    int left = 1;
    int right = 2;


    const FieldVector<int, 2>& thisEdge = boundary_segments[i];

    ((double*)grid.extra_boundary_data_)[4*i]   = am_node_coordinates[2*thisEdge[0]];
    ((double*)grid.extra_boundary_data_)[4*i+1] = am_node_coordinates[2*thisEdge[0]+1];
    ((double*)grid.extra_boundary_data_)[4*i+2] = am_node_coordinates[2*thisEdge[1]];
    ((double*)grid.extra_boundary_data_)[4*i+3] = am_node_coordinates[2*thisEdge[1]+1];

    int renumNode[2];
    renumNode[0] = isBoundaryNode[thisEdge[0]];
    renumNode[1] = isBoundaryNode[thisEdge[1]];

    if (UG2d::CreateBoundarySegment(segmentName,
                                    left,           /*id of left subdomain */
                                    right,          /*id of right subdomain*/
                                    i,              /*id of segment*/
                                    UG2d::NON_PERIODIC,
                                    1,             // Resolution, only for the UG graphics
                                    renumNode,
                                    &alpha,
                                    &beta,
                                    linearSegmentDescription2d,
                                    (double*)grid.extra_boundary_data_ + 4*i
                                    )==NULL) {
      delete am;
      DUNE_THROW(IOError, "2d AmiraMesh reader:  calling UG2d::CreateBoundarySegment failed!");
    }


  }


  /* That's it!*/

  delete am;
}

/** \todo Extend this such that it also reads double vertex positions */
void Dune::AmiraMeshReader<Dune::UGGrid<2,2> >::read(Dune::UGGrid<2,2>& grid,
                                                     const std::string& filename)
{
  int maxBndNodeID, noOfNodes, noOfElem, noOfCreatedElem;
  UG2d::NODE* theNode;

  std::cout << "Loading 2D Amira mesh " << filename << std::endl;

  // /////////////////////////////////////////////////////
  // Load the AmiraMesh file
  AmiraMesh* am = AmiraMesh::read(filename.c_str());

  if(!am)
    DUNE_THROW(IOError, "2d AmiraMesh reader: File '" << filename << "' could not be read!");

  // Construct a domain name from the multigrid name
  std::string domainname = grid.name() + "_Domain";

  // extract domain from the grid filee
  createDomain(grid, domainname, filename);

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

  // Determine whether grid contains only triangles
  bool containsOnlyTriangles = am->findData("Triangles", HxINT32, 3, "Nodes");

  // get the different data fields
  AmiraMesh::Data* am_coordinateData =  am->findData("Nodes", HxFLOAT, 2, "Coordinates");
  if (!am_coordinateData)
    DUNE_THROW(IOError, "2d AmiraMesh reader: Field 'Nodes' not found");

  float* am_node_coordinates = (float*) am_coordinateData->dataPtr();

  // Get the element list
  int*  elemData = 0;

  if (containsOnlyTriangles) {
    AmiraMesh::Data* triangleData = am->findData("Triangles", HxINT32, 3, "Nodes");
    if (triangleData)
      elemData         = (int*)triangleData->dataPtr();
    else
      DUNE_THROW(IOError, "2D AmiraMesh loader: field 'Triangles(3)' not found!");

  } else {
    AmiraMesh::Data* elementData = am->findData("Triangles", HxINT32, 4, "Nodes");
    if (elementData) {
      elemData = (int*)elementData->dataPtr();
    } else
      DUNE_THROW(IOError, "2D AmiraMesh loader: field 'Triangles(4)' not found!");
  }
  /*
     All Boundary nodes are  assumed to be inserted already.
     We just have to insert the inner nodes and the elements
   */
  maxBndNodeID = -1;
  for (theNode=UG_NS<2>::FirstNode(grid.multigrid_->grids[0]); theNode!=NULL; theNode=theNode->succ)
    maxBndNodeID = std::max(theNode->id, maxBndNodeID);

  dverb << "Already " << maxBndNodeID+1 << " nodes existing" << std::endl;

  noOfNodes = am->nElements("Nodes");
  noOfElem  = am->nElements("Triangles");

  std::cout << "AmiraMesh has " << noOfNodes << " total nodes" << std::endl;

  // Extract boundary faces
  /** \todo The two detect...-methods have been called already.  Don't call
      them again. */
  std::vector<FieldVector<int, 2> > faceList;
  detectBoundarySegments(elemData, noOfElem, faceList, containsOnlyTriangles);
  std::vector<int> isBoundaryNode;
  detectBoundaryNodes(faceList, noOfNodes, isBoundaryNode);

  // Insert interior nodes
  for(int i=0; i < noOfNodes; i++) {

    if (isBoundaryNode[i] != -1)
      continue;

    double nodePos[2];
    nodePos[0] = am_node_coordinates[2*i];
    nodePos[1] = am_node_coordinates[2*i+1];

    if (InsertInnerNode(grid.multigrid_->grids[0], nodePos) == NULL)
      DUNE_THROW(IOError, "2d AmiraMesh reader: Inserting an inner node failed");

    isBoundaryNode[i] = ++maxBndNodeID;

  }

  noOfCreatedElem = 0;
  for (int i=0; i < noOfElem; i++) {

    if (containsOnlyTriangles ||
        (elemData[3*i+2] == elemData[3*i+3])) {

      int cornerIDs[3];

      /* only triangles */
      cornerIDs[0] = isBoundaryNode[elemData[3*i]-1];
      cornerIDs[1] = isBoundaryNode[elemData[3*i+1]-1];
      cornerIDs[2] = isBoundaryNode[elemData[3*i+2]-1];

      if (InsertElementFromIDs(grid.multigrid_->grids[0], 3,cornerIDs, NULL) == NULL)
        DUNE_THROW(IOError, "2d AmiraMesh reader: Inserting an element failed");

    } else {

      int cornerIDs[4];

      /* only quadrilaterals */
      cornerIDs[0] = isBoundaryNode[elemData[4*i]-1];
      cornerIDs[1] = isBoundaryNode[elemData[4*i+1]-1];
      cornerIDs[2] = isBoundaryNode[elemData[4*i+2]-1];
      cornerIDs[3] = isBoundaryNode[elemData[4*i+3]-1];

      if (InsertElementFromIDs(grid.multigrid_->grids[0], 4, cornerIDs, NULL) == NULL)
        DUNE_THROW(IOError, "2d AmiraMesh reader: Inserting an element failed");

    }

    noOfCreatedElem++;

  }

  std::cout << "amiraloadmesh: " << noOfCreatedElem << " elements created" << std::endl;

  delete am;

  grid.createend();

}


#endif // #ifdef _2
