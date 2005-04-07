// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRIDLEVELITERATOR_HH
#define DUNE_UGGRIDLEVELITERATOR_HH

/** \file
 * \brief The UGGridLevelIterator class and its specializations
 */

namespace Dune {

  // Forward declaration
  template <int codim, PartitionIteratorType PiType, class GridImp>
  class UGGridLevelIteratorFactory;

  //**********************************************************************
  //
  // --UGGridLevelIterator
  // --LevelIterator
  /** \brief Iterator over all entities of a given codimension and level of a grid.
   * \ingroup UGGrid
   */
  template<int codim, PartitionIteratorType pitype, class GridImp>
  class UGGridLevelIterator :
    public Dune::UGGridEntityPointer <codim,GridImp>,
    public LevelIteratorDefault <codim,pitype,GridImp,UGGridLevelIterator>
  {
    friend class UGGridEntity<codim,GridImp::dimension,GridImp>;
    friend class UGGridEntity<0,    GridImp::dimension,GridImp>;

    template <int codim_, PartitionIteratorType PiType_, class GridImp_>
    friend class UGGridLevelIteratorFactory;

  public:

    typedef typename GridImp::template codim<codim>::Entity Entity;

    //! Constructor
    explicit UGGridLevelIterator(int travLevel) /*: virtualEntity_(0),
                                                    level_(travLevel),
                                                    target_(0) */
    {
      this->virtualEntity_.setToTarget(0);
    }

    //! prefix increment
    void increment() {
      //setToTarget(UG_NS<GridImp::dimension>::succ(target_));
      this->virtualEntity_.setToTarget(UG_NS<GridImp::dimension>::succ(this->virtualEntity_.getTarget()));
    }

#if 0
    //! equality
    bool equals(const UGGridLevelIterator<codim,pitype,GridImp>& other) const {
      return target_ == other.target_;
    }

    //! dereferencing
    Entity& dereference() const {return virtualEntity_;}

    //! ask for level of entity
    int level () const {return level_;}
#endif

  private:

    /** \todo Move this to base class */
    void setToTarget(typename TargetType<codim,GridImp::dimension>::T* target) {
      //target_ = target;
      this->virtualEntity_.setToTarget(target);
    }

    /** \todo Move this to base class */
    void setToTarget(typename TargetType<codim,GridImp::dimension>::T* target, int level) {
      //target_ = target;
      //level_  = level;
      this->virtualEntity_.setToTarget(target, level);
    }

#if 0
    // private Members
    mutable UGMakeableEntity<codim,GridImp::dimension,GridImp> virtualEntity_;

    //! level
    int level_;

    typename TargetType<codim,GridImp::dimension>::T* target_;
#endif
  };

}  // namespace Dune

#endif
