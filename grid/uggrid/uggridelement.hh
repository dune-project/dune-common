// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRIDELEMENT_HH
#define DUNE_UGGRIDELEMENT_HH

/** \file
 * \brief The UGGridElement class and its specializations
 */

#include "ugtypes.hh"
#include <dune/common/fixedarray.hh>

namespace Dune {

  template<int mydim, int coorddim, class GridImp>
  class UGMakeableGeometry : public Geometry<mydim, coorddim, GridImp, UGGridGeometry>
  {
  public:
    UGMakeableGeometry() :
      Geometry<mydim, coorddim, GridImp, UGGridGeometry>(UGGridGeometry<mydim, coorddim, GridImp>())
    {};
    //void make (Mat<cdim,mydim+1,sgrid_ctype>& __As) { this->realGeometry.make(__As); }

    void setToTarget(typename TargetType<coorddim-mydim,coorddim>::T* target) {
      this->realGeometry.setToTarget(target);
    }

  };

  //**********************************************************************
  //
  // --UGGridElement
  /** \brief Defines the geometry part of a mesh entity.
   * \ingroup UGGrid

     Works for all dimensions, element types and
     world dimensions. Provides a reference element and mapping between local and
     global coordinates.
     The element may have different implementations because the mapping can be
     done more efficiently for structured meshes than for unstructured meshes.

     dim: An element is a polygon in a hyperplane of dimension dim. 0 <= dim <= 3 is typically
     dim=0 is a point.

     dimworld: Each corner is a point with dimworld coordinates.
   */
  template<int mydim, int coorddim, class GridImp>
  class UGGridGeometry :
    public GeometryDefault <mydim, coorddim, GridImp, UGGridGeometry>
  {
    template <int codim_, int dim_, class GridImp_>
    friend class UGGridEntity;

    friend class UGMakeableGeometry<mydim,coorddim,GridImp>;

  public:

    /** Default constructor.
     */
    UGGridGeometry()
    {}

    /** \brief Return the element type identifier
     *
     * UGGrid supports triangles and quadrilaterals in 2D, and
     * tetrahedra, pyramids, prisms, and hexahedra in 3D.
     */
    GeometryType type () const;

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const;

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<UGCtype, coorddim>& operator[] (int i) const;

    /** \brief Return reference element corresponding to this element.
     *
     **If this is a reference element then self is returned.
     */
    //UGGridElement<dim,dim>& refelem ();

    /** \brief Maps a local coordinate within reference element to
     * global coordinate in element  */
    FieldVector<UGCtype, coorddim> global (const FieldVector<UGCtype, mydim>& local) const;

    /** \brief Maps a global coordinate within the element to a
     * local coordinate in its reference element */
    FieldVector<UGCtype, mydim> local (const FieldVector<UGCtype, coorddim>& global) const;

    //! Returns true if the point is in the current element
    bool checkInside(const FieldVector<UGCtype, coorddim> &global) const;

    /**
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

       Each grid module implements the integration element with optimal efficiency. This
       will directly translate in substantial savings in the computation of finite element
       stiffness matrices.
     */
    UGCtype integrationElement (const FieldVector<UGCtype, mydim>& local) const;

    //! The Jacobian matrix of the mapping from the reference element to this element
    const Mat<mydim,mydim>& jacobianInverse (const FieldVector<UGCtype, mydim>& local) const;


  private:

    /** \brief Init the element with a given UG element */
    void setToTarget(typename TargetType<coorddim-mydim,coorddim>::T* target) {target_ = target;}

    //! built the reference element
    //void makeRefElemCoords();

    //! the vertex coordinates
    mutable FixedArray<FieldVector<UGCtype, coorddim>, (mydim==2) ? 4 : 8> coord_;

    //! The jacobian inverse
    mutable Mat<coorddim,coorddim> jac_inverse_;

    //! storage for global coords
    FieldVector<UGCtype, coorddim> globalCoord_;

    //! storage for local coords
    FieldVector<UGCtype, mydim> localCoord_;

    typename TargetType<coorddim-mydim,coorddim>::T* target_;

  };


  /****************************************************************/
  /*       Specialization for faces in 3d                         */
  /****************************************************************/


