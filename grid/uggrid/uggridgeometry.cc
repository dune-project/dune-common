// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//****************************************************************
//
// --UGGridGeometry
//
//****************************************************************

#include <algorithm>

///////////////////////////////////////////////////////////////////////
//
// General implementation of UGGridGeometry mydim-> coorddim
//
///////////////////////////////////////////////////////////////////////


template< int mydim, int coorddim, class GridImp>
inline Dune::GeometryType Dune::UGGridGeometry<mydim,coorddim,GridImp>::type() const
{
  switch (mydim)
  {
  case 0 : return GeometryType(0);
  case 1 : return GeometryType(1);
  case 2 :

    switch (UG_NS<coorddim>::Tag(target_)) {
    case UG2d::TRIANGLE :
      return GeometryType(GeometryType::simplex,2);
    case UG2d::QUADRILATERAL :
      return GeometryType(GeometryType::cube,2);
    default :
      DUNE_THROW(GridError, "UGGridGeometry::type():  ERROR:  Unknown type "
                 << UG_NS<coorddim>::Tag(target_) << " found!");
    }

  case 3 :
    switch (UG_NS<coorddim>::Tag(target_)) {

    case UG3d::TETRAHEDRON :
      return GeometryType(GeometryType::simplex,3);
    case UG3d::PYRAMID :
      return GeometryType(GeometryType::pyramid,3);
    case UG3d::PRISM :
      return GeometryType(GeometryType::prism,3);
    case UG3d::HEXAHEDRON :
      return GeometryType(GeometryType::cube,3);
    default :
      DUNE_THROW(GridError, "UGGridGeometry::type():  ERROR:  Unknown type "
                 << UG_NS<coorddim>::Tag(target_) << " found!");

    }
  }

}

template< int mydim, int coorddim, class GridImp>
inline int Dune::UGGridGeometry<mydim,coorddim,GridImp>::corners() const
{
  return UG_NS<coorddim>::Corners_Of_Elem(target_);
}


template<int mydim, int coorddim, class GridImp>
inline const Dune::FieldVector<typename GridImp::ctype, coorddim>& Dune::UGGridGeometry<mydim,coorddim,GridImp>::
operator [](int i) const
{
  // This geometry is a vertex
  if (mydim==0) {
    assert(i==0);
    // The cast onto typename UGTypes<coorddim>::Node*
    // is only correct if this geometry represents a vertex.  But this is so since
    // we are within an if (mydim==0) clause.
    for (int i=0; i<coorddim; i++)
      coord_[0][i] = ((typename UGTypes<coorddim>::Node*)target_)->myvertex->iv.x[i];

    return coord_[0];
  }

  // ////////////////////////////////
  //  This geometry is an element
  // ////////////////////////////////
  assert(mydim==coorddim);

  i = UGGridRenumberer<mydim>::verticesDUNEtoUG(i,type());

  if (mode_==element_mode) {
    typename UGTypes<coorddim>::Node* corner = UG_NS<coorddim>::Corner(((typename UGTypes<coorddim>::Element*)target_),i);
    for (int j=0; j<coorddim; j++)
      coord_[i][j] = corner->myvertex->iv.x[j];
  }

  return coord_[i];
}

template< int mydim, int coorddim, class GridImp>
inline Dune::FieldVector<typename GridImp::ctype, coorddim> Dune::UGGridGeometry<mydim,coorddim,GridImp>::
global(const FieldVector<UGCtype, mydim>& local) const
{
  FieldVector<UGCtype, coorddim> globalCoord;

  if (mode_==element_mode)
  {
    // coorddim*coorddim is an upper bound for the number of vertices
    UGCtype* cornerCoords[coorddim*coorddim];
    UG_NS<coorddim>::Corner_Coordinates(target_, cornerCoords);

    // Actually do the computation
    UG_NS<coorddim>::Local_To_Global(corners(), cornerCoords, local, globalCoord);
  }
  else
  {
    UG_NS<coorddim>::Local_To_Global(corners(), cornerpointers_, local, globalCoord);
  }

  return globalCoord;
}


