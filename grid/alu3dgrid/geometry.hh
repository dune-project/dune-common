// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDGEOMETRY_HH
#define DUNE_ALU3DGRIDGEOMETRY_HH

// System includes

// Dune includes
#include <dune/common/misc.hh>
#include "../common/grid.hh"

// Local includes
#include "alu3dinclude.hh"
#include "topology.hh"

namespace Dune {
  // Forward declarations
  template<int cd, int dim, class GridImp>
  class ALU3dGridEntity;
  template<int cd, class GridImp >
  class ALU3dGridEntityPointer;
  template<int mydim, int coorddim, class GridImp>
  class ALU3dGridGeometry;
  template<int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGrid;
  template<int dim,class GridImp>
  struct ALU3dGridReferenceGeometry;
  class BilinearSurfaceMapping;
  class TrilinearMapping;

  //**********************************************************************
  //
  // --ALU3dGridGeometry
  // --Geometry
  /*!
     Defines the geometry part of a mesh entity. Works for all dimensions, element types and dimensions
     of world. Provides reference element and mapping between local and global coordinates.
     The element may have different implementations because the mapping can be
     done more efficient for structured meshes than for unstructured meshes.

     dim: An element is a polygonal in a hyperplane of dimension dim. 0 <= dim <= 3 is typically
     dim=0 is a point.

     dimworld: Each corner is a point with dimworld coordinates.
   */
  //! MakeableGeometry
  template<int mydim, int coorddim, class GridImp>
  class ALU3dGridMakeableGeometry : public Geometry<mydim, coorddim,
                                        GridImp, ALU3dGridGeometry>
  {
    typedef Geometry<mydim, coorddim, GridImp, ALU3dGridGeometry> GeometryType;
    typedef typename ALU3dImplTraits<GridImp::elementType>::PLLBndFaceType PLLBndFaceType;

    friend class ALU3dGridIntersectionIterator<GridImp>;

  public:
    typedef FieldMatrix<alu3d_ctype,
        EntityCount<GridImp::elementType>::numVerticesPerFace,
        3> FaceCoordinatesType;

    ALU3dGridMakeableGeometry(bool makeRefelem=false) :
      GeometryType (ALU3dGridGeometry<mydim, coorddim,GridImp>(makeRefelem)) {}

    ALU3dGridMakeableGeometry(GridImp & grid , int level ) :
      GeometryType (ALU3dGridGeometry<mydim, coorddim,GridImp>(false)) {}

    //! build geometry out of different ALU3dGrid Geometrys
    //! ItemType are HElementType, HFaceType, HEdgeType and VertexType
    template <class ItemType>
    bool buildGeom(const ItemType & item)
    {
      return this->realGeometry.buildGeom(item);
    }

    bool buildGeom(const ALU3DSPACE HFaceType& item) {
      return this->realGeometry.buildGeom(item);
    }

    // build a face geometry in the reference element
    //bool buildGeom(int twist, int faceIdx) {
    //  return this->realGeometry.buildGeom(twist, faceIdx);
    //}
    bool buildGeom(const FaceCoordinatesType& coords) {
      return this->realGeometry.buildGeom(coords);
    }

    // call buildGhost of realGeometry
    bool buildGhost(const PLLBndFaceType & ghost)
    {
      return this->realGeometry.buildGhost(ghost);
    }

    // print real entity for debugging
    void print (std::ostream& ss) const
    {
      this->realGeometry.print(ss);
    }

    // for changing the coordinates of one element
    FieldVector<alu3d_ctype, coorddim> & getCoordVec (int i)
    {
      return this->realGeometry.getCoordVec(i);
    }

  };

  //! ALU3dGridGeometry
  //! Empty definition, needs to be specialized for element type
  template <int mydim, int cdim, class GridImp>
  class ALU3dGridGeometry :
    public GeometryDefault <mydim,cdim,GridImp,ALU3dGridGeometry> {};

