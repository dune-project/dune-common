// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//****************************************************************
//
// --UGGridElement
// --Element
//
//****************************************************************

#include <algorithm>

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
static UGGridReferenceElement<3> refhexahedron;

static UGGridReferenceElement<2> reftriangle;
static UGGridReferenceElement<2> refquadrangle;

static UGGridReferenceElement<1> refline;


//****************************************************************
//
//  specialization of mapVertices
//
//****************************************************************


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
    switch (UG_NS<dimworld>::Tag(target_)) {
    case UG2d::TRIANGLE :
      return triangle;
    case UG2d::QUADRILATERAL :
      return quadrilateral;
    default :
      DUNE_THROW(GridError, "UGGridElement::type():  ERROR:  Unknown type "
                 << UG_NS<dimworld>::Tag(target_) << " found!");
    }
#endif

  case 3 :
    switch (UG_NS<dimworld>::Tag(target_)) {
#ifdef _3
    case UG3d::TETRAHEDRON :
      return tetrahedron;
    case UG3d::PYRAMID :
      return pyramid;
    case UG3d::PRISM :
      return prism;
    case UG3d::HEXAHEDRON :
      return hexahedron;
    default :
      DUNE_THROW(GridError, "UGGridElement::type():  ERROR:  Unknown type "
                 << UG_NS<dimworld>::Tag(target_) << " found!");
#endif
    }
  }

  // Just to calm the compiler
  return tetrahedron;
}

template< int dim, int dimworld>
inline int UGGridElement<dim,dimworld>::corners()
{
  return UG_NS<dimworld>::Corners_Of_Elem(target_);
}


///////////////////////////////////////////////////////////////////////

template<int dim, int dimworld>
inline const FieldVector<UGCtype, dimworld>& UGGridElement<dim,dimworld>::
operator [](int i)
{
  std::cerr << "UGGridElement<" << dim << "," << dimworld << ">::operator[]:\n"
  "Default implementation, should not be called!\n";
  return coord_(i);
}

#ifdef _3
template<>
inline const FieldVector<UGCtype, 3>& UGGridElement<0,3>::
operator [](int i)
{
  const UG3d::VERTEX* vertex = target_->myvertex;

  coord_[0][0] = vertex->iv.x[0];
  coord_[0][1] = vertex->iv.x[1];
  coord_[0][2] = vertex->iv.x[2];

  return coord_[0];
}

template<>
inline const FieldVector<UGCtype, 3>& UGGridElement<3,3>::
operator [](int i)
{
  assert(0<=i && i<corners());

#define TAG(p) ReadCW(p, UG3d::TAG_CE)
#define CORNER(p,i) ((UG3d::NODE *) ((p)->ge.refs[UG3d::n_offset[TAG(p)]+(i)]))
  UG3d::VERTEX* vertex = CORNER(target_,i)->myvertex;
#undef CORNER
#undef TAG

  for (int j=0; j<3; j++)
    coord_[i][j] = vertex->iv.x[j];

  return coord_[i];
}
#endif

#ifdef _2
template<>
inline const FieldVector<UGCtype, 2>& UGGridElement<0,2>::
operator [](int i)
{
  const UG2d::VERTEX* vertex = target_->myvertex;

  coord_[0][0] = vertex->iv.x[0];
  coord_[0][1] = vertex->iv.x[1];

  return coord_[i];
}

