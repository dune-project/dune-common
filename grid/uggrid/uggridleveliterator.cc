// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//*******************************************************
//
// --UGGridLevelIterator
// --LevelIterator
//
//*******************************************************


// Make LevelIterator with point to element from previous iterations
template<int codim, int dim, int dimworld>
inline UGGridLevelIterator<codim,dim,dimworld >::
UGGridLevelIterator(int travLevel) : level_ (travLevel) ,  virtualEntity_(0)
{
  target_ = NULL;

  virtualEntity_.setToTarget(NULL);
}

// Make LevelIterator with point to element from previous iterations
template<int codim, int dim, int dimworld>
inline UGGridLevelIterator<codim,dim,dimworld >::UGGridLevelIterator(UGGrid<dim,dimworld> &grid, int travLevel) :
  /*grid_(grid), */ level_ (travLevel) ,  virtualEntity_(0)
{
  target_ = NULL;

  virtualEntity_.setToTarget(NULL);
}


template<>
inline UGGridLevelIterator < 3,3,3 >&
UGGridLevelIterator < 3,3,3 >::operator++()
{

  //     UG3d::vertex* myvertex = NULL;
  //     do {
  target_ = target_->succ;
  //         if (!target_)
  //             break;
  //         myvertex = target_->myvertex;
  // #define OBJT(p) ReadCW(p, UG3d::OBJ_CE)
  //     } while (OBJT(myvertex)!= UG3d::IVOBJ);
  // #undef OBJT

  setToTarget(target_);
  virtualEntity_.elNum_++;

  return (*this);
}

template<>
inline UGGridLevelIterator < 0,3,3 >&
UGGridLevelIterator < 0,3,3 >::operator++()
{
  setToTarget(target_->ge.succ);
  virtualEntity_.elNum_++;
  return (*this);
}

// gehe zum i Schritte weiter , wie auch immer
template<int codim, int dim, int dimworld>
inline UGGridLevelIterator < codim,dim,dimworld >&
UGGridLevelIterator < codim,dim,dimworld >::operator++()
{
  printf("LevelIterator< %d, %d, %d >::operator++  Unimplemented case!\n",
         codim, dim, dimworld);
  return (*this);
}


template<int codim, int dim, int dimworld>
inline bool UGGridLevelIterator<codim,dim,dimworld >::
operator ==(const UGGridLevelIterator<codim,dim,dimworld > &I) const
{
  return target_==I.target_;
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
