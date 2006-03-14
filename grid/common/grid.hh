// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_HH
#define DUNE_GRID_HH

/** \file
    \brief Different resources needed by all grid implementations
 */
#include <iostream>
#include <string>
#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/helpertemplates.hh>
#include <dune/common/typetraits.hh>
#include <dune/common/geometrytype.hh>
#include <dune/common/capabilities.hh>

namespace Dune {

  // compares a and b and if they are equal then throw and NotImplemented
  // exception
#if 0
#ifndef NDEBUG
#define CHECK_INTERFACE_IMPLEMENTATION(a,b) \
  if((a) == (b)) \
    DUNE_THROW(NotImplemented,"Interface method not implemented!");
#else
#define CHECK_INTERFACE_IMPLEMENTATION(a,b)
#endif
#endif
#define CHECK_INTERFACE_IMPLEMENTATION(a,b)

  /**
         @addtogroup Grid Grid

         The Dune Grid module defines a general interface to a parallel, in general
     nonconforming, locally refined and hierarchical
         finite element mesh.  The interface is independent of dimension and
         element type.

         @section Grid1 Terminology


         @subsection subs1 Entity

         An entity is a geometric object that is part of a grid. It is
     generalized polytope that has the same dimensionality as the grid
     or a lower dimension.


         @subsection subs20 Dimension

         A grid has a fixed dimension \f$d\f$ which is the number of coordinates
     required to specify any point in the grid. The dimension is a template parameter
     of a grid.


         @subsection subs21 Codimension of an entity

         Each entity has a codimension \f$c\f$ where \f$0 \leq c \leq d\f$ (the dimension of the grid).
         An entity with codimension \f$ c\f$ in a grid of dimension \f$ d\f$ is a \f$d-c\f$-dimensional
         object.


         @subsection subs5 Subentity

         Entities are hierarchically constructed in the sense that entities of
         codimension 0 are made up of entities of codimension 1 which are themselves
         made up of entities of codimension 2 etc. until entities of codimension \f$d-1\f$
         which consist of entities of codimension \f$ d\f$.


         @subsection subs3 Element

         An element is an entity of codimension 0.


         @subsection subs4 Vertex

         A vertex is an entity of codimension \f$ d\f$ (the same as the grid's dimension).


         @subsection subs22 World dimension

         Each grid has a world dimension \f$ w\f$ with \f$ w\geq d\f$. This is the number
     of coordinates of the positions of the grid's vertices.


         @subsection subs33 Hierarchical grid

         The %Dune grid interface describes not only a single grid but a sequence of
         grids with different resolution. This is achieved by beginning with an
         intentionally coarse grid, the so-called macro grid. Then each
     element may be individually subdivided to yield new (smaller) elements.
         This construction is recursive such that each macro element and
         all the elements that resulted from subdividing it form a tree structure.

         @subsection subs33333 Grid refinement

         The grid can only be modified in special phases, the so-called refinement phase.
         In between refinement phases the entities of the grid can not be modified in any way.
         During refinement currently only the hierachic subdivision can be modified.


         @subsection subs3333 Grid level

         All elements of the macro grid form level 0 of the grid structure. All
         elements that are obtained from an \f$ l\f$-fold subdivision of a macro
         element form level \f$ l\f$ of the grid structure.

         @subsection subs333 Leaf grid

         All elements of a grid that are not subdivided any further make up
         the leaf grid. The leaf grid is the mesh with the finest resolution.

         @subsection subs6 Assignable

         A type is said to be assignable if it has a (public) copy constructor and
         assignment operator. Note that this definition requires always both methods.


         @subsection subs7 Default-constructible

         A type is said to be default-constructible if it has a constructor without arguments.


         @subsection subs8 Copy-constructible from type X

         A type is said to be copy constructible from some other type X if it has
         a copy constructor that takes a reference to an object of type X.


         @subsection subs9 Equality-comparable

         A type is said to be equality-comparable if it has an operator==.


         @subsection subs10 LessThan-comparable

         A type is lessthan-comparable if it has an operator<.


         @subsection subs11 Dereferenceable

         A type is dereferenceable if it has an operator* that delivers
         a reference to a value type.

         @subsection subs11 Iterator

         An iterator is a type that can be dereferenced to yield an object of
         its value type, i.e. it behaves like a pointer, and that can be incremented to
         point to the next element in a linear sequence. In that respect it is comparable to
         ForwardIterator in the Standard Template Library.


         @subsection subs12 Mutable iterator

         An iterator is called mutable if the value it refers to can be changed, i.e. it is
         assignable.


         @subsection subs13 Immutable iterator

         An iterator is called immutable if the value referenced by the iterator can not
         be changed, i. e. the value is not assignable and only methods marked const on the value
         can be called.


         @subsection subs14 Model

         A type M is called a model of another type X if it implements all the methods
         of X with the intended semantics. Typically X is a type that describes an interface.


         @section Grid3 Types common to all grid implementations

         - Dune::ReferenceElement describes the topology and geometry of standard entities.
         Any given entity of the grid can be completely specified by a reference element
         and a map from this reference element to world coordinate space.

         - Dune::GeometryType defines names for the reference elements.



         @section Grid2 Types making up a grid implementation

     Each implementation of the Dune grid interface consist of a number of related types which
         together form a model of the grid interface. These types are the following:

         - %Grid which is a model of Dune::Grid where the template parameters are at least the
         dimension and the world dimension. It is a container of entities that allows to access
         these entities and that knows the number of entities.

         - %Entity which is a model of Dune::Entity. This class is parametrized by dimension and
         codimension. The entity encapsulates the topological part of an entity, i.e. its hierarchical
         construction from subentities and the relation to other entities. Entities cannot
         be created, copied or modified by the user. They can only be read-accessed through
         immutable iterators.

         - %Geometry which is a model of Dune::Geometry. This class encapsulates the geometric part
         of an entity by mapping local coordinates in a reference element to world coordinates.

         - %EntityPointer which is a model of Dune::EntityPointer. This is a dereferenceable
         type that delivers a reference to an entity. Moreover it is immutable, i.e. the
         referenced entity can not be modified.

         - %LevelIterator which is a model of Dune::LevelIterator is an immutable iterator
     that provides access to all entities of a given codimension and level of the
         grid. %EntityPointer is copy-constructible from a %LevelIterator.

         - %LeafIterator which is a model of Dune::LeafIterator is an immutable iterator
         that provides access to all entities of a given codimension of the leaf grid.
         %EntityPointer is copy-constructible from a %LeafIterator.

         - %HierarchicIterator which is a model of Dune::HierarchicIterator is an immutable
         iterator that provides access to all entities of codimension 0 that resulted from subdivision
         of a given entity of codimension 0. %EntityPointer is copy-constructible from a
         %HierarchicIterator.

         - %IntersectionIterator which is a model of Dune::IntersectionIterator
         provides access to all entities of codimension 0 that have an intersection of
         codimension 1 with a given entity of codimension 0. In a conforming mesh these
         are the face neighbors of an element. For two entities with a common intersection
         the %IntersectionIterator also provides information about the geometric location
         of the intersection. Furthermore it also provides information about intersections
         of an entity with the internal or external boundaries.

         - %LevelIndexSet and %LeafIndexSet which are both models of Dune::IndexSet are
         used to attach any kind of user-defined data to (subsets of) entities of the grid.
         This data is supposed to be stored in one-dimensional arrays for reasons
         of efficiency.

         - %LocalIdSet and %GlobalIdSet which are both models of Dune::IdSet are used to
         save user data during a grid refinement phase and during dynamic load balancing
         in the parallel case.


         @section Grid22 Overview of basic capabilities of the types

         <TABLE>
         <TR>
         <TD>Class</TD>
         <TD>Assignable</TD>
         <TD>DefaultConstructible</TD>
         <TD>EqualityComparable</TD>
         <TD>LessThanComparable</TD>
         </TR>
         <TR>
         <TD>Grid</TD>
         <TD>no</TD>
         <TD>no</TD>
         <TD>no</TD>
         <TD>no</TD>
         </TR>
         <TR>
         <TD>Entity</TD>
         <TD>no</TD>
         <TD>no</TD>
         <TD>no</TD>
         <TD>no</TD>
         </TR>
         <TR>
         <TD>GeometryType</TD>
         <TD>yes</TD>
         <TD>yes</TD>
         <TD>yes</TD>
         <TD>yes</TD>
         </TR>
         <TR>
         <TD>Geometry</TD>
         <TD>no</TD>
         <TD>no</TD>
         <TD>no</TD>
         <TD>no</TD>
         </TR>
         <TR>
         <TD>EntityPointer</TD>
         <TD>yes</TD>
         <TD>no</TD>
         <TD>yes</TD>
         <TD>no</TD>
         </TR>
         <TR>
         <TD>LevelIterator</TD>
         <TD>yes</TD>
         <TD>no</TD>
         <TD>yes</TD>
         <TD>no</TD>
         </TR>
         <TR>
         <TD>LeafIterator</TD>
         <TD>yes</TD>
         <TD>no</TD>
         <TD>yes</TD>
         <TD>no</TD>
         </TR>
         <TR>
         <TD>HierarchicIterator</TD>
         <TD>yes</TD>
         <TD>no</TD>
         <TD>yes</TD>
         <TD>no</TD>
         </TR>
         <TR>
         <TD>IntersectionIterator</TD>
         <TD>yes</TD>
         <TD>no</TD>
         <TD>yes</TD>
         <TD>no</TD>
         </TR>
         <TR>
         <TD>IndexSet</TD>
         <TD>no</TD>
         <TD>no</TD>
         <TD>no</TD>
         <TD>no</TD>
         </TR>
         <TR>
         <TD>IdSet</TD>
         <TD>no</TD>
         <TD>no</TD>
         <TD>no</TD>
         <TD>no</TD>
         </TR>
         </TABLE>



   */

