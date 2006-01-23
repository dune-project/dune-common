// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UG_FUNCTIONS_3D_HH
#define DUNE_UG_FUNCTIONS_3D_HH

/** \file
 * \brief Encapsulates some UG macros and functions
 */

#include "ugtypes.hh"

namespace Dune {

  /** \brief Encapsulates a few UG methods and macros
   *
   * This class provides a wrapper to several methods and macros from
   * UG.  There are two reasons for doing this.  First, we don't want
   * to call UG macros directly from DUNE, because they pollute the
   * namespace and therefore we undefine them all.  Secondly,  UG methods
   * appear in the namespaces UG2d and UG3d, but we need the dimension
   * as a template parameter.
   */
  template<>
  class UG_NS<3> {
  public:

    typedef UG3d::RefinementRule RefinementRule;

    typedef UG3d::CoeffProcPtr CoeffProcPtr;

    typedef UG3d::UserProcPtr UserProcPtr;

    typedef UG3d::BndSegFuncPtr BndSegFuncPtr;

    enum {GM_REFINE_NOT_CLOSED = UG3d::GM_REFINE_NOT_CLOSED};

    enum {GM_COPY_ALL = UG3d::GM_COPY_ALL};

    enum {GM_REFINE_TRULY_LOCAL = UG3d::GM_REFINE_TRULY_LOCAL};

    enum {GM_REFINE_PARALLEL = UG3d::GM_REFINE_PARALLEL};

    enum {GM_REFINE_NOHEAPTEST = UG3d::GM_REFINE_NOHEAPTEST};

    enum {NEWEL_CE = UG3d::NEWEL_CE};

    enum {COARSEN_CE = UG3d::COARSEN_CE};

    enum {REFINECLASS_CE = UG2d::REFINECLASS_CE};

    enum {RED = UG3d::RED};

    enum {YELLOW_CLASS = UG2d::YELLOW_CLASS};

    enum {COARSE = UG3d::COARSE};

    enum {GM_OK = UG3d::GM_OK};

    /** \brief The PFIRSTNODE macro which returns the first node in a
     * grid even in a parallel setting.
     */
    static TargetType<3,3>::T* PFirstNode(UGTypes<3>::GridType* grid) {
      using UG::PrioHGhost;
      using UG::PrioVGhost;
      using UG::PrioVHGhost;
      using UG::PrioMaster;
      using UG::PrioBorder;
      return PFIRSTNODE(grid);
    }

    /** \brief The FIRSTNODE macro which returns the first node in a
     * grid even in a parallel setting.
     */
    static TargetType<3,3>::T* FirstNode(UGTypes<3>::GridType* grid) {
      using UG::PrioHGhost;
      using UG::PrioVGhost;
      using UG::PrioVHGhost;
      using UG::PrioMaster;
      using UG::PrioBorder;
      return FIRSTNODE(grid);
    }

    /** \brief The PFIRSTELEMENT macro which returns the first element in a
     * grid even in a parallel setting.
     */
    static TargetType<0,3>::T* PFirstElement(UGTypes<3>::GridType* grid) {
      using UG::PrioHGhost;
      using UG::PrioVGhost;
      using UG::PrioVHGhost;
      using UG::PrioMaster;
      using UG::PrioBorder;
      return PFIRSTELEMENT(grid);
    }

    /** \brief The FIRSTELEMENT macro which returns the first element in a
     * grid even in a parallel setting.
     */
    static TargetType<0,3>::T* FirstElement(UGTypes<3>::GridType* grid) {
      using UG::PrioHGhost;
      using UG::PrioVGhost;
      using UG::PrioVHGhost;
      using UG::PrioMaster;
      using UG::PrioBorder;
      return FIRSTELEMENT(grid);
    }

    /** \brief Returns pointers to the coordinate arrays of an UG element */
    static void Corner_Coordinates(TargetType<0,3>::T* theElement, double* x[]) {

      using UG3d::TETRAHEDRON;
      using UG3d::NODE;
      using UG3d::PYRAMID;
      using UG3d::PRISM;
      using UG3d::HEXAHEDRON;
      using UG3d::n_offset;
      int n;    // Dummy variable just to please the macro
      CORNER_COORDINATES(theElement, n, x);
    }

