// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//****************************************************************
//
// --UGGridElement
// --Element
//
//****************************************************************

// singleton holding reference elements
template<int dim>
struct UGGridReferenceElement
{
  enum { dimension = dim };
  UGGridElement<dim,dim> refelem;

  UGGridReferenceElement () : refelem (true) {};
};

// initialize static variable with bool constructor
// (which makes reference element)
// this sucks but for gcc we do a lot
static UGGridReferenceElement<3> reftetrahedron;
static UGGridReferenceElement<3> refpyramid;
static UGGridReferenceElement<3> refprism;
static UGGridReferenceElement<3> refpinchedhexa;
static UGGridReferenceElement<3> refhexahedron;

static UGGridReferenceElement<2> reftriangle;
static UGGridReferenceElement<2> refquadrangle;

static UGGridReferenceElement<1> refline;


//****************************************************************
//
//  specialization of mapVertices
//
//****************************************************************
//
// default, do nothing
template< int dim, int dimworld>
inline int UGGridElement<dim,dimworld>::mapVertices (int i) const
{
  return i;
}

template< int dim, int dimworld>
inline UGGridElement<dim,dimworld>::
UGGridElement(bool makeRefElement)
{
#if 0
  // make empty element
  initGeom();

  // make coords for reference elements, spezial for different dim
  if(makeRefElement)
    makeRefElemCoords();
#endif
}



template< int dim, int dimworld>
inline ElementType UGGridElement<dim,dimworld>::type()
{



#define TAG(p) ReadCW(p, UG3d::TAG_CE)

  switch (dim)
  {
  case 1 : return line;
  case 2 :
    switch (TAG(target_)) {
    case UG3d::TRIANGLE :
      return triangle;
    case UG3d::QUADRILATERAL :
      return quadrilateral;
    default :
      std::cerr << "UGGridElement::type():  ERROR:  Unknown type found!\n";
    }

  case 3 :
    switch (TAG(target_)) {
    case UG3d::TETRAHEDRON :
      return tetrahedron;
    case UG3d::PYRAMID :
      return pyramid;
    case UG3d::PRISM :
      return prism;
    case UG3d::HEXAHEDRON :
      return hexahedron;
    default :
      std::cerr << "UGGridElement::type():  ERROR:  Unknown type found!\n";
    }
  }
#undef TAG
}

template< int dim, int dimworld>
inline int UGGridElement<dim,dimworld>::corners()
{
#define TAG(p) ReadCW(p, UG3d::TAG_CE)
#define CORNERS_OF_ELEM(p)(UG3d::element_descriptors[TAG(p)]->corners_of_elem)
  return CORNERS_OF_ELEM(target_);
#undef CORNERS_OF_ELEM
#undef TAG
}


///////////////////////////////////////////////////////////////////////
template<>
inline Vec<3,UGCtype>& UGGridElement<0,3>::
operator [](int i)
{
  UG3d::VERTEX* vertex = target_->myvertex;

  coord_(0,0) = vertex->iv.x[0];
  coord_(1,0) = vertex->iv.x[1];
  coord_(2,0) = vertex->iv.x[2];

  return coord_(i);
}

template<>
inline Vec<3,UGCtype>& UGGridElement<3,3>::
operator [](int i)
{
  assert(0<=i && i<corners());

#define TAG(p) ReadCW(p, UG3d::TAG_CE)
#define CORNER(p,i) ((UG3d::NODE *) ((p)->ge.refs[UG3d::n_offset[TAG(p)]+(i)]))
  UG3d::VERTEX* vertex = CORNER(target_,i)->myvertex;
#undef CORNER
#undef TAG

  for (int j=0; j<3; j++)
    coord_(j,i) = vertex->iv.x[j];

  return coord_(i);
}


/** \todo It should be able to write this more concisely
    using partial spezialization.
 */
template<>
inline UGGridElement<3,3>& UGGridElement<3,3>::
refelem()
{
  switch (type()) {
  case tetrahedron :
    return reftetrahedron.refelem;
  case pyramid :
    return refpyramid.refelem;
  case prism :
    return refprism.refelem;
  case hexahedron :
    return refhexahedron.refelem;
  default :
    std::cerr << "Unknown element type in refelem()\n";
  }

  return reftetrahedron.refelem;
}

template<>
inline UGGridElement<2,2>& UGGridElement<2,3>::
refelem()
{
  if (type() == triangle)
    return reftriangle.refelem;
  else
    return refquadrangle.refelem;
}

template<>
inline UGGridElement<1,1>& UGGridElement<1,3>::
refelem()
{
  return refline.refelem;
}

template<>
inline UGGridElement<2,2>& UGGridElement<2,2>::
refelem()
{
  if (type() == triangle)
    return reftriangle.refelem;
  else
    return refquadrangle.refelem;
}

