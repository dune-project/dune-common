// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_STACK_HH
#define DUNE_STACK_HH

#include <dune/common/dlist.hh>
#include <dune/common/exceptions.hh>

namespace Dune {

  /*! \addtogroup Common
     @{
   */

  /*! \file

     This file implements two stack-classes Stack and FiniteStack. They are
     mainly used by the grid iterators where exact knowledge of the stack
     implementation is needed to guarantee efficient execution
   */

  //! Exception thrown by the stack
  class StackException : public Exception {};

  /** dynamic stack implemented with a double linked list

     This class can be used instead of the standard STL-stack if
     detailed knowledge about the stacks implementation is needed. For
     example, it is unknown if a copy of an empty STL-stack requires
     time or not

     \todo change interface to be STL-conforming
   */
  template<class T>
  class Stack : private DoubleLinkedList<T> {
  public:

    //! Returns true if the stack is emptry
    bool isempty () const;

    //! Always returns false because the stack is never full
    bool isfull () const;

    //! Put a new object onto the stack
    void push_front (T& t);


    //! Removes and returns the uppermost object from the stack
    T pop_front ();

    //! Returns the uppermost object from the stack
    T front () const;

    //! Number of elements on the stack
    int size() const;
  } ;

  template<class T>
  inline int Stack<T>::size () const
  {
    return DoubleLinkedList<T>::size();
  }

  template<class T>
  inline bool Stack<T>::isempty () const
  {
    return size()==0;
  }

  template<class T>
  inline bool Stack<T>::isfull () const
  {
    return false;
  }

  template<class T>
  inline void Stack<T>::push_front (T& t)
  {
    insert_after(this->rbegin(),t);
  }

  template<class T>
  inline T Stack<T>::pop_front ()
  {
    if (isempty())
      DUNE_THROW(StackException,  "Stack::pop_front(): The stack is empty!");

    typename DoubleLinkedList<T>::Iterator i=this->rbegin();
    T t = *i;
    erase(i);
    return t;
  }

  template<class T>
  inline T Stack<T>::front () const
  {
    if (isempty())
      DUNE_THROW(StackException,  "Stack::front(): The stack is empty!");

    typename DoubleLinkedList<T>::Iterator i=this->rbegin();
    T t = *i;
    return t;
  }


  /** \brief A stack with static memory allocation

     This class implements a very efficient stack where the maximum
     depth is known in advance

     \tparam n Maximum number of stack entries
   */
  template<class T, int n>
  class FiniteStack {
  public:

    //! Returns true if the stack is empty
    bool isempty () const
    {
      return f==0;
    }

    //! Returns true if the stack is full
    bool isfull () const
    {
      return f>=n;
    }

    //! Puts a new object onto the stack
    void push_front (const T& t)
    {
      s[f++] = t;
    }

    //! Removes and returns the uppermost object from the stack
    T pop_front ()
    {
      return s[--f];
    }

    //! Returns the uppermost object on the stack
    T front () const
    {
      return s[f-1];
    }

    //! Dynamic stacksize
    int size ()
    {
      return f;
    }

    //! Makes empty stack
    FiniteStack ()
    {
      f = 0;
    }

  private:
    T s[n];
    int f;
  };

}

//! @}

#endif
