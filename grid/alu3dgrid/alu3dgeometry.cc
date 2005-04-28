// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGEOMETRY_CC
#define DUNE_ALU3DGEOMETRY_CC

const double TrilinearMapping :: _epsilon = 1.0e-8 ;

inline TrilinearMapping ::
TrilinearMapping (const coord_t& x0, const coord_t& x1,
                  const coord_t& x2, const coord_t& x3,
                  const coord_t& x4, const coord_t& x5,
                  const coord_t& x6, const coord_t& x7)
  : p0(x0), p1(x1), p2(x2), p3(x3), p4(x4), p5(x5), p6(x6), p7(x7) {
  a [0][0] = p0 [0] ;
  a [0][1] = p0 [1] ;
  a [0][2] = p0 [2] ;
  a [1][0] = p1 [0] - p0 [0] ;
  a [1][1] = p1 [1] - p0 [1] ;
  a [1][2] = p1 [2] - p0 [2] ;
  a [2][0] = p2 [0] - p0 [0] ;
  a [2][1] = p2 [1] - p0 [1] ;
  a [2][2] = p2 [2] - p0 [2] ;
  a [3][0] = p4 [0] - p0 [0] ;
  a [3][1] = p4 [1] - p0 [1] ;
  a [3][2] = p4 [2] - p0 [2] ;
  a [4][0] = p3 [0] - p2 [0] - a [1][0] ;
  a [4][1] = p3 [1] - p2 [1] - a [1][1] ;
  a [4][2] = p3 [2] - p2 [2] - a [1][2] ;
  a [5][0] = p6 [0] - p4 [0] - a [2][0] ;
  a [5][1] = p6 [1] - p4 [1] - a [2][1] ;
  a [5][2] = p6 [2] - p4 [2] - a [2][2] ;
  a [6][0] = p5 [0] - p1 [0] - a [3][0] ;
  a [6][1] = p5 [1] - p1 [1] - a [3][1] ;
  a [6][2] = p5 [2] - p1 [2] - a [3][2] ;
  a [7][0] = p7 [0] - p5 [0] + p4 [0] - p6 [0] - p3 [0] + p1 [0] + a [2][0] ;
  a [7][1] = p7 [1] - p5 [1] + p4 [1] - p6 [1] - p3 [1] + p1 [1] + a [2][1] ;
  a [7][2] = p7 [2] - p5 [2] + p4 [2] - p6 [2] - p3 [2] + p1 [2] + a [2][2] ;
  return ;
}

inline TrilinearMapping :: TrilinearMapping (const TrilinearMapping & map)
  : p0(map.p0), p1(map.p1), p2(map.p2), p3(map.p3),
    p4(map.p4), p5(map.p5), p6(map.p6), p7(map.p7) {
  for (int i = 0 ; i < 8 ; i ++)
    for (int j = 0 ; j < 3 ; j ++)
      a [i][j] = map.a [i][j] ;
  return ;
}

inline void TrilinearMapping ::
map2world(const coord_t& p, coord_t& world) const {
  double x = p [0];
  double y = p [1];
  double z = p [2];
  double t3 = y * z ;
  double t8 = x * z ;
  double t13 = x * y ;
  double t123 = x * t3 ;
  world [0] = a [0][0] + a [1][0] * x + a [2][0] * y + a [3][0] * z + a [4][0] * t13 + a [5][0] * t3 + a [6][0] * t8 + a [7][0] * t123 ;
  world [1] = a [0][1] + a [1][1] * x + a [2][1] * y + a [3][1] * z + a [4][1] * t13 + a [5][1] * t3 + a [6][1] * t8 + a [7][1] * t123 ;
  world [2] = a [0][2] + a [1][2] * x + a [2][2] * y + a [3][2] * z + a [4][2] * t13 + a [5][2] * t3 + a [6][2] * t8 + a [7][2] * t123 ;
  return ;
}

inline void TrilinearMapping ::
map2world(const double x1, const double x2,
          const double x3, coord_t& world ) const {
  coord_t map ;
  map [0] = x1 ;
  map [1] = x2 ;
  map [2] = x3 ;
  map2world (map, world) ;
  return ;
}

