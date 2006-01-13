// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRIDGEOMETRY_HH
#define DUNE_UGGRIDGEOMETRY_HH

/** \file
 * \brief The UGGridElement class and its specializations
 */

#include "ugtypes.hh"
#include <dune/common/fixedarray.hh>
#include <dune/common/fmatrix.hh>

namespace Dune {

  template<int mydim, int coorddim, class GridImp>
  class UGGridGeometry;

  template<int mydim, int coorddim, class GridImp>
  class UGMakeableGeometry : public Geometry<mydim, coorddim, GridImp, UGGridGeometry>
  {
    typedef typename GridImp::ctype UGCtype;
  public:
    UGMakeableGeometry() :
      Geometry<mydim, coorddim, GridImp, UGGridGeometry>(UGGridGeometry<mydim, coorddim, GridImp>())
    {};

    void setToTarget(typename TargetType<coorddim-mydim,coorddim>::T* target) {
      this->realGeometry.setToTarget(target);
    }

    void setCoords (int n, const FieldVector<UGCtype, coorddim>& pos) {
      this->realGeometry.setCoords(n,pos);
    }

    void coordmode ()
    {
      this->realGeometry.coordmode();
    }
  };

  template<class GridImp>
  class UGMakeableGeometry<2,3,GridImp> : public Geometry<2, 3, GridImp, UGGridGeometry>
  {
    typedef typename GridImp::ctype UGCtype;

  public:
    UGMakeableGeometry() :
      Geometry<2, 3, GridImp, UGGridGeometry>(UGGridGeometry<2,3,GridImp>())
    {};

    void setCoords(int n, const FieldVector<UGCtype, 3>& pos) {
      this->realGeometry.coord_[n] = pos;
    }

    void setNumberOfCorners(int n) {
      this->realGeometry.setNumberOfCorners(n);
    }

    // UG doesn't actually have Subfaces.  Therefore, this method should never be
    // called.  It is only here to calm the compiler
    void setToTarget(void* target) {
      DUNE_THROW(NotImplemented, "You have called UGMakeableGeometry<2,3>::setToTarget");
    }

  };

  template<class GridImp>
  class UGMakeableGeometry<1,2,GridImp> : public Geometry<1, 2, GridImp, UGGridGeometry>
  {
    typedef typename GridImp::ctype UGCtype;

  public:
    UGMakeableGeometry() :
      Geometry<1, 2, GridImp, UGGridGeometry>(UGGridGeometry<1,2,GridImp>())
    {};

    void setCoords(int n, const FieldVector<UGCtype, 2>& pos) {
      this->realGeometry.coord_[n] = pos;
    }

    // Empty.  Boundary elements in a 2d grid have always two corners
    void setNumberOfCorners(int n) {}

