// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRIDELEMENT_HH
#define DUNE_UGGRIDELEMENT_HH

#include "ugtypes.hh"

namespace Dune {

  //**********************************************************************
  //
  // --UGGridElement
  /*!
     Defines the geometry part of a mesh entity. Works for all dimensions, element types and
     world dimensions. Provides a reference element and mapping between local and
     global coordinates.
     The element may have different implementations because the mapping can be
     done more efficiently for structured meshes than for unstructured meshes.

     dim: An element is a polygon in a hyperplane of dimension dim. 0 <= dim <= 3 is typically
     dim=0 is a point.

     dimworld: Each corner is a point with dimworld coordinates.
   */
  template<int dim, int dimworld>
  class UGGridElement :
    public ElementDefault <dim,dimworld, UGCtype,UGGridElement>
  {
    //friend class UGGridBoundaryEntity<dim,dimworld>;
  public:

    //! for makeRefElement == true a Element with the coordinates of the
    //! reference element is made
    UGGridElement(bool makeRefElement=false);

    //! return the element type identifier
    //! line , triangle or tetrahedron, depends on dim
    ElementType type ();

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners ();

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<UGCtype, dimworld>& operator[] (int i);

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned.
     */
    UGGridElement<dim,dim>& refelem ();

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    FieldVector<UGCtype, dimworld> global (const FieldVector<UGCtype, dim>& local);

    //! Maps a global coordinate within the element to a
    //! local coordinate in its reference element
    FieldVector<UGCtype, dim> local (const FieldVector<UGCtype, dimworld>& global);

    //! Returns true if the point is in the current element
    bool checkInside(const FieldVector<UGCtype, dimworld> &global);

    /*!
       Copy from sgrid.hh:

       Integration over a general element is done by integrating over the reference element
       and using the transformation from the reference element to the global element as follows:
       \f[\int\limits_{\Omega_e} f(x) dx = \int\limits_{\Omega_{ref}} f(g(l)) A(l) dl \f] where
       \f$g\f$ is the local to global mapping and \f$A(l)\f$ is the integration element.

       For a general map \f$g(l)\f$ involves partial derivatives of the map (surface element of
       the first kind if \f$d=2,w=3\f$, determinant of the Jacobian of the transformation for
       \f$d=w\f$, \f$\|dg/dl\|\f$ for \f$d=1\f$).

       For linear elements, the derivatives of the map with respect to local coordinates
       do not depend on the local coordinates and are the same over the whole element.

       For a structured mesh where all edges are parallel to the coordinate axes, the
       computation is the length, area or volume of the element is very simple to compute.

       Each grid module implements the integration element with optimal efficieny. This
       will directly translate in substantial savings in the computation of finite element
       stiffness matrices.
     */

    // A(l)
    UGCtype integration_element (const FieldVector<UGCtype, dim>& local);

    //! can only be called for dim=dimworld!
    const Mat<dim,dim>& Jacobian_inverse (const FieldVector<UGCtype, dim>& local);

    //***********************************************************************
    //  Methods that not belong to the Interface, but have to be public
    //***********************************************************************

    void setToTarget(typename TargetType<dimworld-dim,dimworld>::T* target) {target_ = target;}

  private:


    //! built the reference element
    void makeRefElemCoords();

    //! the vertex coordinates
    Mat<dimworld,dim+1, UGCtype> coord_;

    //! The jacobian inverse
    Mat<dimworld,dimworld> jac_inverse_;

    //! storage for global coords
    FieldVector<UGCtype, dimworld> globalCoord_;

    //! storage for local coords
    FieldVector<UGCtype, dim> localCoord_;

    typename TargetType<dimworld-dim,dimworld>::T* target_;

  };


  /****************************************************************/
  /*       Specialization for faces in 3d                         */
  /****************************************************************/


