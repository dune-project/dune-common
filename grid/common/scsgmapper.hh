// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef __DUNE_SCSGMAPPER_HH__
#define __DUNE_SCSGMAPPER_HH__

#include <iostream>
#include "mapper.hh"

/**
 * @file
 * @brief  Mapper classes are used to attach data to a grid
 * @author Peter Bastian
 */

namespace Dune
{
  /**
   * @addtogroup Mapper
   *
   * @{
   */

  /** @brief Implementation class for a single codim and single geometry type mapper.
   *
   * In this implementation of a mapper the entity set used as domain for the map consists
   * of the entities of a given codimension c for all entities in the given index set. The index
   * set may only contain entities of a single geometry type, otherwise an exception is thrown. This
   * version is usually not used directly but is used to implement versions for leafwise and levelwise
   * entity sets.
   *
   * Template parameters are:
   *
   * \par G
   *    A Dune grid type.
   * \par IS
   *    LeafIndexSet or LevelIndexSet type of the given grid.
   * \par c
   *    A valid codimension.
   */
  template <typename G, typename IS, int c>
  class SingleCodimSingleGeomTypeMapper : Mapper<G,SingleCodimSingleGeomTypeMapper<G,IS,c> > {
  public:

    /** @brief Construct mapper from grid and one fo its index sets.

       \param grid A Dune grid object.
       \param indexset IndexSet object returned by grid.

     */
    SingleCodimSingleGeomTypeMapper (const G& grid, const IS& indexset);

    /** @brief Map entity to array index.

            \param e Reference to codim cc entity, where cc is the template parameter of the function.
            \return An index in the range 0 ... Max number of entities in set - 1.
     */
    template<class EntityType>
    int map (const EntityType& e) const;

    /** @brief Map subentity of codim 0 entity to array index.

       \param e Reference to codim 0 entity.
       \param i Number of codim cc subentity of e, where cc is the template parameter of the function.
       \return An index in the range 0 ... Max number of entities in set - 1.
     */
    template<int cc>
    int map (const typename G::Traits::template Codim<0>::Entity& e, int i) const;

    /** @brief Return total number of entities in the entity set managed by the mapper.

       This number can be used to allocate a vector of data elements associated with the
       entities of the set. In the parallel case this number is per process (i.e. it
       may be different in different processes).

       \return Size of the entity set.
     */
    int size () const;

    /** @brief Returns true if the entity is contained in the index set

       \param e Reference to entity
       \param result integer reference where corresponding index is  stored if true
       \return true if entity is in entity set of the mapper
     */
    template<class EntityType>
    bool contains (const EntityType& e, int& result) const;

    /** @brief Returns true if the entity is contained in the index set

       \param e Reference to codim 0 entity
       \param i subentity number
       \param result integer reference where corresponding index is  stored if true
       \return true if entity is in entity set of the mapper
     */
    template<int cc>     // this is now the subentity's codim
    bool contains (const typename G::Traits::template Codim<0>::Entity& e, int i, int& result) const;

    /** @brief Recalculates map after mesh adaptation
     */
    void update ()
    {     // nothing to do here
    }

  private:
    const G& g;
    const IS& is;
  };

  /** @} */

  template <typename G, typename IS, int c>
  SingleCodimSingleGeomTypeMapper<G,IS,c>::SingleCodimSingleGeomTypeMapper (const G& grid, const IS& indexset)
    : g(grid), is(indexset)
  {
    // check that grid has only a single geometry type
    if (is.geomTypes(c).size() != 1)
      DUNE_THROW(GridError, "mapper treats only a single codim and a single geometry type");
  }

  template <typename G, typename IS, int c>
  template<class EntityType>
  int SingleCodimSingleGeomTypeMapper<G,IS,c>::map (const EntityType& e) const
  {
    enum { cc = EntityType::codimension };
    IsTrue< cc == c >::yes();
    return is.template index<cc>(e);
  }

  template <typename G, typename IS, int c>
  template<int cc>
  int SingleCodimSingleGeomTypeMapper<G,IS,c>::map (const typename G::Traits::template Codim<0>::Entity& e, int i) const
  {
    IsTrue< cc == c >::yes();
    return is.template subIndex<cc>(e,i);
  }

  template <typename G, typename IS, int c>
  int SingleCodimSingleGeomTypeMapper<G,IS,c>::size () const
  {
    return is.size(c,is.geomTypes(c)[0]);
  }

  template <typename G, typename IS, int c>
  template<class EntityType>
  bool SingleCodimSingleGeomTypeMapper<G,IS,c>::contains (const EntityType& e, int& result) const
  {
    result = map(e);
    return true;
  }

  template <typename G, typename IS, int c>
  template<int cc>
  bool SingleCodimSingleGeomTypeMapper<G,IS,c>::contains (const typename G::Traits::template Codim<0>::Entity& e, int i, int& result) const
  {
    result = this->template map<cc>(e,i);
    return true;
  }

  /**
   * @addtogroup Mapper
   *
   * @{
   */
  /** @brief Single codim and single geometry type mapper for leaf entities.


     This mapper uses all leaf entities of a certain codimension as its entity set. It is
     assumed (and checked) that the given grid contains only entities of a single geometry type.

     Template parameters are:

     \par G
     A Dune grid type.
     \par c
     A valid codimension.
   */
  template <typename G, int c>
  class LeafSingleCodimSingleGeomTypeMapper : public SingleCodimSingleGeomTypeMapper<G,typename G::Traits::LeafIndexSet,c> {
  public:
    /* @brief The constructor
       @param grid A reference to a grid.
     */
    LeafSingleCodimSingleGeomTypeMapper (const G& grid)
      : SingleCodimSingleGeomTypeMapper<G,typename G::Traits::LeafIndexSet,c>(grid,grid.leafIndexSet())
    {}
  };

  /** @brief Single codim and single geometry type mapper for entities of one level.


     This mapper uses all entities of a certain codimension on a given level as its entity set. It is
     assumed (and checked) that the given grid contains only entities of a single geometry type.

     Template parameters are:

     \par G
     A Dune grid type.
     \par c
     A valid codimension.
   */
  template <typename G, int c>
  class LevelSingleCodimSingleGeomTypeMapper : public SingleCodimSingleGeomTypeMapper<G,typename G::Traits::LevelIndexSet,c> {
  public:
    /* @brief The constructor
       @param grid A reference to a grid.
       @param level A valid level of the grid.
     */
    LevelSingleCodimSingleGeomTypeMapper (const G& grid, int level)
      : SingleCodimSingleGeomTypeMapper<G,typename G::Traits::LevelIndexSet,c>(grid,grid.levelIndexSet(level))
    {}
  };

  /** @} */
}
#endif
