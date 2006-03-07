// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_INDEXIDSET_HH
#define DUNE_INDEXIDSET_HH

#include <iostream>
#include <vector>
#include <dune/common/exceptions.hh>
#include "dune/common/helpertemplates.hh"

/** @file
        @author Peter Bastian
        @brief Provides base classes for index and id sets
 */

/** @addtogroup IndexIdSets Index and Id Sets
        \ingroup Grid

        @section Intro Introduction

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
     \par IndexSetTypes
     A class providing the types returned by the methods of IndexSet.
   */
  template<class GridImp, class IndexSetImp, class IndexSetTypes>
  class IndexSet {
  public:
    /** @brief Define types needed to iterate over the entities in the index set
     */
    template <int cd>
    struct Codim
    {

      /** \brief Define types needed to iterate over entities of a given partition type */
      template <PartitionIteratorType pitype>
      struct Partition
      {
        /** \brief The iterator needed to iterate over the entities of a given codim and
            partition type of this index set */
        typedef typename IndexSetTypes::template Codim<cd>::template Partition<pitype>::Iterator Iterator;
      };
    };

    /** @brief Map entity to index.

            \param e Reference to codim cc entity, where cc is the template parameter of the function.
            \return An index in the range 0 ... Max number of entities in set - 1.
     */
    /*
       We use the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
     */
    template<int cc>
    int index (const typename RemoveConst<GridImp>::Type::
               Traits::template Codim<cc>::Entity& e) const
    {
      return asImp().template index<cc>(e);
    }

    /** @brief Map entity to index.

            \param e Reference to codim cc entity. Since
           entity knows its codimension, automatic extraction is possible.
            \return An index in the range 0 ... Max number of entities in set - 1.
     */
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
    /*
       We use the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
     */
    template<int cc>
    int subIndex (const typename RemoveConst<GridImp>::Type::
                  Traits::template Codim<0>::Entity& e, int i) const
    {
      return asImp().template subIndex<cc>(e,i);
    }

    /** @brief Return total number of entities of given type in this index set.

       \param type A valid geometry type.

       \return number of entities.
     */
    int size (GeometryType type) const
    {
      return asImp().size(type);
    }

    /** @brief Return total number of entities of given codim as a sum
       for all geometry types in this index set.
       \param codim A valid codimension
       To be moved to the DefaultImplementation class.
     */
    int size (int codim) const
    {
      int s=0;
      const std::vector<GeometryType>& geomTs = this->geomTypes(codim);
      for (unsigned int i=0; i<geomTs.size(); i++)
        s += this->size(geomTs[i]);

      return s;
    }

    /** @brief Return vector with all geometry types of entities in domain of index map.

       \return reference to vector of geometry types.
     */
    const std::vector<GeometryType>& geomTypes (int codim) const
    {
      return asImp().geomTypes(codim);
    }

    /** @brief Does the index set contain this Entity?
     */
    template<class EntityType>
    bool contains (const EntityType& e) const
    {
      enum { cd = EntityType::codimension };
      /*
         return asImp().template contains<cd>(e);
       */
      typename Codim<cd>::template Partition<All_Partition>::Iterator it=begin<cd,All_Partition>();
      typename Codim<cd>::template Partition<All_Partition>::Iterator iend=end<cd,All_Partition>();
      for (; it != iend; ++it)
        if (it->level() == e.level() && index(*it) == index(e)) return true;
      return false;
    }

    /** @brief Iterator to first entity of given codimension and partition type.
     */
    template<int cd, PartitionIteratorType pitype>
    typename Codim<cd>::template Partition<pitype>::Iterator begin () const
    {
      return asImp().begin<cd,pitype>();
    }

    /** @brief Iterator to one past the last entity of given codim for partition type
     */
    template<int cd, PartitionIteratorType pitype>
    typename Codim<cd>::template Partition<pitype>::Iterator end () const
    {
      return asImp().end<cd,pitype>();
    }

    //! Default constructor
    IndexSet() {}

  private:
    //! Forbid the copy constructor
    IndexSet(const IndexSet&);
    //! Forbid the assignment operator
    IndexSet& operator=(const IndexSet&);

    //!  Barton-Nackman trick
    IndexSetImp& asImp () {return static_cast<IndexSetImp &> (*this);}
    //!  Barton-Nackman trick
    const IndexSetImp& asImp () const {return static_cast<const IndexSetImp &>(*this);}
  };

  template<class GridImp, class IndexSetImp, class IndexSetTypes>
  class IndexSetDefaultImplementation :
    public IndexSet<GridImp,IndexSetImp,IndexSetTypes>
  {
  public:
    /** @brief Define types needed to iterate over the entities in the index set
     */
    template <int cd>
    struct Codim
    {

      /** \brief Define types needed to iterate over entities of a given partition type */
      template <PartitionIteratorType pitype>
      struct Partition
      {
        /** \brief The iterator needed to iterate over the entities of a given codim and
            partition type of this index set */
        typedef typename IndexSetTypes::template Codim<cd>::template Partition<pitype>::Iterator Iterator;
      };
    };

    /** @brief Return total number of entities of given codim as a sum
          for all geometry types in this index set.
          \param codim A valid codimension
     */
    /** @brief Map subentity of codim 0 entity to index.

       \param e Reference to codim 0 entity.
       \param i Number of codim cc subentity of e, where cc is the template parameter of the function.
       \return An index in the range 0 ... Max number of entities in set - 1.
       Here the method entity of Entity is used to get the subEntity and
       then the index of this Entity is returned.
     */
    template<int cc>
    int subIndex (const typename RemoveConst<GridImp>::Type::
                  Traits::template Codim<0>::Entity& e, int i) const
    {
      return this->index( *(e.template entity<cc>(i) ));
    }
  };


  /** @brief Id Set %Interface.

     This class template is used as a base class for all id set implementations.
     It uses the Barton-Nackman trick to ensure conformity to the interface.

     Template parameters are:

     \par GridImp
     A %Dune grid type.
     \par IdSetImp
     An implementation of the id set interface.
     \par IdTypeImp
     The type used for the ids (which is returned by functions returning an id).
   */
  template<class GridImp, class IdSetImp, class IdTypeImp>
  class IdSet
  {
  public:
    //! define the type used for persistent indices
    typedef IdTypeImp IdType;

    //! get id of an entity
    template<class EntityType>
    IdType id (const EntityType& e) const
    {
      enum { cc = EntityType::codimension };
      return asImp().template id<cc>(e);
    }

    //! get id of an entity of codim cc
    /*
       We use the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
     */
    template<int cc>
    IdType id (const typename RemoveConst<GridImp>::Type::
               Traits::template Codim<cc>::Entity& e) const
    {
      return asImp().template id<cc>(e);
    }

    //! get id of subentity i of codim cc
    /*
       We use the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
     */
    template<int cc>
    IdType subId (const typename RemoveConst<GridImp>::Type::
                  Traits::template Codim<0>::Entity& e, int i) const
    {
      return asImp().template subId<cc>(e,i);
    }

    //! Default constructor
    IdSet() {}

  private:
    //! Forbid the copy constructor
    IdSet(const IdSet&);
    //! Forbid the assignment operator
    IdSet& operator=(const IdSet&);

    //!  Barton-Nackman trick
    IdSetImp& asImp () {return static_cast<IdSetImp &> (*this);}
    //!  Barton-Nackman trick
    const IdSetImp& asImp () const {return static_cast<const IdSetImp &>(*this);}
  };

  /** @brief Id Set %DefaultImplementation.

     Template parameters are:

     \par GridImp
     A %Dune grid type.
     \par IdSetImp
     An implementation of the id set interface.
     \par IdTypeImp
     The type used for the ids (which is returned by functions returning an id).
   */
  template<class GridImp, class IdSetImp, class IdTypeImp>
  class IdSetDefaultImplementation : public IdSet<GridImp,IdSetImp,IdTypeImp>
  {
  public:
    //! define the type used for persistent indices
    typedef IdTypeImp IdType;

    //! get id of subentity i of codim cc
    /*
       We use the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
       This default implementation uses the entities entity method. This is
       slow but works by default for ervery id set imeplementation.
     */
    template<int cc>
    IdType subId (const typename RemoveConst<GridImp>::Type::
                  Traits::template Codim<0>::Entity& e, int i) const
    {
      return this->id( *(e.template entity<cc>(i)) );
    }
  };


  /** @} */

}
#endif
