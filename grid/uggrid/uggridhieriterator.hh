// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_UGHIERITERATOR_HH__
#define __DUNE_UGHIERITERATOR_HH__

#include <dune/common/stack.hh>

namespace Dune {

  //**********************************************************************
  //
  // --UGGridHierarchicIterator
  // --HierarchicIterator
  /*!
     Mesh entities of codimension 0 ("elements") allow to visit all entities of
     codimension 0 obtained through nested, hierarchic refinement of the entity.
     Iteration over this set of entities is provided by the HIerarchicIterator,
     starting from a given entity.
     This is redundant but important for memory efficient implementations of unstru
     hierarchically refined meshes.
   */

  template<int dim, int dimworld>
  class UGGridHierarchicIterator :
    public HierarchicIteratorDefault <dim,dimworld, UGCtype,
        UGGridHierarchicIterator,UGGridEntity>
  {

    // Either UG3d::ELEMENT or UG2d:ELEMENT
    typedef typename TargetType<0,dim>::T UGElementType;
  public:
    // Stack entry
    struct StackEntry {
      UGElementType* element;
      int level;
    };

  public:

    //! the default Constructor
    UGGridHierarchicIterator(int actLevel,int maxLevel);

    //! prefix increment
    UGGridHierarchicIterator& operator ++();

    //! equality
    bool operator== (const UGGridHierarchicIterator& i) const;

    //! inequality
    bool operator!= (const UGGridHierarchicIterator& i) const;

    //! dereferencing
    UGGridEntity<0,dim,dimworld>& operator*();

    //! arrow
    UGGridEntity<0,dim,dimworld>* operator->();

    //! implement with virtual element
    UGGridEntity<0,dim,dimworld> virtualEntity_;

  private:
    //! know the grid were im comming from
    //UGGrid<dim,dimworld> &grid_;

    //! max level to go down
    int maxlevel_;

  public:
    Stack<StackEntry> elemStack;

    UGElementType* target_;
  };

}  // end namespace Dune

#endif