template<>
inline const FieldVector<UGCtype, 2>& UGGridElement<2,2>::
operator [](int i)
{
  assert(0<=i && i<corners());

  UG2d::VERTEX* vertex = UG_NS<2>::Corner(target_,i)->myvertex;

  for (int j=0; j<2; j++)
    coord_[i][j] = vertex->iv.x[j];

  return coord_[i];
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

//template<>
inline UGGridElement<1,1>& UGGridElement<1,2>::
refelem()
{
  return refline.refelem;
}


template< int dim, int dimworld>
inline FieldVector<UGCtype, dimworld> UGGridElement<dim,dimworld>::
global(const FieldVector<UGCtype, dim>& local)
{
  FieldVector<UGCtype, dimworld> globalCoord;

  // dimworld*dimworld is an upper bound for the number of vertices
  UGCtype* cornerCoords[dimworld*dimworld];
  UG_NS<dimworld>::Corner_Coordinates(target_, cornerCoords);

  // Actually do the computation
  UG_NS<dimworld>::Local_To_Global(corners(), cornerCoords, local, globalCoord);

  return globalCoord;
}

// Specialization for dim==1, dimworld==2.  This is necessary
// because we specialized the whole class
inline FieldVector<UGCtype, 2> UGGridElement<1,2>::
global(const FieldVector<UGCtype, 1>& local)
{
  FieldVector<UGCtype, 2> globalCoord;

  // I want operator +...  (sigh)
  globalCoord[0] = local[0]*coord_[1][0] + (1-local[0])*coord_[0][0];
  globalCoord[1] = local[0]*coord_[1][1] + (1-local[0])*coord_[0][1];

  return globalCoord;
}

// Specialization for dim==2, dimworld==3.  This is necessary
// because we specialized the whole class
inline FieldVector<UGCtype, 3> UGGridElement<2,3>::
global(const FieldVector<UGCtype, 2>& local)
{
  DUNE_THROW(GridError, "UGGridElement<2,3>::global not implemented yet!");
  //return FieldVector<UGCtype, 3> dummy;
}


// Maps a global coordinate within the element to a
// local coordinate in its reference element
template< int dim, int dimworld>
inline FieldVector<UGCtype, dim> UGGridElement<dim,dimworld>::
local (const FieldVector<UGCtype, dimworld>& global)
{
  FieldVector<UGCtype, dim> result;
  UGCtype localCoords[dim];

  // Copy input ADT into C-style array
  UGCtype global_c[dimworld];
  for (int i=0; i<dimworld; i++)
    global_c[i] = global[i];

  // dimworld*dimworld is an upper bound for the number of vertices
  UGCtype* cornerCoords[dimworld*dimworld];
  UG_NS<dimworld>::Corner_Coordinates(target_, cornerCoords);

  // Actually do the computation
  /** \todo Why is this const_cast necessary? */
#ifdef _2
  UG2d::UG_GlobalToLocal(corners(), const_cast<const double**>(cornerCoords), global_c, localCoords);
#else
  UG3d::UG_GlobalToLocal(corners(), const_cast<const double**>(cornerCoords), global_c, localCoords);
#endif

  // Copy result into array
  for (int i=0; i<dim; i++)
    result[i] = localCoords[i];

  return result;
}


template< int dim, int dimworld>
inline UGCtype UGGridElement<dim,dimworld>::
integration_element (const FieldVector<UGCtype, dim>& local)
{
  //std::cout << "integration element: " << std::abs(Jacobian_inverse(local).determinant()) << std::endl;
  return std::abs(1/Jacobian_inverse(local).determinant());
}

inline UGCtype UGGridElement<1,2>::
integration_element (const FieldVector<UGCtype, 1>& local)
{
  FieldVector<UGCtype, 2> diff = coord_[0];
  diff -= coord_[1];
  return diff.two_norm();
}

inline UGCtype UGGridElement<2,3>::
integration_element (const FieldVector<UGCtype, 2>& local)
{
  DUNE_THROW(GridError, "integration_element for UGGridElement<2,3> not implemented yet!");
}

template< int dim, int dimworld>
inline const Mat<dim,dim>& UGGridElement<dim,dimworld>::
Jacobian_inverse (const FieldVector<UGCtype, dim>& local)
{
  // dimworld*dimworld is an upper bound for the number of vertices
  UGCtype* cornerCoords[dimworld*dimworld];
  UG_NS<dimworld>::Corner_Coordinates(target_, cornerCoords);


  // compute the transformation onto the reference element (or vice versa?)
  UG_NS<dimworld>::Transformation(corners(), cornerCoords, local, jac_inverse_);
  //std::cout << jac_inverse_ << std::endl;
  return jac_inverse_;
}
