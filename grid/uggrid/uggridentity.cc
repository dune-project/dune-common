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


template<int codim, int dim, int dimworld>
inline UGGridEntity < codim, dim ,dimworld >::
UGGridEntity(int level) :
  geo_(false),
  level_ (level)
{}

template<int codim, int dim, int dimworld>
inline void UGGridEntity < codim, dim ,dimworld >::
setToTarget(typename TargetType<codim,dim>::T* target)
{
  target_ = target;
  geo_.setToTarget(target);
}

template<int codim, int dim, int dimworld>
inline void UGGridEntity < codim, dim ,dimworld >::
setToTarget(typename TargetType<codim,dim>::T* target, int level)
{
  target_ = target;
  level_  = level;
  geo_.setToTarget(target);
}

template<int codim, int dim, int dimworld>
inline int UGGridEntity < codim, dim ,dimworld >::
level()
{
  return level_;
}


template<int codim, int dim, int dimworld>
inline int UGGridEntity < codim, dim ,dimworld >::
index()
{
  std::cerr << "UGGridEntity <" << codim << ", " << dim << ", " << dimworld
            << ">::index() not implemented!\n";
  return -1;
}

#ifdef _3
template<>
inline int UGGridEntity < 3, 3 ,3 >::
index()
{
  return target_->myvertex->iv.id;
}

template<>
inline int UGGridEntity < 0, 3 ,3 >::
index()
{
  return target_->ge.id;
}
#endif

#ifdef _2
template<>
inline int UGGridEntity < 2, 2 ,2 >::
index()
{
  return target_->myvertex->iv.id;
}

template<>
inline int UGGridEntity < 0, 2 ,2 >::
index()
{
  return target_->ge.id;
}
#endif

template< int codim, int dim, int dimworld>
inline UGGridElement<dim-codim,dimworld>&
UGGridEntity < codim, dim ,dimworld >::
geometry()
{
  return geo_;
}

#ifdef _3
template<>
inline UGGridLevelIterator<0,3,3, All_Partition>
UGGridEntity < 0, 3 ,3>::father()
{
  UGGridLevelIterator<0,3,3, All_Partition> it(level()-1);
#define TAG(p) ReadCW(p, UG3d::TAG_CE)
#define EFATHER(p) ((UG3d::ELEMENT *) (p)->ge.refs[UG3d::father_offset[TAG(p)]])
  UG3d::ELEMENT* fatherTarget = EFATHER(target_);
#undef TAG
#undef EFATHER

  it.setToTarget(fatherTarget);
  return it;
}
#endif

//************************************
//
//  --UGGridEntity codim = 0
//  --Entity codim = 0
//
//************************************
template< int dim, int dimworld>
inline bool UGGridEntity < 0, dim ,dimworld >::
mark( int refCount )
{
#ifdef _3
  if (!UG3d::EstimateHere(target_))
    return false;

  return UG3d::MarkForRefinement(target_,
                                 UG3d::RED,
                                 0);    // no user data
#else
  if (!UG2d::EstimateHere(target_))
    return false;

  return UG2d::MarkForRefinement(target_,
                                 UG2d::RED,   // red refinement rule
                                 0);    // no user data
#endif
}



//*****************************************************************8
// count
template <int codim, int dim, int dimworld> template <int cc>
inline int UGGridEntity<codim,dim,dimworld>::count ()
{
  DUNE_THROW(GridError, "UGGridEntity<" << codim << ", " << dim << ", " << dimworld
                                        << ">::count() not implemented yet!");
  return -1;
}

template <int codim, int dim, int dimworld>
template <int cc>
inline int UGGridEntity<codim, dim, dimworld>::subIndex(int i)
{
  DUNE_THROW(GridError, "UGGridEntity<" << codim << ", " << dim << ", " << dimworld
                                        << ">::subIndex(int i) not implemented yet!");
  return 0;
}


