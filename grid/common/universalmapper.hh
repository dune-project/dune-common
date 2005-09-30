// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef __DUNE_UNIVERSALMAPPER_HH__
#define __DUNE_UNIVERSALMAPPER_HH__

#include <iostream>
#include <map>
#include "mapper.hh"

/**
 * @file
 * @brief  Mapper for any set of entities
 * @author Peter Bastian
 */

namespace Dune
{
  /**
   * @addtogroup Mapper
   *
   * @{
   */

  /** @brief Implements a mapper for an arbitrary subset of entities

      This implementation uses an ID set and a map, thus it has log complexity for each access.
          Template parameters are:

          \par G
          A Dune grid type.
          \par IDS
          An Id set for the given grid
   */
  template <typename G, typename IDS>
  class UniversalMapper : Mapper<G,UniversalMapper<G,IDS> > {
    typedef typename IDS::IdType IdType;
  public:

    /** @brief Construct mapper from grid and one of its id sets

       \param grid A Dune grid object.
       \param indexset IndexSet object returned by grid.

     */
    UniversalMapper (const G& grid, const IDS& idset)
      : g(grid), ids(idset), index()
    {
      n=0;     // zero data elements
    }

    /** @brief Map entity to array index.

            \param e Reference to codim cc entity, where cc is the template parameter of the function.
            \return An index in the range 0 ... Max number of entities in set - 1.
     */
    template<class EntityType>
    int map (const EntityType& e) const
    {
      IdType id = ids.id(e);                                 // get id
      typename std::map<IdType,int>::iterator it = index.find(id);    // look up in map
      if (it!=index.end()) return it->second;                // return index if found
      index[id] = n++;                                       // put next index in map
      return n-1;                                            // and return it
    }


    /** @brief Map subentity of codim 0 entity to array index.

       \param e Reference to codim 0 entity.
       \param i Number of codim cc subentity of e, where cc is the template parameter of the function.
       \return An index in the range 0 ... Max number of entities in set - 1.
     */
    template<int cc>
    int map (const typename G::Traits::template Codim<0>::Entity& e, int i) const
    {
      IdType id = ids.template subid<cc>(e,i);               // get id
      typename std::map<IdType,int>::iterator it = index.find(id);    // look up in map
      if (it!=index.end()) return it->second;                // return index if found
      index[id] = n++;                                       // put next index in map
      return n-1;                                            // and return it
    }

    /** @brief Return total number of entities in the entity set managed by the mapper.

       This number can be used to allocate a vector of data elements associated with the
       entities of the set. In the parallel case this number is per process (i.e. it
       may be different in different processes).

       \return Size of the entity set.
     */
    int size () const
    {
      return n;
    }

    /** @brief Returns true if the entity is contained in the index set

       \param e Reference to entity
       \param result integer reference where corresponding index is  stored if true
       \return true if entity is in entity set of the mapper
     */
    template<class EntityType>
    bool contains (const EntityType& e, int& result) const
    {
      IdType id = ids.id(e);                                 // get id
      typename std::map<IdType,int>::iterator it = index.find(id);    // look up in map
      if (it!=index.end())
      {
        result = it->second;
        return true;
      }
      else
        return false;
    }

    /** @brief Returns true if the entity is contained in the index set

       \param e Reference to codim 0 entity
       \param i subentity number
       \param result integer reference where corresponding index is  stored if true
       \return true if entity is in entity set of the mapper
     */
    template<int cc>     // this is now the subentity's codim
    bool contains (const typename G::Traits::template Codim<0>::Entity& e, int i, int& result) const
    {
      IdType id = ids.template subid<cc>(e,i);               // get id
      typename std::map<IdType,int>::iterator it = index.find(id);    // look up in map
      if (it!=index.end())
      {
        result = it->second;
        return true;
      }
      else
        return false;
    }

    /** @brief Recalculates map after mesh adaptation
     */
    void update ()
    {     // nothing to do here
    }

    // clear the mapper
    void clear ()
    {
      index.clear();
      n = 0;
    }

  private:
    mutable int n;     // number of data elements required
    const G& g;
    const IDS& ids;
    mutable std::map<IdType,int> index;
  };




  /** @brief Universal mapper based on global ids

     Template parameters are:

     \par G
     A Dune grid type.
   */
  template <typename G>
  class GlobalUniversalMapper : public UniversalMapper<G,typename G::Traits::GlobalIdSet>
  {
  public:
    /* @brief The constructor
       @param grid A reference to a grid.
     */
    GlobalUniversalMapper (const G& grid)
      : UniversalMapper<G,typename G::Traits::GlobalIdSet>(grid,grid.globalIdSet())
    {}
  };

  /** @brief Universal mapper based on local ids

     Template parameters are:

     \par G
     A Dune grid type.
   */
  template <typename G>
  class LocalUniversalMapper : public UniversalMapper<G,typename G::Traits::LocalIdSet>
  {
  public:
    /* @brief The constructor
       @param grid A reference to a grid.
     */
    LocalUniversalMapper (const G& grid)
      : UniversalMapper<G,typename G::Traits::LocalIdSet>(grid,grid.localIdSet())
    {}
  };


  /** @} */
}
#endif
