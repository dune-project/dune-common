// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ALUMEMORY_HH__
#define __DUNE_ALUMEMORY_HH__

#include <stack>

//! organize the memory management for entitys used by the NeighborIterator
template <class Object>
class MemoryProvider
{
  stack < Object * > objStack_;

  typedef MemoryProvider < Object > MyType;
public:
  typedef Object ObjectType;
  //typedef MemoryPointer< ObjectType, MyType > MemoryPointerType;

  //! delete all objects stored in stack
  MemoryProvider() {};

  //! call deleteEntity
  ~MemoryProvider ();

  //! i.e. return pointer to Entity
  template <class GridType>
  ObjectType * getNewObjectEntity(const GridType &grid, int level);

  //! free, move element to stack, returns NULL
  void freeObjectEntity (ObjectType * obj);

private:
  //! do not copy pointers
  MemoryProvider(const MemoryProvider<Object> & org) {}
};


//************************************************************************
//
//  MemoryProvider implementation
//
//************************************************************************
template <class Object> template <class GridType>
inline typename MemoryProvider<Object>::ObjectType *
MemoryProvider<Object>::getNewObjectEntity
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
inline MemoryProvider<Object>::~MemoryProvider()
{
  while ( !objStack_.empty() )
  {
    ObjectType * obj = objStack_.top();
    objStack_.pop();
    if( obj ) delete obj;
  }
}

template <class Object>
inline void MemoryProvider<Object>::freeObjectEntity(ObjectType * obj)
{
  objStack_.push( obj );
}

#endif
