// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <assert.h>

namespace Dune {

  template <int cc, int dim, class GridImp>
  class OneDGridSubEntityFactory
  {
  public:
    static OneDGridEntity<cc,dim,GridImp>* get(OneDGridEntity<0,dim,GridImp>& me, int i)
    {
      DUNE_THROW(NotImplemented, "OneDGridSubEntityFactor::get() default");
    }
  };

  template<class GridImp>
  class OneDGridSubEntityFactory<0,1,GridImp>
  {
  public:
    static OneDGridEntity<0,1,GridImp>* get(OneDGridEntity<0,1,GridImp>& me, int i)
    {
      assert(i==0);
      return &me;
    }
  };

  template<class GridImp>
  class OneDGridSubEntityFactory<1,1,GridImp>
  {
  public:
    static OneDGridEntity<1,1,GridImp>* get(OneDGridEntity<0,1,GridImp>& me, int i)
    {
      assert(i==0 || i==1);
      return me.geo_.vertex[i];
    }
  };

  template <int dim, class GridImp>
  template <int cc>
  OneDGridLevelIterator<cc,All_Partition,GridImp>
  OneDGridEntity<0,dim,GridImp>::entity ( int i ) const
  {
    /** \todo Remove this const cast */
    OneDGridEntity<0,dim,GridImp>* nonconst_this = const_cast< OneDGridEntity<0,dim,GridImp>*>(this);
    return OneDGridLevelIterator<cc,All_Partition,GridImp>(OneDGridSubEntityFactory<cc,dim,GridImp>::get(*nonconst_this, i));
  }



} // End namespace Dune
