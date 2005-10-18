// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
template<class GridImp>
inline typename TargetType<0,GridImp::dimensionworld>::T* UGGridIntersectionIterator< GridImp >::getNeighbor () const
{
  // if we have a neighbor on this level, then return it
  if (UG_NS<GridImp::dimension>::NbElem(center_, neighborCount_)!=NULL)
    return UG_NS<GridImp::dimension>::NbElem(center_, neighborCount_);

  // now go down the stack of copies to find a lower level leaf neighbor
  typename TargetType<0,GridImp::dimensionworld>::T* father_ = UG_NS<GridImp::dimensionworld>::EFather(center_);
  while (father_!=0)
  {
    if (!UG_NS<GridImp::dimensionworld>::hasCopy(father_)) break;       // father must be a copy
    if (UG_NS<GridImp::dimension>::NbElem(father_, neighborCount_)!=NULL)       // check existence of neighbor
      if (UG_NS<GridImp::dimension>::isLeaf(UG_NS<GridImp::dimension>::NbElem(father_, neighborCount_)))           // check leafness
        return UG_NS<GridImp::dimension>::NbElem(father_, neighborCount_);
    father_ = UG_NS<GridImp::dimensionworld>::EFather(father_);
  }

  // nothing found, return 0 (might be a processor boundary
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
  return UG_NS<GridImp::dimension>::Side_On_Bnd(center_, neighborCount_);
}

