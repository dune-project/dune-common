// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
template<class GridImp>
inline typename UGTypes<GridImp::dimension>::Element* UGGridIntersectionIterator< GridImp >::getNeighbor () const
{
  // if we have a neighbor on this level, then return it
  if (UG_NS<dim>::NbElem(center_, neighborCount_)!=NULL)
    return UG_NS<dim>::NbElem(center_, neighborCount_);

  // now go down the stack of copies to find a lower level leaf neighbor
  typename UGTypes<dim>::Element* father_ = UG_NS<GridImp::dimensionworld>::EFather(center_);
  while (father_!=0)
  {
    if (!UG_NS<dim>::hasCopy(father_)) break;       // father must be a copy
    if (UG_NS<dim>::NbElem(father_, neighborCount_)!=NULL)       // check existence of neighbor
      if (UG_NS<dim>::isLeaf(UG_NS<dim>::NbElem(father_, neighborCount_)))           // check leafness
        return UG_NS<dim>::NbElem(father_, neighborCount_);
    father_ = UG_NS<dim>::EFather(father_);
  }

  // nothing found, return 0 (might be a processor boundary)
  return NULL;
}

template<class GridImp>
inline bool UGGridIntersectionIterator< GridImp >::neighbor() const
{
  return getNeighbor() != NULL;
}

template<class GridImp>
inline bool
UGGridIntersectionIterator<GridImp>::boundary() const
{
  return UG_NS<dim>::Side_On_Bnd(center_, neighborCount_);
}

template<class GridImp>
inline const FieldVector<typename GridImp::ctype, GridImp::dimensionworld>&
UGGridIntersectionIterator <GridImp>::outerNormal (const FieldVector<UGCtype, GridImp::dimension-1>& local) const
{
  // //////////////////////////////////////////////////////
  //   Implementation for 3D
  // //////////////////////////////////////////////////////

  if (dim == 3) {

    // Get the first three vertices of this side.  Since quadrilateral faces
    // are plane in UG, the normal doesn't depend on the fourth vertex
    // \todo PB: There is nothing that prevents us to use non-planar quad faces ...
    const UGCtype* aPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, 0))->myvertex->iv.x;
    const UGCtype* bPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, 1))->myvertex->iv.x;
    const UGCtype* cPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, 2))->myvertex->iv.x;

    FieldVector<UGCtype, 3> ba, ca;

    for (int i=0; i<3; i++) {
      ba[i] = bPos[i] - aPos[i];
      ca[i] = cPos[i] - aPos[i];
    }

    outerNormal_[0] = ba[1]*ca[2] - ba[2]*ca[1];
    outerNormal_[1] = ba[2]*ca[0] - ba[0]*ca[2];
    outerNormal_[2] = ba[0]*ca[1] - ba[1]*ca[0];

  } else {

    // //////////////////////////////////////////////////////
    //   Implementation for 2D
    // //////////////////////////////////////////////////////

    // Get the vertices of this side.
    const UGCtype* aPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, 0))->myvertex->iv.x;
    const UGCtype* bPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, 1))->myvertex->iv.x;

    // compute normal
    outerNormal_[0] = bPos[1] - aPos[1];
    outerNormal_[1] = aPos[0] - bPos[0];

  }

  return outerNormal_;
}

template< class GridImp>
inline const typename UGGridIntersectionIterator<GridImp>::LocalGeometry&
UGGridIntersectionIterator<GridImp>::
intersectionSelfLocal() const
{
  int numCornersOfSide = UG_NS<dim>::Corners_Of_Side(center_, neighborCount_);

  selfLocal_.setNumberOfCorners(numCornersOfSide);

  for (int i=0; i<numCornersOfSide; i++)
  {
    // get number of corner in UG's numbering system
    int cornerIdx = UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, i);

    // we need a temporary to be filled
    FieldVector<UGCtype, dim> tmp;

    // get the corners local coordinates
    UG_NS<dim>::getCornerLocal(center_,cornerIdx,tmp);

    // and poke them into the Geometry
    selfLocal_.setCoords(i,tmp);
  }

  return selfLocal_;
}

template< class GridImp>
inline const typename UGGridIntersectionIterator<GridImp>::Geometry&
UGGridIntersectionIterator<GridImp>::
intersectionGlobal() const
{
  int numCornersOfSide = UG_NS<dim>::Corners_Of_Side(center_, neighborCount_);

  neighGlob_.setNumberOfCorners(numCornersOfSide);

  for (int i=0; i<numCornersOfSide; i++) {

    int cornerIdx = UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, i);
    typename UGTypes<dim>::Node* node = UG_NS<dim>::Corner(center_, cornerIdx);

#if 0
    /** \todo Avoid the temporary */
    FieldVector<UGCtype, dimworld> tmp;
    for (int j=0; j<GridImp::dimensionworld; j++)
      tmp[j] = node->myvertex->iv.x[j];
    neighGlob_.setCoords(i, tmp);
#else
    neighGlob_.setCoords(i, node->myvertex->iv.x);
#endif

  }

  return neighGlob_;
}

