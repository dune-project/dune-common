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

namespace Dune
{
  /** @brief Index Set %Interface base class.

     This class template is used as a base class for all index set implementations.
     It uses the Barton-Nackman trick to ensure conformity to the interface.

     Template parameters are:

     - <tt>GridImp</tt> Type that is a model of Dune::Grid.
     - <tt>IndexSetImp</tt> Type that is a model of Dune::IndexSet.
     - <tt>IndexSetTypes</tt> Traits class containing return types depending on implementation.

     <H3>Overview</H3>

     An index set provides a map \f[ m : E \to \mathbf{N}\f] where
     \f$E\f$ is a subset of the entities of a grid and \f$\mathbf{N}\f$ is the set of
     natural numbers (including 0).

     We define the subsets
     \f[ E_g^c = \{e\in E \ | \ \textrm{$e$ has codimension $c$ and geometry type $g$} \}.\f]

     The index map \f$m\f$ has the following properties:

     - It is unique within the subsets \f$E_g^c\f$, i.e. for any \f$e,e^\prime\in E_g^c\f$
     we have \f$e\neq e^\prime \rightarrow m(e)\neq m(e^\prime)\f$.
     - It is consecutive and zero-starting within the subsets \f$E_g^c\f$, i.e. we have
     \f$0\leq m(e) < |E_g^c|\f$ for any \f$e\in E_g^c\f$.

     Index sets are used to assign user defined data (e.g. degrees of freedom
     of a discretization) to entities of the grid. For efficiency reasons the prefered
     data structure for user data is the array. In order to access the data from the
     entity, its index (with respect to an index set - there may be several) is evaluated
     and used as an index to an array (or some other container providing random access).

     Usually an index set is not used directly but a Mapper is used to
     compute the array index from the information supplied by an index set.

     It is important to note that the index assigned to an entity may change during
     grid modification (i.e. refinement or dynamic load balancing). The user is reponsible
     for reorganizing the information stored in the external arrays appropriately. In
     order to do this the IdSet concept is supplied.

     <H3>Level index</H3>

     Index set where \f$E\f$ corresponds to all entities of a given grid level.
     All grid implementations provide level indices.

     <H3>Leaf Index</H3>

     Index set where \f$E\f$ corresponds to all entities of the leaf grid.
     All grid implementations provide a leaf index.

     @ingroup IndexIdSets
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

    //===========================================================
    /** @name Index access from entity
     */
    //@{
    //===========================================================

    /** @brief Map entity to index. The result of calling this method with an entity that is not
            in the index set is undefined.

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

    /** @brief Map entity to index. Easier to use than the above because codimension template
            parameter need not be supplied explicitely.
            The result of calling this method with an entity that is not
            in the index set is undefined.

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

    /** @brief Map subentity of codim cc of codim 0 entity to index.
            The result of calling this method with an entity that is not
            in the index set is undefined.

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
    //@}


    //===========================================================
    /** @name Access to entity set
     */
    //@{
    //===========================================================

    /** @brief Return vector with all geometry types of entities in domain of index map.
            Return a vector with all geometry types of a given codimension
            contained in the Entity set \f$E\f$.

       \param[in] codim A valid codimension.
       \return Const reference to a vector of geometry types.
     */
    const std::vector<GeometryType>& geomTypes (int codim) const
    {
      return asImp().geomTypes(codim);
    }

    /** @brief Return total number of entities of given geometry type in entity set \f$E\f$.

       \param[in] type A valid geometry type.
       \return         number of entities.
     */
    int size (GeometryType type) const
    {
      return asImp().size(type);
    }

    /** @brief Return total number of entities of given codim in the entity set \f$E\f$. This
            is simply a sum over all geometry types.

       \param[in] codim A valid codimension
            \return    number of entities.
     */
    int size (int codim) const
    {
      int s=0;
      const std::vector<GeometryType>& geomTs = this->geomTypes(codim);
      for (unsigned int i=0; i<geomTs.size(); i++)
        s += this->size(geomTs[i]);
      return s;
    }

