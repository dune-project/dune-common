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
  template<class GridImp>
  class OneDGridHierarchicIterator :
    public Dune::OneDGridEntityPointer <0,GridImp>,
    public HierarchicIteratorDefaultImplementation <GridImp, OneDGridHierarchicIterator>
  {
    enum { dim = GridImp::dimension };
    friend class OneDGridEntity<0,dim,GridImp>;

    // Stack entry
    struct StackEntry {
      OneDEntityImp<1>* element;
      /** \todo Do we need the level ? */
      int level;
    };

  public:

    typedef typename GridImp::template Codim<0>::Entity Entity;

    //! Constructor
    OneDGridHierarchicIterator(int maxlevel) : OneDGridEntityPointer<0,GridImp>(NULL),
                                               maxlevel_(maxlevel), elemStack()
    {}

    //! prefix increment
    void increment() {

      if (elemStack.empty())
        return;

      StackEntry old_target = elemStack.pop();

      // Traverse the tree no deeper than maxlevel
      if (old_target.level < maxlevel_) {

        // Load sons of old target onto the iterator stack
        if (!old_target.element->isLeaf()) {
          StackEntry se0;
          se0.element = old_target.element->sons_[0];
          se0.level   = old_target.level + 1;
          elemStack.push(se0);

          // Add the second son only if it is different from the first one
          // i.e. the son is not just a copy of the father
          if (old_target.element->sons_[0] != old_target.element->sons_[1]) {
            StackEntry se1;
            se1.element = old_target.element->sons_[1];
            se1.level   = old_target.level + 1;
            elemStack.push(se1);
          }
        }

      }

      this->virtualEntity_.setToTarget((elemStack.empty()) ? NULL : elemStack.top().element);
    }

  private:

    //! max level to go down
    int maxlevel_;

    Stack<StackEntry> elemStack;

  };

}  // end namespace Dune

#endif
