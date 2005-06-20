// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
template<class GridImp>
inline UGGridIntersectionIterator<GridImp>::
UGGridIntersectionIterator() : center_(0), neighborCount_(-1)
{}

template< class GridImp>
inline typename TargetType<0,GridImp::dimensionworld>::T* UGGridIntersectionIterator<GridImp>::
target() const
{
  if (!isValid())
    return center_;

  return UG_NS<dimworld>::NbElem(center_, neighborCount_);
}

template< class GridImp>
inline void UGGridIntersectionIterator<GridImp>::
setToTarget(typename TargetType<0,GridImp::dimensionworld>::T* center, int nb)
{
  //printf("entering II::setToTarget %d %d\n", (int)center, nb);
  center_ = center;
  neighborCount_ = nb;
  this->virtualEntity_.setToTarget(target());
}

template< class GridImp>
inline void UGGridIntersectionIterator<GridImp>::
setToTarget(typename TargetType<0,GridImp::dimensionworld>::T* center, int nb, int level)
{
  center_ = center;
  neighborCount_ = nb;
  this->virtualEntity_.setToTarget(target(), level);
}

template< class GridImp>
inline bool
UGGridIntersectionIterator<GridImp>::
isValid() const
{
  return center_
         && neighborCount_ >=0
         && neighborCount_ < UG_NS<GridImp::dimensionworld>::Sides_Of_Elem(center_);
}

template<class GridImp>
inline bool UGGridIntersectionIterator< GridImp >::neighbor() const
{
  return UG_NS<GridImp::dimension>::NbElem(center_, neighborCount_) != NULL;
}

template<class GridImp>
inline bool
UGGridIntersectionIterator<GridImp>::boundary() const
{
  return UG_NS<GridImp::dimension>::Side_On_Bnd(center_, neighborCount_);
}

template<class GridImp>
inline FieldVector<typename GridImp::ctype, GridImp::dimensionworld>&
UGGridIntersectionIterator <GridImp>::outerNormal () const
{
  // //////////////////////////////////////////////////////
  //   Implementation for 3D
  // //////////////////////////////////////////////////////

#ifdef _3
  // Get the first three vertices of this side.  Since quadrilateral faces
  // are plane in UG, the normal doesn't depend on the fourth vertex
  const UGCtype* aPos = UG_NS<3>::Corner(center_,UG_NS<3>::Corner_Of_Side(center_, neighborCount_, 0))->myvertex->iv.x;
  const UGCtype* bPos = UG_NS<3>::Corner(center_,UG_NS<3>::Corner_Of_Side(center_, neighborCount_, 1))->myvertex->iv.x;
  const UGCtype* cPos = UG_NS<3>::Corner(center_,UG_NS<3>::Corner_Of_Side(center_, neighborCount_, 2))->myvertex->iv.x;

  FieldVector<UGCtype, 3> ba, ca;

  for (int i=0; i<3; i++) {
    ba[i] = bPos[i] - aPos[i];
    ca[i] = cPos[i] - aPos[i];
  }

  outerNormal_[0] = ba[1]*ca[2] - ba[2]*ca[1];
  outerNormal_[1] = ba[2]*ca[0] - ba[0]*ca[2];
  outerNormal_[2] = ba[0]*ca[1] - ba[1]*ca[0];

#endif

  // //////////////////////////////////////////////////////
  //   Implementation for 2D
  // //////////////////////////////////////////////////////

#ifdef _2
  // Get the vertices of this side.
  const UGCtype* aPos = UG_NS<2>::Corner(center_,UG_NS<2>::Corner_Of_Side(center_, neighborCount_, 0))->myvertex->iv.x;
  const UGCtype* bPos = UG_NS<2>::Corner(center_,UG_NS<2>::Corner_Of_Side(center_, neighborCount_, 1))->myvertex->iv.x;

  // compute normal
  outerNormal_[0] = bPos[1] - aPos[1];
  outerNormal_[1] = aPos[0] - bPos[0];

#endif
  return outerNormal_;
}