template< class GridImp>
inline const typename UGGridIntersectionIterator<GridImp>::LocalGeometry&
UGGridIntersectionIterator<GridImp>::
intersectionNeighborLocal() const
{
  typename UGTypes<dim>::Element *other,*self;

  // if we have a neighbor on this level, then return it
  if (UG_NS<dim>::NbElem(center_, neighborCount_)!=NULL)
  {
    other = UG_NS<dim>::NbElem(center_, neighborCount_);
    self = center_;
  }
  else
  {
    // now go down the stack of copies to find a lower level leaf neighbor
    typename UGTypes<dim>::Element* father_ = UG_NS<dim>::EFather(center_);
    while (father_!=0)
    {
      if (!UG_NS<dim>::hasCopy(father_))
        DUNE_THROW(GridError,"no neighbor found");
      if (UG_NS<dim>::NbElem(father_, neighborCount_)!=NULL)             // check existence of neighbor
        if (UG_NS<dim>::isLeaf(UG_NS<dim>::NbElem(father_, neighborCount_)))
        {
          other = UG_NS<dim>::NbElem(father_, neighborCount_);
          self = father_;
          break;
        }
      // try father
      father_ = UG_NS<dim>::EFather(father_);
    }
    if (father_==0)
      DUNE_THROW(GridError,"no neighbor found");
  }

  // we have other and self
  const int nSides = UG_NS<dim>::Sides_Of_Elem(other);
  int otherCount;
  for (otherCount=0; otherCount<nSides; otherCount++)
    if (UG_NS<dim>::NbElem(other,otherCount) == self)
      break;

  // now otherCount is the number of the face in other (in UG's numbering system)
  // go on and get the local coordinates
  int numCornersOfSide = UG_NS<dim>::Corners_Of_Side(other,otherCount);
  neighLocal_.setNumberOfCorners(numCornersOfSide);

  for (int i=0; i<numCornersOfSide; i++)
  {
    // get number of corner in UG's numbering system
    int cornerIdx = UG_NS<dim>::Corner_Of_Side(other,otherCount,i);

    // we need a temporary to be filled
    FieldVector<UGCtype, dim> tmp;

    // get the corners local coordinates
    UG_NS<GridImp::dimension>::getCornerLocal(other,cornerIdx,tmp);

    // and poke them into the Geometry
    neighLocal_.setCoords(i,tmp);
  }

  return neighLocal_;
}

template< class GridImp>
inline int UGGridIntersectionIterator<GridImp>::
numberInSelf ()  const
{
  return UGGridRenumberer<dim>::facesUGtoDUNE(neighborCount_, UG_NS<dimworld>::Sides_Of_Elem(center_));
}

template< class GridImp>
inline int UGGridIntersectionIterator<GridImp>::
numberInNeighbor () const
{
  typename UGTypes<dim>::Element *other, *self;

  // if we have a neighbor on this level, then return it
  if (UG_NS<dim>::NbElem(center_, neighborCount_)!=NULL)
  {
    other = UG_NS<dim>::NbElem(center_, neighborCount_);
    self = center_;
  }
  else
  {
    // now go down the stack of copies to find a lower level leaf neighbor
    typename UGTypes<dim>::Element* father_ = UG_NS<dim>::EFather(center_);
    while (father_!=0)
    {
      if (!UG_NS<dim>::hasCopy(father_))
        DUNE_THROW(GridError,"no neighbor found");
      if (UG_NS<dim>::NbElem(father_, neighborCount_)!=NULL)             // check existence of neighbor
        if (UG_NS<dim>::isLeaf(UG_NS<dim>::NbElem(father_, neighborCount_)))
        {
          other = UG_NS<dim>::NbElem(father_, neighborCount_);
          self = father_;
          break;
        }
      // try father
      father_ = UG_NS<dim>::EFather(father_);
    }
    if (father_==0)
      DUNE_THROW(GridError,"no neighbor found");
  }

  // we have other and self
  const int nSides = UG_NS<dim>::Sides_Of_Elem(other);
  int i;
  for (i=0; i<nSides; i++)
    if (UG_NS<dim>::NbElem(other,i) == self)
      break;

  // now we have to renumber the side i
  return UGGridRenumberer<dim>::facesUGtoDUNE(i, nSides);
}