void TrilinearMapping :: linear(const coord_t& p ) {
  double x = p[0];
  double y = p[1];
  double z = p[2];
  // ? get rid of that  double t0 = .5 ;
  double t3 = y * z ;
  double t8 = x * z ;
  double t13 = x * y ;
  Df[2][0] = a[1][2] + y * a[4][2] + z * a[6][2] + t3 * a[7][2] ;
  Df[2][1] = a[2][2] + x * a[4][2] + z * a[5][2] + t8 * a[7][2] ;
  Df[1][2] = a[3][1] + y * a[5][1] + x * a[6][1] + t13 * a[7][1] ;
  Df[2][2] = a[3][2] + y * a[5][2] + x * a[6][2] + t13 * a[7][2] ;
  Df[0][0] = a[1][0] + y * a[4][0] + z * a[6][0] + t3 * a[7][0] ;
  Df[0][2] = a[3][0] + y * a[5][0] + x * a[6][0] + t13 * a[7][0] ;
  Df[1][0] = a[1][1] + y * a[4][1] + z * a[6][1] + t3 * a[7][1] ;
  Df[0][1] = a[2][0] + x * a[4][0] + z * a[5][0] + t8 * a[7][0] ;
  Df[1][1] = a[2][1] + x * a[4][1] + z * a[5][1] + t8 * a[7][1] ;

}

double TrilinearMapping :: det(const coord_t& point ) {
  //  Determinante der Abbildung f:[-1,1]^3 -> Hexaeder im Punkt point.
  linear (point) ;
  return (DetDf =
            Df[0][0] * Df[1][1] * Df[2][2] - Df[0][0] * Df[1][2] * Df[2][1] -
            Df[1][0] * Df[0][1] * Df[2][2] + Df[1][0] * Df[0][2] * Df[2][1] +
            Df[2][0] * Df[0][1] * Df[1][2] - Df[2][0] * Df[0][2] * Df[1][1]) ;
}

void TrilinearMapping :: inverse(const coord_t& p ) {
  //  Kramer - Regel, det() rechnet Df und DetDf neu aus.
  double val = 1.0 / det(p) ;
  Dfi[0][0] = ( Df[1][1] * Df[2][2] - Df[1][2] * Df[2][1] ) * val ;
  Dfi[0][1] = ( Df[0][2] * Df[2][1] - Df[0][1] * Df[2][2] ) * val ;
  Dfi[0][2] = ( Df[0][1] * Df[1][2] - Df[0][2] * Df[1][1] ) * val ;
  Dfi[1][0] = ( Df[1][2] * Df[2][0] - Df[1][0] * Df[2][2] ) * val ;
  Dfi[1][1] = ( Df[0][0] * Df[2][2] - Df[0][2] * Df[2][0] ) * val ;
  Dfi[1][2] = ( Df[0][2] * Df[1][0] - Df[0][0] * Df[1][2] ) * val ;
  Dfi[2][0] = ( Df[1][0] * Df[2][1] - Df[1][1] * Df[2][0] ) * val ;
  Dfi[2][1] = ( Df[0][1] * Df[2][0] - Df[0][0] * Df[2][1] ) * val ;
  Dfi[2][2] = ( Df[0][0] * Df[1][1] - Df[0][1] * Df[1][0] ) * val ;
  return ;
}

void TrilinearMapping :: world2map (const coord_t& wld , coord_t& map ) {
  //  Newton - Iteration zum Invertieren der Abbildung f.
  double err = 10.0 * _epsilon ;
#ifndef NDEBUG
  int count = 0 ;
#endif
  map [0] = map [1] = map [2] = .0 ;
  do {
    coord_t upd ;
    map2world (map, upd) ;
    inverse (map) ;
    double u0 = upd [0] - wld [0] ;
    double u1 = upd [1] - wld [1] ;
    double u2 = upd [2] - wld [2] ;
    double c0 = Dfi [0][0] * u0 + Dfi [0][1] * u1 + Dfi [0][2] * u2 ;
    double c1 = Dfi [1][0] * u0 + Dfi [1][1] * u1 + Dfi [1][2] * u2 ;
    double c2 = Dfi [2][0] * u0 + Dfi [2][1] * u1 + Dfi [2][2] * u2 ;
    map [0] -= c0 ;
    map [1] -= c1 ;
    map [2] -= c2 ;
    err = fabs (c0) + fabs (c1) + fabs (c2) ;
    assert (count ++ < 1000) ;
  } while (err > _epsilon) ;
  return ;
}

