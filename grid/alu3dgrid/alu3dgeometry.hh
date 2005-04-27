// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGEOMETRY_HH
#define DUNE_ALU3DGEOMETRY_HH

// * temporary, should be moved to misc.hh or something
// calculates m^p at compile-time
template <int m, int p>
struct POWER_M_P
{
  // power stores m^p
  enum { power = (m * POWER_M_P<m,p-1>::power ) };
};

// end of recursion via specialization
template <int m>
struct POWER_M_P< m , 0>
{
  // m^0 = 1
  enum { power = 1 };
};

//! A trilinear mapping from the Dune reference hexahedron into the physical
//! space (same as in mapp_cube_3d.h, but for a different reference hexahedron)
class TrilinearMapping {
  static const double _epsilon ;
  const double (&p0)[3], (&p1)[3], (&p2)[3], (&p3)[3] ;
  const double (&p4)[3], (&p5)[3], (&p6)[3], (&p7)[3] ;
  double a [8][3] ;
  double Df [3][3] ;
  double Dfi [3][3] ;
  double DetDf ;
  void linear (const double (&)[3]) ;
  void inverse (const double (&)[3]) ;
public:
  inline TrilinearMapping (const double (&)[3], const double (&)[3],
                           const double (&)[3], const double (&)[3],
                           const double (&)[3], const double (&)[3],
                           const double (&)[3], const double (&)[3]) ;
  inline TrilinearMapping (const TrilinearMapping &) ;
  ~TrilinearMapping () {}
  double det (const double (&)[3]) ;
  inline void map2world (const double (&)[3], double (&)[3]) const ;
  inline void map2world (const double , const double , const double ,
                         double (&)[3]) const ;
  void world2map (const double (&)[3], double (&)[3]) ;
};

//! A bilinear surface mapping


//! Empty definition, needs to be specialized for element type
template <int mydim, int cdim, class GridImp>
class ALU3dGridGeometry :
  public GeometryDefault <mydim,cdim,GridImp,ALU3dGridGeometry> {};

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
  //! know dimension of barycentric coordinates
  enum { dimbary=mydim+1};
public:
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
  // generate Jacobian Inverse and calculate integration_element
  void buildJacobianInverse() const;

  // calculates the element matrix for calculation of the jacobian inverse
  void calcElMatrix () const;

  //! the vertex coordinates
  mutable FieldMatrix<alu3d_ctype,mydim+1,cdim> coord_;

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

template <int mydim, int cdim>
class ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> > :
  public GeometryDefault<mydim, cdim, const ALU3dGrid<3, 3, hexa>,
      ALU3dGridGeometry> {
  typedef const ALU3dGrid<3, 3, hexa> GridImp;
  friend class ALU3dGridBoundaryEntity<GridImp>;

  typedef ALU3dImplTraits<hexa>::IMPLElementType IMPLElementType;
  typedef ALU3dImplTraits<hexa>::PLLBndFaceType PLLBndFaceType;
  typedef ALU3dImplTraits<hexa>::GEOFaceType GEOFaceType;
  typedef ALU3dImplTraits<hexa>::GEOEdgeType GEOEdgeType;
  typedef ALU3dImplTraits<hexa>::GEOVertexType GEOVertexType;

public:
  //! for makeRefGeometry == true a Geometry with the coordinates of the
  //! reference element is made
  ALU3dGridGeometry(bool makeRefGeometry=false);

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
  //! generate the geometry for out of given ALU3dGridElement
  bool buildGeom(const IMPLElementType & item);
  bool buildGeom(const ALU3DSPACE HFaceType & item);
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
  mutable FieldMatrix<alu3d_ctype,mydim+1,cdim> coord_;

  /* OLD
     //const static int alu2duneVol[8] = {0, 1, 3, 2, 4, 5, 7, 6};
     const static int dune2aluVol[8] = {0, 1, 3, 2, 4, 5, 7, 6};

     //const static int alu2duneFace[6] = {2, 5, 4, 1, 3, 0};
     //const static int dune2aluFace[6] = {5, 3, 0, 4, 2, 1};

     //const static int alu2duneQuad[4] = {0, 1, 3, 2};
     const static int dune2aluQuad[4] = {0, 1, 3, 2};
   */

  const static int alu2duneVol[8] = {1, 3, 2, 0, 5, 7, 6, 4};
  const static int dune2aluVol[8] = {3, 0, 2, 1, 7, 4, 6, 5};

  const static int alu2duneFace[6] = {2, 5, 1, 3, 0, 4};
  const static int dune2aluFace[6] = {4, 2, 0, 3, 5, 1};

  const static int alu2duneQuad[4] = {0, 1, 3, 2};
  const static int dune2aluQuad[4] = {0, 1, 3, 2};
};



#endif