  /**
         @addtogroup GridInterface Abstract Grid Interface
         @ingroup Grid
         @brief Interfaces needed when working with a \ref Grid "Dune::Grid"
   */

  /**
         @addtogroup GridDevel Grid Developer API
         @ingroup Grid
         @brief Interfaces needed to implement a new \ref Grid "Dune::Grid"
   */

  /**
         @addtogroup GridImplementations Implementations Overview
         @ingroup Grid
         @brief A List of the different Implementations of the Dune Grid Interface.
   */

  /*! Possible states of an entity before and/or after grid adaptation
     @ingroup GIRelatedTypes
   */
  enum AdaptationState {
    NONE ,   //!< notin' to do and notin' was done
    COARSEN, //!< entity could be coarsend in adaptation step
    REFINED  //!< enity was refined in adaptation step
  };


  /*! \brief Attributes used in the generic overlap model

     The values are ordered intentionally in order to be able to
     define ranges of partition types.

     @ingroup GIRelatedTypes
   */
  enum PartitionType {
    InteriorEntity=0,     //!< all interior entities
    BorderEntity=1  ,     //!< on boundary between interior and overlap
    OverlapEntity=2 ,     //!< all entites lying in the overlap zone
    FrontEntity=3  ,      //!< on boundary between overlap and ghost
    GhostEntity=4         //!< ghost entities
  };