template<class GridImp>
inline const FieldVector<typename GridImp::ctype, GridImp::dimensionworld>&
UGGridIntersectionIterator <GridImp>::outerNormal (const FieldVector<UGCtype, GridImp::dimension-1>& local) const
{
  // //////////////////////////////////////////////////////
  //   Implementation for 3D
  // //////////////////////////////////////////////////////
  const int dim = GridImp::dimension;

  if (dim == 3) {

    // Get the first three vertices of this side.  Since quadrilateral faces
    // are plane in UG, the normal doesn't depend on the fourth vertex
    // \todo PB: There is nothing that prevents us to use non-polanar quad faces ...
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
  int numCornersOfSide = UG_NS<GridImp::dimension>::Corners_Of_Side(center_, neighborCount_);

  selfLocal_.setNumberOfCorners(numCornersOfSide);

  for (int i=0; i<numCornersOfSide; i++)
  {
    // get number of corner in UG's numbering system
    int cornerIdx = UG_NS<GridImp::dimension>::Corner_Of_Side(center_, neighborCount_, i);

    // we need a temporary to be filled
    FieldVector<UGCtype, dim> tmp;

    // get the corners local coordinates
    UG_NS<GridImp::dimension>::getCornerLocal(center_,cornerIdx,tmp);

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
inline const typename UGGridIntersectionIterator<GridImp>::LocalGeometry&
UGGridIntersectionIterator<GridImp>::
intersectionNeighborLocal() const
{
  typename TargetType<0,GridImp::dimensionworld>::T *other,*self;

  // if we have a neighbor on this level, then return it
  if (UG_NS<GridImp::dimension>::NbElem(center_, neighborCount_)!=NULL)
  {
    other = UG_NS<GridImp::dimension>::NbElem(center_, neighborCount_);
    self = center_;
  }
  else
  {
    // now go down the stack of copies to find a lower level leaf neighbor
    typename TargetType<0,GridImp::dimensionworld>::T* father_ = UG_NS<GridImp::dimensionworld>::EFather(center_);
    while (father_!=0)
    {
      if (!UG_NS<GridImp::dimensionworld>::hasCopy(father_))
        DUNE_THROW(GridError,"no neighbor found");
      if (UG_NS<GridImp::dimension>::NbElem(father_, neighborCount_)!=NULL)             // check existence of neighbor
        if (UG_NS<GridImp::dimension>::isLeaf(UG_NS<GridImp::dimension>::NbElem(father_, neighborCount_)))
        {
          other = UG_NS<GridImp::dimension>::NbElem(father_, neighborCount_);
          self = father_;
          break;
        }
      // try father
      father_ = UG_NS<GridImp::dimensionworld>::EFather(father_);
    }
    if (father_==0)
      DUNE_THROW(GridError,"no neighbor found");
  }

  // we have other and self
  const int nSides = UG_NS<GridImp::dimensionworld>::Sides_Of_Elem(other);
  int otherCount;
  for (otherCount=0; otherCount<nSides; otherCount++)
    if (UG_NS<GridImp::dimensionworld>::NbElem(other,otherCount) == self)
      break;

  // now otherCount is the number of the face in other (in UG's numbering system)
  // go on and get the local coordinates
  int numCornersOfSide = UG_NS<GridImp::dimension>::Corners_Of_Side(other,otherCount);
  neighLocal_.setNumberOfCorners(numCornersOfSide);

  for (int i=0; i<numCornersOfSide; i++)
  {
    // get number of corner in UG's numbering system
    int cornerIdx = UG_NS<GridImp::dimension>::Corner_Of_Side(other,otherCount,i);

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
renumberFaceUGToDune (int nSides, int i)  const
{
  if (nSides==6) {   // Hexahedron
    // Dune numbers the faces of a hexahedron differently than UG.
    // The following two lines do the transformation
    const int renumbering[6] = {4, 2, 1, 3, 0, 5};
    return renumbering[i];

  } else if (nSides==4 && GridImp::dimension==3) {   // Tetrahedron
    // Dune numbers the faces of a tetrahedron differently than UG.
    // The following two lines do the transformation
    const int renumbering[4] = {3, 0, 1, 2};
    return renumbering[i];

  } else if (nSides==4 && GridImp::dimension==2) {   // Quadrilateral
    // Dune numbers the faces of a quadrilateral differently than UG.
    // The following two lines do the transformation
    const int renumbering[4] = {2, 1, 3, 0};
    return renumbering[i];

  } else if (nSides==3) {   // Triangle
    // Dune numbers the faces of a triangle differently from UG.
    // The following two lines do the transformation
    const int renumbering[3] = {2, 0, 1};
    return renumbering[i];

  } else
    return i;
}


template< class GridImp>
inline int UGGridIntersectionIterator<GridImp>::
numberInSelf ()  const
{
  return renumberFaceUGToDune(UG_NS<dimworld>::Sides_Of_Elem(center_),neighborCount_);
}

template< class GridImp>
inline int UGGridIntersectionIterator<GridImp>::
numberInNeighbor () const
{
  typename TargetType<0,GridImp::dimensionworld>::T *other,*self;

  // if we have a neighbor on this level, then return it
  if (UG_NS<GridImp::dimension>::NbElem(center_, neighborCount_)!=NULL)
  {
    other = UG_NS<GridImp::dimension>::NbElem(center_, neighborCount_);
    self = center_;
  }
  else
  {
    // now go down the stack of copies to find a lower level leaf neighbor
    typename TargetType<0,GridImp::dimensionworld>::T* father_ = UG_NS<GridImp::dimensionworld>::EFather(center_);
    while (father_!=0)
    {
      if (!UG_NS<GridImp::dimensionworld>::hasCopy(father_))
        DUNE_THROW(GridError,"no neighbor found");
      if (UG_NS<GridImp::dimension>::NbElem(father_, neighborCount_)!=NULL)             // check existence of neighbor
        if (UG_NS<GridImp::dimension>::isLeaf(UG_NS<GridImp::dimension>::NbElem(father_, neighborCount_)))
        {
          other = UG_NS<GridImp::dimension>::NbElem(father_, neighborCount_);
          self = father_;
          break;
        }
      // try father
      father_ = UG_NS<GridImp::dimensionworld>::EFather(father_);
    }
    if (father_==0)
      DUNE_THROW(GridError,"no neighbor found");
  }

  // we have other and self
  const int nSides = UG_NS<GridImp::dimensionworld>::Sides_Of_Elem(other);
  int i;
  for (i=0; i<nSides; i++)
    if (UG_NS<GridImp::dimensionworld>::NbElem(other,i) == self)
      break;

  // now we have to renumber the side i
  return renumberFaceUGToDune(nSides,i);
}