template<>
inline UGGridElement<1,1>& UGGridElement<1,2>::
refelem()
{
  return refline.refelem;
}


/** \todo It should be possible to avoid all this copying! */
template< int dim, int dimworld>
inline Vec<dimworld,UGCtype> UGGridElement<dim,dimworld>::
global(const Vec<dim>& local)
{
  Vec<dimworld, UGCtype> globalCoord;

  // I have to do this copying because I currently cannot pipe Vecs into
  // UG macros because of the different index operators
  UGCtype local_c[dimworld], global_c[dimworld];
  for (int i=0; i<dimworld; i++)
    local_c[i] = local(i);

  // dimworld*dimworld is an upper bound for the number of vertices
  UGCtype* cornerCoords[dimworld*dimworld];
  Corner_Coordinates(target_, cornerCoords);

  // Actually do the computation
  Local_To_Global(corners(), cornerCoords, local_c, global_c);

  for (int i=0; i<dimworld; i++)
    globalCoord(i) = global_c[i];

  return globalCoord;
}


template< int dim, int dimworld>
inline UGCtype UGGridElement<dim,dimworld>::
integration_element (const Vec<dim,UGCtype>& local)
{
  UGCtype area;

  printf("myCoords = (%g %g %g)  (%g %g %g)  (%g %g %g)  (%g %g %g)\n",
         (*this)[0](0), (*this)[0](1), (*this)[0](2),
         (*this)[1](0), (*this)[1](1), (*this)[1](2),
         (*this)[2](0), (*this)[2](1), (*this)[2](2),
         (*this)[3](0), (*this)[3](1), (*this)[3](2));

  Vec<3,double> testVec = (*this)[2];

  // dimworld*dimworld is an upper bound for the number of vertices
  UGCtype* cornerCoords[dimworld*dimworld];
  Corner_Coordinates(target_, cornerCoords);
  printf("y = (%g %g %g)  (%g %g %g)  (%g %g %g)  (%g %g %g)\n",
         cornerCoords[0][0], cornerCoords[0][1], cornerCoords[0][2],
         cornerCoords[1][0], cornerCoords[1][1], cornerCoords[1][2],
         cornerCoords[2][0], cornerCoords[2][1], cornerCoords[2][2],
         cornerCoords[3][0], cornerCoords[3][1], cornerCoords[3][2]);
  // compute the volume of the element
  Area_Of_Element(corners(), cornerCoords, area);

  return area;
}

#if 0

template< int dim, int dimworld>
inline Mat<dim,dim>& AlbertGridElement<dim,dimworld>::
Jacobian_inverse (const Vec<dim,albertCtype>& local)
{
  if(builtinverse_)
    return Jinv_;

  // builds the jacobian inverse and calculates the volume
  builtJacobianInverse(local);
  return Jinv_;
}

// calc volume of face of tetrahedron
template <>
inline void AlbertGridElement<2,3>::
builtJacobianInverse(const Vec<2,albertCtype>& local)
{
  //std::cout << "To be implemented! \n";
  //abort();
  enum { dim = 2 };
  enum { dimworld = 3 };

  // is faster than the lower method
  volume_ = 0.1;
  builtinverse_ = true;
}

template< int dim, int dimworld>
inline void AlbertGridElement<dim,dimworld>::
builtJacobianInverse(const Vec<dim,albertCtype>& local)
{
  //std::cout << dim << " " << dimworld<< " Dim|Dimworld \n";
  enum { div = (dim < 3) ? 1 : 2 };
  // volFactor should be 0.5 for dim==2 and (1.0/6.0) for dim==3
  const albertCtype volFactor = static_cast<albertCtype> (0.5/div);

  ALBERT REAL lambda[dim+1][dimworld];

  // is faster than the lower method
  volume_ = volFactor * ALBERT el_grd_lambda(elInfo_,lambda);
  for(int i=0; i<dim; i++)
  {
    for(int j=0; j<dimworld; j++)
      Jinv_(i,j) = lambda[i][j];
  }
  builtinverse_ = true;
}
#endif
#if 0
inline void AlbertGridElement<2,2>::
builtJacobianInverse(const Vec<2,albertCtype>& local)
{
  // volFactor should be 1/6, see ALBERT Doc
  const albertCtype volFactor = 0.5;
  enum { dimworld = 2 };
  enum { dim = 2 };

  REAL e1[dimworld], e2[dimworld], det, adet;
  const REAL  *v0;
  REAL a11, a12, a21, a22;

  v0 = elInfo_->coord[0];
  for (int i = 0; i < dimworld; i++)
  {
    e1[i] = elInfo_->coord[1][i] - v0[i];
    e2[i] = elInfo_->coord[2][i] - v0[i];
  }

  det = e1[0] * e2[1] - e1[1] * e2[0];
  adet = ABS(det);
  if (adet < 1.0E-25)
  {
    std::cout << "abs(det) = " << adet << "\n";
    Jinv_ = 0.0;
  }
  else
  {
    det = 1.0 / det;
    a11 =  e2[1] * det;       /* (a_ij) = A^{-T} */
    a21 = -e2[0] * det;
    a12 = -e1[1] * det;
    a22 =  e1[0] * det;

    Jinv_(1,0) = a11;
    Jinv_(1,1) = a21;
    Jinv_(2,0) = a12;
    Jinv_(2,1) = a22;
    Jinv_(0,0) = - Jinv_(1,0) - Jinv_(2,0);
    Jinv_(0,1) = - Jinv_(1,1) - Jinv_(2,1);
  }

  volume_ = volFactor * adet;
  builtinverse_ = true;
  return;
}

