// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// ///////////////////////////////////////////////
// Specialization of the AmiraMesh reader for UGGrid<3,3>
// ///////////////////////////////////////////////

#include "../../grid/uggrid.hh"

#include <amiramesh/AmiraMesh.h>
#include "../../../AmiraLibs/include/AmiraParamAccess.h"

namespace Dune {

  template<>
  class AmiraMeshReader<UGGrid<3,3> > {

  public:

    static void read(UGGrid<3,3>& grid,
                     const std::string& filename);

  protected:
    static int CreateDomain(UGGrid<3,3>& grid,
                            const std::string& domainName,
                            const std::string& filename);

    static void readHexaGrid(UGGrid<3,3>& grid, AmiraMesh* am);

    static int createHexaDomain(UGGrid<3,3>& grid, AmiraMesh* am,
                                const std::string& filename);

    static void detectBoundarySegments(int* elemData,
                                       int noOfElem,
                                       std::vector<Vec<4, int> >& face_list);

    static int detectBoundaryNodes(const std::vector<Vec<4, int> >& face_list,
                                   unsigned int noOfNodes,
                                   std::vector<bool>& isBoundaryNode);

    AmiraMeshReader() {}

  };

}


// //////////////////////////////////////////////////
// //////////////////////////////////////////////////
static int SegmentDescriptionByAmira(void *data, double *param, double *result)
{

  int triNum = * (int*) data;

  //printf("tri = %d, l0 = %5.5f, l1 = %5.5f\n", triNum, param[0], param[1]);

  double barCoords[2];
  double A[4] = {-1, 1, 0, -1};
  double b[2] = {1, 0};

  // barCoords = A*param + b;
  barCoords[0] = A[0]*param[0] + A[2]*param[1];
  barCoords[1] = A[1]*param[0] + A[3]*param[1];

  barCoords[0] += b[0];
  barCoords[1] += b[1];

  /*
     An dieser Stelle wird vorausgesetzt, dass das bereits gesetzte Gebiet
     nicht mehr geaendert wird
   */
  //printf("barCoords:  (%g %g)\n", barCoords[0], barCoords[1]);

  const float eps = 1e-6;
  if (barCoords[0]<-eps || barCoords[1]<-eps || (1-barCoords[0]-barCoords[1])<-eps) {
    printf("Illegal barycentric coordinate\n");
    assert(false);
  }

  AmiraCallPositionParametrization(triNum, barCoords, result);

  return(0);
}

#ifdef _3
int Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::CreateDomain(UGGrid<3,3>& grid,
                                                            const std::string& domainName,
                                                            const std::string& filename)
{
  const int CORNERS_OF_BND_SEG = 4;


  int point[CORNERS_OF_BND_SEG] = {-1, -1, -1, -1};
  double radius, MidPoint[3], alpha[2], beta[2];

  /* Alle Aufrufe an Deine Bibliothek beginnen mit "Amira" */

  /* Load data */
  if(AmiraLoadMesh(domainName.c_str(), filename.c_str()) != AMIRA_OK)
  {
    UG3d::PrintErrorMessage('E', "CreateAmiraParametrization", "Domain not found");
    return(1);
  }

  if(AmiraStartEditingDomain(domainName.c_str()) != AMIRA_OK)
  {
    UG3d::PrintErrorMessage('E', "AmiraStartEditing", "Domain not found");
    return(1);
  }


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


  /* Das naechste ist neu. Wir brauchen eine Kugel, die das Gebiet komplett  enthaelt.
     Diese Information wird fuer die UG-Graphik benoetigt, die Kugel kann also durchaus
     zu gross oder zu klein sein */

  AmiraGetMidpointAndRadius(MidPoint, &radius);

  //     printf("Amira radius = %5.2f, Midpoint = (%5.2f, %5.2f, %5.2f)\n",
  //            radius, MidPoint[0], MidPoint[1], MidPoint[2]);


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

  int* segmentIndexList = NULL;
  /** \bug This memory gets never freed! */
  segmentIndexList = (int*) ::malloc(noOfSegments*sizeof(int));

  if(segmentIndexList == NULL)
    return(1);

  for(int i = 0; i < noOfSegments; i++) {

    //std::string segmentName;
    char segmentName[200];
    int left, right;

    AmiraGetNodeNumbersOfSegment(point, i);

    /* Es werden die Ecknummern eines Randsegmentes zurueckgegeben
       point[0] = 0; point[1]=1; point[2]=2; point[3]=3; */

    //segmentName = "AmiraSegment";
    if(sprintf(segmentName, "AmiraSegment %d", i) < 0)
      return(1);

    /* left = innerRegion, right = outerRegion */
    AmiraGetLeftAndRightSideOfSegment(&left, &right, i);

    //             printf("id = %d l = %d, r = %d\n", i, left, right);
    //             printf("id = %d nd0 = %d, nd1 = %d, nd2 = %d\n", i, point[0], point[1], point[2]);

    segmentIndexList[i] = i;

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
                                    (void *) (segmentIndexList+i)
                                    )==NULL)
      return(1);

  }

  printf("%d segments created!\n", noOfSegments);

  /* That's it!*/
  return(0);


}