//- Bilinear surface mapping
inline BilinearSurfaceMapping ::
BilinearSurfaceMapping (const coord3_t& x0, const coord3_t& x1,
                        const coord3_t& x2, const coord3_t& x3)
  : _p0 (x0), _p1 (x1), _p2 (x2), _p3 (x3) {
  _b [0][0] = _p0 [0] ;
  _b [0][1] = _p0 [1] ;
  _b [0][2] = _p0 [2] ;
  _b [1][0] = _p1 [0] - _p0 [0] ;
  _b [1][1] = _p1 [1] - _p0 [1] ;
  _b [1][2] = _p1 [2] - _p0 [2] ;
  _b [2][0] = _p2 [0] - _p0 [0] ;
  _b [2][1] = _p2 [1] - _p0 [1] ;
  _b [2][2] = _p2 [2] - _p0 [2] ;
  _b [3][0] = _p2 [0] - _p3 [0] - _b [1][0] ;
  _b [3][1] = _p2 [1] - _p3 [1] - _b [1][1] ;
  _b [3][2] = _p2 [2] - _p3 [2] - _b [1][2] ;
  _n [0][0] = _b [1][1] * _b [2][2] - _b [1][2] * _b [2][1] ;
  _n [0][1] = _b [1][2] * _b [2][0] - _b [1][0] * _b [2][2] ;
  _n [0][2] = _b [1][0] * _b [2][1] - _b [1][1] * _b [2][0] ;
  _n [1][0] = _b [1][1] * _b [3][2] - _b [1][2] * _b [3][1] ;
  _n [1][1] = _b [1][2] * _b [3][0] - _b [1][0] * _b [3][2] ;
  _n [1][2] = _b [1][0] * _b [3][1] - _b [1][1] * _b [3][0] ;
  _n [2][0] = _b [3][1] * _b [2][2] - _b [3][2] * _b [2][1] ;
  _n [2][1] = _b [3][2] * _b [2][0] - _b [3][0] * _b [2][2] ;
  _n [2][2] = _b [3][0] * _b [2][1] - _b [3][1] * _b [2][0] ;
  return ;
}

inline BilinearSurfaceMapping ::
BilinearSurfaceMapping (const BilinearSurfaceMapping & m)
  : _p0(m._p0), _p1(m._p1), _p2(m._p2), _p3(m._p3) {
  {for (int i = 0 ; i < 4 ; i ++)
     for (int j = 0 ; j < 3 ; j ++ )
       _b [i][j] = m._b [i][j] ;}
  {for (int i = 0 ; i < 3 ; i ++)
     for (int j = 0 ; j < 3 ; j ++ )
       _n [i][j] = m._n [i][j] ;}
  return ;
}

inline void BilinearSurfaceMapping ::
map2world (const coord2_t& map, coord3_t& wld) const {
  double x = map [0];
  double y = map [1];
  double xy = x * y ;
  wld[0] = _b [0][0] + x * _b [1][0] + y * _b [2][0] + xy * _b [3][0] ;
  wld[1] = _b [0][1] + x * _b [1][1] + y * _b [2][1] + xy * _b [3][1] ;
  wld[2] = _b [0][2] + x * _b [1][2] + y * _b [2][2] + xy * _b [3][2] ;
  return ;
}

inline void BilinearSurfaceMapping ::
map2world (double x, double y, coord3_t& w) const {
  coord2_t p ;
  p [0] = x ;
  p [1] = y ;
  map2world (p,w) ;
  return ;
}

inline void BilinearSurfaceMapping ::
normal (const coord2_t& map, coord3_t& normal) const {
  double x = map [0];
  double y = map [1];
  normal [0] = -( _n [0][0] + _n [1][0] * x + _n [2][0] * y) ;
  normal [1] = -( _n [0][1] + _n [1][1] * x + _n [2][1] * y) ;
  normal [2] = -( _n [0][2] + _n [1][2] * x + _n [2][2] * y) ;
  return ;
}

//- Tetra specialization
template<int mydim, int cdim>
inline ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> >::
ALU3dGridGeometry(bool makeRefElement)
  : builtinverse_ (false) , builtA_ (false) , builtDetDF_ (false)
{
  // create reference element
  if(makeRefElement)
  {
    coord_ = 0.0;
    for(int i=1; i<mydim+1; i++)
      coord_[i][i-1] = 1.0;
  }
}

//   B U I L T G E O M   - - -

