// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// /////////////////////////////////////////////////////////////////////////
// Specialization of the AmiraMesh reader for UGGrid<3,3> and UGGrid<2,2>
// /////////////////////////////////////////////////////////////////////////

#include "../../grid/uggrid.hh"

#include <amiramesh/AmiraMesh.h>

//#define __USE_PARAMETRIZATION_LIBRARY__

#ifdef __USE_PARAMETRIZATION_LIBRARY__
#include "../../../AmiraLibs/include/AmiraParamAccess.h"
#endif

namespace Dune {

  template<>
  class AmiraMeshReader<UGGrid<3,3> > {

  public:

    static void read(UGGrid<3,3>& grid,
                     const std::string& filename);

#ifdef __USE_PARAMETRIZATION_LIBRARY__
    static void read(UGGrid<3,3>& grid,
                     const std::string& gridFilename,
                     const std::string& domainFilename);
#endif

  protected:

#ifdef __USE_PARAMETRIZATION_LIBRARY__
    static int CreateDomain(UGGrid<3,3>& grid,
                            const std::string& domainName,
                            const std::string& filename);
#endif

    static int CreateDomain(UGGrid<3,3>& grid,
                            const std::string& domainName,
                            AmiraMesh* am);

    static void buildGrid(UGGrid<3,3>& grid, AmiraMesh* am);

    static void readHexaGrid(UGGrid<3,3>& grid, AmiraMesh* am);

    static int createHexaDomain(UGGrid<3,3>& grid, AmiraMesh* am,
                                const std::string& filename);

    static void detectBoundarySegments(int* elemData,
                                       int noOfElem,
                                       std::vector<Vec<4, int> >& face_list);

    static void detectBoundarySegments(int* elemData,
                                       int noOfElem,
                                       std::vector<Vec<3, int> >& face_list);

    AmiraMeshReader() {}

  };

}


// //////////////////////////////////////////////////
// //////////////////////////////////////////////////
#ifdef __USE_PARAMETRIZATION_LIBRARY__
static int SegmentDescriptionByAmira(void *data, double *param, double *result)
{

  int triNum = * (int*) data;

  double barCoords[2];
  double A[4] = {-1, 1, 0, -1};
  double b[2] = {1, 0};

  // barCoords = A*param + b;
  barCoords[0] = A[0]*param[0] + A[2]*param[1];
  barCoords[1] = A[1]*param[0] + A[3]*param[1];

  barCoords[0] += b[0];
  barCoords[1] += b[1];

#ifndef NDEBUG
  const float eps = 1e-6;
  if (barCoords[0]<-eps || barCoords[1]<-eps || (1-barCoords[0]-barCoords[1])<-eps) {
    printf("Illegal barycentric coordinate\n");
    assert(false);
  }
#endif

  AmiraCallPositionParametrization(triNum, barCoords, result);

  return(0);
}
#endif // #define __USE_PARAMETRIZATION_LIBRARY__

/** This method implements a linear function in order to be able to
 *  work with straight line boundaries.
 *  We interpret data as a DOUBLE* to the world coordinates of the
 *  three endpoints.
 *
 * \todo This should actually be replaced by using LinearSegments
 * instead of BoundarySegments.  But LinearSegments are buggy in UG.
 */
#ifdef _3
static int linearSegmentDescription3d(void *data, double *param, double *result)
{
  Dune::Vec<3> a,b,c;
  a[0] = ((double*)data)[0];
  a[1] = ((double*)data)[1];
  a[2] = ((double*)data)[2];
  b[0] = ((double*)data)[3];
  b[1] = ((double*)data)[4];
  b[2] = ((double*)data)[5];
  c[0] = ((double*)data)[6];
  c[1] = ((double*)data)[7];
  c[2] = ((double*)data)[8];

  // UG expects the three triangle vertices to have the local coordinates
  // (0,0), (1,0), and (1,1).
  double barCoords[2];
  double A[4] = {-1, 1, 0, -1};

  // barCoords = A*param + b;
  barCoords[0] = A[0]*param[0] + A[2]*param[1];
  barCoords[1] = A[1]*param[0] + A[3]*param[1];

  barCoords[0] += 1.0;
  barCoords[1] += 0.0;

  // linear interpolation
  for (int i=0; i<3; i++)
    result[i] = barCoords[0]*a[i] + barCoords[1]*b[i] + (1-barCoords[0]-barCoords[1])*c[i];

  return 0;
}
#endif // #ifdef _3

/** \todo This is quadratic --> very slow */
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::detectBoundarySegments(int* elemData,
                                                                       int numTetras,
                                                                       std::vector<Vec<3, int> >& face_list)
{
  int i, j;

  static const int idx[][3] = {
    {3,2,0},{1,2,3},{1,3,0},{2,1,0}
  };

  face_list.resize(0);

  for (i=0; i<numTetras; i++) {

    for (int k=0; k<4; k++) {
      Vec<3, int> v;
      v[0] = elemData[4*i+idx[k][0]];
      v[1] = elemData[4*i+idx[k][1]];
      v[2] = elemData[4*i+idx[k][2]];


      // Check if new face exists already in the list
      // (then it is no boundary face
      for (j=0; j<(int)face_list.size(); j++) {

        const Vec<3,int>& o = face_list[j];
        if ( (v[0]==o[0] && v[1]==o[1] && v[2]==o[2]) ||
             (v[0]==o[0] && v[1]==o[2] && v[2]==o[1]) ||
             (v[0]==o[1] && v[1]==o[0] && v[2]==o[2]) ||
             (v[0]==o[1] && v[1]==o[2] && v[2]==o[0]) ||
             (v[0]==o[2] && v[1]==o[0] && v[2]==o[1]) ||
             (v[0]==o[2] && v[1]==o[1] && v[2]==o[0]) ) {

          break;
        }

      }

      if (j<(int)face_list.size()) {
        // face has been found
        face_list[j] = face_list.back();
        face_list.pop_back();

      } else {

        // Insert k-th face of i-th tetrahedron into face list
        face_list.push_back(v);

      }
    }
  }

  // Switch from AmiraMesh numbering (1,2,3,...) to internal numbering (0,1,2,...)
  for (i=0; i<(int)face_list.size(); i++)
    for (j=0; j<3; j++)
      face_list[i][j]--;

}