/** \todo Clear grid before reading! */
//template<>
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
  CreateDomain(grid, "olisDomain", "cube.par.am");

  //configure @PROBLEM $d @DOMAIN;
  char* configureArgs[2] = {"configure DuneDummyProblem", "d olisDomain"};
  UG3d::ConfigureCommand(2, configureArgs);

  //new @PROBLEM $b @PROBLEM $f @FORMAT $h @HEAP;
  char* newArgs[4] = {"new DuneMG", "b DuneDummyProblem", "f DuneFormat", "h 1G"};
  if (UG3d::NewCommand(4, newArgs))
    assert(false);


  // ////////////////////////////////////////////
  // loadmesh $file @GRID_FILE $name @DOMAIN;

  const int DIM = 3;

  int i;
  double nodePos[DIM];
  UG3d::NODE* theNode;

  std::cout << "Loading Amira mesh " <<  filename << "\n";



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
    if (InsertInnerNode(theMG->grids[0], nodePos) == NULL)
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

    if (InsertElementFromIDs(theMG->grids[0], 4,cornerIDs, NULL) == NULL)
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



  //   if (MG_COARSE_FIXED(theMG))
  //     return (GM_OK);
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

int Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::detectBoundaryNodes(const std::vector<Vec<4, int> >& face_list,
                                                                   unsigned int noOfNodes,
                                                                   std::vector<bool>& isBoundaryNode)
{
  unsigned int i, j;

  //! \todo Should be a bitfield
  isBoundaryNode.resize(noOfNodes);
  for (i=0; i<noOfNodes; i++)
    isBoundaryNode[i] = false;

  for (i=0; i<face_list.size(); i++)
    for (j=0; j<4; j++)
      isBoundaryNode[face_list[i][j]] = true;

  // Count number of boundary nodes
  int count=0;
  for (i=0; i<noOfNodes; i++)
    count += isBoundaryNode[i];

  return count;
}

#if 0
/** This method implements a linear function in order to be able to
 *  work with straight line boundaries.
 *  We interpret data as a DOUBLE* to the world coordinates of the
 *  two endpoints.
 *
 * \todo This should actually be replaced by using LinearSegments
 * instead of BoundarySegments.  But LinearSegments are buggy in UG.
 */
static int LinearSegmentDescription(void *data, double *param, double *result)
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

  // linear interpolation
  for (int i=0; i<3; i++)
    result[i] = a[i] + param[0]*(b[i]-a[i]) + param[1]*(c[i]-a[i]);

  printf("param: %g %g\n", param[0], param[1]);
  printf("corners:  (%d %d %d %d)\n",
         ((int*)data) [0],  ((int*)data) [1],  ((int*)data) [2],
         ((int*)data) [3]);

  return 0;
}
#endif

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

  printf("%d boundary segments found!\n", face_list.size());
  for (unsigned int i=0; i<face_list.size(); i++) {
    //face_list[i].print(cout, 3);

    cout << face_list[i] << "\n";
  }

  int noOfNodes = am->nElements("Nodes");

  std::vector<bool> isBoundaryNode;

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
  printf("A\n");

  grid.makeNewUGMultigrid();