  template<class GridImp>
  class UGGridGeometry<2, 3, GridImp> :
    public GeometryDefault <2, 3, GridImp, UGGridGeometry>
  {

    template <int codim_, int dim_, class GridImp_>
    friend class UGGridEntity;

    template <class GridImp_>
    friend class UGGridIntersectionIterator;

  public:

    /** \brief Default constructor */
    UGGridGeometry()
    {}

    //! return the element type identifier (triangle or quadrilateral)
    GeometryType type () const {return elementType_;}

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const {return (elementType_==triangle) ? 3 : 4;}

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<UGCtype, 3>& operator[] (int i) {
      return coord_[i];
    }

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned.
     */
    UGGridGeometry<2,2,GridImp>& refelem ();

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    FieldVector<UGCtype, 3> global (const FieldVector<UGCtype, 2>& local) const;

    //! Maps a global coordinate within the element to a
    //! local coordinate in its reference element
    FieldVector<UGCtype, 2> local (const FieldVector<UGCtype, 3>& global) const;

    //! Returns true if the point is in the current element
    bool checkInside(const FieldVector<UGCtype, 3> &global) const;

    // A(l)
    UGCtype integrationElement (const FieldVector<UGCtype, 2>& local) const;

    //! can only be called for dim=dimworld!
    const Mat<2,2>& jacobianInverse (const FieldVector<UGCtype, 2>& local) const;

  private:

    void setNumberOfCorners(int n) {
      assert(n==3 || n==4);
      elementType_ = (n==3) ? triangle : quadrilateral;
    }

    //! The element type, either triangle or quadrilateral
    GeometryType elementType_;

    //! built the reference element
    void makeRefElemCoords();

    //! the vertex coordinates
    mutable FixedArray<FieldVector<UGCtype, 3>, 4> coord_;

    //! The jacobian inverse
    mutable Mat<3,3> jac_inverse_;

    //! storage for global coords
    FieldVector<UGCtype, 4> globalCoord_;

    //! storage for local coords
    FieldVector<UGCtype, 2> localCoord_;

  };


  /****************************************************************/
  /*       Specialization for faces in 2d                         */
  /****************************************************************/


  template<class GridImp>
  class UGGridGeometry <1, 2, GridImp> :
    public GeometryDefault <1, 2, GridImp, UGGridGeometry>
  {

    template <int codim_, int dim_, class GridImp_>
    friend class UGGridEntity;

    template <class GridImp_>
    friend class UGGridIntersectionIterator;

    friend class UGMakeableGeometry<1,2,GridImp>;

  public:

    /** \brief Default constructor */
    UGGridGeometry()
    {}

    /** Return the element type identifier.  This class always returns 'line' */
    GeometryType type () const {return line;}

    //! return the number of corners of this element. This class always returns 2
    int corners () const {return 2;}

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<UGCtype, 2>& operator[] (int i) const {
      return coord_[i];
    }

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned.
     */
    UGGridGeometry<1,1,GridImp>& refelem ();

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    FieldVector<UGCtype, 2> global (const FieldVector<UGCtype, 1>& local) const;

    //! Maps a global coordinate within the element to a
    //! local coordinate in its reference element
    FieldVector<UGCtype, 1> local (const FieldVector<UGCtype, 2>& global) const;

    //! Returns true if the point is in the current element
    bool checkInside(const FieldVector<UGCtype, 1> &global) const;

    // A(l)
    UGCtype integrationElement (const FieldVector<UGCtype, 1>& local) const;

    //! can only be called for dim=dimworld!
    const Mat<1,1>& jacobianInverse (const FieldVector<UGCtype, 1>& local) const;

  private:

    // This method needs to be here to compile, but it should never be called
    void setToTarget(TargetType<1,2>::T* target) {
      DUNE_THROW(GridError, "UGGridElement<1,2>::setToTarget called!");
    }

    // Do nothing: faces in a 2d grid always have 2 corners
    void setNumberOfCorners(int n) {}

    //! the vertex coordinates
    FixedArray<FieldVector<UGCtype, 2>, 2> coord_;

#if 0
    //! The jacobian inverse
    Mat<2,2> jac_inverse_;

    //! storage for global coords
    FieldVector<UGCtype, 2> globalCoord_;

    //! storage for local coords
    FieldVector<UGCtype, 1> localCoord_;
#endif

  };

  // Include method definitions
#include "uggridelement.cc"

}  // namespace Dune

#endif