template<int mydim, int cdim>
inline void ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> >::
calcElMatrix () const
{
  if(!builtA_)
  {
    // creat Matrix A (=Df)               INDIZES: col/row
    // Mapping: R^dim -> R^3,  F(x) = A x + p_0
    // columns:    p_1 - p_0  |  p_2 - p_0  |  p_3 - p_0

    for (int i=0; i<mydim; i++)
    {
      //FieldVector<alu3d_ctype,cdim> & row = const_cast<FieldMatrix<alu3d_ctype,matdim,matdim> &> (A_)[i];
      //row = coord_[i+1] - coord_[0];
    }
    builtA_ = true;
  }
}

// matrix for mapping from reference element to current element
template<>
inline void ALU3dGridGeometry<3,3, const ALU3dGrid<3,3,tetra> >::
calcElMatrix () const
{
  if(!builtA_)
  {
    enum { mydim = 3 };
    // creat Matrix A (=Df)               INDIZES: col/row
    // Mapping: R^dim -> R^3,  F(x) = A x + p_0
    // columns:    p_1 - p_0  |  p_2 - p_0  |  p_3 - p_0

    const FieldVector<alu3d_ctype,mydim> & coord0 = coord_[0];
    for (int i=0; i<mydim; i++)
    {
      A_[i][0] = coord_[1][i] - coord0[i];
      A_[i][1] = coord_[2][i] - coord0[i];
      A_[i][2] = coord_[3][i] - coord0[i];
    }
    builtA_ = true;
  }
}

//dim = dimworld = 3
template<int mydim, int cdim>
inline void ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > :: buildJacobianInverse() const
{
  if(!builtinverse_)
  {
    calcElMatrix();

    // DetDf = integrationElement
    detDF_ = std::abs( FMatrixHelp::invertMatrix(A_,Jinv_) );
    builtinverse_ = builtDetDF_ = true;
  }
}

template<>  //dim = 2 , dimworld = 3
inline void ALU3dGridGeometry<2,3, const ALU3dGrid<3,3,tetra> > :: buildJacobianInverse() const
{
  if(!builtinverse_)
  {
    enum { dim = 3 };

    //std::cerr << "WARNING: ALU3dGridGeometry::buildJacobianInverse not tested yet! " << __LINE__ <<"\n";
    // create vectors of face
    tmpV_ = coord_[1] - coord_[0];
    tmpU_ = coord_[2] - coord_[1];

    // calculate scaled outer normal
    for(int i=0; i<dim; i++)
    {
      globalCoord_[i] = (  tmpU_[(i+1)%dim] * tmpV_[(i+2)%dim]
                           - tmpU_[(i+2)%dim] * tmpV_[(i+1)%dim] );
    }

    detDF_ = std::abs ( globalCoord_.two_norm() );
    builtinverse_ = builtDetDF_ = true;
  }
}

template<>  //dim = 1 , dimworld = 3
inline void ALU3dGridGeometry<1,3, const ALU3dGrid<3,3,tetra> > :: buildJacobianInverse() const
{
  if(!builtinverse_)
  {
    enum { dim = 3 };
    //std::cerr << "WARNING: ALU3dGridGeometry::buildJacobianInverse not tested yet! " << __LINE__ <<"\n";
    // create vectors of face
    globalCoord_ = coord_[1] - coord_[0];
    detDF_ = std::abs ( globalCoord_.two_norm() );
    builtinverse_ = builtDetDF_ = true;
  }
}

template<>  //dim = 1 , dimworld = 3
inline void ALU3dGridGeometry<0,3, const ALU3dGrid<3,3,tetra> > :: buildJacobianInverse() const
{
  if(!builtinverse_)
  {
    enum { dim = 3 };
    detDF_ = 1.0;
    builtinverse_ = builtDetDF_ = true;
  }
}

template <>
inline bool ALU3dGridGeometry<3,3, const ALU3dGrid<3,3,tetra> > ::
buildGeom(const IMPLElementType & item)
{
  enum { dim = 3 };
  enum { dimworld = 3};

  builtinverse_ = builtA_ = builtDetDF_ = false;

  for (int i=0; i<(dim+1); i++)
  {
    const double (&p)[3] = item.myvertex(i)->Point();
    for (int j=0; j<dimworld; j++)
    {
      coord_[i][j] = p[j];
    }
  }
  return true;
}

