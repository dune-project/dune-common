// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONE_D_GRID_LEVELITERATOR_HH
#define DUNE_ONE_D_GRID_LEVELITERATOR_HH

/** \file
 * \brief The OneDGridLevelIterator class
 */

#include <dune/common/dlist.hh>

namespace Dune {

  //**********************************************************************
  //
  // --OneDGridLevelIterator
  // --LevelIterator
  /** \brief Iterator over all entities of a given codimension and level of a grid.
   * \ingroup OneDGrid
   */
  template<int codim, PartitionIteratorType pitype, class GridImp>
  class OneDGridLevelIterator :
    public LevelIteratorDefault <codim, pitype, GridImp, OneDGridLevelIterator>
  {

    enum {dim=GridImp::dimension};
    friend class OneDGridLevelIteratorFactory<codim>;
    friend class OneDGrid<dim,GridImp::dimensionworld>;
    friend class OneDGridEntity<codim,dim,GridImp>;
    friend class OneDGridEntity<0,dim,GridImp>;

    typedef typename GridImp::template codim<codim>::Entity Entity;

  protected:

    /** \brief Constructor from a given iterator */
    OneDGridLevelIterator<codim,pitype, GridImp>(OneDEntityImp<dim-codim>* it)
    /*: virtualEntity_()*/ {
      target_ = it;
      virtualEntity_.setToTarget(it);
    }

  public:

    //! prefix increment
    void increment() {
      target_ = target_->succ_;
      virtualEntity_.setToTarget(target_);
    }

    //! equality
    bool equals (const OneDGridLevelIterator<codim, pitype, GridImp>& other) const {
      return other.target_ == target_;
    }

    //! dereferencing
    Entity& dereference() const {return virtualEntity_;}

    //! ask for level of entity
    int level () const {return target_->level();}

  private:

    mutable OneDEntityWrapper<codim,GridImp::dimension,GridImp> virtualEntity_;

    OneDEntityImp<dim-codim>* target_;

  };

}  // namespace Dune

#endif
