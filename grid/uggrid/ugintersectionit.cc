// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
template<class GridImp>
inline UGGridIntersectionIterator<GridImp>::
UGGridIntersectionIterator() : virtualEntity_(-1), center_(0), neighborCount_(-1)
{}

template< class GridImp>
inline typename TargetType<0,GridImp::dimensionworld>::T* UGGridIntersectionIterator<GridImp>::
target() const
{
  if (!isValid())
    return NULL;

  return UG_NS<dimworld>::NbElem(center_, neighborCount_);
}

template< class GridImp>
inline void UGGridIntersectionIterator<GridImp>::
setToTarget(typename TargetType<0,GridImp::dimensionworld>::T* center, int nb)
{
  //printf("entering II::setToTarget %d %d\n", (int)center, nb);
  center_ = center;
  neighborCount_ = nb;
  virtualEntity_.setToTarget(target());
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

template< class GridImp>
inline bool UGGridIntersectionIterator<GridImp>::
equals (const UGGridIntersectionIterator& I) const
{
  // Two intersection iterators are equal iff they have the same
  // validity.  Furthermore, if they are both valid, they have
  // to have the same center and neighborCount_
  return (!isValid() && !I.isValid())
         || (isValid() && I.isValid() &&
             (center_ == I.center_ && neighborCount_ == I.neighborCount_));
}

#if 0
template< class GridImp>
inline bool UGGridIntersectionIterator<GridImp>::
operator !=(const UGGridIntersectionIterator& I) const
{
  return !((*this)==I);
}


template<class GridImp>
inline UGGridEntity< 0,GridImp::dimension,GridImp>*
UGGridIntersectionIterator< GridImp >::operator ->()
{
  return &virtualEntity_;
}


template<class GridImp>
inline UGGridEntity< 0,GridImp::dimension,GridImp>&
UGGridIntersectionIterator< GridImp >::operator *()
{
  return virtualEntity_;
}

template<class GridImp>
inline UGGridIntersectionIterator<GridImp>&
UGGridIntersectionIterator< GridImp >::operator++()
{
  setToTarget(center_, neighborCount_+1);
  return (*this);
}
#endif

template<class GridImp>
inline bool UGGridIntersectionIterator< GridImp >::neighbor() const
{
  return UG_NS<GridImp::dimension>::NbElem(center_, neighborCount_) != NULL;
}

/** \todo Doesn't work for locally refined grids! */
template<class GridImp>
inline bool
UGGridIntersectionIterator<GridImp>::boundary() const
{
  return !neighbor();
}

template<class GridImp>
inline FieldVector<UGCtype, GridImp::dimensionworld>&
UGGridIntersectionIterator <GridImp>::unitOuterNormal () const
{
  // //////////////////////////////////////////////////////
  //   Implementation for 3D
  // //////////////////////////////////////////////////////

#ifdef _3
  // Get the first three vertices of this side.  Since quadrilateral faces
  // are plane in UG, the normal doesn't depend on the fourth vertex
#define CORNER_OF_SIDE(p, s, c)   (UG3d::element_descriptors[UG_NS<3>::Tag(p)]->corner_of_side[(s)][(c)])
  UG3d::VERTEX* a = UG_NS<3>::Corner(center_,CORNER_OF_SIDE(center_, neighborCount_, 0))->myvertex;
  UG3d::VERTEX* b = UG_NS<3>::Corner(center_,CORNER_OF_SIDE(center_, neighborCount_, 1))->myvertex;
  UG3d::VERTEX* c = UG_NS<3>::Corner(center_,CORNER_OF_SIDE(center_, neighborCount_, 2))->myvertex;
#undef CORNER_OF_SIDE

  FieldVector<UGCtype, 3> aPos, bPos, cPos;

#define CVECT(p)   ((p)->iv.x)
#define V3_COPY(A,C)    {(C)[0] = (A)[0];   (C)[1] = (A)[1];   (C)[2] = (A)[2];}
  V3_COPY(CVECT(a), aPos);
  V3_COPY(CVECT(b), bPos);
  V3_COPY(CVECT(c), cPos);
#undef CVECT
#undef V3_COPY

  FieldVector<UGCtype, 3> ba = bPos - aPos;
  FieldVector<UGCtype, 3> ca = cPos - aPos;

#define V3_VECTOR_PRODUCT(A,B,C) {(C)[0] = (A)[1]*(B)[2] - (A)[2]*(B)[1];\
                                  (C)[1] = (A)[2]*(B)[0] - (A)[0]*(B)[2];\
                                  (C)[2] = (A)[0]*(B)[1] - (A)[1]*(B)[0];}

  V3_VECTOR_PRODUCT(ba, ca, outerNormal_);
#undef V3_VECTOR_PRODUCT

  // normalize
  outerNormal_ *= (1/outerNormal_.two_norm());
#endif

  // //////////////////////////////////////////////////////
  //   Implementation for 2D
  // //////////////////////////////////////////////////////

#ifdef _2
  // Get the vertices of this side.
#define CORNER_OF_SIDE(p, s, c)   (UG2d::element_descriptors[UG_NS<2>::Tag(p)]->corner_of_side[(s)][(c)])
  UGCtype* aPos = UG_NS<2>::Corner(center_,CORNER_OF_SIDE(center_, neighborCount_, 0))->myvertex->iv.x;
  UGCtype* bPos = UG_NS<2>::Corner(center_,CORNER_OF_SIDE(center_, neighborCount_, 1))->myvertex->iv.x;
#undef CORNER_OF_SIDE

  // compute normal
  outerNormal_[0] = bPos[1] - aPos[1];
  outerNormal_[1] = aPos[0] - bPos[0];

  // normalize
  outerNormal_ *= (1/outerNormal_.two_norm());
#endif
  return outerNormal_;
}

template<class GridImp>
inline FieldVector<UGCtype, GridImp::dimensionworld>&
UGGridIntersectionIterator < GridImp >::
unitOuterNormal (const FieldVector<UGCtype, GridImp::dimension-1>& local) const
{
  return unitOuterNormal();
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
    //         for (int j=0; j<GridImp::dimensionworld; j++)
    //             neighGlob_.coord_[i][j] = node->myvertex->iv.x[j];
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
  /** \todo Muﬂ ich die Seitennummer wirklich umrechnen? */
  const int nSides = UG_NS<GridImp::dimensionworld>::Sides_Of_Elem(center_);

  return (neighborCount_ + nSides -1)%nSides;
}

template< class GridImp>
inline int UGGridIntersectionIterator<GridImp>::
numberInNeighbor () const
{
  const typename TargetType<0,GridImp::dimensionworld>::T* other = target();

  /** \todo Muﬂ ich die Seitennummer wirklich umrechnen? */
  const int nSides = UG_NS<GridImp::dimensionworld>::Sides_Of_Elem(other);

  int i;
  for (i=0; i<Sides_Of_Elem(other); i++)
    if (NbElem(other,i) == center_)
      break;

  return (i+nSides-1)%nSides;
}

template<class GridImp>
inline
UGGridBoundaryEntity<GridImp>&
UGGridIntersectionIterator<GridImp>::
boundaryEntity ()
{
  return boundaryEntity_;
}
