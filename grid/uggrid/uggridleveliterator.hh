// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRIDLEVELITERATOR_HH
#define DUNE_UGGRIDLEVELITERATOR_HH

/** \file
 * \brief The UGGridLevelIterator class and its specializations
 */

namespace Dune {

  // Forward declaration
  template <int codim, int dim, int dimworld, PartitionIteratorType PiType>
  class UGGridLevelIteratorFactory;

  //**********************************************************************
  //
  // --UGGridLevelIterator
  // --LevelIterator
  /** \brief Iterator over all entities of a given codimension and level of a grid.
   * \ingroup UGGrid
   */
  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  class UGGridLevelIterator :
    public LevelIteratorDefault <codim,dim,dimworld, pitype, UGCtype,
        UGGridLevelIterator,UGGridEntity>
  {
    friend class UGGridEntity<codim,dim,dimworld>;
    friend class UGGridEntity<0,dim,dimworld>;

    friend class UGGridLevelIteratorFactory<2,2,2,All_Partition>;
    friend class UGGridLevelIteratorFactory<0,2,2,All_Partition>;
    friend class UGGridLevelIteratorFactory<3,3,3,All_Partition>;
    friend class UGGridLevelIteratorFactory<0,3,3,All_Partition>;

  public:

    //! Constructor
    explicit UGGridLevelIterator(int travLevel);

    //! prefix increment
    UGGridLevelIterator<codim,dim,dimworld,pitype>& operator ++();

    //! equality
    bool operator== (const UGGridLevelIterator<codim,dim,dimworld, pitype>& i) const;

    //! inequality
    bool operator!= (const UGGridLevelIterator<codim,dim,dimworld, pitype>& i) const;

    //! dereferencing
    UGGridEntity<codim,dim,dimworld>& operator*() ;

    //! arrow
    UGGridEntity<codim,dim,dimworld>* operator->() ;

    //! ask for level of entity
    int level ();

  private:

    void makeIterator();

    void setToTarget(typename TargetType<codim,dim>::T* target) {
      target_ = target;
      virtualEntity_.setToTarget(target);
    }

    void setToTarget(typename TargetType<codim,dim>::T* target, int level) {
      target_ = target;
      level_  = level;
      virtualEntity_.setToTarget(target, level);
    }

    // private Members
    UGGridEntity<codim,dim,dimworld> virtualEntity_;

    //! element number
    int elNum_;

    //! level
    int level_;

    typename TargetType<codim,dim>::T* target_;

  };

  // Include method definitions
#include "uggridleveliterator.cc"

}  // namespace Dune

#endif
