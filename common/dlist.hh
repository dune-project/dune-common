// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DLIST_HH__
#define __DUNE_DLIST_HH__

namespace Dune {

  /*! \addtogroup Common
     @{
   */

  /*! \file

     Declaration of a doubly linked list
   */


  /*! A doubly-linked list */
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

      //! constructor
      Iterator();

      //! comparison
      bool operator!= (Iterator x);

      //! comparison
      bool operator== (Iterator x);

      //! Prefix increment
      Iterator operator++ ();

      //! Postfix increment
      Iterator operator++ (int);

      //! Prefix decrement
      Iterator operator-- ();

      //! Postfix decrement
      Iterator operator-- (int);

      //! dereferenciation
      T& operator* () const;

      //! dereferenciation (Stroustrup p. 289)
      T* operator-> () const;

      friend class DoubleLinkedList<T>;
    } ;

    //! iterator at the lists start
    Iterator begin () const;

    //! iterator behind last element
    Iterator end () const;

    //! iterator at the lists end
    Iterator rbegin () const;

    //! iterator before the lists start
    Iterator rend () const;


    //! empty constructor
    DoubleLinkedList();

    //! copy constructor
    DoubleLinkedList (const DoubleLinkedList<T>&);

    //! destructor
    ~DoubleLinkedList();

    //!
    DoubleLinkedList<T>& operator= (const DoubleLinkedList<T>&);

    //! current list size
    int size () const;

    //! insert after an iterators position
    Iterator insert_after (Iterator i, T& t);

    //! insert befor an iterators position
    Iterator insert_before (Iterator i, T& t);

    //! remove selected element
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

//! }@

#include "dlist.cc"

#endif
