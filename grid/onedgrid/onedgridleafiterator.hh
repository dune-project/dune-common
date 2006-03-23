// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONE_D_GRID_LEAFITERATOR_HH
#define DUNE_ONE_D_GRID_LEAFITERATOR_HH

#include <dune/grid/onedgrid/onedgridentitypointer.hh>

/** \file
 * \brief The OneDGridLeafIterator class
 */

namespace Dune {

  /** \brief Iterator over all entities of a given codimension and level of a grid.
   * \ingroup OneDGrid
   */
  template<int codim, PartitionIteratorType pitype, class GridImp>
  class OneDGridLeafIterator :
    public Dune::OneDGridEntityPointer <codim,GridImp>
  {
    enum {dim = GridImp::dimension};

    friend class OneDGridEntity<codim,dim,GridImp>;

  public:

    OneDGridLeafIterator(const GridImp& grid) : grid_(&grid) {

      /** \todo Can a make the fullRefineLevel work somehow? */
      const int fullRefineLevel = 0;

      this->virtualEntity_.setToTarget(grid_->vertices[fullRefineLevel].begin);

      if (!this->virtualEntity_.target()->isLeaf())
        increment();
    }

    //! Constructor
    OneDGridLeafIterator()
    {
      this->virtualEntity_.setToTarget(NULL);
    }

    //! prefix increment
    void increment() {
      // Increment until you find a leaf entity
      do {
        globalIncrement();
      } while (this->virtualEntity_.target() && !this->virtualEntity_.target()->isLeaf());
    }

  private:

    /** \brief This increment makes the iterator wander over all entities on all levels */
    void globalIncrement() {

      // Backup current level because it may not be accessible anymore after
      // setting the pointer to the next entity.
      const int oldLevel = this->virtualEntity_.level();

      // Increment on this level
      this->virtualEntity_.setToTarget(this->virtualEntity_.target()->succ_);

      // If beyond the end of this level set to first of next level
      if (!this->virtualEntity_.target() && oldLevel < grid_->maxLevel())
        this->virtualEntity_.setToTarget(grid_->vertices[oldLevel+1].begin);

    }

    // /////////////////////////////////////
    //   Data members
    // /////////////////////////////////////
    const GridImp* grid_;

  };


  /** \brief Iterator over all entities of a given codimension and level of a grid.
   * \ingroup OneDGrid
   */
  template<PartitionIteratorType pitype, class GridImp>
  class OneDGridLeafIterator<0,pitype,GridImp> :
    public Dune::OneDGridEntityPointer <0,GridImp>
  {
    friend class OneDGridEntity<0,1,GridImp>;

  public:

    OneDGridLeafIterator(const GridImp& grid) : grid_(&grid) {

      /** \todo Can a make the fullRefineLevel work somehow? */
      const int fullRefineLevel = 0;

      this->virtualEntity_.setToTarget(grid_->elements[fullRefineLevel].begin);

      if (!this->virtualEntity_.target()->isLeaf())
        increment();
    }

    //! Constructor
    OneDGridLeafIterator()
    {
      this->virtualEntity_.setToTarget(NULL);
    }

    //! prefix increment
    void increment() {
      // Increment until you find a leaf entity
      do {
        globalIncrement();
      } while (this->virtualEntity_.target() && !this->virtualEntity_.isLeaf());
    }

  private:

    /** \brief This increment makes the iterator wander over all entities on all levels */
    void globalIncrement() {

      // Backup current level because it may not be accessible anymore after
      // setting the pointer to the next entity.
      const int oldLevel = this->virtualEntity_.level();

      // Increment on this level
      this->virtualEntity_.setToTarget(this->virtualEntity_.target()->succ_);

      // If beyond the end of this level set to first of next level
      if (!this->virtualEntity_.target() && oldLevel < grid_->maxLevel())
        this->virtualEntity_.setToTarget(grid_->elements[oldLevel+1].begin);

    }

    // /////////////////////////////////////
    //   Data members
    // /////////////////////////////////////
    const GridImp* grid_;

  };


}  // namespace Dune

#endif