    static int GlobalToLocal(int n, const double** cornerCoords,
                             const double* EvalPoint, double* localCoord) {
      return UG3d::UG_GlobalToLocal(n, cornerCoords, EvalPoint, localCoord);
    }

    //! return true if element has an exact copy on the next level
    static bool hasCopy (TargetType<0,3>::T* theElement) {
      using UG3d::ELEMENT;
      using UG3d::control_entries;
      return REFINECLASS(theElement) == YELLOW_CLASS;
    }

    //! \todo Please doc me!
    static int Sides_Of_Elem(TargetType<0,3>::T* theElement) {
      using UG3d::element_descriptors;
      return SIDES_OF_ELEM(theElement);
    }

    //! Encapsulates the NBELEM macro
    static TargetType<0,3>::T* NbElem(TargetType<0,3>::T* theElement, int nb) {
      using UG3d::ELEMENT;
      using UG3d::nb_offset;
      return NBELEM(theElement, nb);
    }

    //! Returns true if the i-th side of the element is on the domain boundary
    static bool Side_On_Bnd(TargetType<0,3>::T* theElement, int i) {
      using UG3d::BNDS;
      using UG3d::BEOBJ;
      using UG3d::side_offset;
      return OBJT(theElement)==BEOBJ && SIDE_ON_BND(theElement, i);
    }

    //! \todo Please doc me!
    static int Edges_Of_Elem(const TargetType<0,3>::T* theElement) {
      using UG3d::element_descriptors;
      return EDGES_OF_ELEM(theElement);
    }

    //! \todo Please doc me!
    static int Corners_Of_Elem(const TargetType<0,3>::T* theElement) {
      using UG3d::element_descriptors;
      return CORNERS_OF_ELEM(theElement);
    }

    //! \todo Please doc me!
    // Dummy implementation for vertices
    static int Corners_Of_Elem(const TargetType<3,3>::T* theElement) {
      return 1;
    }

    //! \todo Please doc me!
    static int Corners_Of_Side(const TargetType<0,3>::T* theElement, int side) {
      using UG3d::element_descriptors;
      return CORNERS_OF_SIDE(theElement, side);
    }

    //! \todo Please doc me!
    static int Corner_Of_Side(const TargetType<0,3>::T* theElement, int side, int corner) {
      using UG3d::element_descriptors;
      return CORNER_OF_SIDE(theElement, side, corner);
    }

    static int nSons(const UG3d::element* element) {
      return UG3d::ReadCW(element, UG3d::NSONS_CE);
    }

    static int myLevel (TargetType<0,3>::T* theElement) {
      using UG3d::ELEMENT;
      return LEVEL(theElement);
    }

    static int myLevel (TargetType<3,3>::T* theNode) {
      using UG3d::NODE;
      return LEVEL(theNode);
    }

    //! get father element of vertex
    static TargetType<0,3>::T* NFather(TargetType<3,3>::T* theNode) {
      return theNode->myvertex->iv.father;
    }

    //! get father node of vertex
    static TargetType<3,3>::T* NodeNodeFather(TargetType<3,3>::T* theNode) {
      using UG3d::NDOBJ;
      if (theNode->father==0)
        return 0;         // no father at all
      if (OBJT(theNode->father)==NDOBJ)
        return (UG3d::node*) theNode->father;
      else
        return 0;         // may be edge or element
    }

    //! get father element of vertex
    static void PositionInFather(TargetType<3,3>::T* theNode, FieldVector<double, 3>& local) {
      local[0] = theNode->myvertex->iv.xi[0];
      local[1] = theNode->myvertex->iv.xi[1];
      local[2] = theNode->myvertex->iv.xi[2];
    }

    //! get father element of vertex
    static void NodePositionGlobal(TargetType<3,3>::T* theNode, FieldVector<double, 3>& global) {
      global[0] = theNode->myvertex->iv.x[0];
      global[1] = theNode->myvertex->iv.x[1];
      global[2] = theNode->myvertex->iv.x[2];
    }