// Maps a global coordinate within the element to a
// local coordinate in its reference element
template< int mydim, int coorddim, class GridImp>
inline Dune::FieldVector<typename GridImp::ctype, mydim> Dune::UGGridGeometry<mydim,coorddim, GridImp>::
local (const Dune::FieldVector<typename GridImp::ctype, coorddim>& global) const
{
  FieldVector<UGCtype, mydim> result;
  UGCtype localCoords[mydim];

  // Copy input ADT into C-style array
  UGCtype global_c[coorddim];
  for (int i=0; i<coorddim; i++)
    global_c[i] = global[i];

  if (mode_==element_mode)
  {
    // coorddim*coorddim is an upper bound for the number of vertices
    UGCtype* cornerCoords[coorddim*coorddim];
    UG_NS<coorddim>::Corner_Coordinates(target_, cornerCoords);

    // Actually do the computation
    /** \todo Why is this const_cast necessary? */
    UG_NS<coorddim>::GlobalToLocal(corners(), const_cast<const double**>(cornerCoords), global_c, localCoords);
  }
  else
  {
    // Actually do the computation
    /** \todo Why is this const_cast necessary? */
    UG_NS<coorddim>::GlobalToLocal(corners(), const_cast<const double**>(cornerpointers_), global_c, localCoords);
  }

  // Copy result into array
  for (int i=0; i<mydim; i++)
    result[i] = localCoords[i];

  return result;
}

template<int mydim, int coorddim, class GridImp>
inline bool Dune::UGGridGeometry<mydim,coorddim,GridImp>::
checkInside(const Dune::FieldVector<UGCtype, mydim> &loc) const
{
  switch (mydim) {

  case 0 :  // vertex
    return false;
  case 1 :  // line
    return 0 <= loc[0] && loc[0] <= 1;
  case 2 :

    switch (UG_NS<coorddim>::Tag(target_)) {
    case UG2d::TRIANGLE :
      return 0 <= loc[0] && 0 <= loc[1] && (loc[0]+loc[1])<=1;
    case UG2d::QUADRILATERAL :
      return 0 <= loc[0] && loc[0] <= 1
             && 0 <= loc[1] && loc[1] <= 1;
    default :
      DUNE_THROW(GridError, "UGGridGeometry::checkInside():  ERROR:  Unknown type "
                 << UG_NS<coorddim>::Tag(target_) << " found!");
    }

  case 3 :
    switch (UG_NS<coorddim>::Tag(target_)) {

    case UG3d::TETRAHEDRON :
      return 0 <= loc[0] && 0 <= loc[1] && 0 <= loc[2]
             && (loc[0]+loc[1]+loc[2]) <= 1;
    case UG3d::PYRAMID :
      return 0 <= loc[0] && 0 <= loc[1] && 0 <= loc[2]
             && (loc[0]+loc[2]) <= 1
             && (loc[1]+loc[2]) <= 1;
    case UG3d::PRISM :
      return 0 <= loc[0] && 0 <= loc[1]
             && (loc[0]+loc[1])<=1
             && 0 <= loc[2] && loc[2] <= 1;
    case UG3d::HEXAHEDRON :
      return 0 <= loc[0] && loc[0] <= 1
             && 0 <= loc[1] && loc[1] <= 1
             && 0 <= loc[2] && loc[2] <= 1;
    default :
      DUNE_THROW(GridError, "UGGridGeometry::checkInside():  ERROR:  Unknown type "
                 << UG_NS<coorddim>::Tag(target_) << " found!");

    }
  }

}


template< int mydim, int coorddim, class GridImp>
inline typename GridImp::ctype Dune::UGGridGeometry<mydim,coorddim,GridImp>::
integrationElement (const Dune::FieldVector<typename GridImp::ctype, mydim>& local) const
{
  return std::abs(1/jacobianInverseTransposed(local).determinant());
}


template< int mydim, int coorddim, class GridImp>
inline const Dune::FieldMatrix<typename GridImp::ctype, mydim,mydim>& Dune::UGGridGeometry<mydim,coorddim, GridImp>::
jacobianInverseTransposed (const Dune::FieldVector<typename GridImp::ctype, mydim>& local) const
{
  if (mode_==element_mode)
  {
    // coorddim*coorddim is an upper bound for the number of vertices
    // compile array of pointers to corner coordinates
    UGCtype* cornerCoords[coorddim*coorddim];
    UG_NS<coorddim>::Corner_Coordinates(target_, cornerCoords);

    // compute the transformation onto the reference element (or vice versa?)
    UG_NS<coorddim>::Transformation(corners(), cornerCoords, local, jac_inverse_);
  }
  else
  {
    // compute the transformation onto the reference element (or vice versa?)
    UG_NS<coorddim>::Transformation(corners(), cornerpointers_, local, jac_inverse_);
  }

  return jac_inverse_;
}



///////////////////////////////////////////////////////////////////////
//
// The specializations 1->2, 2->3
// (only methods that are not yet defined in header file
//
///////////////////////////////////////////////////////////////////////