template <>
inline bool ALU3dGridGeometry<3,3, const ALU3dGrid<3,3,tetra> > :: buildGhost(const PLLBndFaceType & ghost)
{
  enum { dim = 3 };
  enum { dimworld = 3};

  builtinverse_ = builtA_ = builtDetDF_ = false;

  GEOFaceType & face = dynamic_cast<GEOFaceType &> (*(ghost.myhface3(0)));

  // here apply the negative twist, because the twist is from the
  // neighbouring elements point of view which is outside of the ghost
  // element
  const int map[3] = { (ghost.twist(0) < 0) ? 2 : 0 , 1 , (ghost.twist(0) < 0) ? 0 : 2 };

  for (int i=0; i<dim; i++) // col is the point vector
  {
    const double (&p)[3] = face.myvertex(map[i])->Point();
    for (int j=0; j<dimworld; j++) // row is the coordinate of the point
    {
      coord_[i][j] = p[j];
    }
  }

  {
    const double (&p)[3] = ghost.oppositeVertex(0);
    for (int j=0; j<dimworld; j++)
    {
      coord_[3][j] = p[j];
    }
  }

  return true;
}

template <>
inline bool ALU3dGridGeometry<2,3, const ALU3dGrid<3,3,tetra> > :: buildGeom(const ALU3DSPACE HFaceType & item)
{
  enum { dim = 2 };
  enum { dimworld = 3};

  builtinverse_ = builtA_ = builtDetDF_ = false;

  for (int i=0; i<(dim+1); i++)
  {
    const double (&p)[3] = static_cast<const GEOFaceType &> (item).myvertex(i)->Point();
    for (int j=0; j<dimworld; j++)
    {
      coord_[i][j] = p[j];
    }
  }

  buildJacobianInverse();
  return true;
}

template <> // for edges
inline bool ALU3dGridGeometry<1,3, const ALU3dGrid<3,3,tetra> > :: buildGeom(const ALU3DSPACE HEdgeType & item)
{
  enum { dim = 1 };
  enum { dimworld = 3};

  builtinverse_ = builtA_ = builtDetDF_ = false;

  for (int i=0; i<(dim+1); i++)
  {
    const double (&p)[3] = static_cast<const GEOEdgeType &> (item).myvertex(i)->Point();
    for (int j=0; j<dimworld; j++)
    {
      coord_[i][j] = p[j];
    }
  }

  buildJacobianInverse();
  return true;
}

template <> // for Vertices ,i.e. Points
inline bool ALU3dGridGeometry<0,3, const ALU3dGrid<3,3,tetra> > :: buildGeom(const ALU3DSPACE VertexType & item)
{
  enum { dim = 0 };
  enum { dimworld = 3};

  builtinverse_ = builtA_ = builtDetDF_ = false;

  const double (&p)[3] = static_cast<const GEOVertexType &> (item).Point();
  for (int j=0; j<dimworld; j++) coord_[0][j] = p[j];

  buildJacobianInverse();
  return true;
}


template <GeometryType eltype , int dim> struct ALU3dGridElType {
  static GeometryType type () { return unknown; }
};
template <> struct ALU3dGridElType<tetrahedron,3> {
  static GeometryType type () { return tetrahedron; }
};
template <> struct ALU3dGridElType<tetrahedron,2> {
  static GeometryType type () { return triangle; }
};
template <GeometryType eltype> struct ALU3dGridElType<eltype,1> {
  static GeometryType type () { return line; }
};
template <GeometryType eltype> struct ALU3dGridElType<eltype,0> {
  static GeometryType type () { return vertex; }
};
template <> struct ALU3dGridElType<hexahedron,3> {
  static GeometryType type () { return hexahedron; }
};
template <> struct ALU3dGridElType<hexahedron,2> {
  static GeometryType type () { return quadrilateral; }
};

template<int mydim, int cdim>
inline GeometryType ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > ::type () const
{
  return ALU3dGridElType<tetrahedron,mydim>::type();
}

template<int mydim, int cdim>
inline int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > ::corners () const
{
  return dimbary;
}

template<int mydim, int cdim>
inline const FieldVector<alu3d_ctype, cdim>&
ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > :: operator[] (int i) const
{
  assert((i>=0) && (i < mydim+1));
  return coord_[i];
}

template<int mydim, int cdim>
inline FieldVector<alu3d_ctype, cdim>&
ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > :: getCoordVec (int i)
{
  assert((i>=0) && (i < mydim+1));
  return coord_[i];
}


//   G L O B A L   - - -