  /*! Provide names for the partition types
     @ingroup GIRelatedTypes
   */
  inline std::string PartitionName(PartitionType type)
  {
    switch(type) {
    case InteriorEntity :
      return "interior";
    case BorderEntity :
      return "border";
    case OverlapEntity :
      return "overlap";
    case FrontEntity :
      return "front";
    case GhostEntity :
      return "ghost";
    default :
      DUNE_THROW(NotImplemented, "name of unknown partition type requested");
    }
  }

  /*! Parameter to be used for the communication functions
     @ingroup GIRelatedTypes
   */
  enum InterfaceType {
    InteriorBorder_InteriorBorder_Interface=0,     //!< send/receive interior and border entities
    InteriorBorder_All_Interface=1,                //!< send interior and border, receive all entities
    Overlap_OverlapFront_Interface=2,              //!< send overlap, receive overlap and front entities
    Overlap_All_Interface=3,                       //!< send overlap, receive all entities
    All_All_Interface=4                            //!< send all and receive all entities
  };

  /*! Parameter to be used for the parallel level iterators
     @ingroup GIRelatedTypes
   */
  enum PartitionIteratorType {
    Interior_Partition=0,           //!< only interior entities
    InteriorBorder_Partition=1,     //!< interior and border entities
    Overlap_Partition=2,            //!< only overlap entities
    OverlapFront_Partition=3,       //!< overlap and front entities
    All_Partition=4,                //!< all entities
    Ghost_Partition=5               //!< only ghost entities
  };


  /*! Define a type for communication direction parameter
     @ingroup GIRelatedTypes
   */
  enum CommunicationDirection {
    ForwardCommunication,         //!< communicate as given in InterfaceType
    BackwardCommunication         //!< reverse communication direction
  };

  /*! GridIndexType specifies which Index of the Entities of the grid
        should be used, i.e. globalIndex() or index()
     @ingroup GIRelatedTypes
   */
  enum GridIndexType { GlobalIndex , //!< use globalIndex() of entity
                       LevelIndex  ,  //!< use index() of entity
                       LeafIndex    //!< use index() of entity
  };

  /*! Id for each model of the grid interface
        @ingroup GIRelatedTypes
   */
  enum GridIdentifier {
    SGrid_Id,            //!< Id for SGrid
    AlbertaGrid_Id ,     //!< Id for AlbertaGrid
    UGGrid_Id,           //!< Id for UGGrid
    YaspGrid_Id ,        //!< Id for YaspGrid
    ALU3dGrid_Id,        //!< Id for ALU3dGrid
    OneDGrid_Id          //!< Id for OneDGrid
  };

  /*! Provide names for the different grid types.
        @ingroup GIRelatedTypes
   */
  inline std::string transformToGridName(GridIdentifier type)
  {
    switch(type) {
    case SGrid_Id :
      return "SGrid";
    case AlbertaGrid_Id :
      return "AlbertaGrid";
    case ALU3dGrid_Id :
      return "ALU3dGrid";
    case UGGrid_Id :
      return "UGGrid";
    case YaspGrid_Id :
      return "YaspGrid";
    case OneDGrid_Id :
      return "OneDGrid";
    default :
      return "unknown";
    }
  }


  //************************************************************************
  // G R I D E R R O R
  //************************************************************************

  /*!
     exceptions in Dune grid modules.
   */

  class GridError : public Exception {};

  // Forward Declarations
  template<int mydim, int cdim, class GridImp,template<int,int,class> class GeometryImp> class Geometry;
  // dim is necessary because Entity will be specialized for codim==0 _and_ codim==dim
  // EntityImp gets GridImp as 3rd template parameter to distinguish between const and mutable grid
  template<int codim, int dim, class GridImp,template<int,int,class> class EntityImp> class Entity;
  template<class GridImp, class EntityPointerImp> class EntityPointer;
  template<int codim, PartitionIteratorType pitype, class GridImp,
      template<int,PartitionIteratorType,class> class LevelIteratorImp> class LevelIterator;
  template<class GridImp, template<class> class IntersectionIteratorImp> class IntersectionIterator;
  template<class GridImp, template<class> class HierarchicIteratorImp> class HierarchicIterator;
  template<int codim, PartitionIteratorType pitype, class GridImp,
      template<int,PartitionIteratorType,class> class LeafIteratorImp> class LeafIterator;
  template<class GridImp> class GenericLeafIterator;
  template<class GridImp, class IndexSetIteratorImp, class IndexSetImp> class IndexSet;
  template<class GridImp, class IdSetImp, class IdTypeImp> class IdSet;


  //************************************************************************
  // G R I D
  //************************************************************************

  /**
     \brief Grid abstract base class
     @ingroup GIGrid

     This class is the base class for all grid implementations. Although
     no virtual functions are used we call it abstract since its
     methods do not contain an implementation but forward to the methods of
     the derived class via the Barton-Nackman trick.

     Template parameters:

     - <tt>dim</tt> specifies the dimension of the grid.
     - <tt>dimworld</tt> specifies the dimension of the surrounding space, this can be
       different from dim, if the grid is defined on a manifold .
     - <tt>ct</tt> field type of the world vector space.
     - <tt>GridFamily</tt> traits class providing all types
       associated with the grid implementation.

     \nosubgrouping
   */
  template< int dim, int dimworld, class ct, class GridFamily>
  class Grid {
    typedef typename GridFamily::Traits::Grid GridImp;
    typedef Grid<dim,dimworld,ct,GridFamily> ThisType;
  public:

    //===========================================================
    /** @name Exported constants
     */
    //@{
    //===========================================================

    //! A constant that exports the template parameter dim
    enum {
      //! \brief The dimension of the grid
      dimension=dim
    };

    //! A constant that exports the template parameter dimworld
    enum {
      //! \brief The dimension of the world the grid lives in.
      dimensionworld=dimworld
    };
    //@}

    //===========================================================
    /** @name Exported types
     */
    //@{
    //===========================================================

    /** \brief A Traits struct that collects all associated types of one implementation

       Template parameter:

       - <tt>cd</tt> codimension. Note that not all types in this struct depend on this template parameter.
     */
    template <int cd>
    struct Codim
    {
      //! A type that is a model of Dune::Geometry<dim-cd,dimworld>.
      typedef typename GridFamily::Traits::template Codim<cd>::Geometry Geometry;

      //! A type that is a model of Dune::Geometry<dim-cd,dim>.
      typedef typename GridFamily::Traits::template Codim<cd>::LocalGeometry LocalGeometry;

      //! A type that is a model of a Dune::Entity<cd,dim,...>.
      typedef typename GridFamily::Traits::template Codim<cd>::Entity Entity;

      //! A type that is a model of Dune::EntityPointer<cd,dim,...>.
      typedef typename GridFamily::Traits::template Codim<cd>::EntityPointer EntityPointer;

      //! A struct collecting all types depending on the partition iterator type.
      template <PartitionIteratorType pitype>
      struct Partition
      {
        /*! \brief A type that is a model of Dune::LevelIterator<cd,pitype,...>
              which is s type of iterator that may be used to examine, but not to modify, the
              entities of codimension cd with partition type
              pitype  on a certain level of the grid, i. e. the increment of
              the iterator adjusts it to the next entity on that level.
         */
        typedef typename GridFamily::Traits::template Codim<cd>::template Partition<pitype>::LevelIterator LevelIterator;
        /*! \brief A type that is a model of Dune::LeafIterator<cd,pitype,...>
              which is a type of iterator that may be used to examine, but not to modify, the
              entities of codimension cd with partition type
              pitype in the leaf grid, i. e. the increment of
              the iterator adjusts it to the next entity in the leaf grid.
         */
        typedef typename GridFamily::Traits::template Codim<cd>::template Partition<pitype>::LeafIterator LeafIterator;
      };

      /*! \brief A type that is a model of Dune::IntersectionIterator
         which is an iterator that allows to examine, but not to modify, the
         intersections of codimension 1 of an element (entity of codimension 0)
         with other elements.
       */
      typedef typename GridFamily::Traits::IntersectionIterator IntersectionIterator;

      /*! \brief A type that is a model of Dune::HierarchicIterator
         A type of iterator that allows to examine, but not to modify, entities
         of codimension 0 that result from refinement of an entity of
         codimension 0.
       */
      typedef typename GridFamily::Traits::HierarchicIterator HierarchicIterator;

      /*! \brief A type that is a model of Dune::LevelIterator with partition type All_Partition
       */
      typedef typename GridFamily::Traits::template Codim<cd>::LevelIterator LevelIterator;

      /*! \brief A type that is a model of Dune::LeafIterator with partition type All_Partition
       */
      typedef typename GridFamily::Traits::template Codim<cd>::LeafIterator LeafIterator;

      /*!  \brief A type that is a model of Dune::IndexSet
         which provides a consecutive, but non persistent, numbering for
         entities on a grid level.
       */
      typedef typename GridFamily::Traits::LevelIndexSet LevelIndexSet;

      /*! \brief A type that is a model of Dune::IndexSet
         which provides a consecutive, but non persistent, numbering for
         entities in the leaf grid.
       */
      typedef typename GridFamily::Traits::LeafIndexSet LeafIndexSet;

      /*!  \brief A type that is a model of Dune::IdSet
         which provides a unique and persistent numbering for
         all entities in the grid. The numbering is unique over all processes
         over which the grid is partitioned. The numbering is not necessarily
         consecutive.
       */
      typedef typename GridFamily::Traits::GlobalIdSet GlobalIdSet;

      /*! \brief A type that is a model of Dune::IdSet
         which provides a unique and persistent numbering for
         all entities in the grid. The numbering is only unique in a single process
         and it is not necessarily consecutive.
       */
      typedef typename GridFamily::Traits::LocalIdSet LocalIdSet;

      /*! \brief A type that is a model of Dune::CollectiveCommunication.
         It provides a portable way for collective communication on the set
         of processes used by the grid.
       */
      typedef typename GridFamily::Traits::CollectiveCommunication CollectiveCommunication;

    };

    //! Define type used for coordinates in grid module
    typedef ct ctype;
    //@}


    //===========================================================
    /** @name Grid id
     */
    //@{
    //===========================================================

    //! Return the id of the grid
    GridIdentifier type() const
    {
      CHECK_INTERFACE_IMPLEMENTATION(
        ((GridIdentifier (GridImp::*)() const) &(ThisType::type)),
        ((GridIdentifier (GridImp::*)() const) &(GridImp::type)));
      return asImp().type();
    }
    //@}