// Specialization for dim==1, coorddim==2.  This is necessary
// because we specialized the whole class
template <class GridImp>
inline Dune::FieldVector<typename GridImp::ctype, 2> Dune::UGGridGeometry<1,2,GridImp>::
global(const FieldVector<typename GridImp::ctype, 1>& local) const
{
  FieldVector<UGCtype, 2> globalCoord;

  /** \todo Rewrite this once there are expression templates */
  globalCoord[0] = local[0]*coord_[1][0] + (1-local[0])*coord_[0][0];
  globalCoord[1] = local[0]*coord_[1][1] + (1-local[0])*coord_[0][1];

  return globalCoord;
}

// Specialization for dim==2, coorddim==3.  This is necessary
// because we specialized the whole class
template <class GridImp>
inline Dune::FieldVector<typename GridImp::ctype, 3> Dune::UGGridGeometry<2,3,GridImp>::
global(const FieldVector<typename GridImp::ctype, 2>& local) const
{

  FieldVector<UGCtype, 3> result;

  if (elementType_.isSimplex()) {

    for (int i=0; i<3; i++)
      result[i] = (1.0-local[0]-local[1])*coord_[0][i]
                  + local[0]*coord_[1][i]
                  + local[1]*coord_[2][i];

  } else {

    // quadrilateral

    for (int i=0; i<3; i++)
      result[i] = (1.0-local[0])*(1.0-local[1])*coord_[0][i]
                  + local[0]*(1.0-local[1])*coord_[1][i]
                  + local[0]*local[1]*coord_[2][i]
                  + (1.0-local[0])*local[1]*coord_[3][i];

  }

  return result;

}

template <class GridImp>
inline typename GridImp::ctype Dune::UGGridGeometry<1,2,GridImp>::
integrationElement (const Dune::FieldVector<typename GridImp::ctype, 1>& local) const
{
  // We could call UG_NS<2>::SurfaceElement, but this is faster, and not more complicated
  FieldVector<UGCtype, 2> diff = coord_[0];
  diff -= coord_[1];
  return diff.two_norm();
}

template <class GridImp>
const Dune::FieldMatrix<typename GridImp::ctype,1,1>& Dune::UGGridGeometry<1,2,GridImp>::
jacobianInverseTransposed(const Dune::FieldVector<typename GridImp::ctype, 1>& local) const
{
  jacobianInverseTransposed_[0][0] = 1 / (coord_[0]-coord_[1]).two_norm();
  return jacobianInverseTransposed_;
}

template <class GridImp>
inline typename GridImp::ctype Dune::UGGridGeometry<2,3,GridImp>::
integrationElement (const Dune::FieldVector<typename GridImp::ctype, 2>& local) const
{
  // The cast in the second argument works because a FixedArray<FieldVector<T,3>,4>
  // has the same memory layout as a T[4][3].
  /** \todo Maybe there should be a test for this */
  return UG_NS<3>::SurfaceElement(corners(), (const double(*)[3])&coord_,&local[0]);
}

template <class GridImp>
const Dune::FieldMatrix<typename GridImp::ctype,2,2>& Dune::UGGridGeometry<2,3,GridImp>::
jacobianInverseTransposed(const Dune::FieldVector<typename GridImp::ctype, 2>& local) const
{
  // I don't really know how to implement this for quadrilateral faces,
  // especially since they may be nonplanar.
  if (!type().isTriangle())
    DUNE_THROW(NotImplemented, "jacobianInverse only implemented for triangular faces!");

  // The spatial triangle is first mapped isometrically onto the plane.  We map
  // the first vertex onto the origin, the second one on the positive x-axis,
  // and the third one such that is has positive y-coordinate.  Then we call
  // the UG-routine for planar triangles.  This is certainly not the most elegant
  // way, but the first one that comes to my mind.

  double l0 = (coord_[2]-coord_[1]).two_norm();
  double l1 = (coord_[2]-coord_[0]).two_norm();
  double l2 = (coord_[1]-coord_[0]).two_norm();

  double q0 = (l2*l2 - l0*l0 + l1*l1) / (2*l2);
  double h  = sqrt(l1*l1 - q0*q0);

  FieldVector<double,2> p0(0);
  FieldVector<double,2> p1(0);    p1[0] = l2;
  FieldVector<double,2> p2(0);    p2[0] = q0;   p2[1] = h;

  // Check that this was really an isometry
  assert( fabs(p2.two_norm()      - l1)      < 1e-6 );
  assert( fabs((p2-p1).two_norm() - l0) < 1e-6 );

  double* cornerCoords[3] = {&p0[0], &p1[0], &p2[0]};

  UG_NS<2>::Transformation(3, cornerCoords, local, jacobianInverseTransposed_);
  return jacobianInverseTransposed_;
}
