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
//*********************************************************************8


template<int codim, int dim, int dimworld>
inline UGGridEntity < codim, dim ,dimworld >::
UGGridEntity(int level) :
  level_ (level)
  , geo_ ( false )
{
#if 0
  travStack_ = NULL;
  makeDescription();
#endif
}

template<int codim, int dim, int dimworld>
inline void UGGridEntity < codim, dim ,dimworld >::
setToTarget(TargetType<codim,dim>::T* target)
{
  target_ = target;
  geo_.setToTarget(target);
  //printf("UGGridEntity::setToTarget  %d\n", target);
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

template< int codim, int dim, int dimworld>
inline UGGridElement<dim-codim,dimworld>&
UGGridEntity < codim, dim ,dimworld >::
geometry()
{
  return geo_;
}

#if 0
template<int codim, int dim, int dimworld>
inline Vec<dim,albertCtype>&
AlbertGridEntity < codim, dim ,dimworld >::local()
{
  return localFatherCoords_;
}
#endif

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

#if 0
//************************************
//
//  --AlbertGridEntity codim = 0
//  --0Entity codim = 0
//
//************************************
template< int dim, int dimworld>
inline bool AlbertGridEntity < 0, dim ,dimworld >::
mark( int refCount )
{
  if(! hasChildren() )
  {
    elInfo_->el->mark = refCount;
    return true;
  }

  std::cout << "Element not marked!\n";
  return false;
}

template< int dim, int dimworld>
inline bool AlbertGridEntity < 0, dim ,dimworld >::hasChildren()
{
  //return (elInfo_->el->child[0] != NULL);
  return ( level_ != grid_.maxlevel() );
}

template< int dim, int dimworld>
inline int AlbertGridEntity < 0, dim ,dimworld >::refDistance()
{
  return 0;
}

//***************************

template< int dim, int dimworld>
inline void AlbertGridEntity < 0, dim ,dimworld >::
makeDescription()
{
  elInfo_ = NULL;
  builtgeometry_ = false;

  // not fast , and also not needed
  //geo_.initGeom();
}

template<int dim, int dimworld>
inline void AlbertGridEntity < 0, dim ,dimworld >::
setTraverseStack(ALBERT TRAVERSE_STACK * travStack)
{
  travStack_ = travStack;
}

//#endif
template< int dim, int dimworld>
inline UGGridEntity < 0, dim ,dimworld >::
UGGridEntity(UGGrid<dim,dimworld> &grid, int level) : grid_(grid)
                                                      , level_ (level)
                                                      , vxEntity_ ( grid_ , NULL, 0, 0, 0, 0)
                                                      , geo_(false) , travStack_ (NULL) , elInfo_ (NULL)
                                                      , builtgeometry_ (false)
{}
#endif
//*****************************************************************8
// count
/** \todo So far only works in 3d */
template <int codim, int dim, int dimworld> template <int cc>
inline int UGGridEntity<codim,dim,dimworld>::count ()
{
#define TAG(p) ReadCW(p, UG3d::TAG_CE)
#define SIDES_OF_ELEM(p) (UG3d::element_descriptors[TAG(p)]->sides_of_elem)
#define EDGES_OF_ELEM(p) (UG3d::element_descriptors[TAG(p)]->edges_of_elem)
#define CORNERS_OF_ELEM(p)(UG3d::element_descriptors[TAG(p)]->corners_of_elem)

  if (dim==3) {

    switch (cc) {
    case 0 :
      return 1;
    case 1 :

      return SIDES_OF_ELEM(target_);
    case 2 :
      return EDGES_OF_ELEM(target_);
    case 3 :
      return CORNERS_OF_ELEM(target_);
    }

  } else {

    switch (cc) {
    case 0 :
      return 1;
    case 1 :
      return EDGES_OF_ELEM(target_);
    case 2 :
      return CORNERS_OF_ELEM(target_);
    }

  }
  return -1;
#undef SIDES_OF_ELEM
#undef EDGES_OF_ELEM
#undef CORNERS_OF_ELEM
#undef TAG
}

template <int codim, int dim, int dimworld>
template <int cc>
inline int UGGridEntity<codim, dim, dimworld>::subIndex(int i)
{
#define TAG(p) ReadCW(p, UG3d::TAG_CE)
#define CORNER(p,i) ((UG3d::node *) (p)->ge.refs[UG3d::n_offset[TAG(p)]+(i)])
  UG3d::node* node = CORNER(target_,i);
#undef CORNER
#undef TAG
  UG3d::vertex* vertex = node->myvertex;
  return vertex->iv.id;
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
    //         printf("element has %d neighbors:\n", SIDES_OF_ELEM(((UG3d::element*)target_)));
    //         for (i=0; i<4; i++)
    //             printf("Neighbor %d:  %d\n", i, NBELEM(((UG3d::element*)target_), i));
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

#if 0
template< int dim, int dimworld>
inline void UGGridEntity < 0, dim ,dimworld >::
ibegin(UGGridIntersectionIterator<dim,dimworld> &it)
{
  it.makeBegin( grid_ , level() , elInfo_ );
}

template< int dim, int dimworld>
inline void UGGridEntity < 0, dim ,dimworld >::
iend(AlbertGridIntersectionIterator<dim,dimworld> &it)
{
  it.makeEnd( grid_ , level() );
}
#endif
// end UGGridEntity
