// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_AGMEMORY_HH__
#define __DUNE_AGMEMORY_HH__

namespace Dune
{

  // organize the memory management for entitys used by the NeighborIterator
  template <class Object>
  class MemoryProvider
  {
  public:
    typedef Object ObjectType;

    struct ObjectEntity
    {
      ObjectEntity () : next (NULL), item (NULL) {};

      ObjectEntity *next;
      Object       *item;
    };

    // freeEntity_ = NULL
    MemoryProvider() : freeEntity_ (NULL) {};

    // do not copy pointers
    MemoryProvider(const MemoryProvider<Object> & org) : freeEntity_ (NULL) {};

    // call deleteEntity
    ~MemoryProvider ();

    // delete recursive all free ObjectEntitys
    void deleteEntity(ObjectEntity *obj);

    // i.e. return pointer to Entity
    template <class GridType>
    ObjectEntity *getNewObjectEntity(GridType &grid, int level);

    // i.e. return pointer to Entity
    template <class FuncSpaceType, class DofVecType>
    ObjectEntity *getNewObjectEntity(const FuncSpaceType &f, DofVecType &d);

    // i.e. get pointer to element
    ObjectEntity *getNewObjectEntity();

    // free, move element to stack
    void freeObjectEntity (ObjectEntity *obj);

  private:
    ObjectEntity  *freeEntity_;
  };


  //************************************************************************
  //
  //  MemoryProvider
  //
  //************************************************************************
  template <class Object> template <class GridType>
  inline typename MemoryProvider<Object>::ObjectEntity *
  MemoryProvider<Object>::getNewObjectEntity
    (GridType &grid, int level )
  {
    if(!freeEntity_)
    {
      ObjectEntity *oe = new ObjectEntity ();
      oe->item = new Object (grid,level);
      return oe;
    }
    else
    {
      ObjectEntity *oe = freeEntity_;
      freeEntity_ = oe->next;
      return oe;
    }
  }

  template <class Object> template <class FuncSpaceType, class DofVecType>
  inline typename MemoryProvider<Object>::ObjectEntity *
  MemoryProvider<Object>::getNewObjectEntity(const FuncSpaceType &f , DofVecType &d )
  {
    if(!freeEntity_)
    {
      ObjectEntity *oe = new ObjectEntity ();
      oe->item = new Object (f,d);
      return oe;
    }
    else
    {
      ObjectEntity *oe = freeEntity_;
      freeEntity_ = oe->next;
      return oe;
    }
  }

  template <class Object>
  inline typename MemoryProvider<Object>::ObjectEntity *
  MemoryProvider<Object>::getNewObjectEntity()
  {
    if(!freeEntity_)
    {
      ObjectEntity *oe = new ObjectEntity ();
      oe->item = new Object ();
      return oe;
    }
    else
    {
      ObjectEntity *oe = freeEntity_;
      freeEntity_ = oe->next;
      return oe;
    }
  }

  //template <class Object>
  inline MemoryProvider<ALBERT EL_INFO>::ObjectEntity *
  MemoryProvider<ALBERT EL_INFO>::getNewObjectEntity()
  {
    if(!freeEntity_)
    {
      ObjectEntity *oe = new ObjectEntity ();
      oe->item = (ALBERT EL_INFO *) std::malloc (sizeof(ALBERT EL_INFO));
      return oe;
    }
    else
    {
      ObjectEntity *oe = freeEntity_;
      freeEntity_ = oe->next;
      return oe;
    }
  }

  template <class Object>
  inline MemoryProvider<Object>::~MemoryProvider()
  {
    if(freeEntity_) deleteEntity(freeEntity_);
  }

  template <class Object>
  inline void MemoryProvider<Object>::freeObjectEntity(ObjectEntity *obj)
  {
    obj->next = freeEntity_;
    freeEntity_ = obj;
  }

  template <class Object>
  inline void MemoryProvider<Object>::deleteEntity(ObjectEntity *obj)
  {
    if(obj)
    {
      if(obj->next)
        deleteEntity(obj->next);

      if(obj->item) delete obj->item;
      delete obj;
    }
  }

  inline void MemoryProvider<ALBERT EL_INFO>::deleteEntity(ObjectEntity *obj)
  {
    std::free(obj->item);
    delete obj;
  }

  typedef MemoryProvider < ALBERT EL_INFO > ElInfoProvider;
  static ElInfoProvider elinfoProvider;


} // end namespace Dune

#endif
