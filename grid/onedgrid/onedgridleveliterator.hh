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
    //friend class OneDGridEntity<codim,dim,dimworld>;
    //     friend class OneDGridEntity<0,dim,dimworld>;

  protected:

    /** \brief Constructor from a given iterator */
    OneDGridLevelIterator<codim,pitype, GridImp>(OneDGridEntity<codim,dim,GridImp>* it) {
      target_ = it;
    }

  public:

    //! prefix increment
    OneDGridLevelIterator<codim,pitype,GridImp>& operator ++() {
      target_ = target_->succ_;
      return (*this);
    }

    //! equality
    bool operator== (const OneDGridLevelIterator<codim, pitype, GridImp>& i) const {
      return i.target_ == target_;
    }

    //! inequality
    bool operator!= (const OneDGridLevelIterator<codim, pitype, GridImp>& i) const {
      return i.target_ != target_;
    }

    //! dereferencing
    OneDGridEntity<codim,dim,GridImp>& operator*() {return *target_;}

    //! arrow
    OneDGridEntity<codim,dim,GridImp>* operator->() {return target_;}

    //! ask for level of entity
    int level () const {return target_->level();}

  private:

    OneDGridEntity<codim,dim,GridImp>* target_;

  };

}  // namespace Dune

#endif
