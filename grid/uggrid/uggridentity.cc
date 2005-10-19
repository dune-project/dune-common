// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//*************************************************************************
//
//  --UGGridEntity
//  --Entity
//
//*************************************************************************


//
//  codim > 0
//
//*********************************************************************


template<int codim, int dim, class GridImp>
inline UGGridEntity < codim, dim ,GridImp>::
UGGridEntity(int level) :
  geo_(),
  level_ (level)
{}

template<int codim, int dim, class GridImp>
inline void UGGridEntity < codim, dim ,GridImp>::
setToTarget(typename TargetType<codim,dim>::T* target)
{
  target_ = target;
  geo_.setToTarget(target);
}

template<int codim, int dim, class GridImp>
inline void UGGridEntity < codim, dim ,GridImp>::
setToTarget(typename TargetType<codim,dim>::T* target, int level)
{
  target_ = target;
  level_  = level;
  geo_.setToTarget(target);
}

template<int codim, int dim, class GridImp>
inline int UGGridEntity < codim, dim ,GridImp>::
level() const
{
  return level_;
}

template< int codim, int dim, class GridImp>
inline const typename UGGridEntity<codim,dim,GridImp>::Geometry&
UGGridEntity < codim, dim ,GridImp>::
geometry() const
{
  return geo_;
}




