// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:



// gehe zum i Schritte weiter , wie auch immer
template<int codim, int dim, int dimworld>
inline UGGridLevelIterator < codim,dim,dimworld >&
UGGridLevelIterator < codim,dim,dimworld >::operator++(int steps)
{
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
  return false /*(virtualEntity_.getElInfo() == I.virtualEntity_.getElInfo())*/;
}

template<int codim, int dim, int dimworld>
inline bool UGGridLevelIterator < codim,dim,dimworld >::
operator !=(const UGGridLevelIterator< codim,dim,dimworld > & I) const
{
  return false /*(virtualEntity_.getElInfo() != I.virtualEntity_.getElInfo() )*/;
}