    // UG doesn't actually have Subfaces.  Therefore, this method should never be
    // called.  It is only here to calm the compiler
    void setToTarget(void* target) {
      DUNE_THROW(NotImplemented, "You have called UGMakeableGeometry<1,2>::setToTarget");
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

     This version is actually used only for mydim==coorddim. The manifold
     versions are in specializations below.
   */
  template<int mydim, int coorddim, class GridImp>
  class UGGridGeometry :
    public GeometryDefaultImplementation <mydim, coorddim, GridImp, UGGridGeometry>
  {
    typedef typename GridImp::ctype UGCtype;

    template <int codim_, int dim_, class GridImp_>
    friend class UGGridEntity;

    friend class UGMakeableGeometry<mydim,coorddim,GridImp>;

  public:

    /** Default constructor. Puts geometry in element mode
     */
    UGGridGeometry()
    {
      mode_ = element_mode;
    }

    //! put object in coord_mode
    void coordmode ()
    {
      // set the mode
      mode_ = coord_mode;

      // initialize pointers to data
      for (int i=0; i<((mydim==2) ? 4 : 8); i++)
        cornerpointers_[i] = &(coord_[i][0]);
    }

    /** \brief Return the element type identifier
     *
     * UGGrid supports triangles and quadrilaterals in 2D, and
     * tetrahedra, pyramids, prisms, and hexahedra in 3D.
     */
    NewGeometryType type () const;

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const;

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<UGCtype, coorddim>& operator[] (int i) const;

    /** \brief Maps a local coordinate within reference element to
     * global coordinate in element  */
    FieldVector<UGCtype, coorddim> global (const FieldVector<UGCtype, mydim>& local) const;

    /** \brief Maps a global coordinate within the element to a
     * local coordinate in its reference element */
    FieldVector<UGCtype, mydim> local (const FieldVector<UGCtype, coorddim>& global) const;

    //! Returns true if the point is in the current element
    bool checkInside(const FieldVector<UGCtype, mydim> &local) const;

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
    const FieldMatrix<UGCtype, mydim,mydim>& jacobianInverseTransposed (const FieldVector<UGCtype, mydim>& local) const;


  private:
    // mode that decides whether coordinates are taken from the element or given explicitely
    enum SourceMode {element_mode, coord_mode};

    // mode is set by constructor
    SourceMode mode_;

    /** \brief Init the element with a given UG element */
    void setToTarget(typename TargetType<coorddim-mydim,coorddim>::T* target)
    {
      target_ = target;
    }

    //! \brief set a corner
    void setCoords (int i, const FieldVector<UGCtype,coorddim>& pos)
    {
      if (mode_!=coord_mode)
        DUNE_THROW(GridError,"mode must be coord_mode!");
      coord_[i] = pos;
    }

    //! the vertex coordinates
    mutable FixedArray<FieldVector<UGCtype, coorddim>, (mydim==2) ? 4 : 8> coord_;

    //! The jacobian inverse
    mutable FieldMatrix<UGCtype,coorddim,coorddim> jac_inverse_;

    // in element mode this points to the element we map to
    // in coord_mode this is the element whose reference element is mapped into the father's one
    typename TargetType<coorddim-mydim,coorddim>::T* target_;

    // in coord_mode we explicitely store an array of coordinates
    // containing the position in the fathers reference element
    mutable UGCtype* cornerpointers_[(mydim==2) ? 4 : 8];
  };



  /****************************************************************/
  /*                                                              */
  /*       Specialization for faces in 3d                         */
  /*                                                              */
  /****************************************************************/

  template<class GridImp>
  class UGGridGeometry<2, 3, GridImp> :
    public GeometryDefaultImplementation <2, 3, GridImp, UGGridGeometry>
  {

    template <int codim_, int dim_, class GridImp_>
    friend class UGGridEntity;

    template <class GridImp_>
    friend class UGGridIntersectionIterator;

    friend class UGMakeableGeometry<2,3,GridImp>;

    typedef typename GridImp::ctype UGCtype;

  public:

    /** \brief Default constructor */
    UGGridGeometry()
    {elementType_=simplex;}

    //! return the element type identifier (triangle or quadrilateral)
    NewGeometryType type () const {
      return (elementType_==simplex)
             ? NewGeometryType(NewGeometryType::simplex,2)
             : NewGeometryType(NewGeometryType::cube,2);
    }

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const {return (elementType_==simplex) ? 3 : 4;}

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<UGCtype, 3>& operator[] (int i) const
    {
      if (elementType_==cube) {
        // Dune numbers the vertices of a hexahedron and quadrilaterals differently than UG.
        // The following two lines do the transformation
        // The renumbering scheme is {0,1,3,2} for quadrilaterals, therefore, the
        // following code works for 2d and 3d.
        // It also works in both directions UG->DUNE, DUNE->UG !
        const int renumbering[8] = {0, 1, 3, 2, 4, 5, 7, 6};
        return coord_[renumbering[i]];
      } else
        return coord_[i];
    }

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    FieldVector<UGCtype, 3> global (const FieldVector<UGCtype, 2>& local) const;

    //! Maps a global coordinate within the element to a
    //! local coordinate in its reference element
    FieldVector<UGCtype, 2> local (const FieldVector<UGCtype, 3>& global) const;

    //! Returns true if the point is in the current element
    /** \todo Not implemented yet! */
    bool checkInside(const FieldVector<UGCtype, 2> &local) const {
      DUNE_THROW(NotImplemented, "UGGridGeometry<2,3>::checkInside() not implemented yet!");
      return true;
    }

    // A(l)
    UGCtype integrationElement (const FieldVector<UGCtype, 2>& local) const;

    //! can only be called for dim=dimworld!
    const FieldMatrix<UGCtype,2,2>& jacobianInverseTransposed (const FieldVector<UGCtype, 2>& local) const;

  private:

    void setNumberOfCorners(int n) {
      assert(n==3 || n==4);
      elementType_ = (n==3) ? simplex : cube;
    }

    //! The element type, either triangle or quadrilateral
    GeometryType elementType_;

    //! the vertex coordinates
    mutable FixedArray<FieldVector<UGCtype, 3>, 4> coord_;

    //! The jacobian inverse
    mutable FieldMatrix<UGCtype,3,3> jac_inverse_;

  };


  /****************************************************************/
  /*                                                              */
  /*       Specialization for faces in 2d                         */
  /*                                                              */
  /****************************************************************/

  template<class GridImp>
  class UGGridGeometry <1, 2, GridImp> :
    public GeometryDefaultImplementation <1, 2, GridImp, UGGridGeometry>
  {

    template <int codim_, int dim_, class GridImp_>
    friend class UGGridEntity;

    template <class GridImp_>
    friend class UGGridIntersectionIterator;

    friend class UGMakeableGeometry<1,2,GridImp>;

    typedef typename GridImp::ctype UGCtype;

  public:

    /** \brief Default constructor */
    UGGridGeometry()
    {}

    /** \brief Return the element type identifier.  */
    NewGeometryType type () const {return NewGeometryType(NewGeometryType::simplex,1);}

    //! return the number of corners of this element. This class always returns 2
    int corners () const {return 2;}

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<UGCtype, 2>& operator[] (int i) const {
      // 1D -> 2D, nothing to renumber
      return coord_[i];
    }

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    FieldVector<UGCtype, 2> global (const FieldVector<UGCtype, 1>& local) const;

    //! Maps a global coordinate within the element to a
    //! local coordinate in its reference element
    FieldVector<UGCtype, 1> local (const FieldVector<UGCtype, 2>& global) const {
      DUNE_THROW(NotImplemented, "local not implemented yet!");
    }

    //! Returns true if the point is in the reference element
    bool checkInside(const FieldVector<UGCtype, 1>& local) const {
      return local[0]>=0 && local[0]<=1;
    }

    // A(l)
    UGCtype integrationElement (const FieldVector<UGCtype, 1>& local) const;

    //! can only be called for dim=dimworld!
    const FieldMatrix<UGCtype,1,1>& jacobianInverseTransposed (const FieldVector<UGCtype, 1>& local) const;

  private:

    // This method needs to be here to compile, but it should never be called
    void setToTarget(TargetType<1,2>::T* target) {
      DUNE_THROW(GridError, "UGGridElement<1,2>::setToTarget called!");
    }

    // Do nothing: faces in a 2d grid always have 2 corners
    void setNumberOfCorners(int n) {}

    //! the vertex coordinates
    FixedArray<FieldVector<UGCtype, 2>, 2> coord_;

  };

  // Include method definitions
#include "uggridgeometry.cc"

}  // namespace Dune

#endif