//*****************************************************************8
// count
template <int codim, int dim, class GridImp>
template <int cc>
inline int UGGridEntity<codim,dim,GridImp>::count () const
{
  DUNE_THROW(GridError, "UGGridEntity<" << codim << ", " << dim
                                        << ">::count() not implemented yet!");
  return -1;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//     Specializations for codim == 0                                     //
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

template< int dim, class GridImp>
inline AdaptationState UGGridEntity < 0, dim ,GridImp >::state() const
{
  if (UG_NS<dim>::ReadCW(target_, UG_NS<dim>::NEWEL_CE))
    return REFINED;

  if (UG_NS<dim>::ReadCW(target_, UG_NS<dim>::COARSEN_CE))
    return COARSEN;

  return NONE;
}


//*****************************************************************8
// count
template <int dim, class GridImp>
template <int cc>
inline int UGGridEntity<0,dim,GridImp>::count() const
{
  if (dim==3) {

    switch (cc) {
    case 0 :
      return 1;
    case 1 :
      return UG_NS<dim>::Sides_Of_Elem(target_);
    case 2 :
      return UG_NS<dim>::Edges_Of_Elem(target_);
    case 3 :
      return UG_NS<dim>::Corners_Of_Elem(target_);
    }

  } else {

    switch (cc) {
    case 0 :
      return 1;
    case 1 :
      return UG_NS<dim>::Edges_Of_Elem(target_);
    case 2 :
      return UG_NS<dim>::Corners_Of_Elem(target_);
    }

  }
  DUNE_THROW(GridError, "You can't call UGGridEntity<0,dim>::count<codim> "
             << "with dim==" << dim << " and codim==" << cc << "!");
}

template <int dim, class GridImp>
inline int UGGridEntity<0, dim, GridImp>::renumberVertex(int i) const {

  if (geometry().type()==cube) {

    // Dune numbers the vertices of a hexahedron and quadrilaterals differently than UG.
    // The following two lines do the transformation
    // The renumbering scheme is {0,1,3,2} for quadrilaterals, therefore, the
    // following code works for 2d and 3d.
    // It also works in both directions UG->DUNE, DUNE->UG !
    const int renumbering[8] = {0, 1, 3, 2, 4, 5, 7, 6};
    return renumbering[i];

  } else
    return i;

}
template <int dim, class GridImp>
inline int UGGridEntity<0, dim, GridImp>::renumberFace(int i) const {

  if (geometry().type()==cube) {

    // Dune numbers the vertices of a hexahedron and quadrilaterals differently than UG.
    // The following two lines do the transformation
    // The renumbering scheme is {0,1,3,2} for quadrilaterals, therefore, the
    // following code works for 2d and 3d.
    const int renumbering[6] = {4, 2, 1, 3, 0, 5};
    return renumbering[i];

  }
  if (geometry().type()==simplex) {

    // Dune numbers the vertices of a hexahedron and quadrilaterals differently than UG.
    // The following two lines do the transformation
    // The renumbering scheme is {0,1,3,2} for quadrilaterals, therefore, the
    // following code works for 2d and 3d.
    const int renumbering[4] = {1, 2, 0, 3};
    return renumbering[i];
  }
  return i;
}
template <int dim, class GridImp>
template <int cc>
inline int UGGridEntity<0, dim, GridImp>::subIndex(int i) const
{
  assert(i>=0 && i<count<cc>());

  if (cc==dim)
    return UG_NS<dim>::levelIndex(UG_NS<dim>::Corner(target_,renumberVertex(i)));
  if (cc==0)
    return UG_NS<dim>::levelIndex(target_);
  if (cc==dim-1)
  {
    int a=ReferenceElements<double,dim>::general(geometry().type()).subEntity(i,dim-1,0,dim);
    int b=ReferenceElements<double,dim>::general(geometry().type()).subEntity(i,dim-1,1,dim);
    return UG_NS<dim>::levelIndex(UG_NS<dim>::GetEdge(UG_NS<dim>::Corner(target_,renumberVertex(a)),UG_NS<dim>::Corner(target_,renumberVertex(b))));
  }
  if (cc==1)
  {
    return UG_NS<dim>::levelIndex(UG_NS<dim>::SideVector(target_,renumberFace(i)));
  }

  DUNE_THROW(GridError, "UGGrid<" << dim << "," << dim << ">::subIndex isn't implemented for cc==" << cc );
}

template <int dim, class GridImp>
template <int cc>
inline int UGGridEntity<0, dim, GridImp>::subLeafIndex(int i) const
{
  assert(i>=0 && i<count<cc>());

  if (cc==dim)
    return UG_NS<dim>::leafIndex(UG_NS<dim>::Corner(target_,renumberVertex(i)));
  if (cc==0)
    return UG_NS<dim>::leafIndex(target_);
  if (cc==dim-1)
  {
    int a=ReferenceElements<double,dim>::general(geometry().type()).subEntity(i,dim-1,0,dim);
    int b=ReferenceElements<double,dim>::general(geometry().type()).subEntity(i,dim-1,1,dim);
    return UG_NS<dim>::leafIndex(UG_NS<dim>::GetEdge(UG_NS<dim>::Corner(target_,renumberVertex(a)),UG_NS<dim>::Corner(target_,renumberVertex(b))));
  }
  if (cc==1)
  {
    return UG_NS<dim>::leafIndex(UG_NS<dim>::SideVector(target_,renumberFace(i)));
  }

  DUNE_THROW(GridError, "UGGrid<" << dim << "," << dim << ">::subLeafIndex isn't implemented for cc==" << cc );
}

template <int dim, class GridImp>
template <int cc>
inline unsigned int UGGridEntity<0, dim, GridImp>::subGlobalId(int i) const
{
  assert(i>=0 && i<count<cc>());

  if (cc==0)
  {
#ifdef ModelP
    return target_->ddd.gid;
#else
    return UG_NS<dim>::id(target_);
#endif
  }
  if (cc==dim)
  {
#ifdef ModelP
    return UG_NS<dim>::Corner(target_,renumberVertex(i))->ddd.gid;
#else
    return UG_NS<dim>::id(UG_NS<dim>::Corner(target_,renumberVertex(i)));
#endif
  }

  DUNE_THROW(GridError, "UGGrid<" << dim << "," << dim << ">::subGlobalId isn't implemented for cc==" << cc );
  return 0;
}

template <int dim, class GridImp>
template <int cc>
inline unsigned int UGGridEntity<0, dim, GridImp>::subLocalId(int i) const
{
  assert(i>=0 && i<count<cc>());

  if (cc==dim)
    return UG_NS<dim>::id(UG_NS<dim>::Corner(target_,renumberVertex(i)));
  else if (cc==0)
    return UG_NS<dim>::id(target_);
  else
    DUNE_THROW(GridError, "UGGrid<" << dim << "," << dim << ">::subLocalId isn't implemented for cc==" << cc );
}


template <int dim, class GridImp>
template <int cc>
inline typename GridImp::template Codim<cc>::EntityPointer
UGGridEntity<0,dim,GridImp>::entity ( int i ) const
{
  assert(i>=0 && i<count<cc>());

  if (cc==dim) {
    typename TargetType<cc,dim>::T* subEntity = UGGridSubEntityFactory<cc,dim>::get(target_,renumberVertex(i));
    return UGGridLevelIterator<cc,All_Partition,GridImp>(subEntity, level_);
  } else if (cc==0) {
    // The following cast is here to make the code compile for all cc.
    // When it gets actually called, cc==0, and the cast is nonexisting.
    typename TargetType<cc,dim>::T* myself = (typename TargetType<cc,dim>::T*)target_;
    return UGGridLevelIterator<cc,All_Partition,GridImp>(myself, level_);
  } else
    DUNE_THROW(GridError, "UGGrid<" << dim << "," << dim << ">::entity isn't implemented for cc==" << cc );
}

template<int dim, class GridImp>
inline UGGridEntity < 0, dim ,GridImp >::
UGGridEntity(int level) : level_ (level)
{}

template<int dim, class GridImp>
inline void UGGridEntity < 0, dim ,GridImp >::
setToTarget(typename TargetType<0,dim>::T* target, int level)
{
  target_ = target;
  level_  = level;
  geo_.setToTarget(target);
}

template<int dim, class GridImp>
inline void UGGridEntity < 0, dim ,GridImp >::
setToTarget(typename TargetType<0,dim>::T* target)
{
  target_ = target;
  geo_.setToTarget(target);
}

template<int dim, class GridImp>
inline UGGridIntersectionIterator<GridImp>
UGGridEntity < 0, dim ,GridImp >::ibegin() const
{
  return UGGridIntersectionIterator<GridImp>(target_, 0, level());
}


template< int dim, class GridImp>
inline UGGridIntersectionIterator<GridImp>
UGGridEntity < 0, dim ,GridImp >::iend() const
{
  return UGGridIntersectionIterator<GridImp>(target_, -1, -1);
}

template<int dim, class GridImp>
inline UGGridHierarchicIterator<GridImp>
UGGridEntity < 0, dim ,GridImp >::hbegin(int maxlevel) const
{
  UGGridHierarchicIterator<GridImp> it(maxlevel);

  if (level()<=maxlevel) {

    // Put myself on the stack
    typename UGGridHierarchicIterator<GridImp>::StackEntry se;
    se.element = target_;
    se.level   = level();
    it.elemStack.push(se);

    // Set intersection iterator to myself
    it.virtualEntity_.setToTarget(target_, level());

    /** \todo Directly put all sons onto the stack */
    it.increment();

  } else {
    it.virtualEntity_.setToTarget(0);
  }

  return it;
}


template< int dim, class GridImp>
inline UGGridHierarchicIterator<GridImp>
UGGridEntity < 0, dim ,GridImp >::hend(int maxlevel) const
{
  return UGGridHierarchicIterator<GridImp>(maxlevel);
}


template<int dim, class GridImp>
inline int UGGridEntity < 0, dim ,GridImp >::
level() const
{
  return level_;
}

template< int dim, class GridImp>
inline const typename UGGridEntity<0,dim,GridImp>::Geometry&
UGGridEntity < 0, dim ,GridImp >::
geometry() const
{
  return geo_;
}


template<int dim, class GridImp>
inline UGGridLevelIterator<0,All_Partition,GridImp>
UGGridEntity < 0, dim, GridImp>::father() const
{
  UGGridLevelIterator<0,All_Partition,GridImp> it(level()-1);
  it.setToTarget(UG_NS<dim>::EFather(target_));
  return it;
}

template<int dim, class GridImp>
inline const typename UGGridEntity<0,dim,GridImp>::Geometry& UGGridEntity < 0, dim, GridImp>::geometryInFather () const
{
  // we need to have a father element
  typename TargetType<0,dim>::T* fatherelement = UG_NS<dim>::EFather(target_);
  assert(fatherelement!=0);
  fathergeo_.coordmode(); // put in the new mode
  fathergeo_.setToTarget(fatherelement);

  // The task is to find out the positions of the vertices of this element
  // in the local coordinate system of the father.

  // loop through all corner nodes
  for (int i=0; i<UG_NS<dim>::Corners_Of_Elem(target_); i++)
  {
    // get corner node pointer
    typename TargetType<dim,dim>::T* fnode = UG_NS<dim>::Corner(target_,i);

    // case I : new node on that level and father is the vertex father
    if (UG_NS<dim>::NFather(fnode)==fatherelement)
    {
      // yes, then get the position in local coordinates
      FieldVector<UGCtype, dim> tmp;
      UG_NS<dim>::PositionInFather(fnode,tmp);

      // and poke them into the Geometry
      fathergeo_.setCoords(i,tmp);

      // continue with next corner
      continue;
    }

    // case II : this is a copy of a coarse grid node
    typename TargetType<dim,dim>::T* cnode = UG_NS<dim>::NodeNodeFather(fnode);
    if (cnode!=0)
    {
      // identify node in the father element
      int cornerIdx = -1;
      for (int j=0; j<UG_NS<dim>::Corners_Of_Elem(fatherelement); j++)
        if (UG_NS<dim>::Corner(fatherelement,j)==cnode)
        {
          cornerIdx = j;
          break;
        }

      // if node cannot be found something is wrong
      if (cornerIdx==-1)
        DUNE_THROW(GridError,"geometryInFather not implemented");

      // we need a temporary to be filled
      FieldVector<UGCtype, dim> tmp;

      // get the corners local coordinates
      UG_NS<dim>::getCornerLocal(fatherelement,cornerIdx,tmp);

      // and poke them into the Geometry
      fathergeo_.setCoords(i,tmp);

      // continue with next corner
      continue;
    }

    // case III : general case uses global to local
    // there could be a case IV with nodes on edges but we ignore that for now
    FieldVector<UGCtype,dim> global;
    UG_NS<dim>::NodePositionGlobal(fnode,global);
    UGCtype global_c[dim];
    for (int k=0; k<dim; k++) global_c[k] = global[k];

    // fill array of pointers to corners of father in global coordinates
    UGCtype* cornerCoords[dim*dim];
    UG_NS<dim>::Corner_Coordinates(fatherelement,cornerCoords);

    // Actually do the computation
    /** \todo Why is this const_cast necessary? */
    UGCtype localCoords[dim];
    if (UG_NS<dim>::GlobalToLocal(UG_NS<dim>::Corners_Of_Elem(fatherelement),
                                  const_cast<const double**>(cornerCoords), global_c, localCoords)!=0)
      DUNE_THROW(GridError,"global to local failed in geometryInFather");

    // and poke them into the Geometry
    FieldVector<UGCtype,dim> tmp;
    for (int k=0; k<dim; k++) tmp[k] = localCoords[k];
    fathergeo_.setCoords(i,tmp);
  }

  return fathergeo_;
}
