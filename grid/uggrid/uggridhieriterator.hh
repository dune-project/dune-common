// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_UGHIERITERATOR_HH__
#define __DUNE_UGHIERITERATOR_HH__

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

  template<int dim, int dimworld>
  class UGGridHierarchicIterator :
    public HierarchicIteratorDefault <dim,dimworld, UGCtype,
        UGGridHierarchicIterator,UGGridEntity>
  {

    friend class UGGridEntity<0,dim,dimworld>;

    // Either UG3d::ELEMENT or UG2d:ELEMENT
    typedef typename TargetType<0,dim>::T UGElementType;

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

  private:

    //! implement with virtual element
    UGGridEntity<0,dim,dimworld> virtualEntity_;

    //! max level to go down
    int maxlevel_;

    Stack<StackEntry> elemStack;

    UGElementType* target_;
  };

  // Include class method definitions
#include "uggridhieriterator.cc"

}  // end namespace Dune

#endif
