// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DLIST_HH__
#define __DLIST_HH__

namespace Dune {

  template <class T> class DoubleLinkedList {
  private:
    struct Element;       // Vorwaertsdeklaration fuer das Listenelement
  public:
    class Iterator {      // Iteratorklasse zum
    private:              // Durchlaufen der Elemente des Containers
      Element* p;             // Iterator ist ein Zeiger auf ein Listenelement
    public:
      Iterator();
      bool operator!= (Iterator x);
      bool operator== (Iterator x);
      Iterator operator++ ();             // prefix Stroustrup p. 292
      Iterator operator++ (int);          // postfix
      Iterator operator-- ();             // prefix
      Iterator operator-- (int);          // postfix
      T& operator* () const;
      T* operator-> () const;           // Stroustrup p. 289
      friend class DoubleLinkedList<T>;
    } ;
    Iterator begin () const;
    Iterator end () const;
    Iterator rbegin () const;
    Iterator rend () const;

    DoubleLinkedList();
    DoubleLinkedList (const DoubleLinkedList<T>&);
    ~DoubleLinkedList();
    DoubleLinkedList<T>& operator= (const DoubleLinkedList<T>&);
    int size () const;
    Iterator insert_after (Iterator i, T t);
    Iterator insert_before (Iterator i, T t);
    void erase (Iterator i);

  private:
    struct Element {          // Typ fuer das Listenelement
      Element* next;              // Nachfolger
      Element* prev;              // Vorgaenger
      T item;                     // Datum
      Element (T &t);             // setze next=prev=0
    };

    Iterator head;            // erstes Element der Liste
    Iterator tail;            // letztes Element der Liste
    int numelements;          // Anzahl Elemente in der Liste
  } ;

}

#include "dlist.cc"

#endif