// dim = 1,2,3 dimworld = 3
template<int mydim, int cdim>
inline FieldVector<alu3d_ctype, cdim>
ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> >::
global(const FieldVector<alu3d_ctype, mydim>& local) const
{
  calcElMatrix();

  globalCoord_ = coord_[0];
  A_.umv(local,globalCoord_);
  return globalCoord_;
}

template<>
inline FieldVector<alu3d_ctype, 3>
ALU3dGridGeometry<3,3, const ALU3dGrid<3,3,tetra> >::
global(const FieldVector<alu3d_ctype, 3> & local) const
{
  calcElMatrix();

  globalCoord_ = coord_[0];
  A_.umv(local,globalCoord_);
  return globalCoord_;
}

template<>  // dim = dimworld = 3
inline FieldVector<alu3d_ctype, 3>
ALU3dGridGeometry<3,3,const ALU3dGrid<3,3,tetra> > ::
local(const FieldVector<alu3d_ctype, 3>& global) const
{
  if (!builtinverse_) buildJacobianInverse();
  enum { dim = 3 };
  for(int i=0; i<dim; i++)
    globalCoord_[i] = global[i] - coord_[0][i];

  FMatrixHelp::multAssign(Jinv_,globalCoord_,localCoord_);
  return localCoord_;
}

template<int mydim, int cdim>
inline bool ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > ::
checkInside(const FieldVector<alu3d_ctype, mydim>& local) const
{
  alu3d_ctype sum = 0.0;

  for(int i=0; i<mydim; i++)
  {
    sum += local[i];
    if(local[i] < 0.0)
    {
      if(std::abs(local[i]) > 1e-15)
      {
        return false;
      }
    }
  }

  if( sum > 1.0 )
  {
    if(sum > (1.0 + 1e-15))
      return false;
  }

  return true;
}

template<int mydim, int cdim>
inline alu3d_ctype
ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> > ::integrationElement (const FieldVector<alu3d_ctype, mydim>& local) const
{
  if(builtDetDF_)
    return detDF_;

  calcElMatrix();

  detDF_ = A_.determinant();

  assert(detDF_ > 0.0);

  builtDetDF_ = true;
  return detDF_;
}

//  J A C O B I A N _ I N V E R S E  - - -

template<>  // dim = dimworld = 3
inline const FieldMatrix<alu3d_ctype,3,3> &
ALU3dGridGeometry<3,3, const ALU3dGrid<3,3,tetra> >:: jacobianInverse (const FieldVector<alu3d_ctype, 3>& local) const
{
  if (!builtinverse_) buildJacobianInverse();
  return Jinv_;
}

// print the ElementInformation
template<int mydim, int cdim>
inline void ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, tetra> >::print (std::ostream& ss) const
{
  ss << "ALU3dGridGeometry<" << mydim << "," << cdim << ", tetra> = {\n";
  for(int i=0; i<corners(); i++)
  {
    ss << " corner " << i << " ";
    ss << "{" << ((*this)[i]) << "}"; ss << std::endl;
  }
  ss << "} \n";
}

template<int mydim, int cdim>
inline const Dune::Geometry<mydim,mydim,const ALU3dGrid<3,3,tetra>, Dune::ALU3dGridGeometry> &
ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3,3,tetra> >:: refelem () {
  return ALU3dGridRefElem<const ALU3dGrid<3,3,tetra>,mydim>::refelem();
}

//- Hexahedron specialization
template <>
ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::
ALU3dGridGeometry(bool makeRefElement) :
  coord_(0.0),
  triMap_(0)
{
  // Dune reference element - hardwired
  if (makeRefElement) {
    coord_[1][0] = 1.0;
    coord_[2][1] = 1.0;
    coord_[3][0] = 1.0;
    coord_[3][1] = 1.0;
    coord_[4][2] = 1.0;
    coord_[5][0] = 1.0;
    coord_[5][2] = 1.0;
    coord_[6][1] = 1.0;
    coord_[6][2] = 1.0;
    coord_[7][0] = 1.0;
    coord_[7][1] = 1.0;
    coord_[7][2] = 1.0;
  }
}

template <>
ALU3dGridGeometry<2, 3, const ALU3dGrid<3, 3, hexa> >::
ALU3dGridGeometry(bool makeRefElement)
  : coord_(0.0),
    triMap_(0) {

  if (makeRefElement) {
    coord_[1][0] = 1.0;
    coord_[2][1] = 1.0;
    coord_[3][0] = 1.0;
    coord_[3][1] = 1.0;
  }
}

