// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONE_D_GRID_LEVELITERATOR_HH
#define DUNE_ONE_D_GRID_LEVELITERATOR_HH

/** \file
 * \brief The OneDGridLevelIterator class
 */

#include <dune/common/dlist.hh>

namespace Dune {

  // Forward declaration
  //     template <int codim, int dim, int dimworld, PartitionIteratorType PiType>
  //     class OneDGridLevelIteratorFactory;

  //**********************************************************************
  //
  // --OneDGridLevelIterator
  // --LevelIterator
  /** \brief Iterator over all entities of a given codimension and level of a grid.
   * \ingroup OneDGrid
   */
  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  class OneDGridLevelIterator :
    public LevelIteratorDefault <codim,dim,dimworld, pitype, OneDCType,
        OneDGridLevelIterator,OneDGridEntity>
  {
    friend class OneDGrid<dim,dimworld>;
    friend class OneDGridEntity<codim,dim,dimworld>;
    friend class OneDGridEntity<0,dim,dimworld>;

  protected:

    /** \brief Constructor from a given iterator */
    OneDGridLevelIterator<codim,dim,dimworld,pitype>(OneDGridEntity<codim,dim,dimworld>* it) {
      target_ = it;
    }

  public:

    //! prefix increment
    OneDGridLevelIterator<codim,dim,dimworld,pitype>& operator ++() {
      target_ = target_->succ_;
      return (*this);
    }

    //! equality
    bool operator== (const OneDGridLevelIterator<codim,dim,dimworld, pitype>& i) const {
      return i.target_ == target_;
    }

    //! inequality
    bool operator!= (const OneDGridLevelIterator<codim,dim,dimworld, pitype>& i) const {
      return i.target_ != target_;
    }

    //! dereferencing
    OneDGridEntity<codim,dim,dimworld>& operator*() {return *target_;}

    //! arrow
    OneDGridEntity<codim,dim,dimworld>* operator->() {return target_;}

    //! ask for level of entity
    int level () const {return target_->level();}

  private:

    OneDGridEntity<codim,dim,dimworld>* target_;

  };

}  // namespace Dune

#endif