#if 0
  //configure @PROBLEM $d @DOMAIN;
  char* configureArgs[2] = {"configure DuneDummyProblem", "d olisDomain"};
  UG3d::ConfigureCommand(2, configureArgs);
  printf("B\n");

  //new @PROBLEM $b @PROBLEM $f @FORMAT $h @HEAP;
  char* newArgs[4] = {"new DuneMG", "b DuneDummyProblem", "f DuneFormat", "h 1G"};
  if (UG3d::NewCommand(4, newArgs))
    assert(false);
#endif
  printf("C\n");


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

    //         static void readHexaGrid(UGGrid<3,3>& grid, AmiraMesh* am);

    //         static int createHexaDomain(UGGrid<3,3>& grid, AmiraMesh* am,
    //                                     const std::string& filename);

    //         static void detectBoundarySegments(int* elemData,
    //                                            int noOfElem,
    //                                            std::vector<Vec<4, int> >& face_list);

    //         static int detectBoundaryNodes(const std::vector<Vec<4, int> >& face_list,
    //                                        unsigned int noOfNodes,
    //                                        std::vector<bool>& isBoundaryNode);

  };

}

#ifdef _2
// A little temporary class I need
class Edge {
public:
  int idx[2];

  Edge(int a, int b) {
    idx[0] = a;
    idx[1] = b;
  }

  bool operator==(const Edge& other) {
    return (idx[0]==other.idx[0] && idx[1]==other.idx[1]) ||
           (idx[0]==other.idx[1] && idx[1]==other.idx[0]);
  }
};
#endif

#ifdef _2
/** This method implements a linear function in order to be able to
 *  work with straight line boundaries.
 *  We interpret data as a DOUBLE* to the world coordinates of the
 *  two endpoints.
 */
static int LinearSegmentDescription(void *data, double *param, double *result)
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
static Edge* DetectBoundarySegments(int* elemData, int noOfElem, int& noOfBSegments)
{
  int i, j, k;
  // I'm allocating a large number of Edges and hope there'll be enough space
  // I would really want some dynamical array for this
  Edge* boundary_edges = (Edge*)malloc(2*noOfElem*sizeof(Edge));

  for (i=0; i<noOfElem; i++) {

    for (j=0; j<3; j++) {

      Edge thisEdge = Edge(elemData[3*i + j], elemData[3*i + ((j+1)%3)]);
      bool isBoundaryEdge = true;

      //printf("Checking edge (%d %d)\n", thisEdge.idx[0], thisEdge.idx[1]);
      for (k=0; k<noOfElem; k++) {

        if (k==i)
          continue;

        Edge otherEdge0 = Edge(elemData[3*k],   elemData[3*k+1]);
        Edge otherEdge1 = Edge(elemData[3*k+1], elemData[3*k+2]);
        Edge otherEdge2 = Edge(elemData[3*k+2], elemData[3*k+0]);

        if (thisEdge==otherEdge0 || thisEdge==otherEdge1 || thisEdge==otherEdge2) {

          isBoundaryEdge = false;
          break;

        }

      }

      if (isBoundaryEdge) {
        //printf("isBoundary!\n");
        boundary_edges[noOfBSegments++] = thisEdge;

        if (noOfBSegments>2*noOfElem)
          assert(false);

      }
    }
  }

  return boundary_edges;
}

