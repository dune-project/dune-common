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
inline int UGGridEntity < codim, dim ,dimworld >::
level()
{
  return level_;
}


template<int codim, int dim, int dimworld>
inline int UGGridEntity < codim, dim ,dimworld >::
index()
{
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

template< int codim, int dim, int dimworld>
inline UGGridElement<dim-codim,dimworld>&
UGGridEntity < codim, dim ,dimworld >::
geometry()
{
  return geo_;
}

#ifdef _3
//template<int codim, int dim, int dimworld>
template<>
inline UGGridLevelIterator<0,3,3>
UGGridEntity < 0, 3 ,3>::father()
{
  // This currently only works for elements
  //assert(codim==0);

  UGGridLevelIterator<0,3,3> it(level()-1);
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

template< int dim, int dimworld>
inline UGGridElement<dim,dimworld>&
UGGridEntity < 0, dim ,dimworld >::
geometry()
{
  return geo_;
}

//*****************************************************************8
// count
/** \todo So far only works in 3d */
template <int codim, int dim, int dimworld> template <int cc>
inline int UGGridEntity<codim,dim,dimworld>::count ()
{
  // #define TAG(p) ReadCW(p, UG3d::TAG_CE)
  // #define SIDES_OF_ELEM(p) (UG3d::element_descriptors[TAG(p)]->sides_of_elem)
  // #define EDGES_OF_ELEM(p) (UG3d::element_descriptors[TAG(p)]->edges_of_elem)
  // #define CORNERS_OF_ELEM(p)(UG3d::element_descriptors[TAG(p)]->corners_of_elem)

  if (dim==3) {

    switch (cc) {
    case 0 :
      return 1;
    case 1 :

      return UG<3>::Sides_Of_Elem(target_);
    case 2 :
      return UG<3>::Edges_Of_Elem(target_);
    case 3 :
      return UG<3>::Corners_Of_Elem(target_);
    }

  } else {

    switch (cc) {
    case 0 :
      return 1;
    case 1 :
      return UG<2>::Edges_Of_Elem(target_);
    case 2 :
      return UG<2>::Corners_Of_Elem(target_);
    }

  }
  return -1;
  // #undef SIDES_OF_ELEM
  // #undef EDGES_OF_ELEM
  // #undef CORNERS_OF_ELEM
  // #undef TAG
}

template <int codim, int dim, int dimworld>
template <int cc>
inline int UGGridEntity<codim, dim, dimworld>::subIndex(int i)
{
#if 0

#define TAG(p) ReadCW(p, UG3d::TAG_CE)
#define CORNER(p,i) ((UG3d::node *) (p)->ge.refs[UG3d::n_offset[TAG(p)]+(i)])
  UG3d::node* node = CORNER(target_,i);
#undef CORNER
#undef TAG
  UG3d::vertex* vertex = node->myvertex;
  return vertex->iv.id;

#else

  typename TargetType<dim,dim>::T* node = CORNER(target_,i);
  //UG3d::vertex* vertex = node->myvertex;
  return node->myvertex->iv.id;

#endif

}



// default is faces
template <int codim, int dim, int dimworld> template <int cc>
inline UGGridLevelIterator<cc,dim,dimworld>
UGGridEntity<codim,dim,dimworld>::entity ( int i )
{
  std::cout << "entity not implemented yet!\n";
  UGGridLevelIterator<cc,dim,dimworld> tmp (level_);
  return tmp;
}


template<int codim, int dim, int dimworld>
inline UGGridIntersectionIterator<dim,dimworld>
UGGridEntity < codim, dim ,dimworld >::ibegin()
{
  UGGridIntersectionIterator<dim,dimworld> it;

  if (codim==0) {
    it.setToTarget(target_, 0);
  } else
    printf("UGGridEntity <%d, %d, %d>::ibegin() not implemented\n", codim, dim, dimworld);

  return it;
}


template< int codim, int dim, int dimworld>
inline UGGridIntersectionIterator<dim,dimworld>
UGGridEntity < codim, dim ,dimworld >::iend()
{
  UGGridIntersectionIterator<dim,dimworld> it;

  if (codim==0) {
    it.setToTarget(NULL, -1);
  } else
    printf("UGGridEntity <%d, %d, %d>::iend() not implemented\n", codim, dim, dimworld);

  return it;
}

template<>
template< int dim, int dimworld>
inline AdaptationState UGGridEntity < 0, dim ,dimworld >::state() const
{
  cerr << "UGGridEntity::state() not yet implemented!\n";

}

template <int dim, int dimworld>
template <int cc>
inline int UGGridEntity<0, dim, dimworld>::subIndex(int i)
{
#if 0

#define TAG(p) ReadCW(p, UG3d::TAG_CE)
#define CORNER(p,i) ((UG3d::node *) (p)->ge.refs[UG3d::n_offset[TAG(p)]+(i)])
  UG3d::node* node = CORNER(target_,i);
#undef CORNER
#undef TAG
  UG3d::vertex* vertex = node->myvertex;
  return vertex->iv.id;

#else

  typename TargetType<dim,dim>::T* node = CORNER(target_,i);
  return node->myvertex->iv.id;

#endif
}

template<int dim, int dimworld>
inline UGGridEntity < 0, dim ,dimworld >::
UGGridEntity(int level) :
  geo_(false),
  level_ (level)
{}

template<int dim, int dimworld>
inline void UGGridEntity < 0, dim ,dimworld >::
setToTarget(typename TargetType<0,dim>::T* target)
{
  target_ = target;
  geo_.setToTarget(target);
}
