// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef __DUNE_INDEXIDSET_HH__
#define __DUNE_INDEXIDSET_HH__

#include <iostream>
#include <vector>
#include <dune/common/exceptions.hh>
#include "dune/common/helpertemplates.hh"

/** @file
        @author Peter Bastian
        @brief Provides base classes for index and id sets
 */

/** @defgroup IndexIdSets Index and Id Sets
   \ingroup Grid

   @section Intro Introduction
   <!--====================-->

   still missing

 */


namespace Dune
{
  /**
   * @addtogroup IndexIdSets
   *
   * @{
   */


  /** @brief Index Set %Interface.

     This class template is used as a base class for all index set implementations.
     It uses the Barton-Nackman trick to ensure conformity to the interface.

     Template parameters are:

     \par GridImp
     A %Dune grid type.
     \par IndexSetImp
     An implementation of the index set interface.
   */
  template<class GridImp, class IndexSetImp>
  class IndexSet {
  public:

    /** @brief Map entity to index.

            \param e Reference to codim cc entity, where cc is the template parameter of the function.
            \return An index in the range 0 ... Max number of entities in set - 1.
     */
    template<int cc>
    int index (const typename GridImp::template Codim<cc>::Entity& e) const
    {
      return asImp().template index<cc>(e);
    }

    //! get index of an entity
    template<class EntityType>
    int index (const EntityType& e) const
    {
      enum { cc = EntityType::codimension };
      return asImp().template index<cc>(e);
    }

    /** @brief Map subentity of codim 0 entity to index.

       \param e Reference to codim 0 entity.
       \param i Number of codim cc subentity of e, where cc is the template parameter of the function.
       \return An index in the range 0 ... Max number of entities in set - 1.
     */
    template<int cc>
    int subIndex (const typename GridImp::template Codim<0>::Entity& e, int i) const
    {
      return asImp().template subIndex<cc>(e,i);
    }

    /** @brief Return total number of entities of given codim and type in this index set.

       \param codim A valid codimension.
       \param type A valid geometry type.

       \return number of entities.
     */
    int size (int codim, GeometryType type) const
    {
      return asImp().size(codim,type);
    }

    /** @brief Return vector with all geometry types of entities in domain of index map.

       \return reference to vector of geometry types.
     */
    const std::vector<GeometryType>& geomTypes () const
    {
      return asImp().geomTypes();
    }

  private:
    //!  Barton-Nackman trick
    IndexSetImp& asImp () {return static_cast<IndexSetImp &> (*this);}
    //!  Barton-Nackman trick
    const IndexSetImp& asImp () const {return static_cast<const IndexSetImp &>(*this);}
  };


  /** @brief Id Set %Interface.

     This class template is used as a base class for all id set implementations.
     It uses the Barton-Nackman trick to ensure conformity to the interface.

     Template parameters are:

     \par GridImp
     A %Dune grid type.
     \par IndexSetImp
     An implementation of the index set interface.
   */
  template<class GridImp, class IdSetImp, class IdTypeImp>
  class IdSet
  {
  public:
    //! define the type used for persisitent indices
    typedef IdTypeImp IdType;

    //! get id of an entity
    template<class EntityType>
    IdType id (const EntityType& e) const
    {
      enum { cc = EntityType::codimension };
      return asImp().template id<cc>(e);
    }

    //! get id of an entity of codim cc
    template<int cc>
    IdType id (const typename GridImp::template Codim<cc>::Entity& e) const
    {
      return asImp().template id<cc>(e);
    }

    //! get id of subentity i of codim cc
    template<int cc>
    IdType subid (const typename GridImp::template Codim<0>::Entity& e, int i) const
    {
      return asImp().template subid<cc>(e,i);
    }

  private:
    //!  Barton-Nackman trick
    IdSetImp& asImp () {return static_cast<IdSetImp &> (*this);}
    //!  Barton-Nackman trick
    const IdSetImp& asImp () const {return static_cast<const IdSetImp &>(*this);}
  };


  /** @} */

}
#endif
