// NOTE: The current revision of this file was left untouched when the DUNE source files were reindented!
// NOTE: It contained invalid syntax that could not be processed by uncrustify.

// /////////////////////////////////////////////////////////////////////////
// Specialization of the AmiraMesh reader for UGGrid<3,3> and UGGrid<2,2>
// /////////////////////////////////////////////////////////////////////////

#include <dune/grid/uggrid.hh>

#include <amiramesh/AmiraMesh.h>

#include <vector>
#include <algorithm>

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

        static void CreateDomain(UGGrid<3,3>& grid,
                                const std::string& domainName,
                                AmiraMesh* am);

        static void buildGrid(UGGrid<3,3>& grid, AmiraMesh* am);

        static void readHexaGrid(UGGrid<3,3>& grid, AmiraMesh* am);

        static void createHexaDomain(UGGrid<3,3>& grid, AmiraMesh* am);

        static void detectBoundarySegments(int* elemData, 
                                           int noOfElem, 
                                           std::vector<FieldVector<int, 4> >& face_list);

        static void detectBoundarySegments(int* elemData, 
                                           int noOfElem, 
                                           std::vector<FieldVector<int, 3> >& face_list);

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
  
    //   printf("result: %g %g %g\n", result[0], result[1], result[2]);
  return 0;
}
#endif // #ifdef _3

