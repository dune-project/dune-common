// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DLIST_HH__
#define __DUNE_DLIST_HH__

namespace Dune {

  /** \brief A doubly-linked list
   */
  template <class T>
  class DoubleLinkedList {
  private:
    struct Element;       // Vorwaertsdeklaration fuer das Listenelement
  public:

    /** \brief Iterator class for the doubly-linked list
     */
    class Iterator {
    private:
      //! Iterator is a pointer to a list element
      Element* p;
    public:

      //! ???
      Iterator();

      //! ???
      bool operator!= (Iterator x);

      //! ???
      bool operator== (Iterator x);

      //! Prefix increment
      Iterator operator++ ();

      //! Postfix increment
      Iterator operator++ (int);

      //! Prefix decrement
      Iterator operator-- ();

      //! Postfix decrement
      Iterator operator-- (int);

      //! ???
      T& operator* () const;

      //! ???
      T* operator-> () const;     // Stroustrup p. 289

      //! ???
      friend class DoubleLinkedList<T>;
    } ;

    //! ???
    Iterator begin () const;

    //! ???
    Iterator end () const;

    //! ???
    Iterator rbegin () const;

    //! ???
    Iterator rend () const;


    //! ???
    DoubleLinkedList();

    //! ???
    DoubleLinkedList (const DoubleLinkedList<T>&);

    //! ???
    ~DoubleLinkedList();

    //! ???
    DoubleLinkedList<T>& operator= (const DoubleLinkedList<T>&);

    //! ???
    int size () const;

    //! ???
    Iterator insert_after (Iterator i, T& t);

    //! ???
    Iterator insert_before (Iterator i, T& t);

    //! ???
    void erase (Iterator i);

  private:
    struct Element {          // Typ fuer das Listenelement
      Element* next;        // Nachfolger
      Element* prev;        // Vorgaenger
      T item;               // Datum
      Element (T &t);       // setze next=prev=0
    };

    Iterator head;            // erstes Element der Liste
    Iterator tail;            // letztes Element der Liste
    int numelements;          // Anzahl Elemente in der Liste
  } ;

}

#include "dlist.cc"

#endif
