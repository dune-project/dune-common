// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONEDGRID_INDEXSETS_HH
#define DUNE_ONEDGRID_INDEXSETS_HH

/** \file
    \brief The index and id sets for the OneDGrid class
 */

#include <vector>

namespace Dune {

  template <class GridImp>
  struct OneDGridLevelIndexSetTypes
  {
    //! The types
    template<int cd>
    struct Codim
    {
      template<PartitionIteratorType pitype>
      struct Partition
      {
        typedef typename GridImp::Traits::template Codim<cd>::template Partition<pitype>::LevelIterator Iterator;
      };
    };
  };


  template<class GridImp>
  class OneDGridLevelIndexSet : public IndexSet<GridImp,OneDGridLevelIndexSet<GridImp>,OneDGridLevelIndexSetTypes<GridImp> >
  {
    typedef IndexSet<GridImp,OneDGridLevelIndexSet<GridImp>,OneDGridLevelIndexSetTypes<GridImp> > Base;
  public:

    /** \brief Default constructor

       Unfortunately we can't force the user to init grid_ and level_, because
       OneDGridLevelIndexSets are meant to be stored in an array.
     */
    OneDGridLevelIndexSet () {}

    //! get index of an entity
    template<int cd>
    int index (const typename GridImp::Traits::template Codim<cd>::Entity& e) const
    {
      return grid_->template getRealEntity<cd>(e).levelIndex();
    }

    //! get index of subentity of a codim 0 entity
    template<int cc>
    int subIndex (const typename GridImp::Traits::template Codim<0>::Entity& e, int i) const
    {
      return grid_->template getRealEntity<0>(e).template subIndex<cc>(i);
    }

    //! get number of entities of given codim, type and on this level
    int size (int codim, GeometryType type) const
    {
      return grid_->size(level_,codim, type);
    }

    //! get number of entities of given codim, type and on this level
    int size (int codim) const
    {
      return grid_->size(level_,codim);
    }

    /** \brief Deliver all geometry types used in this grid */
    const std::vector<GeometryType>& geomTypes (int codim) const
    {
      return myTypes_;
    }

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Base::template Codim<cd>::template Partition<pitype>::Iterator begin () const
    {
      return grid_->template lbegin<cd,pitype>(level_);
    }

    //! Iterator to one past the last entity of given codim on level for partition type
    template<int cd, PartitionIteratorType pitype>
    typename Base::template Codim<cd>::template Partition<pitype>::Iterator end () const
    {
      return grid_->template lend<cd,pitype>(level_);
    }

    /** \todo Should be private */
    void update(const GridImp& grid, int level) {

      // Commit the index set to a specific level of a specific grid
      grid_ = &grid;
      level_ = level;
      // ///////////////////////////////
      //   Init the element indices
      // ///////////////////////////////
      int numElements = 0;
      OneDEntityImp<1>* eIt;
      for (eIt = grid_->elements[level].begin; eIt!=NULL; eIt = eIt->succ_)
        eIt->levelIndex_ = numElements++;

      // Update the list of geometry types present
      if (numElements>0) {
        myTypes_.resize(1);
        myTypes_[0] = cube;
      } else
        myTypes_.resize(0);

      // //////////////////////////////
      //   Init the vertex indices
      // //////////////////////////////

      int id = 0;
      OneDEntityImp<0>* vIt;
      for (vIt = grid_->vertices[level].begin; vIt!=NULL; vIt = vIt->succ_)
        vIt->levelIndex_ = id++;

    }

  private:
    const GridImp* grid_;
    int level_;
    std::vector<GeometryType> myTypes_;
  };

  template <class GridImp>
  struct OneDGridLeafIndexSetTypes
  {
    //! The types
    template<int cd>
    struct Codim
    {
      template<PartitionIteratorType pitype>
      struct Partition
      {
        typedef typename GridImp::Traits::template Codim<cd>::template Partition<pitype>::LeafIterator Iterator;
      };
    };
  };

  template<class GridImp>
  class OneDGridLeafIndexSet : public IndexSet<GridImp,OneDGridLeafIndexSet<GridImp>,OneDGridLeafIndexSetTypes<GridImp> >
  {
    typedef IndexSet<GridImp,OneDGridLevelIndexSet<GridImp>,OneDGridLeafIndexSetTypes<GridImp> > Base;
  public:
    //! constructor stores reference to a grid and level
    OneDGridLeafIndexSet (const GridImp& g) : grid_(g)
    {}

