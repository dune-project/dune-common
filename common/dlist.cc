// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DLIST_CC__
#define __DLIST_CC__

#include <iostream>
#include <new> // fuer std::nothrow

namespace Dune {

  // Iterator interface
  template<class T> inline
  typename DoubleLinkedList<T>::Iterator DoubleLinkedList<T>::begin () const
  {
    return head;
  }

  template<class T> inline
  typename DoubleLinkedList<T>::Iterator DoubleLinkedList<T>::end () const
  {
    Iterator tmp;     // Iterator mit 0 Zeiger !
    return tmp;
  }

  template<class T> inline
  typename DoubleLinkedList<T>::Iterator DoubleLinkedList<T>::rbegin () const
  {
    return tail;
  }

  template<class T> inline
  typename DoubleLinkedList<T>::Iterator DoubleLinkedList<T>::rend () const
  {
    Iterator tmp;     // Iterator mit 0 Zeiger !
    return tmp;
  }

  // Konstruktor
  template<class T>
  inline DoubleLinkedList<T>::DoubleLinkedList ()
  { // ruft vorher Default_konstruktor fuer head, tail
    numelements=0;
  }

  // Copy-Konstruktor
  template<class T>
  inline DoubleLinkedList<T>::DoubleLinkedList (const DoubleLinkedList<T>& l)
  { // ruft vorher Default_konstruktor fuer head, tail
    numelements=0;
    // kopiere alle Elemente der Argumentliste
    for (DoubleLinkedList<T>::Iterator i=l.begin(); i!=l.end(); i++)
      insert_after(rbegin(),*i);
  }

  // Zuweisung
  template<class T>
  inline DoubleLinkedList<T>& DoubleLinkedList<T>::operator= (const DoubleLinkedList<T>& l)
  {
    if (this!=&l)
    {
      // loesche alle Elemente der Liste auf der linken Seite
      while (begin()!=end()) erase(begin());

      // kopiere alle Elemente der Liste auf der rechten Seite
      for (DoubleLinkedList<T>::Iterator i=l.begin(); i!=l.end(); i++)
        insert_after(rbegin(),*i);
    }
    return *this;
  }

  // Destruktor
  template<class T>
  inline DoubleLinkedList<T>::~DoubleLinkedList()
  {
    while (begin()!=end()) erase(begin());
  }

  template<class T>
  inline int DoubleLinkedList<T>::size () const
  {
    return numelements;
  }


  template<class T>
  inline typename DoubleLinkedList<T>::Iterator DoubleLinkedList<T>::insert_after (Iterator i, T t)
  {
    // Teste Eingabe
    if (i.p==0 && head.p!=0) {std::cerr << "wo einfuegen?" << std::endl; return end();}

    // neues Listenelement erzeugen,
    Element* e = new(std::nothrow) Element(t);
    if (e==0) {std::cerr << "Kein Platz mehr" << std::endl; return end();}

    // einfuegen
    if (head.p==0)
    {
      // einfuegen in leere Liste
      head.p=e; tail.p=e;
    }
    else
    {
      // nach Element i.p einsetzen
      e->prev = i.p;
      e->next = i.p->next;
      i.p->next = e;
      if (e->next!=0) e->next->prev = e;
      // tail neu ?
      if (tail==i) tail.p=e;
    }

    // Groesse und Rueckgabeiterator
    numelements = numelements+1;
    Iterator tmp;
    tmp.p = e;
    return tmp;
  }

  template<class T>
  inline typename DoubleLinkedList<T>::Iterator DoubleLinkedList<T>::insert_before (Iterator i, T t)
  {
    // Teste Eingabe
    if (i.p==0 && head.p!=0) {std::cerr << "wo einfuegen?" << std::endl; return end();}

    // neues Listenelement erzeugen,
    Element* e = new(std::nothrow) Element(t);
    if (e==0) {std::cerr << "Kein Platz mehr" << std::endl; return end();}

    // einfuegen
    if (head.p==0)
    {
      // einfuegen in leere Liste
      head.p=e; tail.p=e;
    }
    else
    {
      // vor Element i.p einsetzen
      e->next = i.p;
      e->prev = i.p->prev;
      i.p->prev = e;
      if (e->prev!=0) e->prev->next = e;
      // head neu ?
      if (head==i) head.p=e;
    }

    // Groesse und Rueckgabeiterator
    numelements = numelements+1;
    Iterator tmp;
    tmp.p = e;
    return tmp;
  }

  template<class T>
  inline void DoubleLinkedList<T>::erase (Iterator i)
  {
    // Teste Eingabe
    if (i.p==0) return;

    // Ausfaedeln
    if (i.p->next!=0) i.p->next->prev = i.p->prev;
    if (i.p->prev!=0) i.p->prev->next = i.p->next;

    // head & tail
    if (head==i) head.p=i.p->next;
    if (tail==i) tail.p=i.p->prev;

    // Loeschen
    delete i.p;

    // Groesse
    numelements = numelements-1;

    return;
  }

  template <class T>
  inline std::ostream& operator<< (std::ostream& s, DoubleLinkedList<T>& a)
  {
    T t;
    s << "dlist " << a.size() << " elements = (" << std::endl;
    for (typename DoubleLinkedList<T>::Iterator i=a.begin(); i!=a.end(); i++)
    {
      t = *i;
      s << "    " << t << std::endl;
    }
    s << ")" << std::endl;
    return s;
  }


  template<class T>
  inline DoubleLinkedList<T>::Element::Element (T &t) : item(t)
  {
    next=0; prev=0;
  }

  template<class T>
  inline DoubleLinkedList<T>::Iterator::Iterator ()
  {
    p=0;
  }

  template<class T>
  inline bool DoubleLinkedList<T>::Iterator::operator!=
    (DoubleLinkedList<T>::Iterator x)
  {
    return p != x.p;
  }

  template<class T>
  inline bool DoubleLinkedList<T>::Iterator::operator==
    (DoubleLinkedList::Iterator x)
  {
    return p == x.p;
  }

  template<class T>
  inline typename DoubleLinkedList<T>::Iterator
  DoubleLinkedList<T>::Iterator::operator++ () // prefix
  {
    p = p->next; return *this;
  }

  template<class T>
  inline typename DoubleLinkedList<T>::Iterator
  DoubleLinkedList<T>::Iterator::operator++ (int) // postfix
  {
    Iterator tmp = *this;
    ++*this;
    return tmp;
  }

  template<class T>
  inline typename DoubleLinkedList<T>::Iterator
  DoubleLinkedList<T>::Iterator::operator-- () // prefix
  {
    p = p->prev; return *this;
  }

  template<class T>
  inline typename DoubleLinkedList<T>::Iterator
  DoubleLinkedList<T>::Iterator::operator-- (int) // postfix
  {
    Iterator tmp = *this;
    --*this;
    return tmp;
  }

  template<class T>
  inline T& DoubleLinkedList<T>::Iterator::operator* () const
  {
    return p->item;
  }

  template<class T>
  inline T* DoubleLinkedList<T>::Iterator::operator-> () const
  {
    return &(p->item);
  }

}

#endif
