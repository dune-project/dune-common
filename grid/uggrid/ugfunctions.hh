// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_UG_FUNCTIONS_HH__
#define __DUNE_UG_FUNCTIONS_HH__

/** \file
 * \brief Encapsulates some UG macros and functions
 */

#include "ugtypes.hh"

namespace Dune {
  //! \todo Please doc me!
  template<int dim>
  class UG_NS {
  public:

    /** \brief Returns pointers to the coordinate arrays of an UG element */
    static void Corner_Coordinates(typename TargetType<0,dim>::T* theElement, double* x[]) {

#ifdef _3
      using UG3d::TETRAHEDRON;
      using UG3d::NODE;
      using UG3d::PYRAMID;
      using UG3d::PRISM;
      using UG3d::HEXAHEDRON;
      using UG3d::n_offset;
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
#ifdef _3
      using UG3d::nb_offset;
      using UG3d::element_descriptors;
#else
      using UG2d::element_descriptors;
#endif
      return SIDES_OF_ELEM(theElement);
    }

    //! Encapsulates the NBELEM macro
    static typename TargetType<0,dim>::T* NbElem(typename TargetType<0,dim>::T* theElement, int nb) {
#ifdef _3
      using UG3d::ELEMENT;
      using UG3d::nb_offset;
#else
      using UG2d::ELEMENT;
      using UG2d::nb_offset;
#endif
      return NBELEM(theElement, nb);
    }
    //! \todo Please doc me!
    static int Edges_Of_Elem(const typename TargetType<0,dim>::T* theElement) {
#ifdef _2
      using UG2d::element_descriptors;
#else
      using UG3d::element_descriptors;
#endif
      return EDGES_OF_ELEM(theElement);
    }
    //! \todo Please doc me!
    static int Corners_Of_Elem(const typename TargetType<0,dim>::T* theElement) {
#ifdef _2
      using UG2d::element_descriptors;
#else
      using UG3d::element_descriptors;
#endif
      return CORNERS_OF_ELEM(theElement);
    }
    //! \todo Please doc me!
    static int Corners_Of_Side(const typename TargetType<0,dim>::T* theElement, int side) {
#ifdef _2
      using UG2d::element_descriptors;
#else
      using UG3d::element_descriptors;
#endif
      return CORNERS_OF_SIDE(theElement, side);
    }
    //! \todo Please doc me!
    static int Corner_Of_Side(const typename TargetType<0,dim>::T* theElement, int side, int corner) {
#ifdef _2
      using UG2d::element_descriptors;
#else
      using UG3d::element_descriptors;
#endif
      return CORNER_OF_SIDE(theElement, side, corner);
    }

    //! Encapsulates the TAG macro
    static unsigned int Tag(const typename TargetType<0,dim>::T* theElement) {
      return TAG(theElement);
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
                              const FieldVector<double, dim>& local, Mat<dim,dim,double>& mat) {
      typedef DOUBLE DOUBLE_VECTOR[dim];
      double det;
      INVERSE_TRANSFORMATION(n, x, local, mat, det);
      return 0;
    }

    //! Returns the i-th corner of a UG element
    static typename TargetType<dim,dim>::T* Corner(typename TargetType<0,dim>::T* theElement, int i) {
#ifdef _2
      using UG2d::NODE;
      using UG2d::n_offset;
#else
      using UG3d::NODE;
      using UG3d::n_offset;
#endif
      return CORNER(theElement, i);
    }
    //! \todo Please doc me!
    static typename TargetType<0,dim>::T* EFather(typename TargetType<0,dim>::T* theElement) {
#ifdef _3
      using UG3d::ELEMENT;
      using UG3d::father_offset;
#else
      using UG2d::ELEMENT;
      using UG2d::father_offset;
#endif
      return EFATHER(theElement);
    }

    //! \todo Please doc me!
    static void InitUg(int* argcp, char*** argvp) {
#ifdef _3
      UG3d::InitUg(argcp, argvp);
#else
      UG2d::InitUg(argcp, argvp);
#endif
    }

#ifdef _3
    //! \todo Please doc me!
    static void* CreateBoundaryValueProblem(const char* BVPname,
                                            int numOfCoeffFunc,
                                            UG3d::CoeffProcPtr coeffs[],
                                            int numOfUserFct,
                                            UG3d::UserProcPtr userfct[]) {
      return UG3d::CreateBoundaryValueProblem(BVPname, 0, numOfCoeffFunc, coeffs,
                                              numOfUserFct, userfct);
    }
#else
    //! \todo Please doc me!
    static void* CreateBoundaryValueProblem(const char* BVPname,
                                            int numOfCoeffFunc,
                                            UG2d::CoeffProcPtr coeffs[],
                                            int numOfUserFct,
                                            UG2d::UserProcPtr userfct[]) {
      return UG2d::CreateBoundaryValueProblem(BVPname, 0, numOfCoeffFunc, coeffs,
                                              numOfUserFct, userfct);
    }
#endif
    //! \todo Please doc me!
    static typename UGTypes<dim>::MultiGridType* GetMultigrid(const char* name) {
#ifdef _3
      return UG3d::GetMultigrid(name);
#else
      return UG2d::GetMultigrid(name);
#endif
    }
    //! \todo Please doc me!
    static void SetSubdomain(typename TargetType<0,dim>::T* theElement, int id) {
#ifdef _2
      using UG2d::control_entries;
      using UG2d::SUBDOMAIN_CE;
#else
      using UG3d::control_entries;
      using UG3d::SUBDOMAIN_CE;
#endif
      SETSUBDOMAIN(theElement, id);
    }

  };
  //! \todo Please doc me!
  template <int codim, int dimworld>
  class UGGridSubEntityFactory {
    //     public:
    //         static TargetType<codim,dimworld>::T* get(TargetType<codim,dimworld>::T* c, int i){
    //             DUNE_THROW(GridError, "UGGridSubEnt");
    //         }
  };

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
