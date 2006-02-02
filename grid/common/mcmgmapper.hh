// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_MCMGMAPPER_HH
#define DUNE_MCMGMAPPER_HH

#include <iostream>
#include <map>
#include "mapper.hh"
#include "referenceelements.hh"

/**
 * @file
 * @brief  Mapper for multiple codim and multiple geometry types
 * @author Peter Bastian
 */

namespace Dune
{
  /**
   * @addtogroup Mapper
   *
   * @{
   */

  /** @brief Implementation class for a multiple codim and multiple geometry type mapper.
   *
   * In this implementation of a mapper the entity set used as domain for the map consists
   * of the entities of a subset of codimensions in the given index set. The index
   * set may contain entities of several geometry types. This
   * version is usually not used directly but is used to implement versions for leafwise and levelwise
   * entity sets.
   *
   * Template parameters are:
   *
   * \par G
   *    A Dune grid type.
   * \par IS
   *    LeafIndexSet or LevelIndexSet type of the given grid.
   * \par Layout
   *    A class template with a method contains, that returns true for all codims and geometry
   *    types that are in the map.
   */
  template <typename G, typename IS, template<int> class Layout>
  class MultipleCodimMultipleGeomTypeMapper : Mapper<G,MultipleCodimMultipleGeomTypeMapper<G,IS,Layout> > {
  public:

    /** @brief Construct mapper from grid and one fo its index sets.

       \param grid A Dune grid object.
       \param indexset IndexSet object returned by grid.

     */
    MultipleCodimMultipleGeomTypeMapper (const G& grid, const IS& indexset)
      : g(grid), is(indexset)
    {
      update();
    }

    /** @brief Map entity to array index.

            \param e Reference to codim cc entity, where cc is the template parameter of the function.
            \return An index in the range 0 ... Max number of entities in set - 1.
     */
    template<class EntityType>
    int map (const EntityType& e) const
    {
      enum { cc = EntityType::codimension };
      return is.index(e) + offset[cc].find(e.geometry().type())->second;
    }


    /** @brief Map subentity of codim 0 entity to array index.

       \param e Reference to codim 0 entity.
       \param i Number of codim cc subentity of e, where cc is the template parameter of the function.
       \return An index in the range 0 ... Max number of entities in set - 1.
     */
    template<int cc>
    int map (const typename G::Traits::template Codim<0>::Entity& e, int i) const
    {

      NewGeometryType gt=ReferenceElements<double,G::dimension>::general(e.geometry().type()).type(i,cc);
      //	  std::cout << "map: cc=" << cc << " gt=" << gt << " offset=" << offset[cc].find(gt)->second << std::endl;
      return is.template subIndex<cc>(e,i) + offset[cc].find(gt)->second;
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
      result = map(e);
      return true;
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
      result = this->template map<cc>(e,i);
      return true;
    }


    /** @brief Recalculates map after mesh adaptation
     */
    void update ()
    {
      // get layout object;
      Layout<G::dimension> layout;

      n=0;     // zero data elements
      for (int c=0; c<=G::dimension; c++)
        offset[c].clear();         // clear all maps

      // Compute offsets for the different geometry types.
      // Note that mapper becomes invalid when the grid is modified.
      for (int c=0; c<=G::dimension; c++)
        for (size_t i=0; i<is.geomTypes(c).size(); i++)
          if (layout.contains(c,is.geomTypes(c)[i]))
          {
            //			  std::cout << "offset " << c << " " << is.geomTypes(c)[i] << " is " << n << std::endl;
            if (c<G::dimension-1)
            {
              offset[c][is.geomTypes(c)[i]] = n;
              n += is.size(c,is.geomTypes(c)[i]);
            }
            else               // the grid is only allowed to deliver one geometry type !
            {
              // put entry with arbitrary BasicType in the map because they are equal
              offset[c][NewGeometryType(NewGeometryType::cube,G::dimension-c)] = n;
              n += is.size(c,is.geomTypes(c)[i]);
            }
          }
    }

  private:
    int n;     // number of data elements required
    const G& g;
    const IS& is;
    std::map<NewGeometryType,int> offset[G::dimension+1];     // for each codim provide a map with all geometry types
  };




  /** @brief Multiple codim and multiple geometry type mapper for leaf entities.

     This mapper uses all leaf entities of a certain codimension as its entity set.

     Template parameters are:

     \par G
     A Dune grid type.
     \par Layout
     A class template with a method contains, that returns true for all codims and geometry
     types that are in the map.
   */
  template <typename G, template<int> class Layout>
  class LeafMultipleCodimMultipleGeomTypeMapper
    : public MultipleCodimMultipleGeomTypeMapper<G,typename G::Traits::LeafIndexSet,Layout>
  {
  public:
    /* @brief The constructor
       @param grid A reference to a grid.
     */
    LeafMultipleCodimMultipleGeomTypeMapper (const G& grid)
      : MultipleCodimMultipleGeomTypeMapper<G,typename G::Traits::LeafIndexSet,Layout>(grid,grid.leafIndexSet())
    {}
  };

  /** @brief Multiple codim and multiple geometry type mapper for entities of one level.


     This mapper uses all entities of a certain codimension on a given level as its entity set.

     Template parameters are:

     \par G
     A Dune grid type.
     \par Layout
     A class template with a method contains, that returns true for all codims and geometry
     types that are in the map.
   */
  template <typename G, template<int> class Layout>
  class LevelMultipleCodimMultipleGeomTypeMapper
    : public MultipleCodimMultipleGeomTypeMapper<G,typename G::Traits::LevelIndexSet,Layout> {
  public:
    /* @brief The constructor
       @param grid A reference to a grid.
       @param level A valid level of the grid.
     */
    LevelMultipleCodimMultipleGeomTypeMapper (const G& grid, int level)
      : MultipleCodimMultipleGeomTypeMapper<G,typename G::Traits::LevelIndexSet,Layout>(grid,grid.levelIndexSet(level))
    {}
  };

  /** @} */
}
#endif
