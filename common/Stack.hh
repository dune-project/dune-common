// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __STACK_HH__
#define __STACK_HH__

#include "dlist.hh"

namespace Dune {

  template<class T>
  class Stack : private DoubleLinkedList<T> {
  public:
    bool isempty () const;     // Stack leer ?
    bool isfull () const;      // Stack voll (nur bei vorgegebener Groesse)
    void push_front (T t);     // Einfuegen eines Elementes
    T pop_front ();            // Entfernen eines Elementes
    T front () const;          // Inspizieren des obersten Elementes
    int size() const;          // get number of elements in stack
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
  inline void Stack<T>::push_front (T t)
  {
    insert_after(rbegin(),t);
  }

  template<class T>
  inline T Stack<T>::pop_front ()
  {
    DoubleLinkedList<T>::Iterator i=rbegin();
    T t = *i;
    erase(i);
    return t;
  }

  template<class T>
  inline T Stack<T>::front () const
  {
    DoubleLinkedList<T>::Iterator i=rbegin();
    T t = *i;
    return t;
  }

}

template<class T, int n>
class FiniteStack {
public:
  bool isempty () const
  {
    return f==0;
  }

  bool isfull () const
  {
    return f>=n;
  }

  void push_front (T t)
  {
    s[f++] = t;
  }

  T pop_front ()
  {
    return s[--f];
  }

  T front () const
  {
    return s[f-1];
  }

  int size ()
  {
    return f;
  }

  FiniteStack ()
  {
    f = 0;
  }

private:
  T s[n];
  int f;
} ;



#endif
