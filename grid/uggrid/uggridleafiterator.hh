// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRIDLEAFITERATOR_HH
#define DUNE_UGGRIDLEAFITERATOR_HH

#include <dune/grid/uggrid/uggridentitypointer.hh>

/** \file
 * \brief The UGGridLeafIterator class
 */

namespace Dune {

  /** \brief Iterator over all entities of a given codimension and level of a grid.
   * \ingroup UGGrid
   */
  template<int codim, PartitionIteratorType pitype, class GridImp>
  class UGGridLeafIterator :
    public Dune::UGGridEntityPointer <codim,GridImp>
  {
    enum {dim = GridImp::dimension};

    friend class UGGridEntity<codim,dim,GridImp>;

  public:

    UGGridLeafIterator(const GridImp& grid) : grid_(&grid) {

      // Entities below this level are certainly not leaf entities
      const unsigned int startingLevel = grid.leafIndexSet_.coarsestLevelWithLeafElements_;

      if (pitype==All_Partition || pitype==Ghost_Partition)
        setToTarget(UG_NS<dim>::PFirstNode(grid_->multigrid_->grids[startingLevel]), startingLevel);
      else
        setToTarget(UG_NS<dim>::FirstNode(grid_->multigrid_->grids[startingLevel]), startingLevel);

      if (!UG_NS<dim>::isLeaf(this->virtualEntity_.getTarget()))
        increment();
    }

    //! Constructor
    UGGridLeafIterator()
    {
      this->virtualEntity_.setToTarget(0);
    }

    //! prefix increment
    void increment() {
      // Increment until you find a leaf entity
      do {
        globalIncrement();
      } while (this->virtualEntity_.getTarget() && !UG_NS<dim>::isLeaf(this->virtualEntity_.getTarget()));
    }

  private:

    /** \brief This increment makes the iterator wander over all entities on all levels */
    void globalIncrement() {
      // Increment on this level
      this->virtualEntity_.setToTarget(UG_NS<GridImp::dimension>::succ(this->virtualEntity_.getTarget()));

      // If beyond the end of this level set to first of next level
      if (!this->virtualEntity_.getTarget() && this->level() < grid_->maxLevel()) {

        if (pitype==All_Partition || pitype==Ghost_Partition)
          setToTarget(UG_NS<dim>::PFirstNode(grid_->multigrid_->grids[this->level()+1]), this->level()+1);
        else
          setToTarget(UG_NS<dim>::FirstNode(grid_->multigrid_->grids[this->level()+1]), this->level()+1);

      }

    }

    // /////////////////////////////////////
    //   Data members
    // /////////////////////////////////////
    const GridImp* grid_;

  };


  template<PartitionIteratorType pitype, class GridImp>
  class UGGridLeafIterator<0,pitype,GridImp> :
    public Dune::UGGridEntityPointer <0,GridImp>
  {
    enum {dim = GridImp::dimension};

    friend class UGGridEntity<0,    dim,GridImp>;

  public:

    UGGridLeafIterator(const GridImp& grid) : grid_(&grid) {

      const int startingLevel = grid.leafIndexSet_.coarsestLevelWithLeafElements_;

      if (pitype==All_Partition || pitype==Ghost_Partition)
        setToTarget(UG_NS<dim>::PFirstElement(grid_->multigrid_->grids[startingLevel]), startingLevel);
      else
        setToTarget(UG_NS<dim>::FirstElement(grid_->multigrid_->grids[startingLevel]), startingLevel);

      // If this is not a leaf entity already increment to find the first leaf entity
      if (!UG_NS<dim>::isLeaf(this->virtualEntity_.getTarget()))
        increment();
    }

    //! Constructor
    UGGridLeafIterator()
    {
      this->virtualEntity_.setToTarget(0);
    }

    //! prefix increment
    void increment() {
      // Increment until you find a leaf entity
      do {
        globalIncrement();
      } while (this->virtualEntity_.getTarget() && !UG_NS<dim>::isLeaf(this->virtualEntity_.getTarget()));
    }

  private:

    /** \brief This increment makes the iterator wander over all entities on all levels */
    void globalIncrement() {
      // Increment on this level
      this->virtualEntity_.setToTarget(UG_NS<GridImp::dimension>::succ(this->virtualEntity_.getTarget()));

      // If beyond the end of this level set to first of next level
      if (!this->virtualEntity_.getTarget() && this->level() < grid_->maxLevel()) {

        if (pitype==All_Partition || pitype==Ghost_Partition)
          setToTarget(UG_NS<dim>::PFirstElement(grid_->multigrid_->grids[this->level()+1]), this->level()+1);
        else
          this->setToTarget(UG_NS<dim>::FirstElement(grid_->multigrid_->grids[this->level()+1]), this->level()+1);

      }

    }

    // /////////////////////////////////////
    //   Data members
    // /////////////////////////////////////
    const GridImp* grid_;

  };

}  // namespace Dune

#endif
