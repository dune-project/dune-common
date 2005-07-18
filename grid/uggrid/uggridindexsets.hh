// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRID_INDEXSETS_HH
#define DUNE_UGGRID_INDEXSETS_HH

/** \file
    \brief The index and id sets for the UGGrid class
 */

#include <vector>

namespace Dune {

  template<class GridImp>
  class UGGridLevelIndexSet
  {
  public:
    //! constructor stores reference to a grid and level
    UGGridLevelIndexSet (const GridImp& g, int l) : grid_(g), level_(l)
    {}

    //! get index of an entity
    template<int cd>
    int index (const typename GridImp::Traits::template Codim<cd>::Entity& e) const
    {
      //return grid.template getRealEntity<cd>(e).compressedIndex();
    }

    //! get index of subentity of a codim 0 entity
    template<int cc>
    int subindex (const typename GridImp::Traits::template Codim<0>::Entity& e, int i) const
    {
      //return grid.template getRealEntity<0>(e).template subCompressedIndex<cc>(i);
    }

    //! get number of entities of given codim, type and on this level
    int size (int codim, GeometryType type) const
    {
      return grid_.size(level_,codim, type);
    }

    /** \brief Deliver all geometry types used in this grid
        \todo All types used or just on this level? */
    const std::vector<GeometryType>& geomtypes () const
    {
      return myTypes_;
    }

  private:
    const GridImp& grid_;
    int level_;
    std::vector<GeometryType> myTypes_;
  };

  template<class GridImp>
  class UGGridLeafIndexSet
  {
  public:
    //! constructor stores reference to a grid and level
    UGGridLeafIndexSet (const GridImp& g) : grid(g)
    {
      //mytypes.push_back(cube); // contains a single element type;
    }

    //! get index of an entity
    template<int cd>
    int index (const typename GridImp::Traits::template Codim<cd>::Entity& e) const
    {
      //return grid.template getRealEntity<cd>(e).compressedIndex();
    }

    //! get index of subentity of a codim 0 entity
    template<int cc>
    int subindex (const typename GridImp::Traits::template Codim<0>::Entity& e, int i) const
    {
      //return grid.template getRealEntity<0>(e).template subCompressedIndex<cc>(i);
    }

    //! get number of entities of given codim, type and level (the level is known to the object)
    int size (int codim, GeometryType type) const
    {
      return grid.size(codim, type);
    }

    //! deliver all geometry types used in this grid
    const std::vector<GeometryType>& geomtypes () const
    {
      return mytypes;
    }

  private:
    const GridImp& grid;
    std::vector<GeometryType> mytypes;
  };


  template<class GridImp>
  class UGGridGlobalIdSet
  {
  public:
    //! define the type used for persistent indices
    typedef int GlobalIdType;

    //! constructor stores reference to a grid
    UGGridGlobalIdSet (const GridImp& g) : grid_(g) {}

    //! get id of an entity
    template<int cd>
    GlobalIdType id (const typename GridImp::Traits::template Codim<cd>::Entity& e) const
    {
      //return grid.template getRealEntity<cd>(e).persistentIndex();
    }

    //! get id of subentity
    template<int cc>
    GlobalIdType subid (const typename GridImp::Traits::template Codim<0>::Entity& e, int i) const
    {
      //return grid.template getRealEntity<0>(e).template subPersistentIndex<cc>(i);
    }

  private:
    const GridImp& grid_;
  };


  template<class GridImp>
  class UGGridLocalIdSet
  {
  public:
    //! define the type used for persistent local ids
    typedef int LocalIdType;

    //! constructor stores reference to a grid
    UGGridLocalIdSet (const GridImp& g) : grid_(g) {}

    //! get id of an entity
    template<int cd>
    LocalIdType id (const typename GridImp::Traits::template Codim<cd>::Entity& e) const
    {
      //return grid.template getRealEntity<cd>(e).persistentIndex();
    }

    //! get id of subentity
    template<int cc>
    LocalIdType subid (const typename GridImp::Traits::template Codim<0>::Entity& e, int i) const
    {
      //return grid.template getRealEntity<0>(e).template subPersistentIndex<cc>(i);
    }

  private:
    const GridImp& grid_;
  };


}  // namespace Dune


#endif
