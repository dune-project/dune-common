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

    typedef typename DoubleLinkedList<OneDGridEntity<0,dim,dimworld> >::Iterator ElementIterator;

    // Stack entry
    struct StackEntry {
      ElementIterator element;
      /** \todo Do we need the level ? */
      int level;
    };

  public:

    //! the default Constructor
    OneDGridHierarchicIterator(int maxlevel) : elemStack() {
      maxlevel_ = maxlevel;
      target_   = ElementIterator();
    }

    //! prefix increment
    OneDGridHierarchicIterator& operator ++() {

      if (elemStack.empty())
        return (*this);

      StackEntry old_target = elemStack.pop();

      // Traverse the tree no deeper than maxlevel
      if (old_target.level < maxlevel_) {

        // Load sons of old target onto the iterator stack
        if (old_target.element.hasChildren()) {
          StackEntry se0;
          se0.element = old_target.element.sons_[0];
          se0.level   = old_target.level + 1;
          elemStack.push(se0);

          StackEntry se1;
          se1.element = old_target.element.sons_[1];
          se1.level   = old_target.level + 1;
          elemStack.push(se1);
        }

      }

      if (elemStack.empty())
        //virtualEntity_.setToTarget(0);
        target_ = ElementIterator();
      else
        //virtualEntity_.setToTarget(elemStack.top().element, elemStack.top().level);
        target_ = elemStack.top().element;

      return (*this);
    }

    //! equality
    bool operator== (const OneDGridHierarchicIterator& other) const {
      return ( (elemStack.size()==0 && other.elemStack.size()==0) ||
               ((elemStack.size() == other.elemStack.size()) &&
                (elemStack.top().element == other.elemStack.top().element)));
    }

    //! inequality
    bool operator!= (const OneDGridHierarchicIterator& other) const {
      return !((*this) == other);
    }

    //! dereferencing
    OneDGridEntity<0,dim,dimworld>& operator*() {
      return *target_;
    }

    //! arrow
    OneDGridEntity<0,dim,dimworld>* operator->() {
      return target_;
    }

  private:

    //! max level to go down
    int maxlevel_;

    Stack<StackEntry> elemStack;

    ElementIterator target_;

  };

  // Include class method definitions
  //#include "uggridhieriterator.cc"

}  // end namespace Dune

#endif
