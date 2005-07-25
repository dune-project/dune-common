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
  class UG_NS {
  public:

    enum {GM_REFINE_NOT_CLOSED = NS_DIM_PREFIX GM_REFINE_NOT_CLOSED};

    enum {GM_COPY_ALL = NS_DIM_PREFIX GM_COPY_ALL};

    enum {GM_REFINE_TRULY_LOCAL = NS_DIM_PREFIX GM_REFINE_TRULY_LOCAL};

    enum {GM_REFINE_PARALLEL = NS_DIM_PREFIX GM_REFINE_PARALLEL};

    enum {GM_REFINE_NOHEAPTEST = NS_DIM_PREFIX GM_REFINE_NOHEAPTEST};

    /** \brief The PFIRSTNODE macro which returns the first node in a
     * grid even in a parallel setting.
     */
    static typename TargetType<dim,dim>::T* PFirstNode(typename UGTypes<dim>::GridType* grid) {
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
    static typename TargetType<dim,dim>::T* FirstNode(typename UGTypes<dim>::GridType* grid) {
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
    static typename TargetType<0,dim>::T* PFirstElement(typename UGTypes<dim>::GridType* grid) {
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
    static typename TargetType<0,dim>::T* FirstElement(typename UGTypes<dim>::GridType* grid) {
      using UG::PrioHGhost;
      using UG::PrioVGhost;
      using UG::PrioVHGhost;
      using UG::PrioMaster;
      using UG::PrioBorder;
      return FIRSTELEMENT(grid);
    }

    /** \brief Returns pointers to the coordinate arrays of an UG element */
    static void Corner_Coordinates(typename TargetType<0,dim>::T* theElement, double* x[]) {

#ifdef _3
      using NS_DIM_PREFIX TETRAHEDRON;
      using NS_DIM_PREFIX NODE;
      using NS_DIM_PREFIX PYRAMID;
      using NS_DIM_PREFIX PRISM;
      using NS_DIM_PREFIX HEXAHEDRON;
      using NS_DIM_PREFIX n_offset;
      int n;    // Dummy variable just to please the macro
      CORNER_COORDINATES(theElement, n, x);
#else
      using UG2d::NODE;
      using UG2d::TRIANGLE;
      using UG2d::QUADRILATERAL;
      using UG2d::n_offset;
      int n;    // Dummy variable just to please the macro
      CORNER_COORDINATES(theElement, n, x);
#endif
    }

    //! \todo Please doc me!
    static int Sides_Of_Elem(typename TargetType<0,dim>::T* theElement) {
      using NS_DIM_PREFIX element_descriptors;
      return SIDES_OF_ELEM(theElement);
    }

    //! Encapsulates the NBELEM macro
    static typename TargetType<0,dim>::T* NbElem(typename TargetType<0,dim>::T* theElement, int nb) {
      using NS_DIM_PREFIX ELEMENT;
      using NS_DIM_PREFIX nb_offset;
      return NBELEM(theElement, nb);
    }

    //! Returns true if the i-th side of the element is on the domain boundary
    static bool Side_On_Bnd(typename TargetType<0,dim>::T* theElement, int i) {
      using NS_DIM_PREFIX BNDS;
      using NS_DIM_PREFIX BEOBJ;
      using NS_DIM_PREFIX side_offset;
      return OBJT(theElement)==BEOBJ && SIDE_ON_BND(theElement, i);
    }

    //! \todo Please doc me!
    static int Edges_Of_Elem(const typename TargetType<0,dim>::T* theElement) {
      using NS_DIM_PREFIX element_descriptors;
      return EDGES_OF_ELEM(theElement);
    }

    //! \todo Please doc me!
    static int Corners_Of_Elem(const typename TargetType<0,dim>::T* theElement) {
      using NS_DIM_PREFIX element_descriptors;
      return CORNERS_OF_ELEM(theElement);
    }

    //! \todo Please doc me!
    // Dummy implementation for vertices
    static int Corners_Of_Elem(const typename TargetType<dim,dim>::T* theElement) {
      return 1;
    }

    //! \todo Please doc me!
    static int Corners_Of_Side(const typename TargetType<0,dim>::T* theElement, int side) {
      using NS_DIM_PREFIX element_descriptors;
      return CORNERS_OF_SIDE(theElement, side);
    }

    //! \todo Please doc me!
    static int Corner_Of_Side(const typename TargetType<0,dim>::T* theElement, int side, int corner) {
      using NS_DIM_PREFIX element_descriptors;
      return CORNER_OF_SIDE(theElement, side, corner);
    }

    //! Encapsulates the GRID_ATTR macro
    static int Grid_Attr(const typename UGTypes<dim>::GridType* grid) {
      return GRID_ATTR(grid);
    }

    //! Encapsulates the TAG macro
    static unsigned int Tag(const typename TargetType<0,dim>::T* theElement) {
      return TAG(theElement);
    }

    //! Doesn't ever get called, but needs to be there to calm the compiler
    static unsigned int Tag(const typename TargetType<dim,dim>::T* theNode) {
      DUNE_THROW(GridError, "Called method Tag() for a vertex.  This should never happen!");
      return 0;
    }

    //! Next element in the UG element lists
    static typename TargetType<0,dim>::T* succ(const typename TargetType<0,dim>::T* theElement) {
      return theElement->ge.succ;
    }

    //! Next element in the UG nodes lists
    static typename TargetType<dim,dim>::T* succ(const typename TargetType<dim,dim>::T* theNode) {
      return theNode->succ;
    }

    //! Calm the compiler
    static void* succ(const void* theWhatever) {
      DUNE_THROW(NotImplemented, "No successor available for this kind of object");
      return 0;
    }

    // /////////////////////////////////////////////
    //   Level indices
    // /////////////////////////////////////////////

    //! Gets the level index of a UG element
    static int& levelIndex(typename TargetType<0,dim>::T* theElement) {
#ifdef FOR_DUNE
      return theElement->ge.levelIndex;
#else
      return theElement->ge.id;
#endif
    }

    //! Gets the level index of a UG element
    static const int& levelIndex(const typename TargetType<0,dim>::T* theElement) {
#ifdef FOR_DUNE
      return theElement->ge.levelIndex;
#else
      return theElement->ge.id;
#endif
    }

    //! Gets the level index of a UG node
    static int& levelIndex(typename TargetType<dim,dim>::T* theNode) {
#ifdef FOR_DUNE
      return theNode->levelIndex;
#else
      return theNode->id;
#endif
    }

    //! Gets the level index of a UG node
    static const int& levelIndex(const typename TargetType<dim,dim>::T* theNode) {
#ifdef FOR_DUNE
      return theNode->levelIndex;
#else
      return theNode->id;
#endif
    }

    // /////////////////////////////////////////////
    //   Leaf indices
    // /////////////////////////////////////////////

    //! Gets the leaf index of a UG element
    static int& leafIndex(typename TargetType<0,dim>::T* theElement) {
      return theElement->ge.leafIndex;
    }

    //! Gets the leaf index of a UG element
    static const int& leafIndex(const typename TargetType<0,dim>::T* theElement) {
      return theElement->ge.leafIndex;
    }

    //! Gets the leaf index of a UG node
    static int& leafIndex(typename TargetType<dim,dim>::T* theNode) {
      return theNode->myvertex.id;
    }

    //! Gets the leaf index of a UG node
    static const int& leafIndex(const typename TargetType<dim,dim>::T* theNode) {
      return theNode->myvertex.id;
    }


    //! Gets the index of a UG element
    static int& id(typename TargetType<0,dim>::T* theElement) {
      return theElement->ge.id;
    }

    //! Gets the index of a UG element
    static const int& id(const typename TargetType<0,dim>::T* theElement) {
      return theElement->ge.id;
    }

    //! Gets the index of a UG node
    static int& id(typename TargetType<dim,dim>::T* theNode) {
      return theNode->id;
    }

    //! Gets the index of a UG node
    static const int& id(const typename TargetType<dim,dim>::T* theNode) {
      return theNode->id;
    }

    //! \todo Please doc me!
    static void Local_To_Global(int n, DOUBLE** y,
                                const FieldVector<double, dim>& local,
                                FieldVector<double, dim>& global) {
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
                              const FieldVector<double, dim>& local, FieldMatrix<double,dim,dim>& mat) {
      typedef DOUBLE DOUBLE_VECTOR[dim];
      double det;
      INVERSE_TRANSFORMATION(n, x, local, mat, det);
      return 0;
    }

    //! Returns the i-th corner of a UG element
    static typename TargetType<dim,dim>::T* Corner(typename TargetType<0,dim>::T* theElement, int i) {
      using NS_DIM_PREFIX NODE;
      using NS_DIM_PREFIX n_offset;
      return CORNER(theElement, i);
    }
    //! \todo Please doc me!
    static typename TargetType<0,dim>::T* EFather(typename TargetType<0,dim>::T* theElement) {
      using NS_DIM_PREFIX ELEMENT;
      using NS_DIM_PREFIX father_offset;
      return EFATHER(theElement);
    }

    //! \todo Please doc me!
    static void InitUg(int* argcp, char*** argvp) {
      NS_DIM_PREFIX InitUg(argcp, argvp);
    }

    //! \todo Please doc me!
    static void* CreateBoundaryValueProblem(const char* BVPname,
                                            int numOfCoeffFunc,
                                            NS_DIM_PREFIX CoeffProcPtr coeffs[],
                                            int numOfUserFct,
                                            NS_DIM_PREFIX UserProcPtr userfct[]) {
      return NS_DIM_PREFIX CreateBoundaryValueProblem(BVPname, 0, numOfCoeffFunc, coeffs,
                                                      numOfUserFct, userfct);
    }

    //! \todo Please doc me!
    static typename UGTypes<dim>::MultiGridType* GetMultigrid(const char* name) {
      return NS_DIM_PREFIX GetMultigrid(name);
    }

    //! \todo Please doc me!
    static void SetSubdomain(typename TargetType<0,dim>::T* theElement, int id) {
      using NS_DIM_PREFIX control_entries;
      using NS_DIM_PREFIX SUBDOMAIN_CE;
      SETSUBDOMAIN(theElement, id);
    }

  };
  //! \todo Please doc me!
  template <int codim, int dimworld>
  class UGGridSubEntityFactory {};

#ifdef _2
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
#endif

#ifdef _3
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
#endif

} // namespace Dune

#endif
