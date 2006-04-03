// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDMEMORY_HH
#define DUNE_ALU3DGRIDMEMORY_HH

#include <stack>

namespace Dune {

  //! organize the memory management for entitys used by the NeighborIterator
  template <class Object>
  class ALUFastMemoryProvider
  {
    static std::stack < Object * > objStack_;
    typedef ALUFastMemoryProvider < Object > MyType;
    typedef Object ObjectType;

  public:
    void * operator new (size_t s);
    void operator delete (void *obj, size_t s);
  };

  template <class Object>
  std::stack< Object * > ALUFastMemoryProvider< Object > :: objStack_;

  template <class Object>
  inline void *
  ALUFastMemoryProvider<Object> :: operator new (size_t s)
  {
    assert( s == sizeof(ObjectType) );
    if( objStack_.empty() )
    {
      void * obj = std::malloc( sizeof(ObjectType) );
      assert( obj );
      return obj;
    }
    else
    {
      ObjectType * obj = objStack_.top();
      objStack_.pop();
      return ((void  *) obj);
    }
  }

  template <class Object>
  inline void
  ALUFastMemoryProvider<Object> :: operator delete (void *ptr, size_t s)
  {
    assert( s == sizeof(ObjectType) );
    objStack_.push( (ObjectType *) ptr );
  }


  //! organize the memory management for entitys used by the NeighborIterator
  template <class Object>
  class ALUMemoryProvider
  {
    std::stack < Object * > objStack_;

    typedef ALUMemoryProvider < Object > MyType;
  public:
    typedef Object ObjectType;

    //! delete all objects stored in stack
    ALUMemoryProvider() {};

    //! call deleteEntity
    ~ALUMemoryProvider ();

    //! i.e. return pointer to Entity
    template <class GridType>
    ObjectType * getObject(const GridType &grid, int level);

    //! i.e. return pointer to Entity
    template <class GridType, class EntityImp>
    inline ObjectType * getEntityObject(const GridType &grid, int level , EntityImp * fakePtr )
    {
      if( objStack_.empty() )
      {
        return ( new ObjectType(EntityImp(grid,level) ));
      }
      else
      {
        ObjectType * obj = objStack_.top();
        objStack_.pop();
        return obj;
      }
    }

    //! i.e. return pointer to Entity
    ObjectType * getObjectCopy(const ObjectType & org);

    //! free, move element to stack, returns NULL
    void freeObject (ObjectType * obj);

  private:
    //! do not copy pointers
    ALUMemoryProvider(const ALUMemoryProvider<Object> & org) {}
  };


  //************************************************************************
  //
  //  ALUMemoryProvider implementation
  //
  //************************************************************************
  template <class Object> template <class GridType>
  inline typename ALUMemoryProvider<Object>::ObjectType *
  ALUMemoryProvider<Object>::getObject
    (const GridType &grid, int level )
  {
    if( objStack_.empty() )
    {
      return ( new Object (grid,level) );
    }
    else
    {
      ObjectType * obj = objStack_.top();
      objStack_.pop();
      return obj;
    }
  }

  template <class Object>
  inline typename ALUMemoryProvider<Object>::ObjectType *
  ALUMemoryProvider<Object>::getObjectCopy
    (const ObjectType & org )
  {
    if( objStack_.empty() )
    {
      return ( new Object (org) );
    }
    else
    {
      ObjectType * obj = objStack_.top();
      objStack_.pop();
      return obj;
    }
  }

  template <class Object>
  inline ALUMemoryProvider<Object>::~ALUMemoryProvider()
  {
    while ( !objStack_.empty() )
    {
      ObjectType * obj = objStack_.top();
      objStack_.pop();
      delete obj;
    }
  }

  template <class Object>
  inline void ALUMemoryProvider<Object>::freeObject(Object * obj)
  {
    objStack_.push( obj );
  }

} // end namespace Dune

#endif
