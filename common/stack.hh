// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_STACK_HH
#define DUNE_STACK_HH

#include <dune/common/dlist.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/deprecated.hh>

// Backward compatibility
#include <dune/common/finitestack.hh>

#warning This header is deprecated.  Use std::stack instead of Dune::Stack.  The class Dune::FiniteStack has been moved to finitestack.hh.

namespace Dune {

  /*! \addtogroup Common
     @{
   */

  /*! \file

     This file implements a stack-classes Stack. It is
     mainly used by the grid iterators where exact knowledge of the stack
     implementation is needed to guarantee efficient execution.

     \deprecated std::stack is several times faster than this class.
   */

  //! Exception thrown by the stack
  class StackException : public RangeError {};

  /** \brief (DEPRECATED) dynamic stack implemented with a double linked list

     This class can be used instead of the standard STL-stack if
     detailed knowledge about the stacks implementation is needed. For
     example, it is unknown if a copy of an empty STL-stack requires
     time or not

   */
  template<class T>
  class Stack : private DoubleLinkedList<T> {
  public:

    //! Returns true if the stack is emptry
    bool empty () const;

    //! Always returns false because the stack is never full
    bool full () const;

    //! Put a new object onto the stack
    void push (T& t) DUNE_DEPRECATED;

    //! Removes and returns the uppermost object from the stack
    T pop () DUNE_DEPRECATED;

    //! Returns the uppermost object from the stack
    T top () const;

    //! Number of elements on the stack
    int size() const;
  } DUNE_DEPRECATED ;

  template<class T>
  inline int Stack<T>::size () const
  {
    return DoubleLinkedList<T>::size();
  }

  template<class T>
  inline bool Stack<T>::empty () const
  {
    return size()==0;
  }

  template<class T>
  inline bool Stack<T>::full () const
  {
    return false;
  }

  template<class T>
  inline void Stack<T>::push (T& t)
  {
    insert_after(this->rbegin(),t);
  }

  template<class T>
  inline T Stack<T>::pop ()
  {
    if (empty())
      DUNE_THROW(StackException,  "cannot pop() empty stack!");

    typename DoubleLinkedList<T>::Iterator i=this->rbegin();
    T t = *i;
    erase(i);
    return t;
  }

  template<class T>
  inline T Stack<T>::top () const
  {
    if (empty())
      DUNE_THROW(StackException,  "no top() in empty stack!");

    typename DoubleLinkedList<T>::Iterator i=this->rbegin();
    T t = *i;
    return t;
  }

}

//! }@

#endif
