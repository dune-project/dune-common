// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGHIERITERATOR_HH
#define DUNE_UGHIERITERATOR_HH

/** \file
 * \brief The UGGridHierarchicIterator class
 */

#include <dune/common/stack.hh>

namespace Dune {

  //**********************************************************************
  //
  // --UGGridHierarchicIterator
  // --HierarchicIterator
  /** \brief Iterator over the descendants of an entity.
   * \ingroup UGGrid
     Mesh entities of codimension 0 ("elements") allow to visit all entities of
     codimension 0 obtained through nested, hierarchic refinement of the entity.
     Iteration over this set of entities is provided by the HIerarchicIterator,
     starting from a given entity.
     This is redundant but important for memory efficient implementations of unstru
     hierarchically refined meshes.
   */

  template<class GridImp>
  class UGGridHierarchicIterator :
    public HierarchicIteratorDefault <GridImp,UGGridHierarchicIterator>
  {

    friend class UGGridEntity<0,GridImp::dimension,GridImp>;

    // Either UG3d::ELEMENT or UG2d:ELEMENT
    typedef typename TargetType<0,GridImp::dimension>::T UGElementType;

    // Stack entry
    struct StackEntry {
      UGElementType* element;
      int level;
    };

  public:
    typedef typename GridImp::template codim<0>::Entity Entity;

    //! the default Constructor
    UGGridHierarchicIterator(int actLevel,int maxLevel);

    void increment();

    bool equals (const UGGridHierarchicIterator& i) const;

    //UGGridEntity<0,GridImp::dimension,GridImp>& dereference() const;
    Entity& dereference() const;

#if 0
    //! prefix increment
    UGGridHierarchicIterator& operator ++();

    //! equality
    bool operator== (const UGGridHierarchicIterator& i) const;

    //! inequality
    bool operator!= (const UGGridHierarchicIterator& i) const;

    //! dereferencing
    UGGridEntity<0,GridImp::dimension,GridImp>& operator*();

    //! arrow
    UGGridEntity<0,GridImp::dimension,GridImp>* operator->();
#endif

  private:

    //! implement with virtual element
    mutable UGMakeableEntity<0,GridImp::dimension,GridImp> virtualEntity_;

    //! max level to go down
    int maxlevel_;

    Stack<StackEntry> elemStack;

  };

  // Include class method definitions
#include "uggridhieriterator.cc"

}  // end namespace Dune

#endif
