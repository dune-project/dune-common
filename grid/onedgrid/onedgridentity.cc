// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <assert.h>

namespace Dune {

  template <int cc, int dim, class GridImp>
  class OneDGridSubEntityFactory
  {
#if 0
  public:
    static OneDGridEntity<cc,dim,GridImp>* get(OneDGridEntity<0,dim,GridImp>& me, int i)
    {
      DUNE_THROW(NotImplemented, "OneDGridSubEntityFactor::get() default");
    }
#endif
  };

  template<class GridImp>
  class OneDGridSubEntityFactory<0,1,GridImp>
  {
  public:
    static OneDEntityImp<1>* get(OneDEntityImp<1>* me, int i)
    {
      assert(i==0);
      return me;
    }
  };

  template<class GridImp>
  class OneDGridSubEntityFactory<1,1,GridImp>
  {
  public:
    static OneDEntityImp<0>* get(OneDEntityImp<1>* me, int i)
    {
      assert(i==0 || i==1);
      return me->vertex_[i];
    }
  };

  template <int dim, class GridImp>
  template <int cc>
  typename GridImp::template codim<cc>::EntityPointer
  OneDGridEntity<0,dim,GridImp>::entity ( int i ) const
  {
    /** \todo Remove this const cast */
    //OneDGridEntity<0,dim,GridImp>* nonconst_this = const_cast< OneDGridEntity<0,dim,GridImp>*>(this);
    return OneDGridLevelIterator<cc,All_Partition,GridImp>(OneDGridSubEntityFactory<cc,dim,GridImp>::get(this->target_, i));
  }



} // End namespace Dune
