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


template<int codim, PartitionIteratorType pitype, class GridImp>
inline typename GridImp::template codim<codim>::Entity&
UGGridLevelIterator< codim,pitype,GridImp >::dereference() const
{
  return virtualEntity_;
}


template<int codim, PartitionIteratorType pitype, class GridImp>
inline int UGGridLevelIterator<codim,pitype,GridImp >::level()
{
  return level_;
}