template<int mydim, int cdim>
inline GeometryType
ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::type() const {
  return ALU3dGridElType<hexahedron, mydim>::type();
}

template <int mydim, int cdim>
inline int
ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::corners() const {
  return POWER_M_P<2,mydim>::power;
}

template <int mydim, int cdim>
const FieldVector<alu3d_ctype, cdim>&
ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
operator[] (int i) const {
  assert((i >= 0) && (i < corners()));
  return coord_[i];
}

template <int mydim, int cdim>
const Dune::Geometry<mydim, mydim, const ALU3dGrid<3,3,hexa>,
    Dune::ALU3dGridGeometry>&
ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
refelem () {
  return ALU3dGridRefElem<const ALU3dGrid<3, 3, hexa>, mydim>::refelem();
}

template <>
FieldVector<alu3d_ctype, 3>
ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::
global (const FieldVector<alu3d_ctype, 3>& local) const {
  FieldVector<alu3d_ctype, 3> result;
  triMap_->map2world(local, result);
  return result;
}

template <>
FieldVector<alu3d_ctype, 3>
ALU3dGridGeometry<2, 3, const ALU3dGrid<3, 3, hexa> >::
global (const FieldVector<alu3d_ctype, 2>& local) const {
  FieldVector<alu3d_ctype, 3> result;
  biMap_->map2world(local, result);
  return result;
}

template <>
FieldVector<alu3d_ctype,  3>
ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::
local (const FieldVector<alu3d_ctype, 3>& global) const {
  FieldVector<alu3d_ctype, 3> result;
  triMap_->world2map(global, result);
  return result;
}

template <>
FieldVector<alu3d_ctype,  2>
ALU3dGridGeometry<2, 3, const ALU3dGrid<3, 3, hexa> >::
local (const FieldVector<alu3d_ctype, 3>& global) const {
  FieldVector<alu3d_ctype, 2> result;
  assert(false); // could be a bit tricky, eh?
  //biMap_->world2map(global, result);
  return result;
}

template <int mydim, int cdim>
bool
ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
checkInside(const FieldVector<alu3d_ctype, mydim>& local) const {
  bool result = true;
  for (int i = 1; i < mydim; ++i) {
    result &= (local[i] >= 0.0 && local[i] <= 1.0);
  }
  return result;
}

template <int mydim, int cdim>
const FieldMatrix<alu3d_ctype,mydim,mydim>&
ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
jacobianInverse (const FieldVector<alu3d_ctype, cdim>& local) const {}

template <int mydim, int cdim>
void
ALU3dGridGeometry<mydim, cdim, const ALU3dGrid<3, 3, hexa> >::
print (std::ostream& ss) const {
  ss << "ALU3dGridGeometry<" << mydim << "," << cdim << ", hexa> = {\n";
  for(int i=0; i<corners(); i++)
  {
    ss << " corner " << i << " ";
    ss << "{" << ((*this)[i]) << "}"; ss << std::endl;
  }
  ss << "} \n";
}

template <>
bool
ALU3dGridGeometry<3, 3, const ALU3dGrid<3, 3, hexa> >::
buildGeom(const IMPLElementType& item) {
  enum { dim = 3 };
  enum { dimworld = 3 };

  for (int i = 0; i < corners(); ++i) {
    const double (&p)[3] = item.myvertex(dune2aluVol[i])->Point();
    for (int j = 0; j < dimworld; ++j) {
      coord_[i][j] = p[j];
    }
  }

  triMap_ = new TrilinearMapping(coord_[0], coord_[1], coord_[2], coord_[3],
                                 coord_[4], coord_[5], coord_[6], coord_[7]);

  return true;
}

