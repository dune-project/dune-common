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

  switch (dim)
  {
  case 1 : return line;
  case 2 :
#ifdef _2
    switch (UG<dimworld>::Tag(target_)) {
    case UG2d::TRIANGLE :
      return triangle;
    case UG2d::QUADRILATERAL :
      return quadrilateral;
    default :
      std::cerr << "UGGridElement::type():  ERROR:  Unknown type found!\n";
    }
#endif

  case 3 :
    switch (UG<dimworld>::Tag(target_)) {
#ifdef _3
    case UG3d::TETRAHEDRON :
      return tetrahedron;
    case UG3d::PYRAMID :
      return pyramid;
    case UG3d::PRISM :
      return prism;
    case UG3d::HEXAHEDRON :
      return hexahedron;
#endif
    default :
      std::cerr << "UGGridElement::type():  ERROR:  Unknown type found!\n";
    }
  }
  // Mustn't happen
  assert(false);
  // Just to calm the compiler
  return tetrahedron;
}

template< int dim, int dimworld>
inline int UGGridElement<dim,dimworld>::corners()
{
  // #define TAG(p) ReadCW(p, UG3d::TAG_CE)
  // #define CORNERS_OF_ELEM(p)(UG3d::element_descriptors[TAG(p)]->corners_of_elem)
  //     return CORNERS_OF_ELEM(target_);
  // #undef CORNERS_OF_ELEM
  // #undef TAG
  return UG<dimworld>::Corners_Of_Elem(target_);
}


///////////////////////////////////////////////////////////////////////

template<int dim, int dimworld>
inline const Vec<dimworld,UGCtype>& UGGridElement<dim,dimworld>::
operator [](int i)
{
  std::cerr << "UGGridElement<" << dim << "," << dimworld << ">::operator[]:\n"
  "Default implementation, should not be called!\n";
  return coord_(i);
}

#ifdef _3
template<>
inline const Vec<3,UGCtype>& UGGridElement<0,3>::
operator [](int i)
{
  const UG3d::VERTEX* vertex = target_->myvertex;

  coord_(0,0) = vertex->iv.x[0];
  coord_(1,0) = vertex->iv.x[1];
  coord_(2,0) = vertex->iv.x[2];

  return coord_(0);
}

template<>
inline const Vec<3,UGCtype>& UGGridElement<3,3>::
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
#endif

#ifdef _2
template<>
inline const Vec<2,UGCtype>& UGGridElement<0,2>::
operator [](int i)
{
  const UG2d::VERTEX* vertex = target_->myvertex;

  coord_(0,0) = vertex->iv.x[0];
  coord_(1,0) = vertex->iv.x[1];

  return coord_(i);
}

template<>
inline const Vec<2,UGCtype>& UGGridElement<2,2>::
operator [](int i)
{
  assert(0<=i && i<corners());

  // #define TAG(p) ReadCW(p, UG2d::TAG_CE)
  // #define CORNER(p,i) ((UG3d::NODE *) ((p)->ge.refs[UG3d::n_offset[TAG(p)]+(i)]))
  //     UG3d::VERTEX* vertex = CORNER(target_,i)->myvertex;
  // #undef CORNER
  // #undef TAG
  UG2d::VERTEX* vertex = UG<2>::Corner(target_,i)->myvertex;

  for (int j=0; j<2; j++)
    coord_(j,i) = vertex->iv.x[j];

  return coord_(i);
}
#endif


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

#if 0
template<>
inline UGGridElement<2,2>& UGGridElement<2,3>::
refelem()
{
  if (type() == triangle)
    return reftriangle.refelem;
  else
    return refquadrangle.refelem;
}
#endif

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


template< int dim, int dimworld>
inline Vec<dimworld,UGCtype> UGGridElement<dim,dimworld>::
global(const Vec<dim>& local)
{
  Vec<dimworld, UGCtype> globalCoord;

  // dimworld*dimworld is an upper bound for the number of vertices
  UGCtype* cornerCoords[dimworld*dimworld];
  UG<dimworld>::Corner_Coordinates(target_, cornerCoords);

  // Actually do the computation
  UG<dimworld>::Local_To_Global(corners(), cornerCoords, local, globalCoord);

  return globalCoord;
}


template< int dim, int dimworld>
inline UGCtype UGGridElement<dim,dimworld>::
integration_element (const Vec<dim,UGCtype>& local)
{
  //     // dimworld*dimworld is an upper bound for the number of vertices
  //     UGCtype* cornerCoords[dimworld*dimworld];
  //     UG<dimworld>::Corner_Coordinates(target_, cornerCoords);

  //     // compute the transformation onto the reference element (or vice versa?)
  //     Mat<dimworld,dimworld> mat;
  //     UG<dimworld>::Transformation(corners(), cornerCoords, local, mat);

  return ABS(Jacobian_inverse(local).determinant());
}

template< int dim, int dimworld>
inline const Mat<dim,dim>& UGGridElement<dim,dimworld>::
Jacobian_inverse (const Vec<dim,UGCtype>& local)
{
  // dimworld*dimworld is an upper bound for the number of vertices
  UGCtype* cornerCoords[dimworld*dimworld];
  UG<dimworld>::Corner_Coordinates(target_, cornerCoords);


  // compute the transformation onto the reference element (or vice versa?)
  UG<dimworld>::Transformation(corners(), cornerCoords, local, jac_inverse_);

  return jac_inverse_;
}
