// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//*******************************************************
//
// --UGGridLevelIterator
// --LevelIterator
//
//*******************************************************


// Make LevelIterator with point to element from previous iterations
template<int codim, PartitionIteratorType pitype, class GridImp>
inline UGGridLevelIterator<codim,pitype,GridImp>::
UGGridLevelIterator(int travLevel) : virtualEntity_(0), level_ (travLevel)
{
  target_ = NULL;

  virtualEntity_.setToTarget(NULL);
}


#if 0

#ifdef _3
template<>
inline UGGridLevelIterator < 3,3,3, All_Partition >&
UGGridLevelIterator < 3,3,3,All_Partition >::operator++()
{

  target_ = target_->succ;

  setToTarget(target_);
  virtualEntity_.elNum_++;

  return (*this);
}

template<>
inline UGGridLevelIterator < 0,3,3, All_Partition >&
UGGridLevelIterator < 0,3,3, All_Partition >::operator++()
{
  setToTarget(target_->ge.succ);
  virtualEntity_.elNum_++;
  return (*this);
}
#endif

#ifdef _2
template<class GridImp>
inline UGGridLevelIterator < 2,All_Partition,GridImp>&
UGGridLevelIterator < 2,All_Partition,GridImp>::operator++()
{

  target_ = target_->succ;

  setToTarget(target_);
  virtualEntity_.elNum_++;

  return (*this);
}

template<>
inline UGGridLevelIterator < 0,2,2,All_Partition >&
UGGridLevelIterator < 0,2,2,All_Partition >::operator++()
{
  setToTarget(target_->ge.succ);
  virtualEntity_.elNum_++;
  return (*this);
}

template<>
inline UGGridLevelIterator < 2,2,2,Interior_Partition >&
UGGridLevelIterator < 2,2,2,Interior_Partition>::operator++()
{

  target_ = target_->succ;

  setToTarget(target_);
  virtualEntity_.elNum_++;

  return (*this);
}

template<>
inline UGGridLevelIterator < 0,2,2,Interior_Partition >&
UGGridLevelIterator < 0,2,2,Interior_Partition >::operator++()
{
  setToTarget(target_->ge.succ);
  virtualEntity_.elNum_++;
  return (*this);
}
#endif

// gehe zum i Schritte weiter , wie auch immer
template<int codim, PartitionIteratorType pitype, class GridImp>
inline UGGridLevelIterator <codim,pitype,GridImp>&
UGGridLevelIterator < codim,pitype,GridImp >::operator++()
{
  DUNE_THROW(NotImplemented, "UGGridLevelIterator::operator++  Unimplemented case!");
  return (*this);
}
#endif

template<int codim, PartitionIteratorType pitype, class GridImp>
inline void UGGridLevelIterator < codim,pitype,GridImp >::increment()
{
  setToTarget(UG_NS<GridImp::dimension>::succ(target_));
}



template<int codim, PartitionIteratorType pitype, class GridImp>
inline bool UGGridLevelIterator<codim,pitype,GridImp >::
equals(const UGGridLevelIterator<codim,pitype,GridImp > &I) const
{
  return target_==I.target_;
}

#if 0
template<int codim, PartitionIteratorType pitype, class GridImp>
inline bool UGGridLevelIterator < codim,pitype,GridImp >::
operator !=(const UGGridLevelIterator< codim,pitype,GridImp > & I) const
{
  return !((*this)==I);
}
#endif

template<int codim, PartitionIteratorType pitype, class GridImp>
inline typename GridImp::template codim<codim>::Entity&
UGGridLevelIterator< codim,pitype,GridImp >::dereference() const
{
  return virtualEntity_;
}

#if 0
template<int codim, PartitionIteratorType pitype, class GridImp>
inline UGGridEntity< codim,GridImp::dimension,GridImp>*
UGGridLevelIterator< codim,pitype,GridImp >::operator ->()
{
  return &virtualEntity_;
}
#endif

template<int codim, PartitionIteratorType pitype, class GridImp>
inline int UGGridLevelIterator<codim,pitype,GridImp >::level()
{
  return level_;
}