template<int NUM_VERTICES>
static int detectBoundaryNodes(const std::vector< Dune::Vec<NUM_VERTICES, int> >& face_list,
                               int noOfNodes,
                               std::vector<int>& isBoundaryNode)
{
  int i, j;
  isBoundaryNode.resize(noOfNodes);

  int UGNodeIdxCounter = 0;

  for (i=0; i<noOfNodes; i++)
    isBoundaryNode[i] = -1;

  for (i=0; i<(int)face_list.size(); i++) {

    for (j=0; j<NUM_VERTICES; j++)
      if (isBoundaryNode[face_list[i][j]] == -1)
        isBoundaryNode[face_list[i][j]] = UGNodeIdxCounter++;

  }

  return UGNodeIdxCounter++;
}

#ifdef _3
#ifdef __USE_PARAMETRIZATION_LIBRARY__
// Create the domain from an explicitly given boundary description
int Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::CreateDomain(UGGrid<3,3>& grid,
                                                            const std::string& domainName,
                                                            const std::string& filename)
{
  const int CORNERS_OF_BND_SEG = 4;
  int point[CORNERS_OF_BND_SEG] = {-1, -1, -1, -1};
  double alpha[2], beta[2];
  printf("Hallo Welt!\n");
  /* Load data */
  if(AmiraLoadMesh(domainName.c_str(), filename.c_str()) != AMIRA_OK)
  {
    std::cerr << "Error in AmiraMeshReader<Dune::UGGrid<3,3> >::CreateDomain:"
              << "Domain file could not be opened!" << std::endl;
    return 1;
  }
  printf("Hallo Welt!\n");

  if(AmiraStartEditingDomain(domainName.c_str()) != AMIRA_OK)
  {
    std::cerr << "Error in AmiraMeshReader<Dune::UGGrid<3,3> >::CreateDomain:"
              << "StartEditing failed!" << std::endl;
    return 1;
  }
  printf("Hallo Welt!\n");


  /* Alle weiteren Anfragen an die Bibliothek beziehen sich jetzt auf das eben
     geladen Gebiet. Maessig elegant, sollte aber gehen */

  int noOfSegments = AmiraGetNoOfSegments();
  if(noOfSegments <= 0)
  {
    UG3d::PrintErrorMessage('E', "CreateAmiraDomain", "no segments found");
    return(1);
  }

  int noOfNodes = AmiraGetNoOfNodes();
  if(noOfNodes <= 0)
  {
    UG3d::PrintErrorMessage('E', "CreateAmiraDomain", "no nodes found");
    return(1);
  }


  /* Wir brauchen eine Kugel, die das Gebiet komplett  enthaelt.  Diese Information
     wird für die UG-Graphik benoetigt, die Werte sind hier also komplett egal. */
  double radius = 1;
  double MidPoint[3] = {0, 0, 0};

  /* Jetzt geht es an's eingemachte. Zuerst wird ein neues gebiet konstruiert und
     in der internen UG Datenstruktur eingetragen */

  UG3d::domain* newDomain = UG3d::CreateDomain(const_cast<char*>(domainName.c_str()),
                                               MidPoint, radius,
                                               noOfSegments, noOfNodes,
                                               false );


  if (!newDomain)
    return(1);


  /* Alle weiteren Aufrufe von 'CreateBoundarySegment' beziehen sich jetzt auf das eben
     erzeugte Gebiet */

  /*
     Liste der Nummern der Randsegmente herstellen, wird als user data an das jeweilige
     Segment weitergereicht
   */

  grid.extra_boundary_data_ =  ::malloc(noOfSegments*sizeof(int));
  if (grid.extra_boundary_data_ == NULL)
    return 1;

  for(int i = 0; i < noOfSegments; i++) {
    printf("%d\n", i);
    char segmentName[200];
    int left, right;

    // Gets the vertices of a boundary segment
    AmiraGetNodeNumbersOfSegment(point, i);

    if(sprintf(segmentName, "AmiraSegment %d", i) < 0)
      return 1;

    /* left = innerRegion, right = outerRegion */
    AmiraGetLeftAndRightSideOfSegment(&left, &right, i);

    ((int*)grid.extra_boundary_data_)[i] = i;

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
                                    1,              // resolution, whatever that is
                                    point,
                                    alpha, beta,
                                    SegmentDescriptionByAmira,
                                    ((int*)grid.extra_boundary_data_)+i
                                    )==NULL)
      return(1);

  }

  std::cout << noOfSegments << " segments created!\n";

  /* That's it!*/
  return(0);


}
#endif // #define __USE_PARAMETRIZATION_LIBRARY__


