// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <assert.h>

namespace Dune {

  template <int cc, int dim, int dimworld>
  class OneDGridSubEntityFactory
  {
  public:
    static OneDGridEntity<cc,dim,dimworld>* get(OneDGridEntity<0,dim,dimworld>& me, int i)
    {
      DUNE_THROW(NotImplemented, "OneDGridSubEntityFactor::get() default");
    }
  };

  template<>
  class OneDGridSubEntityFactory<0,1,1>
  {
  public:
    static OneDGridEntity<0,1,1>* get(OneDGridEntity<0,1,1>& me, int i)
    {
      assert(i==0);
      return &me;
    }
  };

  template<>
  class OneDGridSubEntityFactory<1,1,1>
  {
  public:
    static OneDGridEntity<1,1,1>* get(OneDGridEntity<0,1,1>& me, int i)
    {
      assert(i==0 || i==1);
      return me.geo_.vertex[i];
    }
  };

  template <int dim, int dimworld>
  template <int cc>
  OneDGridLevelIterator<cc,dim,dimworld,All_Partition>
  OneDGridEntity<0,dim,dimworld>::entity ( int i ) const
  {
    /** \todo Remove this const cast */
    OneDGridEntity<0,dim,dimworld>* nonconst_this = const_cast< OneDGridEntity<0,dim,dimworld>*>(this);
    return OneDGridLevelIterator<cc,dim,dimworld,All_Partition>(OneDGridSubEntityFactory<cc,dim,dimworld>::get(*nonconst_this, i));
  }



} // End namespace Dune