static int* DetectBoundaryNodes(int* elemData, int noOfElem, int noOfNodes)
{
  int i, j, k;
  int* result = (int*)malloc(noOfNodes*sizeof(int));

  int UGNodeIdxCounter = 0;

  for (i=0; i<noOfNodes; i++)
    result[i] = -1;

  for (i=0; i<noOfElem; i++) {

    for (j=0; j<3; j++) {

      Edge thisEdge = Edge(elemData[3*i + j], elemData[3*i + ((j+1)%3)]);
      bool isBoundaryEdge = true;

      //printf("Checking edge (%d %d)\n", thisEdge.idx[0], thisEdge.idx[1]);
      for (k=0; k<noOfElem; k++) {

        if (k==i)
          continue;

        Edge otherEdge0 = Edge(elemData[3*k],   elemData[3*k+1]);
        Edge otherEdge1 = Edge(elemData[3*k+1], elemData[3*k+2]);
        Edge otherEdge2 = Edge(elemData[3*k+2], elemData[3*k+0]);

        if (thisEdge==otherEdge0 || thisEdge==otherEdge1 || thisEdge==otherEdge2) {

          isBoundaryEdge = false;
          break;

        }

      }

      if (isBoundaryEdge) {

        if (result[thisEdge.idx[0]] == -1)
          result[thisEdge.idx[0]] = UGNodeIdxCounter++;
        if (result[thisEdge.idx[1]] == -1)
          result[thisEdge.idx[1]] = UGNodeIdxCounter++;

      }
    }
  }

  return result;
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

  cout << "Loading 2D Amira domain " << filename << "\n";

  ///////////////////////////////////////////////////////
  // Load the AmiraMesh file
  AmiraMesh* am = AmiraMesh::read(filename.c_str());

  if(!am) {
    cerr << "AmiraMeshReader<UGGrid<2,2> >::CreateDomain: Can't open input file\n";
    return(1);
  }

  // get the different data fields
  AmiraMesh::Data* am_coordinateData =  am->findData("Nodes", HxFLOAT, 2, "Coordinates");
  if (!am_coordinateData) {
    cerr << "2D AmiraMesh loader: field 'Nodes' not found!\n";
    return(1);
  }
  float*   am_node_coordinates = (float*) am_coordinateData->dataPtr();

  AmiraMesh::Data* triangleData = am->findData("Triangles", HxINT32, 3, "Nodes");
  if (!triangleData) {
    cerr << "2D AmiraMesh loader: field 'Triangles' not found!\n";
    return(1);
  }
  int*  elemData         = (int*)triangleData->dataPtr();


  int noOfNodes = am->nElements("Nodes");
  int noOfElem  = am->nElements("Triangles");
  int noOfBSegments = 0;

  std::cout << "AmiraMesh has " << noOfNodes << " total nodes and "
            << noOfElem << " triangles\n";

  // Extract boundary segments
  Edge* boundary_segments = DetectBoundarySegments(elemData, noOfElem, noOfBSegments);
  if (!boundary_segments) {
    delete am;
    return 1;
  }

  std::cout << noOfBSegments << " Boundary segments found!\n";

  // extract boundary nodes
  int* boundary_nodes = DetectBoundaryNodes(elemData, noOfElem, noOfNodes);
  if (!boundary_nodes) {
    delete am;
    return (1);
  }

  int noOfBNodes = 0;
  for (i=0; i<noOfNodes; i++)
    if (boundary_nodes[i] != -1)
      noOfBNodes++;

  std::cout << noOfBNodes << " boundary nodes found!\n";


  /* Jetzt geht es an's eingemachte. Zuerst wird ein neues gebiet konstruiert und
     in der internen UG Datenstruktur eingetragen */
  /* allocate memory for the different descriptors */

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
  /** \bug This memory gets never freed! */
  double* segmentCoordList = (double *) ::malloc(4*noOfBSegments*sizeof(double));

  if(segmentCoordList == NULL) {
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


    const Edge& thisEdge = boundary_segments[i];

    segmentCoordList[4*i]   = am_node_coordinates[2*thisEdge.idx[0]];
    segmentCoordList[4*i+1] = am_node_coordinates[2*thisEdge.idx[0]+1];
    segmentCoordList[4*i+2] = am_node_coordinates[2*thisEdge.idx[1]];
    segmentCoordList[4*i+3] = am_node_coordinates[2*thisEdge.idx[1]+1];

    /* map Amira Material ID's to UG material ID's */
    /** \todo Make sure, this is neccessary */

    left++;
    right++;

    int renumNode[2];
    renumNode[0] = boundary_nodes[thisEdge.idx[0]];
    renumNode[1] = boundary_nodes[thisEdge.idx[1]];

    if (UG2d::CreateBoundarySegment(segmentName,
                                    left,           /*id of left subdomain */
                                    right,          /*id of right subdomain*/
                                    i,              /*id of segment*/
                                    UG2d::NON_PERIODIC,
                                    20,             // Resolution, whatever that means
                                    renumNode,
                                    &alpha,
                                    &beta,
                                    LinearSegmentDescription,
                                    segmentCoordList + 4*i
                                    )==NULL) {
      cerr << "UG2d::CreateBoundarySegment failed!\n";
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


  cout << "Loading 2D Amira mesh " << filename << "\n";

  ///////////////////////////////////////////////////////
  // Load the AmiraMesh file
  AmiraMesh* am = AmiraMesh::read(filename.c_str());

  if(!am) {
    cerr << "2d AmiraMesh reader: File could not be read!\n";
    return;
  }


  //loaddomain $file @PARA_FILE $name @DOMAIN
  CreateDomain(grid, "olisDomain", filename);

  //configure @PROBLEM $d @DOMAIN;
  char* configureArgs[2] = {"configure DuneDummyProblem", "d olisDomain"};
  UG2d::ConfigureCommand(2, configureArgs);

  //new @PROBLEM $b @PROBLEM $f @FORMAT $h @HEAP;
  char* newArgs[4] = {"new DuneMG", "b DuneDummyProblem", "f DuneFormat", "h 1G"};
  if (UG2d::NewCommand(4, newArgs))
    assert(false);


  // get the different data fields
  AmiraMesh::Data* am_coordinateData =  am->findData("Nodes", HxFLOAT, 2, "Coordinates");
  if (!am_coordinateData) {
    cerr << "AmiraMesh loader: field 'Nodes' not found\n";
    return;
  }
  float*   am_node_coordinates       = (float*) am_coordinateData->dataPtr();

  AmiraMesh::Data* triangleData = am->findData("Triangles", HxINT32, 3, "Nodes");
  if (!triangleData) {
    cerr << "AmiraMesh loader: field 'Triangles' not found\n";
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
  cout << "AmiraMesh has " << noOfNodes << " total nodes\n";

  int* isBoundaryNode = DetectBoundaryNodes(elemData, noOfElem, noOfNodes);


  for(i = 0; i < noOfNodes; i++) {

    if (isBoundaryNode[i] != -1)
      continue;

    double nodePos[2];
    nodePos[0] = am_node_coordinates[2*i];
    nodePos[1] = am_node_coordinates[2*i+1];

    if (InsertInnerNode(theMG->grids[0], nodePos) == NULL)
    {
      cerr << "2d AmiraMesh reader: Inserting an inner node failed\n";
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
      cerr << "2d AmiraMesh reader: Inserting an element failed\n";
      return;
    }
    noOfCreatedElem++;

  }

  cout << "amiraloadmesh: " << noOfCreatedElem << " elements created\n";

  // set the subdomainIDs
  AmiraMesh::Data* am_material_ids = am->findData("Triangles", HxBYTE, 1, "Materials");
  if (!am_material_ids)
    throw("Field 'Materials' not found.");

  char* material_ids         = (char*)am_material_ids->dataPtr();

  i = 0;
  UG2d::ELEMENT* theElement;
  for (theElement=theMG->grids[0]->elements[0]; theElement!=NULL; theElement=theElement->ge.succ)
  {

    /* get subdomain of element */
    int id = material_ids[i];

#define ControlWord(p,ce) (((unsigned int *)(p))[UG2d::control_entries[ce].offset_in_object])
#define CW_WRITE(p, ce, n)   ControlWord(p,ce) = (ControlWord(p,ce)&UG2d::control_entries[ce].xor_mask)|(((n)<<UG2d::control_entries[ce].offset_in_word)&UG2d::control_entries[ce].mask)
#define SETSUBDOMAIN(p,n) CW_WRITE(p,UG2d::SUBDOMAIN_CE,n)

    SETSUBDOMAIN(theElement, /*id+*/ 1);

#undef ControlWord
#undef CW_WRITE
#undef SETSUBDOMAIN

    i++;
  }

  delete am;

  //   if(SetSubdomainIDsFromAmira(theMG, filename, noOfElem))
  //     PrintErrorMessage('W', "SetSubdomainIDsFromAmira","setting id's failed");
  UG2d::SetEdgeAndNodeSubdomainFromElements(theMG->grids[0]);


  /** \bug This needs to be back in for grid refinement! */
  //   if (CreateAlgebra(theMG) != UG2d::GM_OK)
  //       return;

  /* here all temp memory since CreateMultiGrid is released */
#define ReleaseTmpMem(p,k) Release(p, UG2d::FROM_TOP,k)
  ReleaseTmpMem(theMG->theHeap, theMG->MarkKey);
#undef ReleaseTmpMem
  theMG->MarkKey = 0;

}



#endif // #ifdef _2