    static int GetSons(const UG3d::element* element, UG3d::element* sonList[MAX_SONS]) {
      return UG3d::GetSons(element, sonList);
    }

    static int GetNodeContext(const UG3d::element* element, const UG3d::node** context) {
      return UG3d::GetNodeContext(const_cast<UG3d::element*>(element),
                                  const_cast<UG3d::node**>(context));
    }

    //! Encapsulates the GRID_ATTR macro
    /** \todo Why this strange construction? */
    static unsigned char Grid_Attr(const UGTypes<3>::GridType* grid) {
#define GRID_ATTR(g) g->level+32
      return GRID_ATTR(grid);
#undef GRID_ATTR
    }

    static int MarkForRefinement(UG3d::element* element, int rule, int data) {
      return UG3d::MarkForRefinement(element, (UG3d::RefinementRule)rule, data);
    }

    //! Encapsulates the TAG macro
    static unsigned int Tag(const TargetType<0,3>::T* theElement) {
      return TAG(theElement);
    }

    //! Doesn't ever get called, but needs to be there to calm the compiler
    static unsigned int Tag(const TargetType<3,3>::T* theNode) {
      DUNE_THROW(GridError, "Called method Tag() for a vertex.  This should never happen!");
      return 0;
    }

    //! get corner in local coordinates, corner number in UG's numbering system
    template<class T>
    static void  getCornerLocal (const TargetType<0,3>::T* theElement, int corner, FieldVector<T, 3>& local)
    {
      using UG3d::element_descriptors;
      local[0] = LOCAL_COORD_OF_TAG(TAG(theElement),corner)[0];
      local[1] = LOCAL_COORD_OF_TAG(TAG(theElement),corner)[1];
      local[2] = LOCAL_COORD_OF_TAG(TAG(theElement),corner)[2];
    }

    //! Next element in the UG element lists
    static TargetType<0,3>::T* succ(const TargetType<0,3>::T* theElement) {
      return theElement->ge.succ;
    }

    //! Next element in the UG nodes lists
    static TargetType<3,3>::T* succ(const TargetType<3,3>::T* theNode) {
      return theNode->succ;
    }

    //! Calm the compiler
    static void* succ(const void* theWhatever) {
      DUNE_THROW(NotImplemented, "No successor available for this kind of object");
      return 0;
    }

    //! Return true if the element is a leaf element
    static bool isLeaf(const TargetType<0,3>::T* theElement) {
      return UG3d::EstimateHere(theElement);
    }

    //! Return true if the node is a leaf node
    static bool isLeaf(const TargetType<3,3>::T* theNode) {
#ifndef ModelP
      return !theNode->son;
#else
      DUNE_THROW(NotImplemented, "isLeaf for nodes in a parallel grid");
#endif
    }

    // /////////////////////////////////////////////
    //   Level indices
    // /////////////////////////////////////////////

    //! Gets the level index of a UG element
    static int& levelIndex(TargetType<0,3>::T* theElement) {
      return theElement->ge.levelIndex;
    }

    //! Gets the level index of a UG element
    static const int& levelIndex(const TargetType<0,3>::T* theElement) {
      return theElement->ge.levelIndex;
    }

    //! Gets the level index of a UG sidevector
    static int& levelIndex(UGVectorType<3>::T* theVector) {
      return reinterpret_cast<int&>(theVector->index);
    }

    //! Gets the level index of a UG sidevector
    static const int& levelIndex(const UGVectorType<3>::T* theVector) {
      return reinterpret_cast<const int&>(theVector->index);
    }

    //! Gets the level index of a UG edge
    static int& levelIndex(TargetType<2,3>::T* theEdge) {
      return theEdge->levelIndex;
    }

    //! Gets the level index of a UG edge
    static const int& levelIndex(const TargetType<2,3>::T* theEdge) {
      return theEdge->levelIndex;
    }

    //! Gets the level index of a UG node
    static int& levelIndex(TargetType<3,3>::T* theNode) {
      return theNode->levelIndex;
    }

    //! Gets the level index of a UG node
    static const int& levelIndex(const TargetType<3,3>::T* theNode) {
      return theNode->levelIndex;
    }