// Create the domain by extracting the boundary of the given grid
int Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::CreateDomain(UGGrid<3,3>& grid,
                                                            const std::string& domainName,
                                                            AmiraMesh* am)
{

  //const int DIM = 3;
  //const int MAX_CORNERS_OF_LINEAR_PATCH = DIM;
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
    else
      throw("No vertex coordinates found in the file!");

  }

  AmiraMesh::Data* tetrahedronData = am->findData("Tetrahedra", HxINT32, 4, "Nodes");
  int*  elemData         = (int*)tetrahedronData->dataPtr();
  int noOfElem = am->nElements("Tetrahedra");

  // Extract boundary faces
  std::vector<Vec<3, int> > face_list;
  detectBoundarySegments(elemData, noOfElem, face_list);

  if(face_list.size() == 0)
  {
    UG3d::PrintErrorMessage('E', "createTetraDomain", "no segments found");
    delete am;
    return(1);
  }

  int nBndSegments = face_list.size();

  printf("%d boundary segments found!\n", face_list.size());
  for (unsigned int i=0; i<face_list.size(); i++) {
    std::cout << face_list[i] << "\n";
  }

  int noOfNodes = am->nElements("Nodes");

  std::vector<int> isBoundaryNode;

  int nBndNodes = detectBoundaryNodes(face_list, noOfNodes, isBoundaryNode);
  if(nBndNodes <= 0)
  {
    UG3d::PrintErrorMessage('E', "CreateAmiraDomain", "no nodes found");
    return(1);
  }

  //     for (int ii=0; ii<isBoundaryNode.size(); ii++)
  //         printf("%d\n", isBoundaryNode[ii]);

  printf("%d boundary nodes found!\n", nBndNodes);


  /* Zuerst wird ein neues gebiet konstruiert und
     in der internen UG Datenstruktur eingetragen */

  UG3d::domain* newDomain = UG3d::CreateDomain("olisDomain",
                                               midPoint, radius,
                                               face_list.size(), nBndNodes,
                                               false);


  if (!newDomain)
    return(1);


  /* Alle weiteren Aufrufe von 'CreateBoundarySegment' beziehen sich jetzt auf das eben
     erzeugte Gebiet */


  /*
     Die Koordinaten der Eckknoten wird als user data an das jeweilige
     Segment weitergereicht.
   */
  grid.extra_boundary_data_ = ::malloc(nBndSegments*3*3*sizeof(double));

  if(grid.extra_boundary_data_ == NULL)
    return 1;

  for(int i = 0; i < nBndSegments; i++) {

    //std::string segmentName;
    char segmentName[20];

    // bordering subdomains
    int left = 1;
    int right = 2;

    point[0] = face_list[i][0];
    point[1] = face_list[i][1];
    point[2] = face_list[i][2];
    //point[3] = face_list[i][3];

    if(sprintf(segmentName, "Segment %d", i) < 0)
      return 1;

    /* left = innerRegion, right = outerRegion */
    //segmentIndexList[i] = i;
    ((double*)grid.extra_boundary_data_)[9*i+0] = am_node_coordinates_float[3*face_list[i][0] + 0];
    ((double*)grid.extra_boundary_data_)[9*i+1] = am_node_coordinates_float[3*face_list[i][0] + 1];
    ((double*)grid.extra_boundary_data_)[9*i+2] = am_node_coordinates_float[3*face_list[i][0] + 2];
    ((double*)grid.extra_boundary_data_)[9*i+3] = am_node_coordinates_float[3*face_list[i][1] + 0];
    ((double*)grid.extra_boundary_data_)[9*i+4] = am_node_coordinates_float[3*face_list[i][1] + 1];
    ((double*)grid.extra_boundary_data_)[9*i+5] = am_node_coordinates_float[3*face_list[i][1] + 2];
    ((double*)grid.extra_boundary_data_)[9*i+6] = am_node_coordinates_float[3*face_list[i][2] + 0];
    ((double*)grid.extra_boundary_data_)[9*i+7] = am_node_coordinates_float[3*face_list[i][2] + 1];
    ((double*)grid.extra_boundary_data_)[9*i+8] = am_node_coordinates_float[3*face_list[i][2] + 2];

#if 1
    double alpha[2] = {0, 0};
    double beta[2]  = {1, 1};

    if (UG3d::CreateBoundarySegment(segmentName,
                                    left,             /*id of left subdomain */
                                    right,            /*id of right subdomain*/
                                    i,                /*id of segment*/
                                    UG3d::NON_PERIODIC,
                                    1,              // resolution, whatever that is
                                    point,
                                    alpha, beta,
                                    linearSegmentDescription3d,
                                    ((double*)grid.extra_boundary_data_)+9*i
                                    )==NULL)
      return(1);
#else
    double paramCoords[3][2] = {{0,0}, {1,0}, {0,1}};
    if (UG3d::CreateLinearSegment(segmentName,
                                  left,             /*id of left subdomain */
                                  right,            /*id of right subdomain*/
                                  i,                /*id of segment*/
                                  4,                // Number of corners
                                  point,
                                  paramCoords
                                  )==NULL)
      return(1);
#endif
  }

  std::cout << nBndSegments << " segments created!\n";

  return 0;

}


#ifdef __USE_PARAMETRIZATION_LIBRARY__
/** \todo Clear grid before reading! */
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::read(Dune::UGGrid<3,3>& grid,
                                                     const std::string& filename,
                                                     const std::string& domainFilename)  try
{
  printf("This is the AmiraMesh reader for UGGrid<3,3>!\n");

  // /////////////////////////////////////////////////////
  // Load the AmiraMesh file
  AmiraMesh* am = AmiraMesh::read(filename.c_str());

  if(!am)
    throw("Could not open AmiraMesh file");

  if (am->findData("Hexahedra", HxINT32, 8, "Nodes")) {
    readHexaGrid(grid, am);
    return;
  }

  //loaddomain $file @PARA_FILE $name @DOMAIN
  CreateDomain(grid, "olisDomain", domainFilename);

  // read and build the grid
  buildGrid(grid, am);
}
catch (const char* msg) {

  printf("%s\n", msg);
  return;
}
#endif // #define __USE_PARAMETRIZATION_LIBRARY__


/** \todo Clear grid before reading! */
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::read(Dune::UGGrid<3,3>& grid,
                                                     const std::string& filename)  try
{
  printf("This is the AmiraMesh reader for UGGrid<3,3>!\n");

  // /////////////////////////////////////////////////////
  // Load the AmiraMesh file
  AmiraMesh* am = AmiraMesh::read(filename.c_str());

  if(!am)
    throw("Could not open AmiraMesh file");

  if (am->findData("Hexahedra", HxINT32, 8, "Nodes")) {
    readHexaGrid(grid, am);
    return;
  }

  //loaddomain $file @PARA_FILE $name @DOMAIN
  CreateDomain(grid, "olisDomain", am);

  buildGrid(grid, am);

}
catch (const char* msg) {

  printf("%s\n", msg);
  return;
}