    //===========================================================
    /** @name Size methods
     */
    //@{
    //===========================================================

    /*! \brief Return maximum level defined in this grid. Levels are numbered
       0 ... maxLevel with 0 the coarsest level.
     */
    int maxLevel() const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)() const) &(ThisType::maxLevel)),
        ((int (GridImp::*)() const) &(GridImp::maxLevel)));
      return asImp().maxLevel();
    }

    //! Return number of grid entities of a given codim on a given level in this process
    int size (int level, int codim) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(int,int) const) &(ThisType::size)),
        ((int (GridImp::*)(int,int) const) &(GridImp::size )));
      return asImp().size(level,codim);
    }

    //! Return number of leaf entities of a given codim in this process
    int size (int codim) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(int) const) &(ThisType::size)),
        ((int (GridImp::*)(int) const) &(GridImp::size)));
      return asImp().size(codim);
    }

    //! Return number of entities per level and geometry type in this process
    int size (int level, GeometryType type) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(int,GeometryType) const) &(ThisType::size)),
        ((int (GridImp::*)(int,GeometryType) const) &(GridImp::size)));
      return asImp().size(level,type);
    }

    //! Return number of leaf entities per geometry type in this process
    int size (GeometryType type) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(GeometryType) const) &(ThisType::size)),
        ((int (GridImp::*)(GeometryType) const) &(GridImp::size)));
      return asImp().size(type);
    }
    //@}


    //===========================================================
    /** @name Iterators
     */
    //@{
    //===========================================================

    //! Iterator to first entity of given codim on level
    template<int cd, PartitionIteratorType pitype>
    typename Codim<cd>::template Partition<pitype>::LevelIterator lbegin (int level) const
    {
      typedef typename Codim<cd>::template Partition<pitype>::LevelIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)(int) const) &(ThisType::template lbegin<cd,pitype>)),
        ((ItType (GridImp::*)(int) const) &(GridImp ::template lbegin<cd,pitype>)));
      return asImp().template lbegin<cd,pitype>(level);
    }

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Codim<cd>::template Partition<pitype>::LevelIterator lend (int level) const
    {
      typedef typename Codim<cd>::template Partition<pitype>::LevelIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)(int) const) &(ThisType::template lend<cd,pitype>)),
        ((ItType (GridImp::*)(int) const) &(GridImp ::template lend<cd,pitype>)));
      return asImp().template lend<cd,pitype>(level);
    }

    //! Iterator to first entity of given codim on level for PartitionType All_Partition
    template<int cd>
    typename Codim<cd>::template Partition<All_Partition>::LevelIterator lbegin (int level) const
    {
      typedef typename Codim<cd>::LevelIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)(int) const) &(ThisType::template lbegin<cd>)),
        ((ItType (GridImp::*)(int) const) &(GridImp ::template lbegin<cd>)));
      return asImp().template lbegin<cd>(level);
    }

    //! one past the end on this level for PartitionType All_Partition
    template<int cd>
    typename Codim<cd>::template Partition<All_Partition>::LevelIterator lend (int level) const
    {
      typedef typename Codim<cd>::LevelIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)(int) const) &(ThisType::template lend<cd>)),
        ((ItType (GridImp::*)(int) const) &(GridImp ::template lend<cd>)));
      return asImp().template lend<cd>(level);
    }

    //! Iterator to first entity of given codim on leaf grid
    template<int cd, PartitionIteratorType pitype>
    typename Codim<cd>::template Partition<pitype>::LeafIterator leafbegin () const
    {
      typedef typename Codim<cd>::template Partition<pitype>::LeafIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)() const) &(ThisType::template leafbegin<cd,pitype>)),
        ((ItType (GridImp::*)() const) &(GridImp ::template leafbegin<cd,pitype>)));
      return asImp().template leafbegin<cd,pitype>();
    }

    //! one past the end on the leaf level grid
    template<int cd, PartitionIteratorType pitype>
    typename Codim<cd>::template Partition<pitype>::LeafIterator leafend () const
    {
      typedef typename Codim<cd>::template Partition<pitype>::LeafIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)() const) &(ThisType::template leafend<cd,pitype>)),
        ((ItType (GridImp::*)() const) &(GridImp ::template leafend<cd,pitype>)));
      return asImp().template leafend<cd,pitype>();
    }

    //! Iterator to first entity of given codim on leaf grid for PartitionType All_Partition
    template<int cd>
    typename Codim<cd>::template Partition<All_Partition>::LeafIterator leafbegin () const
    {
      typedef typename Codim<cd>::LeafIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)() const) &(ThisType::template leafbegin<cd>)),
        ((ItType (GridImp::*)() const) &(GridImp ::template leafbegin<cd>)));
      return asImp().template leafbegin<cd,All_Partition>();
    }

    //! one past the end on the leaf grid for PartitionType All_Partition
    template<int cd>
    typename Codim<cd>::template Partition<All_Partition>::LeafIterator leafend () const
    {
      typedef typename Codim<cd>::LeafIterator ItType;
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((ItType (GridImp::*)() const) &(ThisType::template leafend<cd>)),
        ((ItType (GridImp::*)() const) &(GridImp ::template leafend<cd>)));
      return asImp().template leafend<cd,All_Partition>();
    }
    //@}


    //===========================================================
    /** @name Access to index and id sets
     */
    //@{
    //===========================================================

    //! return const reference to the grids global id set
    const typename Codim<0>::GlobalIdSet& globalIdSet() const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((const typename Codim<0>::GlobalIdSet& (GridImp::*)() const) &ThisType::globalIdSet),
        ((const typename Codim<0>::GlobalIdSet& (GridImp::*)() const) &GridImp::globalIdSet));
      return asImp().globalIdSet();
    }

    //! return const reference to the grids local id set
    const typename Codim<0>::LocalIdSet& localIdSet() const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((const typename Codim<0>::LocalIdSet& (GridImp::*)() const) &ThisType::localIdSet),
        ((const typename Codim<0>::LocalIdSet& (GridImp::*)() const) &GridImp::localIdSet));
      return asImp().localIdSet();
    }

    //! return const reference to the grids level index set for level level
    const typename Codim<0>::LevelIndexSet& levelIndexSet(int level) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((const typename Codim<0>::LevelIndexSet& (GridImp::*)(int) const) &ThisType::levelIndexSet),
        ((const typename Codim<0>::LevelIndexSet& (GridImp::*)(int) const) &GridImp::levelIndexSet));
      return asImp().levelIndexSet(level);
    }

    //! return const reference to the grids leaf index set
    const typename Codim<0>::LeafIndexSet& leafIndexSet() const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((const typename Codim<0>::LeafIndexSet& (GridImp::*)() const) &ThisType::leafIndexSet),
        ((const typename Codim<0>::LeafIndexSet& (GridImp::*)() const) &GridImp::leafIndexSet));
      return asImp().leafIndexSet();
    }
    //@}


    //===========================================================
    /** @name Adaptivity and grid refinement
     */
    //@{
    //===========================================================

    //! Refine the grid refCount times using the default refinement rule.
    void globalRefine (int refCount)
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((void (GridImp::*)() ) &ThisType::globalRefine),
        ((void (GridImp::*)() ) &GridImp::globalRefine));
      asImp().globalRefine(refCount);
      return;
    }

    /** \brief Marks an entity to be refined in a subsequent adapt.

       \param[in] refCount Number of subdivisions that should be applied. Negative value means coarsening.
       \param[in] e        Entity that should be refined

       \return true if entity was marked, false otherwise.
     */
    template <class T>
    bool mark( int refCount, T & e )
    {
      IsTrue<Conversion<T, typename Grid<dim,dimworld,ct,GridFamily>::template Codim<0>::EntityPointer>::exists >::yes();
      return asImp.template mark<T>(refCount,e);
    }

    /*! \brief To be called after entities have been marked and before adapt() is called.

       \return true if an entity may be coarsened during a subsequent adapt(), false otherwise.
     */
    bool preAdapt ()
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((bool (GridImp::*)() ) &ThisType::preAdapt),
        ((bool (GridImp::*)() ) &GridImp::preAdapt));
      return asImp.preAdapt();
    }

    /** \brief Refine all positive marked leaf entities,
        coarsen all negative marked entities if possible

            \return true if a least one entity was refined

            The complete adaptation process works as follows:

            - mark entities with the mark() method
            - call preAdapt()
            - if preAdapt() returned true: possibly save current solution
            - call adapt()
            - if adapt() returned true: possibly interpolate the (saved) solution
            - call postAdapt()
     */
    bool adapt ()
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((bool (GridImp::*)() ) &ThisType::adapt),
        ((bool (GridImp::*)() ) &GridImp::adapt));
      return asImp.adapt();
    }

    //! To be called after grid has been adapted and information left over by the adaptation has been processed.
    void postAdapt()
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((bool (GridImp::*)() ) &ThisType::postAdapt),
        ((bool (GridImp::*)() ) &GridImp::postAdapt));
      return asImp.postAdapt();
    }
    //@}


    //===========================================================
    /** @name Parallel data distribution and communication
     */
    //@{
    //===========================================================

    //! Return size of overlap for a given codim on a given level
    int overlapSize (int level, int codim) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(int,int) const) &(ThisType::overlapSize)),
        ((int (GridImp::*)(int,int) const) &(GridImp::overlapSize )));
      return asImp().overlapSize(level,codim);
    }

    //! Return size of overlap region for a given codim on the leaf grid
    int overlapSize (int codim) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(int) const) &(ThisType::overlapSize)),
        ((int (GridImp::*)(int) const) &(GridImp::overlapSize )));
      return asImp().overlapSize(codim);
    }

    //! Return size of ghost region for a given codim on a given level
    int ghostSize (int level, int codim) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(int,int) const) &(ThisType::ghostSize)),
        ((int (GridImp::*)(int,int) const) &(GridImp:: ghostSize)));
      return asImp().ghostSize(level,codim);
    }

    //! Return size of ghost region for a given codim on the leaf grid
    int ghostSize (int codim) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((int (GridImp::*)(int) const) &(ThisType::ghostSize)),
        ((int (GridImp::*)(int) const) &(GridImp:: ghostSize)));
      return asImp().ghostSize(codim);
    }

    /*! \brief Communicate information on distributed entities on a given level
          Template parameter is a model of Dune::CommDataHandle
     */
    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir, int level) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      //     CHECK_INTERFACE_IMPLEMENTATION(
      //       ((void (GridImp::*)(DataHandle&,InterfaceType,CommunicationDirection,int) const) &(ThisType::template communicate<DataHandle>)),
      //       ((void (GridImp::*)(DataHandle&,InterfaceType,CommunicationDirection,int) const) &(GridImp ::template communicate<DataHandle>)));
      asImp().template communicate<DataHandle>(data,iftype,dir,level);
      return;
    }

    /*! \brief Communicate information on distributed entities on the leaf grid
          Template parameter is a model of Dune::CommDataHandle
     */
    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir) const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      //     CHECK_INTERFACE_IMPLEMENTATION(
      //       ((void (GridImp::*)(DataHandle&,InterfaceType,CommunicationDirection) const) &(ThisType::template communicate<DataHandle>)),
      //       ((void (GridImp::*)(DataHandle&,InterfaceType,CommunicationDirection) const) &(GridImp ::template communicate<DataHandle>)));
      asImp().template communicate<DataHandle>(data,iftype,dir);
      return;
    }

    //! return const reference to a collective communication object. The return type is a model of Dune::CollectiveCommunication.
    const typename Codim<0>::CollectiveCommunication& comm () const
    {
      // compare addresses of the method, if they are equal then derived
      // class has the method not overloaded which leads to a seg fault
      CHECK_INTERFACE_IMPLEMENTATION(
        ((const typename Codim<0>::CollectiveCommunication& (GridImp::*)() const) &ThisType::comm),
        ((const typename Codim<0>::CollectiveCommunication& (GridImp::*)() const) &GridImp::comm));
      return asImp().comm();
    }
    //@}


  private:
    //!  Barton-Nackman trick
    GridImp& asImp () {return static_cast<GridImp &> (*this);}
    //!  Barton-Nackman trick
    const GridImp& asImp () const {return static_cast<const GridImp &>(*this);}
  };


  //************************************************************************
  //
  //  Default Methods of Grid
  //
  //************************************************************************
  //

  /**
     \ingroup GridDevel
     @{

     A Grid is a container of grid entities. Given a dimension dim
     these entities have a codimension codim with 0 <= codim <= dim.

     The Grid is assumed to be hierachically refined and nested. It
     enables iteration over entities of a given level and codimension.

     The grid can be non-matching.

     All information is provided to allocate degrees of freedom in
     appropriate vector data structures (which are not part of this
     module).

     Template class Grid defines a "base class" for all grids.

     \par Classes implementing the Grid Interface
     \li Dune::AlbertaGrid <br>
         <i> Provides the simplicial meshes of the finite element tool box
             ALBERTA (http://www.alberta-fem.de/)
             written by Kunibert Siebert and Alfred Schmidt.</i>
     \li Dune::ALU3dGrid <br>
         <i> 3D grid with support for hexahedrons and tetrahedrons.</i>
     \li Dune::OneDGrid <br>
         <i> Onedimensional adaptive grid</i>
     \li Dune::SGrid <br>
         <i> A structured mesh in d dimensions consisting of "cubes".</i>
     \li Dune::UGGrid <br>
         <i> Provides the meshes of the finite element toolbox UG.
             (http://cox.iwr.uni-heidelberg.de/~ug).</i>
     \li Dune::YaspGrid (Yet Another Structured Parallel Grid) <br>
         <i> Provides a distributed structured cube mesh.</i>
   */
  template<int dim,
      int dimworld,
      class ct,
      class GridFamily>
  class GridDefaultImplementation : public Grid <dim,dimworld,ct,GridFamily>
  {
    typedef typename GridFamily::Traits::Grid GridImp;

  public:
    //! the traits of this class
    typedef typename GridFamily::Traits Traits;

    //***************************************************************
    //  Interface for Adaptation
    //***************************************************************
    /** \brief marks an entity for refCount refines.
     *
     * If refCount is negative the entity is coarsened -refCount times
     * \return true if entity was marked, false otherwise
     *
     *  - Note:
     *    default implementation is: return false; for grids with no
     *    adaptation.
     *  - for the grid programmer:
     *    this method is implemented as a template method, because the
     *    Entity type is not defined when the class is instantiated
     *
     *    You won't need this trick in the implementation.
     *    In your implementation you should use it as
     *    \code
     *    bool mark( int refCount,
     *               typename Traits::template Codim<0>::EntityPointer & e ).
     *    \endcode
     *    This template method will vanish due to the inheritance
     *    rules.
     */
    template <class T>
    bool mark( int refCount, T & e )
    {
      IsTrue<Conversion<T, typename Grid<dim,dimworld,ct,GridFamily>::template Codim<0>::EntityPointer>::exists >::yes();
      return false;
    }

    /** \brief Refine all positive marked leaf entities
        coarsen all negative marked entities if possible
        \return true if a least one entity was refined

        - Note: this default implementation always returns false
          so grid with no adaptation doesn't need to implement these methods
     */
    bool adapt ()    { return false; }

    //! returns true, if at least one entity is marked for adaption
    bool preAdapt () { return false; }

    //! clean up some markers
    void postAdapt() {}

    /** \brief ghostSize is zero by default */
    int ghostSize (int level, int codim) const { return 0; }

    /** \brief overlapSize is zero by default */
    int overlapSize (int level, int codim) const { return 0; }

    /** \brief ghostSize is zero by default */
    int ghostSize (int codim) const { return 0; }

    /** \brief overlapSize is zero by default */
    int overlapSize (int codim) const { return 0; }

    /** dummy communicate, doing nothing  */
    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir, int level) const
    {}

    /** dummy communicate, doing nothing  */
    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir) const
    {}

  protected:
    //! return real implementation of interface class
    template <class InterfaceType>
    typename InterfaceType :: ImplementationType &
    getRealImplementation (InterfaceType &i) const { return i.getRealImp(); }

    //! return real implementation of interface class
    template <class InterfaceType>
    const typename InterfaceType :: ImplementationType &
    getRealImplementation (const InterfaceType &i) const { return i.getRealImp(); }

  protected:
    //! Barton-Nackman trick
    GridImp& asImp () {return static_cast<GridImp &>(*this);}
    const GridImp& asImp () const {return static_cast<const GridImp &>(*this);}
  };

  /** @} */

  /**
     \brief A traits struct that collects all associated types of one grid model
     @ingroup GIMiscellaneous


     Template parameters:

     - <tt>dim</tt>
   */
  template <int dim, int dimw, class GridImp,
      template<int,int,class> class GeometryImp,
      template<int,int,class> class EntityImp,
      template<int,class> class EntityPointerImp,
      template<int,PartitionIteratorType,class> class LevelIteratorImp,
      template<class> class IntersectionIteratorImp,
      template<class> class HierarchicIteratorImp,
      template<int,PartitionIteratorType,class> class LeafIteratorImp,
      class LevelIndexSetImp, class LevelIndexSetTypes, class LeafIndexSetImp, class LeafIndexSetTypes,
      class GlobalIdSetImp, class GIDType, class LocalIdSetImp, class LIDType, class CCType>
  struct GridTraits
  {
    typedef GridImp Grid;

    typedef Dune::IntersectionIterator<const GridImp, IntersectionIteratorImp> IntersectionIterator;

    typedef Dune::HierarchicIterator<const GridImp, HierarchicIteratorImp> HierarchicIterator;

    template <int cd>
    struct Codim
    {
      //! IMPORTANT: Codim<codim>::Geometry == Geometry<dim-codim,dimw>
      typedef Dune::Geometry<dim-cd, dimw, const GridImp, GeometryImp> Geometry;
      typedef Dune::Geometry<dim-cd, dim, const GridImp, GeometryImp> LocalGeometry;
      // we could - if needed - introduce another struct for dimglobal of Geometry
      typedef Dune::Entity<cd, dim, const GridImp, EntityImp> Entity;

      typedef Dune::LevelIterator<cd,All_Partition,const GridImp,LevelIteratorImp> LevelIterator;

      typedef Dune::LeafIterator<cd,All_Partition,const GridImp,LeafIteratorImp> LeafIterator;

      typedef Dune::EntityPointer<const GridImp,EntityPointerImp<cd,const GridImp> > EntityPointer;

      template <PartitionIteratorType pitype>
      struct Partition
      {
        typedef Dune::LevelIterator<cd,pitype,const GridImp,LevelIteratorImp> LevelIterator;
        typedef Dune::LeafIterator<cd,pitype,const GridImp,LeafIteratorImp> LeafIterator;
      };

    };

    typedef IndexSet<const GridImp,LevelIndexSetImp,LevelIndexSetTypes> LevelIndexSet;
    typedef IndexSet<const GridImp,LeafIndexSetImp,LeafIndexSetTypes> LeafIndexSet;
    typedef IdSet<const GridImp,GlobalIdSetImp,GIDType> GlobalIdSet;
    typedef IdSet<const GridImp,LocalIdSetImp,LIDType> LocalIdSet;

    typedef CCType CollectiveCommunication;
  };

  // define of capabilties for the interface class
  namespace Capabilities
  {
    // capabilities for the interface class depend on the implementation
    template< int dim, int dimworld, typename ct, class GridFamily >
    struct hasLeafIterator< GridDefaultImplementation<dim,dimworld,ct,GridFamily> >
    {
      typedef GridDefaultImplementation<dim,dimworld,ct,GridFamily> GridType;
      typedef typename GridType::Traits::Grid GridImp;
      static const bool v = hasLeafIterator<GridImp>::v;
    };

    // capabilities for the interface class depend on the implementation
    template< int dim, int dimworld, typename ct, class GridFamily , int cdim >
    struct hasEntity< GridDefaultImplementation<dim,dimworld,ct,GridFamily>, cdim >
    {
      typedef GridDefaultImplementation<dim,dimworld,ct,GridFamily> GridType;
      typedef typename GridType::Traits::Grid GridImp;
      static const bool v = hasEntity<GridImp,cdim>::v;
    };

  } // end namespace Capabilities

  //! for creation of an engine interface object like Entity or Geometry
  //! one has to derive a class to create the object because the
  //! contructors of the interface object classes are protected
  //! therefore here a generic implementation for this object creation is
  //! provided
  template <class InterfaceType>
  struct MakeableInterfaceObject : public InterfaceType
  {
    typedef typename InterfaceType::ImplementationType ImplementationType;
    //! create interface object by calling the contructor of the base class
    MakeableInterfaceObject(const ImplementationType & realImp) : InterfaceType(realImp) {}
  };
}

#undef CHECK_INTERFACE_IMPLEMENTATION

#include "geometry.hh"
#include "entity.hh"
#include "entitypointer.hh"
#include "leveliterator.hh"
#include "intersectioniterator.hh"
#include "hierarchiciterator.hh"
#include "leafiterator.hh"
#include "indexidset.hh"

inline std::ostream& operator<< (std::ostream& s, Dune::PartitionType t)
{
  s << Dune::PartitionName(t);
  return s;
}

inline std::ostream& operator<< (std::ostream& s, Dune::GridIdentifier t)
{
  s << Dune::transformToGridName(t);
  return s;
}

#endif
