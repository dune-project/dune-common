// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONE_D_GRID_HIERITERATOR_HH
#define DUNE_ONE_D_GRID_HIERITERATOR_HH

/** \file
 * \brief The OneDGridHierarchicIterator class
 */

#include <dune/common/stack.hh>

namespace Dune {

  //**********************************************************************
  //
  // --OneDGridHierarchicIterator
  // --HierarchicIterator
  /** \brief Iterator over the descendants of an entity.
   * \ingroup OneDGrid
     Mesh entities of codimension 0 ("elements") allow to visit all entities of
     codimension 0 obtained through nested, hierarchic refinement of the entity.
     Iteration over this set of entities is provided by the HIerarchicIterator,
     starting from a given entity.
     This is redundant but important for memory efficient implementations of unstru
     hierarchically refined meshes.
   */

  template<int dim, int dimworld>
  class OneDGridHierarchicIterator :
    public HierarchicIteratorDefault <dim,dimworld, OneDCType,
        OneDGridHierarchicIterator,OneDGridEntity>
  {

    friend class OneDGridEntity<0,dim,dimworld>;

    // Either OneD3d::ELEMENT or OneD2d:ELEMENT
    //typedef typename TargetType<0,dim>::T OneDElementType;

    // Stack entry
    struct StackEntry {
      OneDGridElement<0,dim>* element;
      int level;
    };

  public:

    //! the default Constructor
    OneDGridHierarchicIterator(int actLevel,int maxLevel);

    //! prefix increment
    OneDGridHierarchicIterator& operator ++();

    //! equality
    bool operator== (const OneDGridHierarchicIterator& i) const;

    //! inequality
    bool operator!= (const OneDGridHierarchicIterator& i) const;

    //! dereferencing
    OneDGridEntity<0,dim,dimworld>& operator*();

    //! arrow
    OneDGridEntity<0,dim,dimworld>* operator->();

  private:

    //! max level to go down
    int maxlevel_;

    Stack<StackEntry> elemStack;

  };

  // Include class method definitions
  //#include "uggridhieriterator.cc"

}  // end namespace Dune

#endif