void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::buildGrid(UGGrid<3,3>& grid, AmiraMesh* am) try
{
  printf("before makeNew\n");
  // call configureCommand and newCommand
  grid.makeNewUGMultigrid();
  printf("before makeNew\n");

  // ////////////////////////////////////////////
  // loadmesh $file @GRID_FILE $name @DOMAIN;

  const int DIM = 3;

  int i;
  double nodePos[DIM];
  UG3d::NODE* theNode;

  //std::cout << "Loading Amira mesh " <<  filename << "\n";



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
      throw("No vertex coordinates found in the file!");

  }


  AmiraMesh::Data* tetrahedronData = am->findData("Tetrahedra", HxINT32, 4, "Nodes");
  int*  elemData         = (int*)tetrahedronData->dataPtr();

  /*
     All Boundary nodes are  assumed to be inserted already.
     We just have to insert the inner nodes and the elements
   */
  assert(grid.multigrid_);

  int maxBndNodeID = -1;
  for (theNode=grid.multigrid_->grids[0]->firstNode[0]; theNode!=NULL; theNode=theNode->succ)
  {
    // The following two lines ought to be in here, but the
    // OBJT macros is somewhat complicated, so I leave it out
    // for the time being.
    //       if(OBJT(theNode->myvertex) == UG3d::IVOBJ)
    //           UserWriteF("Warning: Node %d is inner node\n", ID(theNode));
    maxBndNodeID = MAX(theNode->id, maxBndNodeID);
  }

  std::cout << "Already " << maxBndNodeID+1 << " nodes existing\n";


  int noOfBndNodes = maxBndNodeID;


  int noOfNodes = am->nElements("Nodes");

  //  noOfInnerNodes = noOfNodes - noOfBndNodes;
  printf("AmiraMesh has %d total nodes\n", noOfNodes);





  for(i = noOfBndNodes+1; i < noOfNodes; i++) {

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

    /// \todo Warum ist nicht UG3d::InsertInnerNode Pflicht???
    if (InsertInnerNode(grid.multigrid_->grids[0], nodePos) == NULL)
      throw("inserting an inner node failed");

  }



  /* all inner nodes are inserted , now we insert the elements */
  int noOfElem = am->nElements("Tetrahedra");

  int noOfCreatedElem = 0;
  for(i=0; i < noOfElem; i++)
  {
    int cornerIDs[4];

    //AmiraGetCornerIDsOfElem(i, cornerIDs);

    /* only tetrahedrons */
    /* printf("MeshAccess: elem id : %d, node ids : %d %d %d %d\n", i,
       elemData[4*i+0], elemData[4*i+1], elemData[4*i+2], elemData[4*i+3]);*/
    cornerIDs[0] = elemData[4*i]-1;
    cornerIDs[1] = elemData[4*i+1]-1;
    cornerIDs[2] = elemData[4*i+2]-1;
    cornerIDs[3] = elemData[4*i+3]-1;


    /*       printf("elem id : %d, node ids : %d %d %d %d\n", i, cornerIDs[0], cornerIDs[1], cornerIDs[2], cornerIDs[3]); */

    if (InsertElementFromIDs(grid.multigrid_->grids[0], 4,cornerIDs, NULL) == NULL)
      throw("inserting an element failed");

    noOfCreatedElem++;

  }


  if(noOfElem != noOfCreatedElem)
    throw("inserting an element failed");

  UG3d::UserWriteF("amiraloadmesh: %d elements created\n", noOfCreatedElem);

  // set the subdomainIDs
  AmiraMesh::Data* am_material_ids = am->findData("Tetrahedra", HxBYTE, 1, "Materials");
  if (!am_material_ids)
    throw("Field 'Materials' not found.");

  char* material_ids         = (char*)am_material_ids->dataPtr();

  i = 0;
  UG3d::ELEMENT* theElement;
  for (theElement=grid.multigrid_->grids[0]->elements[0]; theElement!=NULL; theElement=theElement->ge.succ)
  {

    /* get subdomain of element */
    int id = material_ids[i];

#define ControlWord(p,ce) (((unsigned int *)(p))[UG3d::control_entries[ce].offset_in_object])
#define CW_WRITE(p, ce, n)   ControlWord(p,ce) = (ControlWord(p,ce)&UG3d::control_entries[ce].xor_mask)|(((n)<<UG3d::control_entries[ce].offset_in_word)&UG3d::control_entries[ce].mask)
#define SETSUBDOMAIN(p,n) CW_WRITE(p,UG3d::SUBDOMAIN_CE,n)

    SETSUBDOMAIN(theElement, id+1);

#undef ControlWord
#undef CW_WRITE
#undef SETSUBDOMAIN

    i++;
  }

  delete am;

  UG3d::SetEdgeAndNodeSubdomainFromElements(grid.multigrid_->grids[0]);


  /** \todo Do we really need to call CreateAlgebra for Dune?
   *
   * So far we do, because the UG grid refinement expects a valid
   * algebra.  Unfortunately, this wastes a lot of resources, because
   * nobody is ever going to use the algebra.  Maybe we can patch UG? */
  if (UG3d::CreateAlgebra(grid.multigrid_) != UG3d::GM_OK)
    throw("Error in UG3d::CreateAlgebra!");

  /** \todo Check whether this release is necessary */
  /* here all temp memory since CreateMultiGrid is released */
  //UG3d::ReleaseTmpMem(MGHEAP(theMG),MG_MARK_KEY(theMG));
#define ReleaseTmpMem(p,k) Release(p, UG3d::FROM_TOP,k)
  ReleaseTmpMem(grid.multigrid_->theHeap, grid.multigrid_->MarkKey);
#undef ReleaseTmpMem
  grid.multigrid_->MarkKey = 0;


  return;


}
catch (const char* msg) {

  printf("%s\n", msg);
  return;
}



/*****************************************************************/
/* Read the UGGrid from an AmiraMesh Hexagrid file               */
/*****************************************************************/