template<class GridImp>
inline FieldVector<typename GridImp::ctype, GridImp::dimensionworld>&
UGGridIntersectionIterator < GridImp >::
outerNormal (const FieldVector<UGCtype, GridImp::dimension-1>& local) const
{
  return outerNormal();
}

template< class GridImp>
inline typename UGGridIntersectionIterator<GridImp>::LocalGeometry&
UGGridIntersectionIterator<GridImp>::
intersectionSelfLocal() const
{
  DUNE_THROW(NotImplemented, "intersection_self_local()");
  return fakeNeigh_;
}

template< class GridImp>
inline typename UGGridIntersectionIterator<GridImp>::Geometry&
UGGridIntersectionIterator<GridImp>::
intersectionGlobal() const
{
  int numCornersOfSide = UG_NS<GridImp::dimensionworld>::Corners_Of_Side(center_, neighborCount_);

  //std::cout << "Element side has " << numCornersOfSide << " corners" << std::endl;
  neighGlob_.setNumberOfCorners(numCornersOfSide);

  for (int i=0; i<numCornersOfSide; i++) {

    int cornerIdx = UG_NS<GridImp::dimensionworld>::Corner_Of_Side(center_, neighborCount_, i);
    typename TargetType<dim,dim>::T* node = UG_NS<GridImp::dimensionworld>::Corner(center_, cornerIdx);

    /** \todo Avoid the temporary */
    FieldVector<UGCtype, dimworld> tmp;
    for (int j=0; j<GridImp::dimensionworld; j++)
      tmp[j] = node->myvertex->iv.x[j];
    neighGlob_.setCoords(i, tmp);

  }

  return neighGlob_;
}

template< class GridImp>
inline typename UGGridIntersectionIterator<GridImp>::LocalGeometry&
UGGridIntersectionIterator<GridImp>::
intersectionNeighborLocal() const
{
  DUNE_THROW(NotImplemented, "intersection_neighbor_local()");
  return fakeNeigh_;
}

template< class GridImp>
inline int UGGridIntersectionIterator<GridImp>::
numberInSelf ()  const
{
  const int nSides = UG_NS<dimworld>::Sides_Of_Elem(center_);
#ifdef _3
  if (nSides==6) {   // Hexahedron
    // Dune numbers the faces of a hexahedron differently than UG.
    // The following two lines do the transformation
    const int renumbering[6] = {4, 2, 1, 3, 0, 5};
    return renumbering[neighborCount_];
  } else if (nSides==4) {   // Tetrahedron
    // Dune numbers the faces of a tetrahedron differently than UG.
    // The following two lines do the transformation
    const int renumbering[4] = {3, 0, 1, 2};
    return renumbering[neighborCount_];
  } else
    return neighborCount_;
#else
  if (nSides==3) {   // Triangle
    // Dune numbers the faces of a triangle differently from UG.
    // The following two lines do the transformation
    const int renumbering[3] = {2, 0, 1};
    return renumbering[neighborCount_];
  } else
    return neighborCount_;
#endif
}

template< class GridImp>
inline int UGGridIntersectionIterator<GridImp>::
numberInNeighbor () const
{
  const typename TargetType<0,GridImp::dimensionworld>::T* other = target();

  /** \todo Programm this correctly */
  const int nSides = UG_NS<GridImp::dimensionworld>::Sides_Of_Elem(other);

  int i;
  for (i=0; i<Sides_Of_Elem(other); i++)
    if (NbElem(other,i) == center_)
      break;

  return (i+nSides-1)%nSides;
}

template<class GridImp>
inline
const UGGridBoundaryEntity<GridImp>&
UGGridIntersectionIterator<GridImp>::
boundaryEntity () const
{
  return boundaryEntity_;
}
