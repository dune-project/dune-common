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

#ifndef UGGRID_WITH_INDEX_SETS
template<int codim, int dim, class GridImp>
inline int UGGridEntity < codim, dim ,GridImp>::
index() const
{
  return UG_NS<dim>::index(target_);
}
#endif

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

#ifndef UGGRID_WITH_INDEX_SETS
template <int codim, int dim, class GridImp>
template <int cc>
inline int UGGridEntity<codim, dim, GridImp>::subIndex(int i) const
{
  DUNE_THROW(GridError, "UGGridEntity<" << codim << ", " << dim
                                        << ">::subIndex(int i) not implemented yet!");
  return 0;
}
#endif



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//     Specializations for codim == 0                                     //
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

template< int dim, class GridImp>
inline AdaptationState UGGridEntity < 0, dim ,GridImp >::state() const
{
  std::cerr << "UGGridEntity::state() not yet implemented!\n";
  return NONE;
}

#ifndef UGGRID_WITH_INDEX_SETS
template<int dim, class GridImp>
inline int UGGridEntity < 0, dim ,GridImp>::
index() const
{
  return UG_NS<dim>::index(target_);
}
#endif

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
#ifdef _3
    case 1 :
      return UG_NS<3>::Sides_Of_Elem(target_);
    case 2 :
      return UG_NS<3>::Edges_Of_Elem(target_);
    case 3 :
      return UG_NS<3>::Corners_Of_Elem(target_);
#endif
    }

  } else {

    switch (cc) {
    case 0 :
      return 1;
#ifdef _2
    case 1 :
      return UG_NS<2>::Edges_Of_Elem(target_);
    case 2 :
      return UG_NS<2>::Corners_Of_Elem(target_);
#endif
    }

  }
  return -1;
}


template <int dim, class GridImp>
template <int cc>
inline int UGGridEntity<0, dim, GridImp>::subIndex(int i) const
{
  assert(i>=0 && i<count<cc>());

  if (cc!=dim)
    DUNE_THROW(GridError, "UGGrid::subIndex isn't implemented for cc != dim");

  if (geometry().type()==hexahedron) {
    // Dune numbers the vertices of a hexahedron differently than UG.
    // The following two lines do the transformation
    const int renumbering[8] = {0, 1, 3, 2, 4, 5, 7, 6};
    i = renumbering[i];
  }

  return UG_NS<dim>::index(UG_NS<dim>::Corner(target_,i));
}


template <int dim, class GridImp>
template <int cc>
inline typename GridImp::template codim<cc>::EntityPointer
UGGridEntity<0,dim,GridImp>::entity ( int i ) const
{
  assert(i>=0 && i<count<cc>());

  if (cc!=dim)
    DUNE_THROW(GridError, "UGGrid::subIndex isn't implemented for cc != dim");

  if (geometry().type()==hexahedron) {
    // Dune numbers the vertices of a hexahedron differently than UG.
    // The following two lines do the transformation
    const int renumbering[8] = {0, 1, 3, 2, 4, 5, 7, 6};
    i = renumbering[i];
  }

  typename TargetType<cc,dim>::T* subEntity = UGGridSubEntityFactory<cc,dim>::get(target_,i);
  UGGridLevelIterator<cc,All_Partition,GridImp> it (level_);
  it.setToTarget(subEntity, level_);
  return it;
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
  UGGridIntersectionIterator<GridImp> it;

  // Set intersection iterator to zeroth neighbor of target_
  it.setToTarget(target_, 0, level());

  return it;
}


template< int dim, class GridImp>
inline UGGridIntersectionIterator<GridImp>
UGGridEntity < 0, dim ,GridImp >::iend() const
{
  UGGridIntersectionIterator<GridImp> it;

  it.setToTarget(NULL, -1);

  return it;
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