    //! get index of an entity
    /*
       We use the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
     */
    template<int cd>
    int index (const typename RemoveConst<GridImp>::Type::Traits::template Codim<cd>::Entity& e) const
    {
      return grid_.template getRealEntity<cd>(e).leafIndex();
    }

    //! get index of subentity of a codim 0 entity
    /*
       We use the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
     */
    template<int cc>
    int subIndex (const typename RemoveConst<GridImp>::Type::Traits::template Codim<0>::Entity& e, int i) const
    {
      return grid_.template getRealEntity<0>(e).template subLeafIndex<cc>(i);
    }

    //! get number of entities of given codim, type on the leaf level
    int size (int codim, GeometryType type) const
    {
      if (codim==GridImp::dimension) {

        return numVertices_;

      } else if (codim==0) {

        switch (type) {
        case simplex :
        case cube :
          return numElements_;
        default :
          return 0;
        }

      } else {
        DUNE_THROW(NotImplemented, "UGGridLeafIndexSet::size(codim,type) for codim neither 0 nor dim");
      }
    }

    /** deliver all geometry types used in this grid */
    const std::vector<GeometryType>& geomTypes (int codim) const
    {
      return myTypes_;
    }

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Base::template Codim<cd>::template Partition<pitype>::Iterator begin () const
    {
      return grid_.template leafbegin<cd,pitype>();
    }

    //! Iterator to one past the last entity of given codim on level for partition type
    template<int cd, PartitionIteratorType pitype>
    typename Base::template Codim<cd>::template Partition<pitype>::Iterator end () const
    {
      return grid_.template leafend<cd,pitype>();
    }

    /** \todo Should be private */
    void update() {

      // ///////////////////////////////
      //   Init the element indices
      // ///////////////////////////////
      numElements_ = 0;
      typename GridImp::Traits::template Codim<0>::LeafIterator eIt    = grid_.template leafbegin<0>();
      typename GridImp::Traits::template Codim<0>::LeafIterator eEndIt = grid_.template leafend<0>();

      for (; eIt!=eEndIt; ++eIt) {

        grid_.template getRealEntity<0>(*eIt).target_->leafIndex_ = numElements_++;

      }

      // Update the list of geometry types present
      if (numElements_>0) {
        myTypes_.resize(1);
        myTypes_[0] = cube;
      } else
        myTypes_.resize(0);


      // //////////////////////////////
      //   Init the vertex indices
      // //////////////////////////////
      typename GridImp::Traits::template Codim<1>::LeafIterator vIt    = grid_.template leafbegin<1>();
      typename GridImp::Traits::template Codim<1>::LeafIterator vEndIt = grid_.template leafend<1>();

      numVertices_ = 0;
      for (; vIt!=vEndIt; ++vIt)
        grid_.template getRealEntity<1>(*vIt).target_->leafIndex_ = numVertices_++;

    }

  private:

    const GridImp& grid_;

    int numElements_;
    int numVertices_;

    std::vector<GeometryType> myTypes_;
  };


  template<class GridImp>
  class OneDGridIdSet : public IdSet<GridImp,OneDGridIdSet<GridImp>,unsigned int>
  {
  public:
    //! define the type used for persistent indices
    typedef unsigned int GlobalIdType;
    typedef unsigned int LocalIdType;

    //! constructor stores reference to a grid
    OneDGridIdSet (const GridImp& g) : grid_(g) {}

    //! get id of an entity
    /*
       We use the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
     */
    template<int cd>
    GlobalIdType id (const typename RemoveConst<GridImp>::Type::Traits::template Codim<cd>::Entity& e) const
    {
      return grid_.template getRealEntity<cd>(e).globalId();
    }

    //! get id of subentity
    /*
       We use the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
     */
    template<int cc>
    GlobalIdType subid (const typename RemoveConst<GridImp>::Type::Traits::template Codim<0>::Entity& e, int i) const
    {
      DUNE_THROW(NotImplemented, "UGGridGlobalIdSet::subid");
      //return grid.template getRealEntity<0>(e).template subPersistentIndex<cc>(i);
    }

    /** \todo Should be private */
    void update() {}

  private:

    const GridImp& grid_;
  };



}  // namespace Dune


#endif