  //! Specialisation for tetrahedra
  template <int mydim, int cdim>
  class ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, tetra> > :
    public GeometryDefault<mydim, cdim, const ALU3dGrid<3, 3, tetra>,
        ALU3dGridGeometry> {
    typedef const ALU3dGrid<3, 3, tetra> GridImp;
    friend class ALU3dGridBoundaryEntity<GridImp>;

    typedef ALU3dImplTraits<tetra>::IMPLElementType IMPLElementType;
    typedef ALU3dImplTraits<tetra>::PLLBndFaceType PLLBndFaceType;
    typedef ALU3dImplTraits<tetra>::GEOFaceType GEOFaceType;
    typedef ALU3dImplTraits<tetra>::GEOEdgeType GEOEdgeType;
    typedef ALU3dImplTraits<tetra>::GEOVertexType GEOVertexType;

    typedef typename ElementTopologyMapping<tetra> ElementTopo;
    typedef typename FaceTopologyMapping<tetra> FaceTopo;
    //! know dimension of barycentric coordinates
    enum { dimbary=mydim+1};
  public:
    typedef FieldMatrix<alu3d_ctype, 3, 3> FaceCoordinatesType;
    //! for makeRefGeometry == true a Geometry with the coordinates of the
    //! reference element is made
    ALU3dGridGeometry(bool makeRefGeometry=false);

    //! return the element type identifier
    //! line , triangle or tetrahedron, depends on dim
    GeometryType type () const;

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const;

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<alu3d_ctype, cdim>& operator[] (int i) const;

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned.
     */
    static const Dune::Geometry<mydim,mydim,GridImp,Dune::ALU3dGridGeometry> & refelem ();

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    FieldVector<alu3d_ctype, cdim> global (const FieldVector<alu3d_ctype, mydim>& local) const;

    //! maps a global coordinate within the element to a
    //! local coordinate in its reference element
    FieldVector<alu3d_ctype,  mydim> local (const FieldVector<alu3d_ctype, cdim>& global) const;

    //! returns true if the point in local coordinates is inside reference element
    bool checkInside(const FieldVector<alu3d_ctype, mydim>& local) const;

    //! A(l) , see grid.hh
    alu3d_ctype integrationElement (const FieldVector<alu3d_ctype, mydim>& local) const;

    //! can only be called for dim=dimworld!
    const FieldMatrix<alu3d_ctype,mydim,mydim>& jacobianInverse (const FieldVector<alu3d_ctype, cdim>& local) const;

    //***********************************************************************
    //!  Methods that not belong to the Interface, but have to be public
    //***********************************************************************
    //! generate the geometry for out of given ALU3dGridElement
    bool buildGeom(const IMPLElementType & item);
    bool buildGeom(const ALU3DSPACE HFaceType & item);
    //bool buildGeom(int twist, int faceIdx);
    bool buildGeom(const FaceCoordinatesType& coords);
    bool buildGeom(const ALU3DSPACE HEdgeType & item);
    bool buildGeom(const ALU3DSPACE VertexType & item);

    //! build ghost out of internal boundary segment
    bool buildGhost(const PLLBndFaceType & ghost);

    //! print internal data
    //! no interface method
    void print (std::ostream& ss) const;

    // for changing the coordinates of one element
    FieldVector<alu3d_ctype, cdim> & getCoordVec (int i);

  private:
    //! calculates the vertex index in the reference element out of a face index
    //! and a local vertex index
    int faceIndex(int faceIdx, int vtxIdx) const;

    // generate Jacobian Inverse and calculate integration_element
    void buildJacobianInverse() const;

    // calculates the element matrix for calculation of the jacobian inverse
    void calcElMatrix () const;

    //! the vertex coordinates
    mutable FieldMatrix<alu3d_ctype, Power_m_p<2,mydim>::power, cdim> coord_;

    //! is true if Jinv_, A and detDF_ is calced
    mutable bool builtinverse_;
    mutable bool builtA_;
    mutable bool builtDetDF_;

    enum { matdim = (mydim > 0) ? mydim : 1 };
    mutable FieldMatrix<alu3d_ctype,matdim,matdim> Jinv_; //!< storage for inverse of jacobian
    mutable alu3d_ctype detDF_;                           //!< storage of integration_element
    mutable FieldMatrix<alu3d_ctype,matdim,matdim> A_;    //!< transformation matrix

    mutable FieldVector<alu3d_ctype, mydim> localCoord_;
    mutable FieldVector<alu3d_ctype, cdim>  globalCoord_;

    mutable FieldVector<alu3d_ctype,cdim> tmpV_; //! temporary memory
    mutable FieldVector<alu3d_ctype,cdim> tmpU_; //! temporary memory
  };

