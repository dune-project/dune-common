// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#if 0
//*******************************************************
//
// --AlbertGridLevelIterator
// --LevelIterator
//
//*******************************************************

//***********************************************************
//  some template specialization of goNextEntity
//***********************************************************
// default implementation, go next elInfo
template<int codim, int dim, int dimworld>
inline ALBERT EL_INFO * AlbertGridLevelIterator<codim,dim,dimworld >::
goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
{
  return goNextElInfo(stack,elinfo_old);
}

// specializations for codim 1, go next face
template <>
inline ALBERT EL_INFO * AlbertGridLevelIterator<1,2,2>::
goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
{
  return goNextFace(stack,elinfo_old);
}
template <>
inline ALBERT EL_INFO * AlbertGridLevelIterator<1,2,3>::
goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
{
  return goNextFace(stack,elinfo_old);
}
template <>
inline ALBERT EL_INFO * AlbertGridLevelIterator<1,3,3>::
goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
{
  return goNextFace(stack,elinfo_old);
}

// specialization for codim 2, if dim > 2, go next edge,
// only if dim == dimworld == 3
template <>
inline ALBERT EL_INFO * AlbertGridLevelIterator<2,3,3>::
goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
{
  return goNextEdge(stack,elinfo_old);
}

// specialization for codim == dim , go next vertex
template <>
inline ALBERT EL_INFO * AlbertGridLevelIterator<2,2,2>::
goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
{
  return goNextVertex(stack,elinfo_old);
}
template <>
inline ALBERT EL_INFO * AlbertGridLevelIterator<2,2,3>::
goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
{
  return goNextVertex(stack,elinfo_old);
}
template <>
inline ALBERT EL_INFO * AlbertGridLevelIterator<3,3,3>::
goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
{
  return goNextVertex(stack,elinfo_old);
}
// end specialization of goNextEntity
//***************************************

template<int codim, int dim, int dimworld>
inline void AlbertGridLevelIterator<codim,dim,dimworld >::
makeIterator()
{
  level_ = 0;
  vertex_ = 0;
  face_ = 0;
  edge_ = 0;
  vertexMarker_ = NULL;

  manageStack_.init();

  virtualEntity_.setTraverseStack(NULL);
  virtualEntity_.setElInfo(NULL,0,0,0,0);
}
#endif

// Make LevelIterator with point to element from previous iterations
template<int codim, int dim, int dimworld>
inline UGGridLevelIterator<codim,dim,dimworld >::UGGridLevelIterator(UGGrid<dim,dimworld> &grid, int travLevel) :
  grid_(grid), level_ (travLevel) ,  virtualEntity_(grid,0)
{
  target = NULL;
  //makeIterator();
}


// gehe zum i Schritte weiter , wie auch immer
template<int codim, int dim, int dimworld>
inline UGGridLevelIterator < codim,dim,dimworld >&
UGGridLevelIterator < codim,dim,dimworld >::operator++(int steps)
{
  switch (codim) {
  case 3 :
    target = ((UG3d::node*)target)->succ;
    break;
  case 0 :
    target = ((UG3d::element*)target)->ge.succ;
    break;
  default :
    printf("LevelIterator::operator++  Unimplemented case!\n");
  }

#if 0
  // die 0 ist wichtig, weil Face 0, heist hier jetzt Element
  ALBERT EL_INFO *elInfo =
    goNextEntity(manageStack_.getStack(), virtualEntity_.getElInfo());
  for(int i=1; i<= steps; i++)
    elInfo = goNextEntity(manageStack_.getStack(),virtualEntity_.getElInfo());

  virtualEntity_.setElInfo(elInfo,face_,edge_,vertex_);

#endif
  return (*this);
}


// gehe zum naechsten Element, wie auch immer
template<int codim, int dim, int dimworld>
inline UGGridLevelIterator < codim,dim,dimworld >&
UGGridLevelIterator < codim,dim,dimworld >::operator ++()
{
#if 0
  elNum_++;
  virtualEntity_.setElInfo(
    goNextEntity(manageStack_.getStack(),virtualEntity_.getElInfo()),
    elNum_,face_,edge_,vertex_);
#endif
  return (*this);
}

template<int codim, int dim, int dimworld>
inline bool UGGridLevelIterator<codim,dim,dimworld >::
operator ==(const UGGridLevelIterator<codim,dim,dimworld > &I) const
{
  return target==I.target;
}

template<int codim, int dim, int dimworld>
inline bool UGGridLevelIterator < codim,dim,dimworld >::
operator !=(const UGGridLevelIterator< codim,dim,dimworld > & I) const
{
  return !((*this)==I);
}

template<int codim, int dim, int dimworld>
inline UGGridEntity<codim, dim, dimworld> &
UGGridLevelIterator< codim,dim,dimworld >::operator *()
{
  return virtualEntity_;
}

template<int codim, int dim, int dimworld>
inline UGGridEntity< codim,dim,dimworld >*
UGGridLevelIterator< codim,dim,dimworld >::operator ->()
{
  return &virtualEntity_;
}

template<int codim, int dim, int dimworld>
inline int UGGridLevelIterator<codim,dim,dimworld >::level()
{
  return level_;
}