/** \todo This is quadratic --> very slow */
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::detectBoundarySegments(int* elemData,
                                                                       int numHexas,
                                                                       std::vector<Vec<4, int> >& face_list)
{
  static const int idx[][4] = {
    {0,4,5,1},{1,5,6,2},{2,6,7,3},{3,7,4,0},{4,7,6,5},{1,2,3,0}
  };

  face_list.resize(0);

  //     int n=0;
  //     int nDuplicate=0;
  for (int i=0; i<numHexas; i++) {

    for (int k=0; k<6; k++) {
      Vec<4, int> v;
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

      // Insert k-th face of i-th hexahedron into face list
      face_list.push_back(v);
    }
  }

}


int Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::createHexaDomain(UGGrid<3,3>& grid,
                                                                AmiraMesh* am,
                                                                const std::string& filename)
{
  //const int DIM = 3;
  //const int MAX_CORNERS_OF_LINEAR_PATCH = DIM;
  const int CORNERS_OF_BND_SEG = 4;


  int point[CORNERS_OF_BND_SEG] = {-1, -1, -1, -1};
  double midPoint[3] = {0,0,0};
  double radius = 1;

  AmiraMesh::Data* hexahedronData = am->findData("Hexahedra", HxINT32, 8, "Nodes");
  int*  elemData         = (int*)hexahedronData->dataPtr();
  int noOfElem = am->nElements("Hexahedra");

  // Extract boundary faces
  std::vector<Vec<4, int> > face_list;
  detectBoundarySegments(elemData, noOfElem, face_list);

  if(face_list.size() == 0)
  {
    UG3d::PrintErrorMessage('E', "createHexaDomain", "no segments found");
    return(1);
  }

  int nBndSegments = face_list.size();

  std::cout << face_list.size() << " boundary segments found!\n";

  int noOfNodes = am->nElements("Nodes");

  std::vector<int> isBoundaryNode;

  int nBndNodes = detectBoundaryNodes(face_list, noOfNodes, isBoundaryNode);
  if(nBndNodes <= 0)
  {
    UG3d::PrintErrorMessage('E', "CreateAmiraDomain", "no nodes found");
    return(1);
  }

  printf("%d boundary nodes found!\n", nBndNodes);


  /* Zuerst wird ein neues gebiet konstruiert und
     in der internen UG Datenstruktur eingetragen */

  UG3d::domain* newDomain = UG3d::CreateDomain("olisDomain",
                                               midPoint, radius,
                                               face_list.size(), nBndNodes,
                                               false);


  if (!newDomain)
    return(1);


  /* Alle weiteren Aufrufe von 'CreateBoundarySegment' beziehen sich jetzt auf das eben
     erzeugte Gebiet */


  /*
     Liste der Nummern der Randsegmente herstellen, wird als user data an das jeweilige
     Segment weitergereicht
   */


  int* segmentIndexList = NULL;
  /** \bug This memory gets never freed! */
  segmentIndexList = (int*) ::malloc(nBndSegments*sizeof(int));

  if(segmentIndexList == NULL)
    return(1);

  for(int i = 0; i < nBndSegments; i++) {

    //std::string segmentName;
    char segmentName[20];
    int left, right;

    point[0] = face_list[i][0];
    point[1] = face_list[i][1];
    point[2] = face_list[i][2];
    point[3] = face_list[i][3];

    /* Es werden die Ecknummern eines Randsegmentes zurueckgegeben
       point[0] = 0; point[1]=1; point[2]=2; point[3]=3; */

    if(sprintf(segmentName, "Segment %d", i) < 0)
      return(1);

    /* left = innerRegion, right = outerRegion */
    segmentIndexList[i] = i;

    /* map Amira Material ID's to UG material ID's */

    left++;
    right++;

#if 0
    double alpha[2] = {0, 0};
    double beta[2]  = {1, 1};

    if (UG3d::CreateBoundarySegment(segmentName,
                                    left,             /*id of left subdomain */
                                    right,            /*id of right subdomain*/
                                    i,                /*id of segment*/
                                    UG3d::NON_PERIODIC,
                                    1,              // resolution, whatever that is
                                    point,
                                    alpha, beta,
                                    LinearSegmentDescription,
                                    (void *) (segmentIndexList+i)
                                    )==NULL)
      return(1);
#else
    double paramCoords[3][2] = {{0,0}, {1,0}, {0,1}};
    if (UG3d::CreateLinearSegment(segmentName,
                                  left,             /*id of left subdomain */
                                  right,            /*id of right subdomain*/
                                  i,                /*id of segment*/
                                  4,                // Number of corners
                                  point,
                                  paramCoords
                                  )==NULL)
      return(1);
#endif
  }

  printf("%d segments created!\n", nBndSegments);

  return 0;
}