    // /////////////////////////////////////////////
    //   Leaf indices
    // /////////////////////////////////////////////

    //! Gets the leaf index of a UG element
    static int& leafIndex(TargetType<0,3>::T* theElement) {
      return theElement->ge.leafIndex;
    }

    //! Gets the leaf index of a UG element
    static const int& leafIndex(const TargetType<0,3>::T* theElement) {
      return theElement->ge.leafIndex;
    }

    //! Gets the level index of a UG sidevector
    static int& leafIndex(UGVectorType<3>::T* theVector) {
      return reinterpret_cast<int &>(theVector->skip);
    }

    //! Gets the level index of a UG sidevector
    static const int& leafIndex(const UGVectorType<3>::T* theVector) {
      return reinterpret_cast<const int &>(theVector->skip);
    }

    //! Gets the leaf index of a UG edge
    static int& leafIndex(TargetType<2,3>::T* theEdge) {
      return theEdge->leafIndex;
    }

    //! Gets the leaf index of a UG edge
    static const int& leafIndex(const TargetType<2,3>::T* theEdge) {
      return theEdge->leafIndex;
    }

    //! Gets the leaf index of a UG node
    static int& leafIndex(TargetType<3,3>::T* theNode) {
      return theNode->myvertex->iv.leafIndex;
    }

    //! Gets the leaf index of a UG node
    static const int& leafIndex(const TargetType<3,3>::T* theNode) {
      return theNode->myvertex->iv.leafIndex;
    }

    // /////////////////////////////////////////////
    //   IDs
    // /////////////////////////////////////////////

    //! Gets the index of a UG element
    static unsigned int id(const TargetType<0,3>::T* theElement) {
      return theElement->ge.id;
    }

    //! Gets the index of a UG node
    static unsigned int id(const TargetType<3,3>::T* theNode) {
      return theNode->myvertex->iv.id | 0xC0000000;
    }

    //! \todo Please doc me!
    static void Local_To_Global(int n, DOUBLE** y,
                                const FieldVector<double, 3>& local,
                                FieldVector<double, 3>& global) {
      LOCAL_TO_GLOBAL(n,y,local,global);
    }

    /**
     * \param n Number of corners of the element
     * \param x Coordinates of the corners of the element
     * \param local Local evaluation point
     *
     * \return The return type is int because the macro INVERSE_TRANSFORMATION
     *  return 1 on failure.
     */
    static int Transformation(int n, double** x,
                              const FieldVector<double, 3>& local, FieldMatrix<double,3,3>& mat) {
      typedef DOUBLE DOUBLE_VECTOR[3];
      double det;
      INVERSE_TRANSFORMATION(n, x, local, mat, det);
      return 0;
    }

    //! Returns the i-th corner of a UG element
    static TargetType<3,3>::T* Corner(TargetType<0,3>::T* theElement, int i) {
      using UG3d::NODE;
      using UG3d::n_offset;
      return CORNER(theElement, i);
    }

    //! get edge from node i to node j (in UG's numbering !
    static TargetType<2,3>::T* GetEdge (TargetType<3,3>::T* nodei, TargetType<3,3>::T* nodej) {
      return UG3d::GetEdge(nodei,nodej);
    }

    //! access side vector from element
    static UGVectorType<3>::T* SideVector (TargetType<0,3>::T* theElement, int i)
    {
      using UG3d::VECTOR;
      using UG3d::svector_offset;
      return SVECTOR(theElement,i);
    }

    //! \todo Please doc me!
    static TargetType<0,3>::T* EFather(TargetType<0,3>::T* theElement) {
      using UG3d::ELEMENT;
      using UG3d::father_offset;
      return EFATHER(theElement);
    }

    static unsigned int ReadCW(void* obj, int ce) {
      return UG3d::ReadCW(obj, ce);
    }

    static void WriteCW(void* obj, int ce, int n) {
      UG3d::WriteCW(obj, ce, n);
    }

    //! \todo Please doc me!
    static void InitUg(int* argcp, char*** argvp) {
      UG3d::InitUg(argcp, argvp);
    }

    static void ExitUg() {
      UG3d::ExitUg();
    }