inline void AlbertGridElement<3,3>::
builtJacobianInverse(const Vec<3,albertCtype>& local)
{
  // volFactor should be 1/6, see ALBERT Doc
  static const albertCtype volFactor = 1.0/6.0;

  enum { dimworld = 3 };
  enum { dim = 3 };

  REAL e1[dimworld], e2[dimworld], e3[dimworld];
  const REAL  *v0;
  REAL det, adet;
  REAL a11, a12, a13, a21, a22, a23, a31, a32, a33;

  v0 = elInfo_->coord[0];
  for (int i = 0; i < dimworld; i++)
  {
    e1[i] = elInfo_->coord[1][i] - v0[i];
    e2[i] = elInfo_->coord[2][i] - v0[i];
    e3[i] = elInfo_->coord[3][i] - v0[i];
  }

  det =   e1[0] * (e2[1]*e3[2] - e2[2]*e3[1])
        - e1[1] * (e2[0]*e3[2] - e2[2]*e3[0])
        + e1[2] * (e2[0]*e3[1] - e2[1]*e3[0]);

  adet = ABS(det);
  if (adet < 1.0E-25)
  {
    std::cout << "abs(det) = " << adet << "\n";
    Jinv_ = 0.0;
  }
  else
  {
    det = 1.0 / det;
    a11 = (e2[1]*e3[2] - e2[2]*e3[1]) * det;    /* (a_ij) = A^{-T} */
    a12 = (e2[2]*e3[0] - e2[0]*e3[2]) * det;
    a13 = (e2[0]*e3[1] - e2[1]*e3[0]) * det;
    a21 = (e1[2]*e3[1] - e1[1]*e3[2]) * det;
    a22 = (e1[0]*e3[2] - e1[2]*e3[0]) * det;
    a23 = (e1[1]*e3[0] - e1[0]*e3[1]) * det;
    a31 = (e1[1]*e2[2] - e1[2]*e2[1]) * det;
    a32 = (e1[2]*e2[0] - e1[0]*e2[2]) * det;
    a33 = (e1[0]*e2[1] - e1[1]*e2[0]) * det;

    Jinv_(1,0) = a11;
    Jinv_(1,1) = a12;
    Jinv_(1,2) = a13;
    Jinv_(2,0) = a21;
    Jinv_(2,1) = a22;
    Jinv_(2,2) = a23;
    Jinv_(3,0) = a31;
    Jinv_(3,1) = a32;
    Jinv_(3,2) = a33;
    Jinv_(0,0) = -Jinv_(1,0) -Jinv_(2,0) -Jinv_(3,0);
    Jinv_(0,1) = -Jinv_(1,1) -Jinv_(2,1) -Jinv_(3,1);
    Jinv_(0,2) = -Jinv_(1,2) -Jinv_(2,2) -Jinv_(3,2);
  }

  volume_ = volFactor * adet;
  builtinverse_ = true;
  return;
}
#endif

#if 0
template <>
inline void AlbertGridElement<1,2>::
builtJacobianInverse(const Vec<1,albertCtype>& local)
{
  // volume is length of edge
  Vec<2,albertCtype> vec = coord_(0) - coord_(1);
  volume_ = vec.norm2();

  builtinverse_ = true;
}

template<int dim, int dimworld>
inline bool AlbertGridElement <dim ,dimworld >::
checkInside(const Vec<dimworld> &global)
{
  Vec<dim+1> localCoords = localBary(global);

  // return true if point is inside element
  bool ret=true;

  // if one of the barycentric coordinates is negativ
  // then the point must be outside of the element
  for(int i=0; i<dim+1; i++)
    if(localCoords(i) < 0.0) ret = false;

  return ret;
}


#endif