    /** @brief Return true if the given entity is contained in \f$E\f$.
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

    /** @brief Iterator to first entity of given codimension and partition type in \f$E\f$.
            The iterator type is available via the public Codim struct.
     */
    template<int cd, PartitionIteratorType pitype>
    typename Codim<cd>::template Partition<pitype>::Iterator begin () const
    {
      return asImp().begin<cd,pitype>();
    }

    /** @brief Iterator to one past the last entity of given codim and partition type in \f$E\f$.
            The iterator type is available via the public Codim struct.
     */
    template<int cd, PartitionIteratorType pitype>
    typename Codim<cd>::template Partition<pitype>::Iterator end () const
    {
      return asImp().end<cd,pitype>();
    }
    //@}

    // Must be explicitely defined although this class should get a default constructor.
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


  /**\brief Provide default implementation of method if IndexSet
         @ingroup GridDevel
   */
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

     - <tt>GridImp</tt> Type that is a model of Dune::Grid.
     - <tt>IdSetImp</tt> Type that is a model of Dune::IdSet.
     - <tt>IdTypeImp</tt> Traits class containing return types depending on implementation.

     <H3>Overview</H3>

     An id set provides a map \f[ m : E \to \mathbf{N}\f] where
     \f$E\f$ is a subset of the entities of a grid and \f$\mathbf{N}\f$ is the set of
     natural numbers (including 0).

     The index map \f$m\f$ has the following properties:

     - It is injective, i.e. for any \f$e,e^\prime\in E\f$
     we have \f$e\neq e^\prime \Rightarrow m(e)\neq m(e^\prime)\f$.
     - It is persistent with respect to grid modification, i.e. if there exits an entity \f$e\f$ with
     id \f$i\f$ before grid modification and an entity \f$e^\prime\f$ with id \f$i\f$ after mesh
     modification it is guaranteed that \f$e=e^\prime\f$.

     The set of ids \f$\{i\in\textbf{N}\ |\ \exists e\in E : m(e)=i\}\f$ used by the
     id set is not necessarily consecutive. In practice the numbers can be quite large, especially
     in a parallel implementation. Therefore the type used to represent the id can be chosen
     by the application.

     <H3>Ids and leaf entities</H3>

     An element is a copy of its father element if it is the only son. This
     concept can be transfered to all higher codimensions because in a nested grid
     structure the entities of any codimension form a set of trees. However, the roots
     of these trees are not necessarily on level 0.
     Thus, we define that an entity is a copy of another entity if it is the only descendant
     of this entity in the refinement tree. This is illustrated in the following figure where,
     for example, vertex w is a copy of vertex v.

     \image html  idlocalref.png "Sharing of ids."
     \image latex idlocalref.eps "Sharing of ids." width=\textwidth

     The copy relation can be trivially extended to be an equivalence relation.
     With respect to ids we define that <EM> all copies of an entity share the same id.</EM>
     In the example of the figure the vertices v and w would have the same id.

     This definition is useful to transfer data related to the leaf grid during grid modification.

     <H3>Global id set</H3>

     A global id set provides ids that are unique over all processes over wich the
     grid is distributed.
     All grid implementations provide a global id set.

     <H3>Local id set</H3>

     A local id set provides ids that are unique within one process but two entities
     in different processes may have the same id. Obviously, a global id set is also
     a local id set. A grid implementation may provide an extra local id set for efficiency reasons.
     In sequential grids local and global id set are identical.
     All grid implementations provide a local id set.

     @ingroup IndexIdSets
   */
  template<class GridImp, class IdSetImp, class IdTypeImp>
  class IdSet
  {
  public:
    //! Type used to represent an id.
    typedef IdTypeImp IdType;

    //! Get id of an entity. This method is simpler to use than the one below.
    template<class EntityType>
    IdType id (const EntityType& e) const
    {
      enum { cc = EntityType::codimension };
      return asImp().template id<cc>(e);
    }

    //! Get id of an entity of codim cc. Unhandy because template parameter must be supplied explicitely.
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

    //! Get id of subentity i of codim cc of a codim 0 entity.
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

    // Default constructor (is not provided automatically because copy constructor is private)
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

     @ingroup GridDevel
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


}
#endif