template <>
inline bool
ALU3dGridGeometry<3,3, const ALU3dGrid<3,3,hexa> >::
buildGhost(const PLLBndFaceType & ghost) {
  enum { dim = 3 };
  enum { dimworld = 3 };

  GEOFaceType & face = dynamic_cast<GEOFaceType &> (*(ghost.myhface4(0)));

  // The ghost element can be oriented to your liking. The convention here is
  // the following: the 0th vertex of the face is mapped to the 0th vertex of
  // the ghost entity. mapFront takes into account the different numbering
  // conventions of dune and alugrid and the twist of the face. (Take into
  // account that a twist is defined with regard to the inner entity, so it is
  // actually the opposite of the twist with respect to the ghost...
  //
  //  (dune)   4 ------ 5     neg. twist: (alu)     pos. twist: (alu)
  //           /|     /|            .      .              .      .
  //          / |    / |           .      .              .      .
  //        0 ------ 1 |         0 ------ 3            0 ------ 1
  //        .| 6 --.|-- 7         |      |              |      |
  //       . | /  . | /           | .    | .            | .    | .
  //      .  |/  .  |/            |.     |.             |.     |.
  //        2 ------ 3           1 ------ 2            3 ------ 2
  //       .       .
  //      .       .
  //
  // mapFront: i \in reference hexahedron vertex index dune -> l \in reference
  // quad face vertex index alu + twist
  // Note: due to the vertex numbering for dune hexahedrons, mapFront can also
  // be used to map the back plane. The entries {0, 1, 2, 3} refer to the local
  // vertex numbers {4, 5, 6, 7} of the (dune) reference hexahedron then
  bool negativeTwist = ghost.twist(0) < 0;
  const int mapFront[4] = { 0,
                            negativeTwist ? 3 : 1,
                            negativeTwist ? 1 : 3,
                            2 };

  // Store the coords of the ghost element incident with the boundary face
  // 4 is the number of vertices of the boundary faces for hexas
  for (int i = 0; i < 4; ++i) {
    const double (&p)[3] = face.myvertex(mapFront[i])->Point();
    for (int j = 0; j < dimworld; ++j) {
      coord_[i][j] = p[j];
    }
  }

  // 4 is the number of vertices of the face opposite the boundary
  for (int i = 0; i < 4; ++i) {
    const double (&p)[3] = ghost.oppositeVertex(mapFront[i]);
    for (int j = 0; j < dimworld; ++j) {
      coord_[4+i][j] = p[j];
    }
  }

  return true;
}

template <>
inline bool
ALU3dGridGeometry<2,3, const ALU3dGrid<3, 3, hexa> > ::
buildGeom(const ALU3DSPACE HFaceType & item) {
  enum { dim = 2 };
  enum { dimworld = 3 };

  for (int i = 0; i < 4; ++i) {
    const double (&p)[3] =
      static_cast<const GEOFaceType &>(item).myvertex(dune2aluQuad[i])->Point();
    for (int j = 0; j < dimworld; ++j) {
      coord_[i][j] = p[j];
    }
  }

  biMap_ = new BilinearSurfaceMapping(coord_[0], coord_[1],
                                      coord_[2], coord_[3]);

  return true;
}

template <> // for edges
inline bool
ALU3dGridGeometry<1,3, const ALU3dGrid<3, 3, hexa> >::
buildGeom(const ALU3DSPACE HEdgeType & item) {
  enum { dim = 1 };
  enum { dimworld = 3 };

  // * potentially wrong, since the edges are renumbered in dune.
  for (int i = 0; i < 2; ++i) {
    const double (&p)[3] = static_cast<const GEOEdgeType &> (item).myvertex(i)->Point();
    for (int j = 0; j < dimworld; ++j) {
      coord_[i][j] = p[j];
    }
  }

  return true;
}

template <> // for Vertices ,i.e. Points
inline bool
ALU3dGridGeometry<0,3, const ALU3dGrid<3,3,hexa> >::
buildGeom(const ALU3DSPACE VertexType & item) {
  enum { dim = 0 };
  enum { dimworld = 3};

  const double (&p)[3] = static_cast<const GEOVertexType &> (item).Point();
  for (int j=0; j<dimworld; j++) coord_[0][j] = p[j];

  return true;
}

template<int mydim, int cdim>
const int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> >::
alu2duneVol[8] = {1, 3, 2, 0, 5, 7, 6, 4};

template<int mydim, int cdim>
const int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> >::
dune2aluVol[8] = {3, 0, 2, 1, 7, 4, 6, 5};

template<int mydim, int cdim>
const int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> >::
alu2duneFace[6] = {4, 5, 1, 3, 0, 2};

template<int mydim, int cdim>
const int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> >::
dune2aluFace[6] = {4, 2, 5, 3, 0, 1};

template<int mydim, int cdim>
const int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> >::
alu2duneQuad[4] = {0, 2, 3, 1};

template<int mydim, int cdim>
const int ALU3dGridGeometry<mydim,cdim,const ALU3dGrid<3, 3, hexa> >::
dune2aluQuad[4] = {0, 3, 1, 2};

#endif