    static void DisposeMultiGrid(UG3d::multigrid* mg) {
      UG3d::DisposeMultiGrid(mg);
    }

    //! \todo Please doc me!
    static void* CreateBoundaryValueProblem(const char* BVPname,
                                            int numOfCoeffFunc,
                                            UG3d::CoeffProcPtr coeffs[],
                                            int numOfUserFct,
                                            UG3d::UserProcPtr userfct[]) {
      return UG3d::CreateBoundaryValueProblem(BVPname, 0, numOfCoeffFunc, coeffs,
                                              numOfUserFct, userfct);
    }

    static void* BVP_GetByName(const char* bvpName) {
      return UG3d::BVP_GetByName(bvpName);
    }

    static void Set_Current_BVP(void** thisBVP) {
      UG3d::Set_Current_BVP(thisBVP);
    }

    //! \todo Please doc me!
    static UGTypes<3>::MultiGridType* GetMultigrid(const char* name) {
      return UG3d::GetMultigrid(name);
    }

    //! \todo Please doc me!
    static void SetSubdomain(TargetType<0,3>::T* theElement, int id) {
      using UG3d::control_entries;
      using UG3d::SUBDOMAIN_CE;
      SETSUBDOMAIN(theElement, id);
    }

    static int LBCommand(int argc, const char** argv) {
      /** \todo Can we remove the cast? */
      return UG3d::LBCommand(argc, (char**)argv);
    }

    static int ConfigureCommand(int argc, const char** argv) {
      /** \todo Kann man ConfigureCommand so ‰ndern daﬂ man auch ohne den const_cast auskommt? */
      return UG3d::ConfigureCommand(argc, (char**)argv);
    }

    static int NewCommand(int argc, char** argv) {
      return UG3d::NewCommand(argc, argv);
    }

    static int CreateFormatCmd(int argc, char** argv) {
      return UG3d::CreateFormatCmd(argc, argv);
    }

    static void* CreateDomain(const char* name, const double* midPoint, double radius,
                              int segments, int corners, int convex) {
      return UG3d::CreateDomain(name, midPoint, radius, segments, corners, convex);
    }

    static void* InsertInnerNode(UG3d::grid* grid, const double* pos) {
      return UG3d::InsertInnerNode(grid, pos);
    }

    static void* CreateBoundarySegment(const char *name, int left, int right,
                                       int index, int res,
                                       int *point,
                                       const double *alpha, const double *beta,
                                       UG3d::BndSegFuncPtr boundarySegmentFunction,
                                       void *userData) {
      return UG3d::CreateBoundarySegment(name,            // internal name of the boundary segment
                                         left,                      //  id of left subdomain
                                         right,                      //  id of right subdomain
                                         index,                  // Index of the segment
                                         UG3d::NON_PERIODIC,     // I don't know what this means
                                         res,                      // Resolution, only for the UG graphics
                                         point,
                                         alpha,
                                         beta,
                                         boundarySegmentFunction,
                                         userData);
    }
  };

  // Specializations for dimworld==3
  template<>
  class UGGridSubEntityFactory<0,3> {
  public:
    static TargetType<0,3>::T* get(TargetType<0,3>::T* c, int i){
      return c;
    }
  };

  template<>
  class UGGridSubEntityFactory<3,3> {
  public:
    static TargetType<3,3>::T* get(TargetType<0,3>::T* c, int i){
      return UG_NS<3>::Corner(c, i);
    }
  };

  template<>
  /** \todo Method not implemented! */
  class UGGridSubEntityFactory<1,3> {
  public:
    static TargetType<1,3>::T* get(TargetType<0,3>::T* c, int i){
      DUNE_THROW(GridError, "UGGridSubEntityFactory<1,3>::get() not implemented!");
    }
  };

  template<>
  /** \todo Method not implemented! */
  class UGGridSubEntityFactory<2,3> {
  public:
    static TargetType<2,3>::T* get(TargetType<0,3>::T* c, int i){
      DUNE_THROW(GridError, "UGGridSubEntityFactory<2,3>::get() not implemented!");
    }
  };


} // namespace Dune

#endif