/** \todo This is quadratic --> very slow */
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::detectBoundarySegments(int* elemData, 
                                                                       int numTetras, 
                                                                       std::vector<FieldVector<int, 3> >& face_list)
{
    int i, j;

    static const int idx[][3] = {
         {3,2,0},{1,2,3},{1,3,0},{2,1,0}
    };

    face_list.resize(0);

    for (i=0; i<numTetras; i++) {

        for (int k=0; k<4; k++) {
            FieldVector<int, 3> v;
            v[0] = elemData[4*i+idx[k][0]];
            v[1] = elemData[4*i+idx[k][1]];
            v[2] = elemData[4*i+idx[k][2]];


            // Check if new face exists already in the list
            // (then it is no boundary face)
            for (j=0; j<(int)face_list.size(); j++) {

                const FieldVector<int, 3>& o = face_list[j];
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
static int detectBoundaryNodes(const std::vector< Dune::FieldVector<int, NUM_VERTICES> >& face_list, 
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
    
    /* Load data */
    if(AmiraLoadMesh(domainName.c_str(), filename.c_str()) != AMIRA_OK)
        {
            std::cerr << "Error in AmiraMeshReader<Dune::UGGrid<3,3> >::CreateDomain:"
                 << "Domain file could not be opened!" << std::endl;
            return 1;
        }

    if(AmiraStartEditingDomain(domainName.c_str()) != AMIRA_OK)
        {
            std::cerr << "Error in AmiraMeshReader<Dune::UGGrid<3,3> >::CreateDomain:"
                      << "StartEditing failed!" << std::endl;
            return 1;
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
  

    /* Wir brauchen eine Kugel, die das Gebiet komplett  enthaelt.  Diese Information 
       wird für die UG-Graphik benoetigt, die Werte sind hier also komplett egal. */
    double radius = 1;
    double MidPoint[3] = {0, 0, 0};
  
    /* Jetzt geht es an's eingemachte. Zuerst wird ein neues gebiet konstruiert und
       in der internen UG Datenstruktur eingetragen */
    
    UG3d::domain* newDomain = UG3d::CreateDomain(domainName.c_str(), 
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
                                        left,         /*id of left subdomain */
                                        right,        /*id of right subdomain*/
                                        i,            /*id of segment*/
                                        UG3d::NON_PERIODIC,
                                        1,          // resolution, whatever that is
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
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::CreateDomain(UGGrid<3,3>& grid,
                                                             const std::string& domainName,
                                                             AmiraMesh* am)
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
    std::vector<FieldVector<int, 3> > face_list;
    detectBoundarySegments(elemData, noOfElem, face_list);

    if(face_list.size() == 0)
        {
            delete am;
            DUNE_THROW(IOError, "createTetraDomain: no segments found");
        }

    int nBndSegments = face_list.size();

    std::cout << face_list.size() << " boundary segments found!" << std::endl;
//     for (unsigned int i=0; i<face_list.size(); i++){
//         std::cout << face_list[i] << "\n";
//     }
  
    int noOfNodes = am->nElements("Nodes");

    std::vector<int> isBoundaryNode;

    int nBndNodes = detectBoundaryNodes(face_list, noOfNodes, isBoundaryNode);
    if(nBndNodes <= 0)
        DUNE_THROW(IOError, "CreateDomain: no boundary nodes found");

//     for (int ii=0; ii<isBoundaryNode.size(); ii++)
//         printf("%d\n", isBoundaryNode[ii]);

    std::cout << nBndNodes << " boundary nodes found!" << std::endl;
  

    /* Zuerst wird ein neues gebiet konstruiert und
       in der internen UG Datenstruktur eingetragen */
    UG3d::domain* newDomain = (UG3d::domain*) UG3d::CreateDomain(domainName.c_str(), 
                                                                 midPoint, radius, 
                                                                 face_list.size(), nBndNodes, 
                                                                 false);


    if (!newDomain) {
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
            
        point[0] = face_list[i][0];
        point[1] = face_list[i][1];
        point[2] = face_list[i][2];
        //point[3] = face_list[i][3];
            
        if(sprintf(segmentName, "Segment %d", i) < 0)
            DUNE_THROW(IOError, "CreateDomain: sprintf returned error value");
            
        /* left = innerRegion, right = outerRegion */
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
                                        left,         /*id of left subdomain */
                                        right,        /*id of right subdomain*/
                                        i,            /*id of segment*/
                                        UG3d::NON_PERIODIC,
                                        1,          // resolution, needed for UG graphics
                                        point,
                                        alpha, beta,
                                        linearSegmentDescription3d,
                                        ((double*)grid.extra_boundary_data_)+9*i
                                      )==NULL) 
            DUNE_THROW(IOError, "Error calling CreateBoundarySegment");;

#else
        // It would be a lot smarter to use this way of describing
        // boundary segments.  But as of yet, UG crashes when using
        // linear segments.
        double paramCoords[3][2] = {{0,0}, {1,0}, {0,1}};
        if (UG3d::CreateLinearSegment(segmentName,
                                      left,         /*id of left subdomain */
                                      right,        /*id of right subdomain*/
                                      i,            /*id of segment*/
                                      4,            // Number of corners      
                                      point,
                                      paramCoords
                                      )==NULL) 
            DUNE_THROW(IOError, "Error calling CreateLinearSegment");;
#endif
    }
    
    std::cout << nBndSegments << " segments created!" << std::endl;
} 


#ifdef __USE_PARAMETRIZATION_LIBRARY__
/** \todo Clear grid before reading! */
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::read(Dune::UGGrid<3,3>& grid, 
                                                     const std::string& filename,
                                                     const std::string& domainFilename)
{
    std::cout << ("This is the AmiraMesh reader for UGGrid<3,3>!" << std::endl;

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
    CreateDomain(grid, domainName, domainFilename);

    // read and build the grid
    buildGrid(grid, am);
} 
#endif // #define __USE_PARAMETRIZATION_LIBRARY__


/** \todo Clear grid before reading! */
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::read(Dune::UGGrid<3,3>& grid, 
                                  const std::string& filename) 
{
    std::cout << "This is the AmiraMesh reader for UGGrid<3,3>!" << std::endl;

    // /////////////////////////////////////////////////////
    // Load the AmiraMesh file
    AmiraMesh* am = AmiraMesh::read(filename.c_str());
      
    if(!am)
        DUNE_THROW(IOError, "read: Could not open AmiraMesh file");

    if (am->findData("Hexahedra", HxINT32, 8, "Nodes")) {
        readHexaGrid(grid, am);
        return;
    }

    // Construct a domain name from the multigrid name
    std::string domainName = grid.name() + "_Domain";

    //loaddomain $file @PARA_FILE $name @DOMAIN
    CreateDomain(grid, domainName, am);

    buildGrid(grid, am);

}


void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::buildGrid(UGGrid<3,3>& grid, AmiraMesh* am)
{
    bool isTetraGrid = am->findData("Tetrahedra", HxINT32, 4, "Nodes");

    // call configureCommand and newCommand
    grid.makeNewUGMultigrid();

    // ////////////////////////////////////////////
    // loadmesh $file @GRID_FILE $name @DOMAIN;

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
            throw("No vertex coordinates found in the file!");
        
    }


  AmiraMesh::Data* elementData = (isTetraGrid) 
      ? am->findData("Tetrahedra", HxINT32, 4, "Nodes")
      : am->findData("Hexahedra", HxINT32, 8, "Nodes");

  int*  elemData         = (int*)elementData->dataPtr();

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
      maxBndNodeID = std::max(theNode->id, maxBndNodeID);
  }

  std::cout << "Already " << maxBndNodeID+1 << " nodes existing\n";


  int noOfBndNodes = maxBndNodeID;


  int noOfNodes = am->nElements("Nodes");
  
  //  noOfInnerNodes = noOfNodes - noOfBndNodes;
  std::cout << "AmiraMesh has " << noOfNodes << " total nodes." << std::endl;

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
  int noOfElem = (isTetraGrid)
      ? am->nElements("Tetrahedra")
      : am->nElements("Hexahedra");

  int noOfCreatedElem = 0;
  int numberOfCorners = (isTetraGrid) ? 4 : 8;

  for(i=0; i < noOfElem; i++) {

      int cornerIDs[numberOfCorners];
      
      for (int j=0; j<numberOfCorners; j++)
          cornerIDs[j] = elemData[numberOfCorners*i+j]-1;
      
      /// \todo Warum ist nicht UG3d::InsertElementFromIDs Pflicht???
      if (InsertElementFromIDs(grid.multigrid_->grids[0], numberOfCorners, cornerIDs, NULL) == NULL)
          throw("inserting an element failed");
      
      noOfCreatedElem++;
      
  }


  if(noOfElem != noOfCreatedElem)
      throw("inserting an element failed");
  
  std::cout << "AmiraMesh reader: " << noOfCreatedElem << " elements created.\n";

  // set the subdomainIDs
  char* material_ids = 0;

  AmiraMesh::Data* am_material_ids = (isTetraGrid)
      ? am->findData("Tetrahedra", HxBYTE, 1, "Materials")
      : am->findData("Hexahedra", HxBYTE, 1, "Materials");

  if (am_material_ids) 
      material_ids = (char*)am_material_ids->dataPtr();

  i = 0;
  UG3d::ELEMENT* theElement;
  for (theElement=grid.multigrid_->grids[0]->elements[0]; theElement!=NULL; theElement=theElement->ge.succ)
    {

      /* get subdomain of element */
      int id = -1;

      if (material_ids)
          id = material_ids[i];
#if 0
#define ControlWord(p,ce) (((unsigned int *)(p))[UG3d::control_entries[ce].offset_in_object])
#define CW_WRITE(p, ce, n)   ControlWord(p,ce) = (ControlWord(p,ce)&UG3d::control_entries[ce].xor_mask)|(((n)<<UG3d::control_entries[ce].offset_in_word)&UG3d::control_entries[ce].mask)
#define SETSUBDOMAIN(p,n) CW_WRITE(p,UG3d::SUBDOMAIN_CE,n)

//       assert(id != -1);
//       SETSUBDOMAIN(theElement, id+1);
      SETSUBDOMAIN(theElement, 1);

#undef ControlWord
#undef CW_WRITE
#undef SETSUBDOMAIN
#endif
      UG_NS<3>::SetSubdomain(theElement, 1);

      i++;
    }

  delete am;

  UG3d::SetEdgeAndNodeSubdomainFromElements(grid.multigrid_->grids[0]);


  /* This call completes the creation of the internal UG
     grid data structure */
   if (UG3d::CreateAlgebra(grid.multigrid_) != UG3d::GM_OK)
       throw("Error in UG3d::CreateAlgebra!");
  
  /** \todo Check whether this release is necessary */
  /* here all temp memory since CreateMultiGrid is released */
  //UG3d::ReleaseTmpMem(MGHEAP(theMG),MG_MARK_KEY(theMG));
#define ReleaseTmpMem(p,k) Release(p, UG::FROM_TOP,k)
  ReleaseTmpMem(grid.multigrid_->theHeap, grid.multigrid_->MarkKey);
#undef ReleaseTmpMem
  grid.multigrid_->MarkKey = 0;
  
}



/*****************************************************************/
/* Read the UGGrid from an AmiraMesh Hexagrid file               */
/*****************************************************************/

/** \todo This is quadratic --> very slow */
/** \todo Handle triangular boundary segments */
void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::detectBoundarySegments(int* elemData, 
                                                                       int numHexas, 
                                                                       std::vector<FieldVector<int, 4> >& face_list)
{
    static const int idx[][4] = {
         {0,4,5,1},{1,5,6,2},{2,6,7,3},{3,7,4,0},{4,7,6,5},{1,2,3,0}
    };

    face_list.resize(0);

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
            for (j=0; j<(int)face_list.size(); j++) {

                const FieldVector<int, 4>& o = face_list[j];
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

        

            if (j<(int)face_list.size()) {
                // face has been found
                face_list[j] = face_list.back();
                face_list.pop_back();

            } else {
                // Insert k-th face of i-th hexahedron into face list
                face_list.push_back(v);
            }
        }
    }

}


void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::createHexaDomain(UGGrid<3,3>& grid, 
                                                                 AmiraMesh* am)
{
    const int CORNERS_OF_BND_SEG = 4;
    const int DIMWORLD = 3;
    
    int point[CORNERS_OF_BND_SEG] = {-1, -1, -1, -1};
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
            throw("No vertex coordinates found in the file!");
        
    }

    AmiraMesh::Data* hexahedronData = am->findData("Hexahedra", HxINT32, 8, "Nodes");
    int*  elemData         = (int*)hexahedronData->dataPtr();
    int noOfElem = am->nElements("Hexahedra");

    // Extract boundary faces
    std::vector<FieldVector<int, 4> > face_list;
    detectBoundarySegments(elemData, noOfElem, face_list);

    if(face_list.size() == 0)
        DUNE_THROW(IOError, "CreateHexaDomain: no boundary segments extracted");

    int nBndSegments = face_list.size();

    std::cout << face_list.size() << " boundary segments found!" << std::endl;
  
    int noOfNodes = am->nElements("Nodes");

    std::vector<int> isBoundaryNode;

    int nBndNodes = detectBoundaryNodes(face_list, noOfNodes, isBoundaryNode);
    if(nBndNodes <= 0)
        DUNE_THROW(IOError, "createHexaDomain: no nodes found");

    std::cout << nBndNodes << " boundary nodes found!" << std::endl;
  
    
    /* Zuerst wird ein neues gebiet konstruiert und
       in der internen UG Datenstruktur eingetragen */

    // Construct a domain name from the multigrid name
    std::string domainName = grid.name() + "_Domain";

    UG3d::domain* newDomain = (UG3d::domain*)UG3d::CreateDomain(domainName.c_str(), 
                                                                midPoint, radius, 
                                                                face_list.size(), nBndNodes, 
                                                                false);

    if (!newDomain)
        DUNE_THROW(IOError, "createHexaDomain: UG3d::CreateDomain returned NULL");
 
    /* Alle weiteren Aufrufe von 'CreateBoundarySegment' beziehen sich jetzt auf das eben
       erzeugte Gebiet */
    
        /*
      Die Koordinaten der Eckknoten wird als user data an das jeweilige
       Segment weitergereicht.
    */
    grid.extra_boundary_data_ = ::malloc(nBndSegments*CORNERS_OF_BND_SEG*DIMWORLD*sizeof(double));
    
    if(grid.extra_boundary_data_ == NULL)
        DUNE_THROW(IOError, "createHexaDomain: couldn't allocate extra_boundary_data");
    
    for(int i = 0; i < nBndSegments; i++) {

        //std::string segmentName;
        char segmentName[20];

        // bordering subdomains
        int left = 1;
        int right = 2;
            
        // change around ordering
        /** \todo This can obviously be programmed more concisely */
        int tmp = face_list[i][0];
        face_list[i][0] = face_list[i][3];
        face_list[i][3] = tmp;
        tmp = face_list[i][1];
        face_list[i][1] = face_list[i][2];
        face_list[i][2] = tmp;
        

        point[0] = face_list[i][0];
        point[1] = face_list[i][1];
        point[2] = face_list[i][2];
        point[3] = face_list[i][3];
            
        if(sprintf(segmentName, "Segment %d", i) < 0)
            DUNE_THROW(IOError, "createHexaDomain: sprintf returned error value");
            
        /* left = innerRegion, right = outerRegion */
        ((double*)grid.extra_boundary_data_)[12*i+0]  = am_node_coordinates_float[DIMWORLD*face_list[i][0] + 0];
        ((double*)grid.extra_boundary_data_)[12*i+1]  = am_node_coordinates_float[DIMWORLD*face_list[i][0] + 1];
        ((double*)grid.extra_boundary_data_)[12*i+2]  = am_node_coordinates_float[DIMWORLD*face_list[i][0] + 2];
        ((double*)grid.extra_boundary_data_)[12*i+3]  = am_node_coordinates_float[DIMWORLD*face_list[i][1] + 0];
        ((double*)grid.extra_boundary_data_)[12*i+4]  = am_node_coordinates_float[DIMWORLD*face_list[i][1] + 1];
        ((double*)grid.extra_boundary_data_)[12*i+5]  = am_node_coordinates_float[DIMWORLD*face_list[i][1] + 2];
        ((double*)grid.extra_boundary_data_)[12*i+6]  = am_node_coordinates_float[DIMWORLD*face_list[i][2] + 0];
        ((double*)grid.extra_boundary_data_)[12*i+7]  = am_node_coordinates_float[DIMWORLD*face_list[i][2] + 1];
        ((double*)grid.extra_boundary_data_)[12*i+8]  = am_node_coordinates_float[DIMWORLD*face_list[i][2] + 2];
        ((double*)grid.extra_boundary_data_)[12*i+9]  = am_node_coordinates_float[DIMWORLD*face_list[i][3] + 0];
        ((double*)grid.extra_boundary_data_)[12*i+10] = am_node_coordinates_float[DIMWORLD*face_list[i][3] + 1];
        ((double*)grid.extra_boundary_data_)[12*i+11] = am_node_coordinates_float[DIMWORLD*face_list[i][3] + 2];
            

        double alpha[2] = {0, 0};
        double beta[2]  = {1, 1};

        if (UG3d::CreateBoundarySegment(segmentName,
                                        left,         /*id of left subdomain */
                                        right,        /*id of right subdomain*/
                                        i,            /*id of segment*/
                                        UG3d::NON_PERIODIC,
                                        1,          // resolution, used by the UG graphics system
                                        point,
                                        alpha, beta,
                                        linearSegmentDescription3d,
                                        ((double*)grid.extra_boundary_data_)+12*i
                                      )==NULL) 
            DUNE_THROW(IOError, "createHexaDomain: UG3d::CreateBoundarySegment returned NULL");

    }
    
    std::cout << nBndSegments << " segments created!" << std::endl;

}

void Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::readHexaGrid(Dune::UGGrid<3,3>& grid, 
                                                             AmiraMesh* am)
{
    std::cout << "This is the AmiraMesh HexaGrid reader for UGGrid<3,3>!" << std::endl;

    //loaddomain $file @PARA_FILE $name @DOMAIN
    createHexaDomain(grid, am);

    // call configureCommand and newCommand
    //grid.makeNewUGMultigrid();   called by buildGrid

    // ////////////////////////////////////////////
    // loadmesh $file @GRID_FILE $name @DOMAIN;
    buildGrid(grid, am);

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
        static void CreateDomain(UGGrid<2,2>& grid,
                                const std::string& domainName, 
                                const std::string& filename);

        static void detectBoundarySegments(int* elemData, 
                                           int numElems, 
                                           std::vector<FieldVector<int, 2> >& face_list,
                                           bool containsOnlyTriangles);

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
                                                                       std::vector<FieldVector<int, 2> >& face_list,
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

    face_list.resize(0);

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
            for (j=0; j<(int)face_list.size(); j++) {

                const FieldVector<int, 2>& o = face_list[j];
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

                // Insert k-th face of i-th element into face list
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

void Dune::AmiraMeshReader<Dune::UGGrid<2,2> >::CreateDomain(UGGrid<2,2>& grid,
                                                          const std::string& domainName, 
                                                          const std::string& filename)
{
  
  int i;

  std::cout << "Loading 2D Amira domain " << filename << std::endl;

  ///////////////////////////////////////////////////////
  // Load the AmiraMesh file
  AmiraMesh* am = AmiraMesh::read(filename.c_str());

  if(!am)
      DUNE_THROW(IOError, "AmiraMeshReader<UGGrid<2,2> >::CreateDomain: Can't open input file");

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

  std::cout << noOfBSegments << " Boundary segments found!" << std::endl;

  // extract boundary nodes
  std::vector<int> boundary_nodes;
  detectBoundaryNodes(boundary_segments, noOfNodes, boundary_nodes);
  if (boundary_nodes.size() == 0) {
      delete am;
      DUNE_THROW(IOError, "2d AmiraMesh reader: couldn't extract any boundary nodes!");
  }

  int noOfBNodes = 0;
  for (i=0; i<noOfNodes; i++) {
      if (boundary_nodes[i] != -1)
          noOfBNodes++;
  }

  std::cout << noOfBNodes << " boundary nodes found!" << std::endl;
  

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
  
  if(grid.extra_boundary_data_ == NULL){
      delete am;
      DUNE_THROW(IOError, "2d AmiraMesh reader: malloc for extra_boundary_data_ failed!");
  }

  for(i = 0; i < noOfBSegments; i++)
    {
      char segmentName[20];
      int left, right;

      // Every boundary segment has a name
      if(sprintf(segmentName, "BS %d", i) < 0) {
          delete am;
          DUNE_THROW(IOError, "2d AmiraMesh reader: sprintf returned error code!");
      }

      /* left = innerRegion, right = outerRegion */
      left = 0;
      right = 1;
         

      const FieldVector<int, 2>& thisEdge = boundary_segments[i];

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
                                      left,         /*id of left subdomain */
                                      right,        /*id of right subdomain*/
                                      i,            /*id of segment*/
                                      UG2d::NON_PERIODIC, 
                                      1,           // Resolution, only for the UG graphics
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
    int i;
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
    CreateDomain(grid, domainname, filename);

    // call configureCommand and newCommand
    grid.makeNewUGMultigrid();
    
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
  UG2d::multigrid* theMG = grid.multigrid_;
  assert(theMG);

  maxBndNodeID = -1;
  for (theNode=theMG->grids[0]->firstNode[0]; theNode!=NULL; theNode=theNode->succ)
  {
      // The following two lines ought to be in here, but the
      // OBJT macros is somewhat complicated, so I leave it out
      // for the time being.
//       if(OBJT(theNode->myvertex) == UG3d::IVOBJ)
//           UserWriteF("Warning: Node %d is inner node\n", ID(theNode));
      maxBndNodeID = std::max(theNode->id, maxBndNodeID);
  }

  std::cout << "Already " << maxBndNodeID+1 << " nodes existing" << std::endl;

  noOfNodes = am->nElements("Nodes");
  noOfElem  = am->nElements("Triangles");

  std::cout << "AmiraMesh has " << noOfNodes << " total nodes" << std::endl;

  // Extract boundary faces
  std::vector<FieldVector<int, 2> > face_list;
  detectBoundarySegments(elemData, noOfElem, face_list, containsOnlyTriangles);
  std::vector<int> isBoundaryNode;
  detectBoundaryNodes(face_list, noOfNodes, isBoundaryNode);


  for(i = 0; i < noOfNodes; i++) {

      if (isBoundaryNode[i] != -1)
          continue;

      double nodePos[2];
      nodePos[0] = am_node_coordinates[2*i];
      nodePos[1] = am_node_coordinates[2*i+1];
      
      /** \todo Why is InsertInnerNode outside of the UG namespaces?? */
      if (InsertInnerNode(theMG->grids[0], nodePos) == NULL)
          DUNE_THROW(IOError, "2d AmiraMesh reader: Inserting an inner node failed");
      
      isBoundaryNode[i] = ++maxBndNodeID;
      
  }
  
  noOfCreatedElem = 0;
  for(i=0; i < noOfElem; i++) {

      if (containsOnlyTriangles ||
          (elemData[3*i+2] == elemData[3*i+3])) {

          int cornerIDs[3];
          
          /* only triangles */
          cornerIDs[0] = isBoundaryNode[elemData[3*i]-1];
          cornerIDs[1] = isBoundaryNode[elemData[3*i+1]-1];
          cornerIDs[2] = isBoundaryNode[elemData[3*i+2]-1];
          //printf("elem id : %d, node ids : %d %d %d\n", i, cornerIDs[0], cornerIDs[1], cornerIDs[2]); 
          
          /** \todo Why is InsertElementFromIDs outside of the UG namespaces?? */
          if (InsertElementFromIDs(theMG->grids[0], 3,cornerIDs, NULL) == NULL)
              DUNE_THROW(IOError, "2d AmiraMesh reader: Inserting an element failed");
      
      } else {
      
          int cornerIDs[4];
          
          /* only quadrilaterals */
          cornerIDs[0] = isBoundaryNode[elemData[4*i]-1];
          cornerIDs[1] = isBoundaryNode[elemData[4*i+1]-1];
          cornerIDs[2] = isBoundaryNode[elemData[4*i+2]-1];
          cornerIDs[3] = isBoundaryNode[elemData[4*i+3]-1];
          
          /** \todo Why is InsertElementFromIDs outside of the UG namespaces?? */
          if (InsertElementFromIDs(theMG->grids[0], 4,cornerIDs, NULL) == NULL)
              DUNE_THROW(IOError, "2d AmiraMesh reader: Inserting an element failed");
      
      }
        
      noOfCreatedElem++;

    }

  std::cout << "amiraloadmesh: " << noOfCreatedElem << " elements created" << std::endl;

  // set the subdomainIDs
  AmiraMesh::Data* am_material_ids = am->findData("Triangles", HxBYTE, 1, "Materials");
  if (!am_material_ids) 
      DUNE_THROW(IOError, "Field 'Materials' not found.");

  i = 0;
  UG2d::ELEMENT* theElement;
  for (theElement=theMG->grids[0]->elements[0]; theElement!=NULL; theElement=theElement->ge.succ)
    {

#if 0
#define ControlWord(p,ce) (((unsigned int *)(p))[UG2d::control_entries[ce].offset_in_object])
#define CW_WRITE(p, ce, n)   ControlWord(p,ce) = (ControlWord(p,ce)&UG2d::control_entries[ce].xor_mask)|(((n)<<UG2d::control_entries[ce].offset_in_word)&UG2d::control_entries[ce].mask)
#define SETSUBDOMAIN(p,n) CW_WRITE(p,UG2d::SUBDOMAIN_CE,n)
      /* get subdomain of element */
//       int id = material_ids[i];
//       SETSUBDOMAIN(theElement, id+1);
        /** \todo Proper subdomain handling */
      SETSUBDOMAIN(theElement, 1);

#undef ControlWord
#undef CW_WRITE
#undef SETSUBDOMAIN
#endif
      UG_NS<2>::SetSubdomain(theElement, 1);

      i++;
    }

  delete am;

  UG2d::SetEdgeAndNodeSubdomainFromElements(theMG->grids[0]);

  // Complete the UG-internal grid data structure
  /** \todo Why is CreateAlgebra not in the proper namespace? */
  if (CreateAlgebra(theMG) != UG2d::GM_OK)
      DUNE_THROW(IOError, "Call of 'UG::CreateAlgebra' failed!");
  
  /* here all temp memory since CreateMultiGrid is released */
#define ReleaseTmpMem(p,k) Release(p, UG::FROM_TOP,k)
  ReleaseTmpMem(theMG->theHeap, theMG->MarkKey);
#undef ReleaseTmpMem
  theMG->MarkKey = 0;
  
}

#endif // #ifdef _2


