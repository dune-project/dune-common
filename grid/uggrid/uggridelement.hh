// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRIDELEMENT_HH
#define DUNE_UGGRIDELEMENT_HH

#include "ugtypes.hh"

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

namespace Dune {

  template<int dim, int dimworld>
  class UGGridElement :
    public ElementDefault <dim,dimworld, UGCtype,UGGridElement>
  {
    //friend class UGGridBoundaryEntity<dim,dimworld>;
  public:

    //! know dimension of world
    enum { dimbary=dim+1};

    //! for makeRefElement == true a Element with the coordinates of the
    //! reference element is made
    UGGridElement(bool makeRefElement=false);

    //! return the element type identifier
    //! line , triangle or tetrahedron, depends on dim
    ElementType type ();

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners ();

    //! access to coordinates of corners. Index is the number of the corner
    const Vec<dimworld, UGCtype>& operator[] (int i);

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned.
     */
    UGGridElement<dim,dim>& refelem ();

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    Vec<dimworld,UGCtype> global (const Vec<dim, UGCtype>& local);

    //! maps a global coordinate within the element to a
    //! local coordinate in its reference element
    Vec<dim, UGCtype>& local (const Vec<dimworld, UGCtype>& global);

    //! returns true if the point is in the current element
    bool checkInside(const Vec<dimworld, UGCtype> &global);

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
    UGCtype integration_element (const Vec<dim, UGCtype>& local);

    //! can only be called for dim=dimworld!
    Mat<dim,dim>& Jacobian_inverse (const Vec<dim, UGCtype>& local);

    //***********************************************************************
    //  Methods that not belong to the Interface, but have to be public
    //***********************************************************************
#if 0
    //! generate the geometry for the ALBERT EL_INFO
    bool builtGeom(ALBERT EL_INFO *elInfo, unsigned char face,
                   unsigned char edge, unsigned char vertex);
    // init geometry with zeros
    void initGeom();

    //! print internal data
    void print (std::ostream& ss, int indent);
#endif

    void setToTarget(typename TargetType<dimworld-dim,dimworld>::T* target) {target_ = target;}

  private:


    // calc the local barycentric coordinates
    template <int dimbary>
    Vec<dimbary, UGCtype>& localB (const Vec<dimworld, UGCtype>& global)
    {
      localBary_ = localBary(global);
      return localBary_;
    }

    //! built the reference element
    void makeRefElemCoords();

    //! built the jacobian inverse and store the volume
    void builtJacobianInverse (const Vec<dim, UGCtype>& local);

    //Vec<dim+1, UGCtype> tmpVec_;

    //! maps a global coordinate within the elements local barycentric
    //! koordinates
    Vec<dim+1, UGCtype> localBary (const Vec<dimworld, UGCtype>& global);

    // template method for map the vertices of EL_INFO to the actual
    // coords with face_,edge_ and vertex_ , needes for operator []
    int mapVertices (int i) const;

    // calculates the volume of the element
    UGCtype elVolume () const;

    //! the vertex coordinates
    Mat<dimworld,dim+1, UGCtype> coord_;

    //! storage for global coords
    Vec<dimworld, UGCtype> globalCoord_;

    //! storage for local coords
    Vec<dim, UGCtype> localCoord_;

    //! storage for barycentric coords
    Vec<dimbary, UGCtype> localBary_;

    //! is true if Jinv_ and volume_ is calced
    //bool builtinverse_;
    //Mat<dim,dim, UGCtype> Jinv_;  //!< storage for inverse of jacobian
    //UGCtype volume_; //!< storage of element volume

    typename TargetType<dimworld-dim,dimworld>::T* target_;
  };

}  // namespace Dune

#endif
