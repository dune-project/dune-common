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

#if 0
// subIndex
template <int dim, int dimworld> template <int cc>
inline int AlbertGridEntity<0,dim,dimworld>::subIndex ( int i )
{
  return entity<cc>(i)->index();
}

#endif

template <int codim, int dim, int dimworld>
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

#if 0
template <> template <>
inline AlbertGridLevelIterator<2,3,3>
AlbertGridEntity<0,3,3>::entity<2> ( int i )
{
  //enum { cc = 2 };
  int num = grid_.indexOnLevel<2>(globalIndex() ,level_);
  if(i < 3)
  { // 0,1,2
    AlbertGridLevelIterator<2,3,3> tmp (grid_,elInfo_,num, 0,i,0);
    return tmp;
  }
  else
  { // 3,4,5
    AlbertGridLevelIterator<2,3,3> tmp (grid_,elInfo_,num, i-2,1,0);
    return tmp;
  }
}

// specialization for vertices
template <> template <>
inline AlbertGridLevelIterator<2,2,2>
AlbertGridEntity<0,2,2>::entity<2> ( int i )
{
  std::cout << "entity<2> ,2,2 !\n";
  // we are looking at vertices
  //enum { cc = dimension };
  enum { cc = 2 };
  AlbertGridLevelIterator<cc,2,2>
  tmp (grid_,elInfo_, grid_.indexOnLevel<cc>( elInfo_->el->dof[i][0],level_),
       0,0,i);
  return tmp;
}
// specialization for vertices
template <> template <>
inline AlbertGridLevelIterator<2,2,3>
AlbertGridEntity<0,2,3>::entity<2> ( int i )
{
  // we are looking at vertices
  //enum { cc = dimension };
  enum { cc = 2 };
  AlbertGridLevelIterator<cc,2,3>
  tmp (grid_,elInfo_, grid_.indexOnLevel<cc>( elInfo_->el->dof[i][0],level_),
       0,0,i);
  return tmp;
}
// specialization for vertices
template <> template <>
inline AlbertGridLevelIterator<3,3,3>
AlbertGridEntity<0,3,3>::entity<3> ( int i )
{
  // we are looking at vertices
  enum { cc = 3 };
  //enum { cc = dimension };
  AlbertGridLevelIterator<cc,3,3>
  tmp (grid_,elInfo_, grid_.indexOnLevel<cc>( elInfo_->el->dof[i][0],level_),
       0,0,i);
  return tmp;
}
//***************************

template<int dim, int dimworld>
inline ALBERT EL_INFO* AlbertGridEntity < 0 , dim ,dimworld >::
getElInfo() const
{
  return elInfo_;
}

template<int dim, int dimworld>
inline int AlbertGridEntity < 0, dim ,dimworld >::
level()
{
  return level_;
  //return elInfo_->level;
}

template<int dim, int dimworld>
inline int AlbertGridEntity < 0, dim ,dimworld >::
index()
{
  return grid_.template indexOnLevel<0>( globalIndex() , level_ );
}

template< int dim, int dimworld>
inline void AlbertGridEntity < 0, dim ,dimworld >::
setLevel(int actLevel)
{
  level_ = actLevel;
}

template< int dim, int dimworld>
inline void AlbertGridEntity < 0, dim ,dimworld >::
setElInfo(ALBERT EL_INFO * elInfo, int elNum,  unsigned char face,
          unsigned char edge, unsigned char vertex )
{
  // in this case the face, edge and vertex information is not used,
  // because we are in the element case
  elInfo_ = elInfo;
  builtgeometry_ = geo_.builtGeom(elInfo_,face,edge,vertex);
}

template< int dim, int dimworld>
inline AlbertGridElement<dim,dimworld>&
AlbertGridEntity < 0, dim ,dimworld >::geometry()
{
  assert(builtgeometry_ == true);
  return geo_;
}


template< int dim, int dimworld>
inline AlbertGridLevelIterator<0,dim,dimworld>
AlbertGridEntity < 0, dim ,dimworld >::father()
{
  ALBERT EL_INFO * fatherInfo = NULL;
  int fatherLevel = level_-1;
  // if this level > 0 return father = elInfoStack -1,
  // else return father = this
  if(level_ > 0)
    fatherInfo = &travStack_->elinfo_stack[travStack_->stack_used-1];
  else
  {
    fatherInfo = elInfo_;
    fatherLevel = 0;
  }

  // new LevelIterator with EL_INFO one level above
  AlbertGridLevelIterator <0,dim,dimworld>
  it(grid_,fatherInfo,grid_.template indexOnLevel<0>(fatherInfo->el->index,fatherLevel),0,0,0);
  return it;
}

template< int dim, int dimworld>
inline AlbertGridElement<dim,dim>&
AlbertGridEntity < 0, dim ,dimworld >::father_relative_local()
{
  //AlbertGridLevelIterator<0,dim,dimworld> daddy = father();
  AlbertGridElement<dim,dimworld> daddy = (*father()).geometry();

  fatherReLocal_.initGeom();
  // compute the local coordinates in father refelem
  for(int i=0; i<fatherReLocal_.corners(); i++)
    fatherReLocal_[i] = daddy.local(geometry()[i]);

  std::cout << "\nfather_realtive_local not tested yet! \n";
  return fatherReLocal_;
}
#endif

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