void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::readHexaGrid(Dune::UGGrid<3,3>& grid,
                                                             AmiraMesh* am)  try
{
  printf("This is the AmiraMesh HexaGrid reader for UGGrid<3,3>!\n");

  //loaddomain $file @PARA_FILE $name @DOMAIN
  createHexaDomain(grid, am, "olisDomain");

  // call configureCommand and newCommand
  grid.makeNewUGMultigrid();

  // ////////////////////////////////////////////
  // loadmesh $file @GRID_FILE $name @DOMAIN;

  const int DIM = 3;

  int i;
  double nodePos[DIM];
  UG3d::NODE* theNode;

  //printf("Loading Amira mesh %s\n", filename.c_str());



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
      throw("No vertex coordinates found in the file!");

  }


  AmiraMesh::Data* hexahedronData = am->findData("Hexahedra", HxINT32, 8, "Nodes");
  int*  elemData         = (int*)hexahedronData->dataPtr();

  /*
     All Boundary nodes are  assumed to be inserted already.
     We just have to insert the inner nodes and the elements
   */
  UG3d::multigrid* theMG = UG3d::GetMultigrid("DuneMG");
  assert(theMG);

  int maxBndNodeID = -1;
  for (theNode=theMG->grids[0]->firstNode[0]; theNode!=NULL; theNode=theNode->succ)
  {
    // The following two lines ought to be in here, but the
    // OBJT macros is somewhat complicated, so I leave it out
    // for the time being.
    //       if(OBJT(theNode->myvertex) == UG3d::IVOBJ)
    //           UserWriteF("Warning: Node %d is inner node\n", ID(theNode));
    maxBndNodeID = MAX(theNode->id, maxBndNodeID);
  }
  UG3d::UserWriteF("Already %d nodes existing\n", maxBndNodeID+1);


  int noOfNodes = am->nElements("Nodes");

  printf("AmiraMesh has %d total nodes\n", noOfNodes);

  // Now insert inner nodes
  for(i = maxBndNodeID+1; i < noOfNodes; i++) {

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

    /// \todo Warum ist nicht UG3d::InsertInnerNode Pflicht???
    if (InsertInnerNode(theMG->grids[0], nodePos) == NULL)
      throw("inserting an inner node failed");

  }



  /* all inner nodes are inserted , now we insert the elements */
  int noOfElem = am->nElements("Hexahedra");

  int noOfCreatedElem = 0;
  for(i=0; i < noOfElem; i++)
  {
    int cornerIDs[8];
    int j;

    /* only tetrahedrons */
    /* printf("MeshAccess: elem id : %d, node ids : %d %d %d %d\n", i,
       elemData[4*i+0], elemData[4*i+1], elemData[4*i+2], elemData[4*i+3]);*/
    for (j=0; j<8; j++)
      cornerIDs[j] = elemData[8*i+j]-1;

    /*       printf("elem id : %d, node ids : %d %d %d %d\n", i, cornerIDs[0], cornerIDs[1], cornerIDs[2], cornerIDs[3]); */

    if (UG3d::InsertElementFromIDs(theMG->grids[0], 8, cornerIDs, NULL) == NULL)
      throw("inserting an element failed");

    noOfCreatedElem++;

  }


  if(noOfElem != noOfCreatedElem)
    throw("inserting an element failed");

  UG3d::UserWriteF("amiraloadmesh: %d elements created\n", noOfCreatedElem);

  // set the subdomainIDs
  AmiraMesh::Data* am_material_ids = am->findData("Tetrahedra", HxBYTE, 1, "Materials");
  if (!am_material_ids)
    throw("Field 'Materials' not found.");

  char* material_ids         = (char*)am_material_ids->dataPtr();

  i = 0;
  UG3d::ELEMENT* theElement;
  for (theElement=theMG->grids[0]->elements[0]; theElement!=NULL; theElement=theElement->ge.succ)
  {

    /* get subdomain of element */
    int id = material_ids[i];

#define ControlWord(p,ce) (((unsigned int *)(p))[UG3d::control_entries[ce].offset_in_object])
#define CW_WRITE(p, ce, n)   ControlWord(p,ce) = (ControlWord(p,ce)&UG3d::control_entries[ce].xor_mask)|(((n)<<UG3d::control_entries[ce].offset_in_word)&UG3d::control_entries[ce].mask)
#define SETSUBDOMAIN(p,n) CW_WRITE(p,UG3d::SUBDOMAIN_CE,n)

    SETSUBDOMAIN(theElement, id+1);

#undef ControlWord
#undef CW_WRITE
#undef SETSUBDOMAIN

    i++;
  }

  delete am;

  UG3d::SetEdgeAndNodeSubdomainFromElements(theMG->grids[0]);


  /** \todo Do we really need to call CreateAlgebra for Dune?
   *
   * So far we do, because the UG grid refinement expects a valid
   * algebra.  Unfortunately, this wastes a lot of resources, because
   * nobody is ever going to use the algebra.  Maybe we can patch UG? */
  if (UG3d::CreateAlgebra(theMG) != UG3d::GM_OK)
    throw("Error in UG3d::CreateAlgebra!");

  /** \todo Check whether this release is necessary */
  /* here all temp memory since CreateMultiGrid is released */
  //UG3d::ReleaseTmpMem(MGHEAP(theMG),MG_MARK_KEY(theMG));
#define ReleaseTmpMem(p,k) Release(p, UG3d::FROM_TOP,k)
  ReleaseTmpMem(theMG->theHeap, theMG->MarkKey);
#undef ReleaseTmpMem
  theMG->MarkKey = 0;


  return;


}
catch (const char* msg) {

  printf("%s\n", msg);
  return;
}

#endif // #ifdef _3


/*********************************************************************************/
/*********************************************************************************/
/*                                                                               */
/* The code for reading 2D grids from an AmiraMesh file into a UGGrid object     */
/*                                                                               */
/*********************************************************************************/
/*********************************************************************************/
namespace Dune {

  template<>
  class AmiraMeshReader<UGGrid<2,2> > {

  public:

    static void read(UGGrid<2,2>& grid,
                     const std::string& filename);

  protected:
    static int CreateDomain(UGGrid<2,2>& grid,
                            const std::string& domainName,
                            const std::string& filename);

    static void detectBoundarySegments(int* elemData,
                                       int numElems,
                                       std::vector<Vec<2, int> >& face_list);

  };

}

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
                                                                       std::vector<Vec<2, int> >& face_list)
{
  int i, j;

  static const int idx[][2] = {
    {0,1},{1,2},{2,0}
  };

  face_list.resize(0);

  for (i=0; i<numElems; i++) {

    for (int k=0; k<3; k++) {
      Vec<2, int> v;
      v[0] = elemData[3*i+idx[k][0]];
      v[1] = elemData[3*i+idx[k][1]];

      // Check if new face exists already in the list
      // (then it is no boundary face
      for (j=0; j<(int)face_list.size(); j++) {

        const Vec<2,int>& o = face_list[j];
        if ( (v[0]==o[0] && v[1]==o[1]) ||
             (v[0]==o[1] && v[1]==o[0]) ) {
          break;
        }

      }

      if (j<(int)face_list.size()) {
        // face has been found
        face_list[j] = face_list.back();
        face_list.pop_back();

      } else {

        // Insert k-th face of i-th tetrahedron into face list
        face_list.push_back(v);

      }
    }
  }

  // Switch from AmiraMesh numbering (1,2,3,...) to internal numbering (0,1,2,...)
  for (i=0; i<(int)face_list.size(); i++)
    for (j=0; j<2; j++)
      face_list[i][j]--;

}



