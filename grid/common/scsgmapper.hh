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

  /** @brief Implementation base class for a single codim and single geometry type mapper.
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
    template<int cc>     // this is necessary for multiple codim mappers
    int map (const typename G::Traits::template Codim<cc>::Entity& e) const;

    /** @brief Map subentity of codim 0 entity to array index.

       \param e Reference to codim 0 entity.
       \param i Number of codim cc subentity of e, where cc is the template parameter of the function.
       \return An index in the range 0 ... Max number of entities in set - 1.
     */
    template<int cc>     // this is now the subentity's codim
    int submap (const typename G::Traits::template Codim<0>::Entity& e, int i) const;

    /** @brief Return total number of entities in the entity set managed by the mapper.

       This number can be used to allocate a vector of data elements associated with the
       entities of the set. In the parallel case this number is per process (i.e. it
       may be different in different processes).

       \return Size of the entity set.
     */
    int size () const;

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
    if (is.geomtypes().size() != 1)
      DUNE_THROW(GridError, "mapper treats only a single codim and a single geometry type");
  }

  template <typename G, typename IS, int c>
  template<int cc>
  int SingleCodimSingleGeomTypeMapper<G,IS,c>::map (const typename G::Traits::template Codim<cc>::Entity& e) const
  {
    IsTrue< cc == c >::yes();
    return is.template index<cc>(e);
  }

  template <typename G, typename IS, int c>
  template<int cc>
  int SingleCodimSingleGeomTypeMapper<G,IS,c>::submap (const typename G::Traits::template Codim<0>::Entity& e, int i) const
  {
    IsTrue< cc == c >::yes();
    return is.template subindex<cc>(e,i);
  }

  template <typename G, typename IS, int c>
  int SingleCodimSingleGeomTypeMapper<G,IS,c>::size () const
  {
    return is.size(c,is.geomtypes()[0]);
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
      : SingleCodimSingleGeomTypeMapper<G,typename G::Traits::LeafIndexSet,c>(grid,grid.leafindexset())
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
      : SingleCodimSingleGeomTypeMapper<G,typename G::Traits::LevelIndexSet,c>(grid,grid.levelindexset(level))
    {}
  };

  /** @} */
}
#endif
