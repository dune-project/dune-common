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
    public OneDGridEntityPointer <codim, GridImp>,
    public LevelIteratorDefaultImplementation <codim, pitype, GridImp, OneDGridLevelIterator>
  {
  public:
    enum {dim=GridImp::dimension};
    friend class OneDGridLevelIteratorFactory<codim>;
    friend class OneDGrid<dim,GridImp::dimensionworld>;
    friend class OneDGridEntity<codim,dim,GridImp>;
    friend class OneDGridEntity<0,dim,GridImp>;

    typedef typename GridImp::template Codim<codim>::Entity Entity;

  protected:

    /** \brief Constructor from a given iterator */
    OneDGridLevelIterator<codim,pitype, GridImp>(OneDEntityImp<dim-codim>* it)
      : OneDGridEntityPointer<codim, GridImp>(it)
    {}

  public:

    //! prefix increment
    void increment() {
      this->virtualEntity_.setToTarget(this->virtualEntity_.target()->succ_);
    }
  };

}  // namespace Dune

#endif