/****************************************************************************/
/****************************************************************************/
/*                             domain definitions                           */
/****************************************************************************/
/****************************************************************************/

int Dune::AmiraMeshReader<Dune::UGGrid<2,2> >::CreateDomain(UGGrid<2,2>& grid,
                                                            const std::string& domainName,
                                                            const std::string& filename)
{

  int i;

  std::cout << "Loading 2D Amira domain " << filename << "\n";

  ///////////////////////////////////////////////////////
  // Load the AmiraMesh file
  AmiraMesh* am = AmiraMesh::read(filename.c_str());

  if(!am) {
    std::cerr << "AmiraMeshReader<UGGrid<2,2> >::CreateDomain: Can't open input file\n";
    return(1);
  }

  // get the different data fields
  AmiraMesh::Data* am_coordinateData =  am->findData("Nodes", HxFLOAT, 2, "Coordinates");
  if (!am_coordinateData) {
    std::cerr << "2D AmiraMesh loader: field 'Nodes' not found!\n";
    return(1);
  }
  float*   am_node_coordinates = (float*) am_coordinateData->dataPtr();

  AmiraMesh::Data* triangleData = am->findData("Triangles", HxINT32, 3, "Nodes");
  if (!triangleData) {
    std::cerr << "2D AmiraMesh loader: field 'Triangles' not found!\n";
    return(1);
  }
  int*  elemData         = (int*)triangleData->dataPtr();


  int noOfNodes = am->nElements("Nodes");
  int noOfElem  = am->nElements("Triangles");

  std::cout << "AmiraMesh has " << noOfNodes << " total nodes and "
            << noOfElem << " triangles\n";

  // Extract boundary segments
  std::vector<Vec<2, int> > boundary_segments;
  detectBoundarySegments(elemData, noOfElem, boundary_segments);
  if (boundary_segments.size() == 0) {
    delete am;
    return 1;
  }

  int noOfBSegments = boundary_segments.size();


  std::cout << noOfBSegments << " Boundary segments found!\n";

  // extract boundary nodes
  std::vector<int> boundary_nodes;
  detectBoundaryNodes(boundary_segments, noOfNodes, boundary_nodes);
  if (boundary_nodes.size() == 0) {
    delete am;
    return (1);
  }

  int noOfBNodes = 0;
  for (i=0; i<noOfNodes; i++) {
    if (boundary_nodes[i] != -1)
      noOfBNodes++;
  }

  std::cout << noOfBNodes << " boundary nodes found!\n";


  /* Jetzt geht es an's eingemachte. Zuerst wird ein neues gebiet konstruiert und
     in der internen UG Datenstruktur eingetragen */

  double MidPoint[2] = {0, 0};
  double radius = 100;



  /** \todo Can I patch UG such as to make the following const_cast unnecessary? */
  if (UG2d::CreateDomain(const_cast<char*>(domainName.c_str()), MidPoint, radius,
                         noOfBSegments, noOfBNodes, false ) == NULL) {
    delete am;
    return(1);
  }

  /* Alle weiteren Aufrufe von 'CreateBoundarySegment' beziehen sich jetzt auf das eben
     erzeugte Gebiet */


  /*
     alpha und beta spannen den  Parameterbereich auf (s. beiliegendes .ps-file)
     wenn ich das richtig verstehe, sollten die konstant sein fuer alle Segmente
   */

  double alpha = 0.0;
  double beta  = 1.0;

  /*
     Koordinate der Endpunkte der Randsegmente herstellen, wird als user data an das jeweilige
     Segment weitergereicht, um eine Parametrisierungsfunktion definieren zu können.
   */
  grid.extra_boundary_data_ = ::malloc(4*noOfBSegments*sizeof(double));

  if(grid.extra_boundary_data_ == NULL) {
    delete am;
    return(1);
  }

  for(i = 0; i < noOfBSegments; i++)
  {
    char segmentName[20];
    int left, right;

    // Every boundary segment has a name
    if(sprintf(segmentName, "BS %d", i) < 0) {
      delete am;
      return(1);
    }

    /* left = innerRegion, right = outerRegion */
    left = 0;
    right = 1;


    const Vec<2,int>& thisEdge = boundary_segments[i];

    ((double*)grid.extra_boundary_data_)[4*i]   = am_node_coordinates[2*thisEdge[0]];
    ((double*)grid.extra_boundary_data_)[4*i+1] = am_node_coordinates[2*thisEdge[0]+1];
    ((double*)grid.extra_boundary_data_)[4*i+2] = am_node_coordinates[2*thisEdge[1]];
    ((double*)grid.extra_boundary_data_)[4*i+3] = am_node_coordinates[2*thisEdge[1]+1];

    /* map Amira Material ID's to UG material ID's */
    /** \todo Make sure, this is neccessary */

    left++;
    right++;

    int renumNode[2];
    renumNode[0] = boundary_nodes[thisEdge[0]];
    renumNode[1] = boundary_nodes[thisEdge[1]];

    if (UG2d::CreateBoundarySegment(segmentName,
                                    left,           /*id of left subdomain */
                                    right,          /*id of right subdomain*/
                                    i,              /*id of segment*/
                                    UG2d::NON_PERIODIC,
                                    20,             // Resolution, whatever that means
                                    renumNode,
                                    &alpha,
                                    &beta,
                                    linearSegmentDescription2d,
                                    (double*)grid.extra_boundary_data_ + 4*i
                                    )==NULL) {
      std::cerr << "UG2d::CreateBoundarySegment failed!\n";
      delete am;
      return(1);
    }


  }


  /* That's it!*/

  delete am;
  return(0);


}