  //! Specialisation for hexahedra
  template <int mydim, int cdim>
  class ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> > :
    public GeometryDefault<mydim, cdim, const ALU3dGrid<3, 3, hexa>,
        ALU3dGridGeometry> {
    typedef const ALU3dGrid<3, 3, hexa> GridImp;
    friend class ALU3dGridBoundaryEntity<GridImp>;
    friend class ALU3dGridIntersectionIterator<GridImp>;

    typedef ALU3dImplTraits<hexa>::IMPLElementType IMPLElementType;
    typedef ALU3dImplTraits<hexa>::PLLBndFaceType PLLBndFaceType;
    typedef ALU3dImplTraits<hexa>::GEOFaceType GEOFaceType;
    typedef ALU3dImplTraits<hexa>::GEOEdgeType GEOEdgeType;
    typedef ALU3dImplTraits<hexa>::GEOVertexType GEOVertexType;

    typedef typename ElementTopologyMapping<hexa> ElementTopo;
    typedef typename FaceTopologyMapping<hexa> FaceTopo;
  public:
    typedef FieldMatrix<alu3d_ctype, 4, 3> FaceCoordinatesType;

    //! for makeRefGeometry == true a Geometry with the coordinates of the
    //! reference element is made
    ALU3dGridGeometry(bool makeRefGeometry=false);

    //! Destructor
    ~ALU3dGridGeometry();

    //! return the element type identifier
    //! line , triangle or tetrahedron, depends on dim
    GeometryType type () const;

    //! return the number of corners of this element. Corners are numbered 0..n-1
    int corners () const;

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<alu3d_ctype, cdim>& operator[] (int i) const;

    //! return reference element corresponding to this element. If this is
    //!  a reference element then self is returned.
    static const Dune::Geometry<mydim,mydim,GridImp,Dune::ALU3dGridGeometry> & refelem ();

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    FieldVector<alu3d_ctype, cdim> global (const FieldVector<alu3d_ctype, mydim>& local) const;

    //! maps a global coordinate within the element to a
    //! local coordinate in its reference element
    FieldVector<alu3d_ctype,  mydim> local (const FieldVector<alu3d_ctype, cdim>& global) const;

    //! returns true if the point in local coordinates is inside reference
    //! element
    bool checkInside(const FieldVector<alu3d_ctype, mydim>& local) const;

    //! A(l) , see grid.hh
    alu3d_ctype integrationElement (const FieldVector<alu3d_ctype, mydim>& local) const;

    //! can only be called for dim=dimworld! (Trivially true, since there is no
    //! other specialization...)
    const FieldMatrix<alu3d_ctype,mydim,mydim>& jacobianInverse (const FieldVector<alu3d_ctype, cdim>& local) const;

    //***********************************************************************
    //!  Methods that not belong to the Interface, but have to be public
    //***********************************************************************
    //! generate the geometry out of a given ALU3dGridElement
    bool buildGeom(const IMPLElementType & item);
    bool buildGeom(const ALU3DSPACE HFaceType & item);
    //bool buildGeom(int twist, int faceIdx);
    bool buildGeom(const FaceCoordinatesType& coords);
    bool buildGeom(const ALU3DSPACE HEdgeType & item);
    bool buildGeom(const ALU3DSPACE VertexType & item);

    //! build ghost out of internal boundary segment
    bool buildGhost(const PLLBndFaceType & ghost);

    //! print internal data
    //! no interface method
    void print (std::ostream& ss) const;

    // for changing the coordinates of one element
    FieldVector<alu3d_ctype, cdim> & getCoordVec (int i);

  private:
    //! the vertex coordinates
    mutable FieldMatrix<alu3d_ctype, Power_m_p<2,mydim>::power, cdim> coord_;
    //mutable FieldVector<alu3d_ctype, mydim> tmp1_;
    mutable FieldVector<alu3d_ctype, cdim> tmp2_;

    TrilinearMapping* triMap_;
    BilinearSurfaceMapping* biMap_;

    mutable FieldMatrix<alu3d_ctype, 3, 3> jInv_;

  };

  // singleton holding reference elements
  template<int dim, class GridImp>
  struct ALU3dGridReferenceGeometry
  {
    ALU3dGridMakeableGeometry<dim,dim,GridImp> refelem;
    ALU3dGridReferenceGeometry () : refelem (true) {};
  };

} // end namespace Dune

#include "geometry_imp.cc"

#endif