template <int codim, int dim, int dimworld> template <int cc>
inline UGGridLevelIterator<cc,dim,dimworld,All_Partition>
UGGridEntity<codim,dim,dimworld>::entity ( int i )
{
  DUNE_THROW(GridError, "UGGridEntity<" << codim << ", " << dim << ", " << dimworld
                                        << ">::entity(int i) not implemented yet!");
  UGGridLevelIterator<cc,dim,dimworld,All_Partition> tmp (level_);
  return tmp;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//     Specializations for codim == 0                                     //
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

template< int dim, int dimworld>
inline AdaptationState UGGridEntity < 0, dim ,dimworld >::state() const
{
  std::cerr << "UGGridEntity::state() not yet implemented!\n";
  return NONE;
}
//*****************************************************************8
// count
template <int dim, int dimworld>
template <int cc>
inline int UGGridEntity<0,dim,dimworld>::count ()
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


template <int dim, int dimworld>
template <int cc>
inline int UGGridEntity<0, dim, dimworld>::subIndex(int i)
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

  typename TargetType<dim,dim>::T* node = UG_NS<dimworld>::Corner(target_,i);
  return node->myvertex->iv.id;
}

template <int dim, int dimworld>
template <int cc>
inline UGGridLevelIterator<cc,dim,dimworld,All_Partition>
UGGridEntity<0,dim,dimworld>::entity ( int i )
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

  typename TargetType<dim,dim>::T* node = UG_NS<dimworld>::Corner(target_,i);
  UGGridLevelIterator<cc,dim,dimworld,All_Partition> it (level_);
  it.setToTarget(node, level_);
  return it;
}

template<int dim, int dimworld>
inline UGGridEntity < 0, dim ,dimworld >::
UGGridEntity(int level) :
  geo_(false),
  level_ (level)
{}

template<int dim, int dimworld>
inline void UGGridEntity < 0, dim ,dimworld >::
setToTarget(typename TargetType<0,dim>::T* target, int level)
{
  target_ = target;
  level_  = level;
  geo_.setToTarget(target);
}

template<int dim, int dimworld>
inline void UGGridEntity < 0, dim ,dimworld >::
setToTarget(typename TargetType<0,dim>::T* target)
{
  target_ = target;
  geo_.setToTarget(target);
}

template<int dim, int dimworld>
inline UGGridIntersectionIterator<dim,dimworld>
UGGridEntity < 0, dim ,dimworld >::ibegin()
{
  UGGridIntersectionIterator<dim,dimworld> it;
  //printf("ibegin:  target: %d\n", (int)target_);
  // Set intersection iterator to zeroth neighbor of target_
  it.setToTarget(target_, 0);

  return it;
}


template< int dim, int dimworld>
inline UGGridIntersectionIterator<dim,dimworld>
UGGridEntity < 0, dim ,dimworld >::iend()
{
  UGGridIntersectionIterator<dim,dimworld> it;

  it.setToTarget(NULL, -1);

  return it;
}

template<int dim, int dimworld>
inline UGGridHierarchicIterator<dim,dimworld>
UGGridEntity < 0, dim ,dimworld >::hbegin(int maxlevel)
{
  UGGridHierarchicIterator<dim,dimworld> it(level(), maxlevel);

  if (level()<maxlevel) {

    // The 30 is the macro MAX_SONS from ug/gm/gm.h
    UGElementType* sonList[30];
#ifdef _2
    UG2d::GetSons(target_,sonList);
#else
    UG3d::GetSons(target_,sonList);
#endif

#ifdef _2
#define NSONS(p) UG2d::ReadCW(p, UG2d::NSONS_CE)
#else
#define NSONS(p) UG3d::ReadCW(p, UG3d::NSONS_CE)
#endif
    // Load sons of entity into the iterator
    for (unsigned int i=0; i<NSONS(target_); i++) {
      typename UGGridHierarchicIterator<dim,dimworld>::StackEntry se;
      se.element = sonList[i];
      //printf("new element %d\n", se.element);
      se.level   = level()+1;
      it.elemStack.push(se);
    }
#undef NSONS

  }

  if (it.elemStack.empty()) {
    it.virtualEntity_.setToTarget(0);
  } else {
    // Set intersection iterator to first son
    it.virtualEntity_.setToTarget(it.elemStack.top().element, it.elemStack.top().level);
  }

  return it;
}


template< int dim, int dimworld>
inline UGGridHierarchicIterator<dim,dimworld>
UGGridEntity < 0, dim ,dimworld >::hend(int maxlevel)
{
  UGGridHierarchicIterator<dim,dimworld> it(level(), maxlevel);

  it.target_ = 0;

  return it;
}


template<int dim, int dimworld>
inline int UGGridEntity < 0, dim ,dimworld >::
level()
{
  return level_;
}

template< int dim, int dimworld>
inline UGGridElement<dim,dimworld>&
UGGridEntity < 0, dim ,dimworld >::
geometry()
{
  return geo_;
}