/** \todo Extend this such that it also reads double vertex positions */
void Dune::AmiraMeshReader<Dune::UGGrid<2,2> >::read(Dune::UGGrid<2,2>& grid,
                                                     const std::string& filename)
{

  //char DomainName[NAMESIZE];
  //double nodePos[DIM];
  int maxBndNodeID, noOfNodes, noOfElem, noOfCreatedElem;
  int i;
  UG2d::NODE* theNode;


  std::cout << "Loading 2D Amira mesh " << filename << "\n";

  ///////////////////////////////////////////////////////
  // Load the AmiraMesh file
  AmiraMesh* am = AmiraMesh::read(filename.c_str());

  if(!am) {
    std::cerr << "2d AmiraMesh reader: File could not be read!\n";
    return;
  }


  //loaddomain $file @PARA_FILE $name @DOMAIN
  CreateDomain(grid, "olisDomain", filename);

  // call configureCommand and newCommand
  grid.makeNewUGMultigrid();

  // get the different data fields
  AmiraMesh::Data* am_coordinateData =  am->findData("Nodes", HxFLOAT, 2, "Coordinates");
  if (!am_coordinateData) {
    std::cerr << "AmiraMesh loader: field 'Nodes' not found\n";
    return;
  }
  float*   am_node_coordinates       = (float*) am_coordinateData->dataPtr();

  AmiraMesh::Data* triangleData = am->findData("Triangles", HxINT32, 3, "Nodes");
  if (!triangleData) {
    std::cerr << "AmiraMesh loader: field 'Triangles' not found\n";
    return;
  }
  int*  elemData         = (int*)triangleData->dataPtr();


  /*
     All Boundary nodes are  assumed to be inserted already.
     We just have to insert the inner nodes and the elements
   */
  UG2d::multigrid* theMG = UG2d::GetMultigrid("DuneMG");
  assert(theMG);

  maxBndNodeID = -1;
  for (theNode=theMG->grids[0]->firstNode[0]; theNode!=NULL; theNode=theNode->succ)
  {
    // The following two lines ought to be in here, but the
    // OBJT macros is somewhat complicated, so I leave it out
    // for the time being.
    //       if(OBJT(theNode->myvertex) == UG3d::IVOBJ)
    //           UserWriteF("Warning: Node %d is inner node\n", ID(theNode));
    maxBndNodeID = MAX(theNode->id, maxBndNodeID);
  }

  std::cout << "Already " << maxBndNodeID+1 << " nodes existing\n";

  //noOfBndNodes = maxBndNodeID;

  noOfNodes = am->nElements("Nodes");
  noOfElem  = am->nElements("Triangles");

  //   noOfInnerNodes = noOfNodes - noOfBndNodes;
  std::cout << "AmiraMesh has " << noOfNodes << " total nodes\n";

  // Extract boundary faces
  std::vector<Vec<2, int> > face_list;
  detectBoundarySegments(elemData, noOfElem, face_list);
  std::vector<int> isBoundaryNode;
  detectBoundaryNodes(face_list, noOfNodes, isBoundaryNode);


  for(i = 0; i < noOfNodes; i++) {

    if (isBoundaryNode[i] != -1)
      continue;

    double nodePos[2];
    nodePos[0] = am_node_coordinates[2*i];
    nodePos[1] = am_node_coordinates[2*i+1];

    if (InsertInnerNode(theMG->grids[0], nodePos) == NULL)
    {
      std::cerr << "2d AmiraMesh reader: Inserting an inner node failed\n";
      return;
    }

    isBoundaryNode[i] = ++maxBndNodeID;

  }

  noOfCreatedElem = 0;
  for(i=0; i < noOfElem; i++) {

    int cornerIDs[3];

    /* only triangles */
    cornerIDs[0] = isBoundaryNode[elemData[3*i]-1];
    cornerIDs[1] = isBoundaryNode[elemData[3*i+1]-1];
    cornerIDs[2] = isBoundaryNode[elemData[3*i+2]-1];
    //printf("elem id : %d, node ids : %d %d %d\n", i, cornerIDs[0], cornerIDs[1], cornerIDs[2]);

    if (InsertElementFromIDs(theMG->grids[0], 3,cornerIDs, NULL) == NULL)
    {
      std::cerr << "2d AmiraMesh reader: Inserting an element failed\n";
      return;
    }
    noOfCreatedElem++;

  }

  std::cout << "amiraloadmesh: " << noOfCreatedElem << " elements created\n";

  // set the subdomainIDs
  AmiraMesh::Data* am_material_ids = am->findData("Triangles", HxBYTE, 1, "Materials");
  if (!am_material_ids)
    throw("Field 'Materials' not found.");

  //char* material_ids         = (char*)am_material_ids->dataPtr();

  i = 0;
  UG2d::ELEMENT* theElement;
  for (theElement=theMG->grids[0]->elements[0]; theElement!=NULL; theElement=theElement->ge.succ)
  {


#define ControlWord(p,ce) (((unsigned int *)(p))[UG2d::control_entries[ce].offset_in_object])
#define CW_WRITE(p, ce, n)   ControlWord(p,ce) = (ControlWord(p,ce)&UG2d::control_entries[ce].xor_mask)|(((n)<<UG2d::control_entries[ce].offset_in_word)&UG2d::control_entries[ce].mask)
#define SETSUBDOMAIN(p,n) CW_WRITE(p,UG2d::SUBDOMAIN_CE,n)
    /* get subdomain of element */
    //       int id = material_ids[i];
    //       SETSUBDOMAIN(theElement, id+1);
    SETSUBDOMAIN(theElement, 0);

#undef ControlWord
#undef CW_WRITE
#undef SETSUBDOMAIN

    i++;
  }

  delete am;

  UG2d::SetEdgeAndNodeSubdomainFromElements(theMG->grids[0]);


  /** \bug This needs to be back in for grid refinement! */
  if (CreateAlgebra(theMG) != UG2d::GM_OK)
    return;

  /* here all temp memory since CreateMultiGrid is released */
#define ReleaseTmpMem(p,k) Release(p, UG2d::FROM_TOP,k)
  ReleaseTmpMem(theMG->theHeap, theMG->MarkKey);
#undef ReleaseTmpMem
  theMG->MarkKey = 0;

}



#endif // #ifdef _2