  template<>
  class UGGridElement<2, 3> :
    public ElementDefault <2, 3, UGCtype,UGGridElement>
  {
    //friend class UGGridBoundaryEntity<dim,dimworld>;
  public:

    //! for makeRefElement == true a Element with the coordinates of the
    //! reference element is made
    UGGridElement(bool makeRefElement=false){
      std::cout << "UGGridElement<2,3> created!" << std::endl;
    }

    //! return the element type identifier (triangle or quadrilateral)
    ElementType type ();

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners ();

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<UGCtype, 3>& operator[] (int i);

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned.
     */
    UGGridElement<2,2>& refelem ();

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    FieldVector<UGCtype, 3> global (const FieldVector<UGCtype, 2>& local);

    //! Maps a global coordinate within the element to a
    //! local coordinate in its reference element
    FieldVector<UGCtype, 2> local (const FieldVector<UGCtype, 3>& global);

    //! Returns true if the point is in the current element
    bool checkInside(const FieldVector<UGCtype, 3> &global);

    // A(l)
    UGCtype integration_element (const FieldVector<UGCtype, 2>& local);

    //! can only be called for dim=dimworld!
    const Mat<2,2>& Jacobian_inverse (const FieldVector<UGCtype, 2>& local);

    //***********************************************************************
    //  Methods that not belong to the Interface, but have to be public
    //***********************************************************************

    //void setToTarget(typename TargetType<dimworld-dim,dimworld>::T* target) {target_ = target;}
    void setToTarget(TargetType<2,3>::T* target) {
      DUNE_THROW(GridError, "UGGridElement<2,3>::setToTarget called!");
    }

  private:


    //! built the reference element
    void makeRefElemCoords();

    //! the vertex coordinates
    Mat<3,3, UGCtype> coord_;

    //! The jacobian inverse
    Mat<3,3> jac_inverse_;

    //! storage for global coords
    FieldVector<UGCtype, 3> globalCoord_;

    //! storage for local coords
    FieldVector<UGCtype, 2> localCoord_;

    //typename TargetType<dimworld-dim,dimworld>::T* target_;

  };


  /****************************************************************/
  /*       Specialization for faces in 2d                         */
  /****************************************************************/


  template<>
  class UGGridElement <1, 2> :
    public ElementDefault <1, 2, UGCtype,UGGridElement>
  {
    //friend class UGGridBoundaryEntity<dim,dimworld>;
  public:

    //! for makeRefElement == true a Element with the coordinates of the
    //! reference element is made
    UGGridElement(bool makeRefElement=false) {
      std::cout << "UGGridElement<1,2> created!" << std::endl;
    }

    //! return the element type identifier
    //! line , triangle or tetrahedron, depends on dim
    ElementType type () const {return line;}

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const {return 2;}

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<UGCtype, 2>& operator[] (int i) const;

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned.
     */
    UGGridElement<1,1>& refelem ();

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    FieldVector<UGCtype, 2> global (const FieldVector<UGCtype, 1>& local);

    //! Maps a global coordinate within the element to a
    //! local coordinate in its reference element
    FieldVector<UGCtype, 1> local (const FieldVector<UGCtype, 2>& global);

    //! Returns true if the point is in the current element
    bool checkInside(const FieldVector<UGCtype, 2> &global);

    // A(l)
    UGCtype integration_element (const FieldVector<UGCtype, 1>& local);

    //! can only be called for dim=dimworld!
    const Mat<1,1>& Jacobian_inverse (const FieldVector<UGCtype, 1>& local);

    //***********************************************************************
    //  Methods that not belong to the Interface, but have to be public
    //***********************************************************************

    //void setToTarget(typename TargetType<dimworld-dim,dimworld>::T* target) {target_ = target;}
    void setToTarget(TargetType<1,2>::T* target) {
      DUNE_THROW(GridError, "UGGridElement<1,2>::setToTarget called!");
    }

  private:


    //! built the reference element
    void makeRefElemCoords();

    //! the vertex coordinates
    Mat<2,2, UGCtype> coord_;

    //! The jacobian inverse
    Mat<2,2> jac_inverse_;

    //! storage for global coords
    FieldVector<UGCtype, 2> globalCoord_;

    //! storage for local coords
    FieldVector<UGCtype, 1> localCoord_;

    //typename TargetType<dimworld-dim,dimworld>::T* target_;

  };


}  // namespace Dune

#endif
