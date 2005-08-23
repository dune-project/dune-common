// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UG_FUNCTIONS_HH
#define DUNE_UG_FUNCTIONS_HH

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
  template<int dim>
  class UG_NS {};

  template<>
  class UG_NS<2> {
  public:

    typedef UG2d::RefinementRule RefinementRule;

    typedef UG2d::CoeffProcPtr CoeffProcPtr;

    typedef UG2d::UserProcPtr UserProcPtr;

    enum {GM_REFINE_NOT_CLOSED = UG2d::GM_REFINE_NOT_CLOSED};

    enum {GM_COPY_ALL = UG2d::GM_COPY_ALL};

    enum {GM_REFINE_TRULY_LOCAL = UG2d::GM_REFINE_TRULY_LOCAL};

    enum {GM_REFINE_PARALLEL = UG2d::GM_REFINE_PARALLEL};

    enum {GM_REFINE_NOHEAPTEST = UG2d::GM_REFINE_NOHEAPTEST};

    enum {NEWEL_CE = UG2d::NEWEL_CE};

    enum {COARSEN_CE = UG2d::COARSEN_CE};

    enum {RED = UG2d::RED};

    enum {COARSE = UG2d::COARSE};

    enum {GM_OK = UG2d::GM_OK};

    /** \brief The PFIRSTNODE macro which returns the first node in a
     * grid even in a parallel setting.
     */
    static TargetType<2,2>::T* PFirstNode(UGTypes<2>::GridType* grid) {
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
    static TargetType<2,2>::T* FirstNode(UGTypes<2>::GridType* grid) {
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
    static TargetType<0,2>::T* PFirstElement(UGTypes<2>::GridType* grid) {
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
    static TargetType<0,2>::T* FirstElement(UGTypes<2>::GridType* grid) {
      using UG::PrioHGhost;
      using UG::PrioVGhost;
      using UG::PrioVHGhost;
      using UG::PrioMaster;
      using UG::PrioBorder;
      return FIRSTELEMENT(grid);
    }

    /** \brief Returns pointers to the coordinate arrays of an UG element */
    static void Corner_Coordinates(TargetType<0,2>::T* theElement, double* x[]) {
      using UG2d::NODE;
      using UG2d::TRIANGLE;
      using UG2d::QUADRILATERAL;
      using UG2d::n_offset;
      int n;    // Dummy variable just to please the macro
      CORNER_COORDINATES(theElement, n, x);
    }

    static int GlobalToLocal(int n, const double** cornerCoords,
                             const double* EvalPoint, double* localCoord) {
      return UG2d::UG_GlobalToLocal(n, cornerCoords, EvalPoint, localCoord);
    }

    //! \todo Please doc me!
    static int Sides_Of_Elem(TargetType<0,2>::T* theElement) {
      using UG2d::element_descriptors;
      return SIDES_OF_ELEM(theElement);
    }

    //! Encapsulates the NBELEM macro
    static TargetType<0,2>::T* NbElem(TargetType<0,2>::T* theElement, int nb) {
      using UG2d::ELEMENT;
      using UG2d::nb_offset;
      return NBELEM(theElement, nb);
    }

    //! Returns true if the i-th side of the element is on the domain boundary
    static bool Side_On_Bnd(TargetType<0,2>::T* theElement, int i) {
      using UG2d::BNDS;
      using UG2d::BEOBJ;
      using UG2d::side_offset;
      return OBJT(theElement)==BEOBJ && SIDE_ON_BND(theElement, i);
    }

    //! \todo Please doc me!
    static int Edges_Of_Elem(const TargetType<0,2>::T* theElement) {
      using UG2d::element_descriptors;
      return EDGES_OF_ELEM(theElement);
    }

    //! \todo Please doc me!
    static int Corners_Of_Elem(const TargetType<0,2>::T* theElement) {
      using UG2d::element_descriptors;
      return CORNERS_OF_ELEM(theElement);
    }

    //! \todo Please doc me!
    // Dummy implementation for vertices
    static int Corners_Of_Elem(const TargetType<2,2>::T* theElement) {
      return 1;
    }

    //! \todo Please doc me!
    static int Corners_Of_Side(const TargetType<0,2>::T* theElement, int side) {
      using UG2d::element_descriptors;
      return CORNERS_OF_SIDE(theElement, side);
    }

    //! \todo Please doc me!
    static int Corner_Of_Side(const TargetType<0,2>::T* theElement, int side, int corner) {
      using UG2d::element_descriptors;
      return CORNER_OF_SIDE(theElement, side, corner);
    }

    static int nSons(const UG2d::element* element) {
      return UG2d::ReadCW(element, UG2d::NSONS_CE);
    }

    static int GetSons(const UG2d::element* element, UG2d::element* sonList[MAX_SONS]) {
      return UG2d::GetSons(element, sonList);
    }

    //! Encapsulates the GRID_ATTR macro
    static int Grid_Attr(const UGTypes<2>::GridType* grid) {
      return GRID_ATTR(grid);
    }

    static int MarkForRefinement(UG2d::element* element, int rule, int data) {
      return UG2d::MarkForRefinement(element, (UG2d::RefinementRule)rule, data);
    }

    //! Encapsulates the TAG macro
    static unsigned int Tag(const TargetType<0,2>::T* theElement) {
      return TAG(theElement);
    }

    //! Doesn't ever get called, but needs to be there to calm the compiler
    static unsigned int Tag(const TargetType<2,2>::T* theNode) {
      DUNE_THROW(GridError, "Called method Tag() for a vertex.  This should never happen!");
      return 0;
    }

    //! Next element in the UG element lists
    static TargetType<0,2>::T* succ(const TargetType<0,2>::T* theElement) {
      return theElement->ge.succ;
    }

    //! Next element in the UG nodes lists
    static TargetType<2,2>::T* succ(const TargetType<2,2>::T* theNode) {
      return theNode->succ;
    }

    //! Calm the compiler
    static void* succ(const void* theWhatever) {
      DUNE_THROW(NotImplemented, "No successor available for this kind of object");
      return 0;
    }

    //! Return true if the element is a leaf element
    static bool isLeaf(const TargetType<0,2>::T* theElement) {
      return UG2d::EstimateHere(theElement);
    }

    //! Return true if the node is a leaf node
    static bool isLeaf(const TargetType<2,2>::T* theNode) {
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
    static int& levelIndex(TargetType<0,2>::T* theElement) {
      return theElement->ge.levelIndex;
    }

    //! Gets the level index of a UG element
    static const int& levelIndex(const TargetType<0,2>::T* theElement) {
      return theElement->ge.levelIndex;
    }

    //! Gets the level index of a UG node
    static int& levelIndex(TargetType<2,2>::T* theNode) {
      return theNode->levelIndex;
    }

    //! Gets the level index of a UG node
    static const int& levelIndex(const TargetType<2,2>::T* theNode) {
      return theNode->levelIndex;
    }

    // /////////////////////////////////////////////
    //   Leaf indices
    // /////////////////////////////////////////////

    //! Gets the leaf index of a UG element
    static int& leafIndex(TargetType<0,2>::T* theElement) {
      return theElement->ge.leafIndex;
    }

    //! Gets the leaf index of a UG element
    static const int& leafIndex(const TargetType<0,2>::T* theElement) {
      return theElement->ge.leafIndex;
    }

    //! Gets the leaf index of a UG node
    static int& leafIndex(TargetType<2,2>::T* theNode) {
      return theNode->myvertex->iv.id;
    }

    //! Gets the leaf index of a UG node
    static const int& leafIndex(const TargetType<2,2>::T* theNode) {
      return theNode->myvertex->iv.id;
    }


    //! Gets the index of a UG element
    static int& id(TargetType<0,2>::T* theElement) {
      return theElement->ge.id;
    }

    //! Gets the index of a UG element
    static const int& id(const TargetType<0,2>::T* theElement) {
      return theElement->ge.id;
    }

    //! Gets the index of a UG node
    static int& id(TargetType<2,2>::T* theNode) {
      return theNode->id;
    }

    //! Gets the index of a UG node
    static const int& id(const TargetType<2,2>::T* theNode) {
      return theNode->id;
    }

    //! \todo Please doc me!
    static void Local_To_Global(int n, DOUBLE** y,
                                const FieldVector<double, 2>& local,
                                FieldVector<double, 2>& global) {
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
                              const FieldVector<double, 2>& local, FieldMatrix<double,2,2>& mat) {
      typedef DOUBLE DOUBLE_VECTOR[2];
      double det;
      INVERSE_TRANSFORMATION(n, x, local, mat, det);
      return 0;
    }

    //! Returns the i-th corner of a UG element
    static TargetType<2,2>::T* Corner(TargetType<0,2>::T* theElement, int i) {
      using UG2d::NODE;
      using UG2d::n_offset;
      return CORNER(theElement, i);
    }
    //! \todo Please doc me!
    static TargetType<0,2>::T* EFather(TargetType<0,2>::T* theElement) {
      using UG2d::ELEMENT;
      using UG2d::father_offset;
      return EFATHER(theElement);
    }

    static unsigned int ReadCW(void* obj, int ce) {
      return UG2d::ReadCW(obj, ce);
    }

    static void WriteCW(void* obj, int ce, int n) {
      UG2d::WriteCW(obj, ce, n);
    }

    //! \todo Please doc me!
    static void InitUg(int* argcp, char*** argvp) {
      UG2d::InitUg(argcp, argvp);
    }

    static void ExitUg() {
      UG2d::ExitUg();
    }

    static void DisposeMultiGrid(UG2d::multigrid* mg) {
      UG2d::DisposeMultiGrid(mg);
    }

    //! \todo Please doc me!
    static void* CreateBoundaryValueProblem(const char* BVPname,
                                            int numOfCoeffFunc,
                                            UG2d::CoeffProcPtr coeffs[],
                                            int numOfUserFct,
                                            UG2d::UserProcPtr userfct[]) {
      return UG2d::CreateBoundaryValueProblem(BVPname, 0, numOfCoeffFunc, coeffs,
                                              numOfUserFct, userfct);
    }

    static void* BVP_GetByName(const char* bvpName) {
      return UG2d::BVP_GetByName(bvpName);
    }

    static void Set_Current_BVP(void** thisBVP) {
      UG2d::Set_Current_BVP(thisBVP);
    }

    //! \todo Please doc me!
    static UGTypes<2>::MultiGridType* GetMultigrid(const char* name) {
      return UG2d::GetMultigrid(name);
    }

    //! \todo Please doc me!
    static void SetSubdomain(TargetType<0,2>::T* theElement, int id) {
      using UG2d::control_entries;
      using UG2d::SUBDOMAIN_CE;
      SETSUBDOMAIN(theElement, id);
    }

    static int LBCommand(int argc, const char** argv) {
      /** \todo Can we remove the cast? */
      return UG2d::LBCommand(argc, (char**)argv);
    }

    static int ConfigureCommand(int argc, const char** argv) {
      /** \todo Kann man ConfigureCommand so ‰ndern daﬂ man auch ohne den cast auskommt? */
      return UG2d::ConfigureCommand(argc, (char**)argv);
    }

    static int NewCommand(int argc, char** argv) {
      return UG2d::NewCommand(argc, argv);
    }

    static int CreateFormatCmd(int argc, char** argv) {
      return UG2d::CreateFormatCmd(argc, argv);
    }

  };
  //! \todo Please doc me!
  template <int codim, int dimworld>
  class UGGridSubEntityFactory {};

  template<>
  class UGGridSubEntityFactory<0,2> {
  public:
    static TargetType<0,2>::T* get(TargetType<0,2>::T* c, int i){
      return c;
    }
  };

  template<>
  class UGGridSubEntityFactory<2,2> {
  public:
    static TargetType<2,2>::T* get(TargetType<0,2>::T* c, int i){
      return UG_NS<2>::Corner(c, i);
    }
  };

  template<>
  /** \todo Method not implemented! */
  class UGGridSubEntityFactory<1,2> {
  public:
    static TargetType<1,2>::T* get(TargetType<0,2>::T* c, int i){
      DUNE_THROW(GridError, "UGGridSubEntityFactory<1,2>::get() not implemented!");
    }
  };

} // namespace Dune

#endif
