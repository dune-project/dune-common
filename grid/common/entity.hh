// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_ENTITY_HH
#define DUNE_GRID_ENTITY_HH

#include <dune/common/typetraits.hh>
#include "grid.hh"

namespace Dune
{

  /**
     @brief Interface Definition for EntityImp
     @brief Encapsulates the implementation of an Entity in a Grid.

     \dot
        digraph entity {
           rankdir=LR;
           node [ shape=record, fontname=Helvetica, fontsize=10, height=0.25 ];
           Entity [ label="Dune::Entity\<cd,dim,GridImp,EntityImp\>"
                    style=filled, bgcolor=lightgrey
                    shape=record, URL="\ref Dune::Entity"];
           Element [ label="Dune::Entity\<0,dim,GridImp,EntityImp\>"
                     URL="\ref Dune::Entity<0,dim,GridImp,EntityImp>"];
           Vertex [ label="Dune::Entity\<dim,dim,GridImp,EntityImp\>"
                    URL="\ref Dune::Entity<dim,dim,GridImp,EntityImp>"];
           Entity -> Element [ dirType="back", arrowType="open",
                               style="dashed"
                               fontname=Helvetica, fontsize=8,
                               label="cd=0" ];
           Entity -> Vertex [ dirType="back", arrowType="open", style="dashed"
                               fontname=Helvetica, fontsize=8,
                               label="cd=dim" ];
        }
     \enddot

     @{
     @note
     There a three versions of Dune::Entity. Two of them are
     template specializations:
     \li Dune::Entity<cd,dim,GridImp,EntityImp> (general version)
     \li Dune::Entity<0,dim,GridImp,EntityImp> (Elements [cd=0])
     \li Dune::Entity<dim,dim,GridImp,EntityImp> (Vertices [cd=dim])
     These template specializations also show differences in the interface.
     \par
     An Entity is only accessible via an Iterator
     \li Dune::EntityPointer
     \li Dune::LevelIterator
     \li Dune::LeafIterator
     \li Dune::HierarchicIterator
     \li Dune::IntersectionIterator
     @}

     A Grid is a container of grid entities. An entity is parametrized
     by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Entities of codimension 0 allow to visit all neighbors, where a
     neighbor is an entity of codimension 0 which has a common entity of
     codimension 1 with the entity.  These neighbors are accessed via an
     iterator. This allows the implementation of non-matching
     meshes. The number of neigbors may be different from the number of
     faces/edges of an element!

     \ingroup GridInterface
   */
  template<int cd, int dim, class GridImp, template<int,int,class> class EntityImp>
  class Entity {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  protected:
    EntityImp<cd,dim,GridImp> realEntity;
  public:
    /** \brief the type of the wrapped implementation */
    typedef EntityImp<cd,dim,GridImp> ImplementationType;

    /** The corresponding geometry type */
    typedef typename GridImp::template Codim<cd>::Geometry Geometry;
    enum {
      /** know your own codimension */
      codimension=cd
    };
    enum {
      /** know the grid dimension */
      dimension=dim
    };
    enum {
      /** know dimension of the entity */
      mydimension=dim-cd
    };
    enum {
      /** know the dimension of world */
      dimensionworld=dimworld
    };
    /**
       @brief coordinate type of the Grid
     */
    typedef ct ctype;

    //! The level of this entity
    int level () const { return realEntity.level(); }

    //! Return partition type attribute
    PartitionType partitionType () const { return realEntity.partitionType(); }
    /**
       \brief Id of the boundary which is associated with
       the entity, returns 0 for inner entities, arbitrary int otherwise
     */
    int boundaryId () const { return realEntity.boundaryId(); }
    //! Geometry of this entity
    const Geometry& geometry () const { return realEntity.geometry(); }

    //! Copy constructor from EntityImp
    explicit Entity(const EntityImp<cd,dim,GridImp> & e) : realEntity(e) {};

    typedef typename RemoveConst<GridImp>::Type mutableGridImp;

  protected:
    // give the GridDefaultImplementation class access to the realImp
    friend class GridDefaultImplementation<
        GridImp::dimension, GridImp::dimensionworld,
        typename GridImp::ctype,
        typename GridImp::GridFamily> ;

    //! return reference to the real implementation
    EntityImp<cd,dim,GridImp> & getRealImp() { return realEntity; }
    //! return reference to the real implementation
    const EntityImp<cd,dim,GridImp> & getRealImp() const { return realEntity; }

  protected:
    /** hide copy constructor */
    Entity(const Entity& rhs) : realEntity(rhs.realEntity) {};
    /** hide assignement operator */
    Entity & operator = (const Entity& rhs) { realEntity = rhs.realEntity; };
  };

  /**
     @brief Template specialization of Dune::Entity for Elements [codim==0]

     @see Dune::Entity (general version) for the full documentation

     \dot
        digraph entity {
           rankdir=LR;
           node [ shape=record, fontname=Helvetica, fontsize=10, height=0.25 ];
           Entity [ label="Dune::Entity\<cd,dim,GridImp,EntityImp\>"
                    shape=record, URL="\ref Dune::Entity"];
           Element [ label="Dune::Entity\<0,dim,GridImp,EntityImp\>"
                    style=filled, bgcolor=lightgrey
                     URL="\ref Dune::Entity<0,dim,GridImp,EntityImp>"];
           Vertex [ label="Dune::Entity\<dim,dim,GridImp,EntityImp\>"
                    URL="\ref Dune::Entity<dim,dim,GridImp,EntityImp>"];
           Entity -> Element [ dirType="back", arrowType="open",
                               style="dashed"
                               fontname=Helvetica, fontsize=8,
                               label="codim=0" ];
           Entity -> Vertex [ dirType="back", arrowType="open", style="dashed"
                               fontname=Helvetica, fontsize=8,
                               label="codim=dim" ];
        }
     \enddot

     \ingroup GridInterface
   */
  template<int dim, class GridImp, template<int,int,class> class EntityImp>
  class Entity <0,dim,GridImp,EntityImp>
  {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;

    typedef typename RemoveConst<GridImp>::Type mutableGridImp;

  protected:
    EntityImp<0,dim,GridImp> realEntity;
  public:
    /** \brief the type of the wrapped implementation */
    typedef EntityImp<0,dim,GridImp> ImplementationType;

    /** \brief The geometry type of this entity */
    typedef typename GridImp::template Codim<0>::Geometry Geometry;

    /** \brief EntityPointer types of the different codimensions */
    template <int cd>
    struct Codim
    {
      typedef typename GridImp::template Codim<cd>::EntityPointer EntityPointer;
    };

    /** \brief The codim==0 EntityPointer type */
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;

    /** \brief The IntersectionIterator type*/
    typedef typename GridImp::template Codim<0>::IntersectionIterator IntersectionIterator;

    /** \brief The HierarchicIterator type*/
    typedef typename GridImp::template Codim<0>::HierarchicIterator HierarchicIterator;

    //! Know your own codimension
    enum { codimension=0 };
    //! Know the grid's dimension
    enum { dimension=dim };
    /** \brief Know dimension of the entity */
    enum { mydimension=dim };
    //! Know the world dimension
    enum { dimensionworld=dimworld };
    //! Type used for coordinates in grid module
    typedef ct ctype;

    //! Level of this entity
    int level () const { return realEntity.level(); }

    //! Partition type attribute
    PartitionType partitionType () const { return realEntity.partitionType(); }
    /** \brief Id of the boundary which is associated with
          the entity, returns 0 for inner entities, arbitrary int otherwise */
    int boundaryId () const { return realEntity.boundaryId(); }
    //! geometry of this entity
    const Geometry& geometry () const { return realEntity.geometry(); }

    /**
       \brief Number of subentities with codimension <tt>cc</tt>.
     */
    template<int cc> int count () const { return realEntity.count<cc>(); }

    /**
       \brief Access to subentity <tt>i</tt> of codimension <tt>cc</tt>.
     */
    template<int cc> typename Codim<cc>::EntityPointer entity (int i) const
    {
      return realEntity.entity<cc>(i);
    }
    /**
       \brief Intra-level access to intersections with neighboring elements.

       A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in common with this entity. Access to neighbors
       is provided using the IntersectionIterator. This allows meshes to be nonmatching.
       This method returns an iterator referencing the first neighbor.
     */
    IntersectionIterator ibegin () const
    {
      return realEntity.ibegin();
    }

    /**
       \brief Reference to an IntersectionIterator  past the last intersection
     */
    IntersectionIterator iend () const
    {
      return realEntity.iend();
    }
    /**
       \brief Inter-level access to father entity on the next-coarser grid.
       Assumes that meshes are nested.
     */
    EntityPointer father () const
    {
      return realEntity.father();
    }

    //! Returns true if the entity is a leaf in the grid hierarchy
    bool isLeaf () const
    {
      return realEntity.isLeaf();
    }

    /**
       \brief Location of this element relative to the reference element of the father.

       This is sufficient to interpolate all dofs in conforming case.
       Nonconforming may require access to neighbors of father and
       computations with local coordinates.
       On the fly case is somewhat inefficient since dofs  are visited several times.
       If we store interpolation matrices, this is tolerable. We assume that on-the-fly
       implementation of numerical algorithms is only done for simple discretizations.
       Assumes that meshes are nested.
     */
    const Geometry& geometryInFather () const
    {
      return realEntity.geometryInFather();
    }

    /**
       \brief Inter-level access to son elements on higher levels<=maxlevel.

       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
       \todo Does this point to the first son or the entity itself?
     */
    HierarchicIterator hbegin (int maxlevel) const
    {
      return realEntity.hbegin(maxlevel);
    }

    /**
       \brief Returns iterator to one past the last son
     */
    HierarchicIterator hend (int maxlevel) const
    {
      return realEntity.hend(maxlevel);
    }

    /**
       \brief The boundaryId of the i-th subentity of codimension <tt>cc</tt>

       This does the same as <code>entity<cc>(i).boundaryId()</code>, but it is
       usually a lot faster.
     */
    template <int cc> int subBoundaryId  ( int i ) const
    {
      return realEntity.subBoundaryId<cc>(i);
    }

    /**
       \brief Return whether entity could be coarsened or was refined
       or nothing happened
     */
    AdaptationState state () const { return realEntity.state(); }

    //! Copy constructor from EntityImp
    explicit Entity(const EntityImp<0,dim,GridImp> & e) : realEntity(e) {};

  protected:
    // give the GridDefaultImplementation class access to the realImp
    friend class GridDefaultImplementation<
        GridImp::dimension, GridImp::dimensionworld,
        typename GridImp::ctype,
        typename GridImp::GridFamily> ;

    //! return reference to the real implementation
    EntityImp<0,dim,GridImp> & getRealImp() { return realEntity; }
    //! return reference to the real implementation
    const EntityImp<0,dim,GridImp> & getRealImp() const { return realEntity; }

  protected:
    /** hide copy constructor */
    Entity(const Entity& rhs) : realEntity(rhs.realEntity) {};
    /** hide assignement operator */
    Entity & operator = (const Entity& rhs) {
      realEntity = rhs.realEntity;
      return *this;
    };
  };

  /**
     @brief Interface Definition for EntityImp
     \brief Template specialization of Dune::Entity for Vertices [codim==dim]

     @see Dune::Entity (general version) for the full documentation

     \dot
        digraph entity {
           rankdir=LR;
           node [ shape=record, fontname=Helvetica, fontsize=10, height=0.25 ];
           Entity [ label="Dune::Entity\<codim,dim,GridImp,EntityImp\>"
                    shape=record, URL="\ref Dune::Entity"];
           Element [ label="Dune::Entity\<0,dim,GridImp,EntityImp\>"
                     URL="\ref Dune::Entity<0,dim,GridImp,EntityImp>"];
           Vertex [ label="Dune::Entity\<dim,dim,GridImp,EntityImp\>"
                    style=filled, bgcolor=lightgrey
                    URL="\ref Dune::Entity<dim,dim,GridImp,EntityImp>"];
           Entity -> Element [ dirType="back", arrowType="open",
                               style="dashed"
                               fontname=Helvetica, fontsize=8,
                               label="codim=0" ];
           Entity -> Vertex [ dirType="back", arrowType="open", style="dashed"
                               fontname=Helvetica, fontsize=8,
                               label="codim=dim" ];
        }
     \enddot

     \ingroup GridInterface
   */
  template<int dim, class GridImp, template<int,int,class> class EntityImp>
  class Entity <dim,dim,GridImp,EntityImp>
  {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;

    typedef typename RemoveConst<GridImp>::Type mutableGridImp;

  protected:
    EntityImp<dim,dim,GridImp> realEntity;
  public:
    /** \brief the type of the wrapped implementation */
    typedef EntityImp<dim,dim,GridImp> ImplementationType;

    /** \brief Geometry type of this entity */
    typedef typename GridImp::template Codim<dim>::Geometry Geometry;

    /** \brief Codim 0 EntityPointer type*/
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;
    //! Know your own codimension
    enum { codimension=dim };
    //! Know the grid's dimension
    enum { dimension=dim };
    /** \brief Know dimension of the entity */
    enum { mydimension=0 };
    //! Know the world dimension
    enum { dimensionworld=dimworld };
    //! Type used for coordinates in grid module
    typedef ct ctype;

    //! Level of this entity
    int level () const { return realEntity.level(); }

    //! Partition type attribute
    PartitionType partitionType () const { return realEntity.partitionType(); }
    /** \brief Id of the boundary which is associated with
          the entity, returns 0 for inner entities, arbitrary int otherwise */
    int boundaryId () const { return realEntity.boundaryId(); }
    //! Geometry of this entity
    const Geometry& geometry () const { return realEntity.geometry(); }

    /** \brief Returns an element on the next-coarser level that contains this vertex

       This method is for fast implementations of interpolation for linear conforming elements.
       Of course, there may be more than one element on the coarser grid containing this
       vertex.  In that case it is not prescribed precisely which of those elements
       gets returned.
     */
    EntityPointer ownersFather () const { return realEntity.ownersFather(); }

    /** \brief This vertex' position in local coordinates of the owners father

       For fast implementation of P1 finite elements
     */
    const FieldVector<ct, dim>& positionInOwnersFather () const { return realEntity.positionInOwnersFather(); }

    //! Copy constructor from EntityImp
    explicit Entity(const EntityImp<dim,dim,GridImp> & e) : realEntity(e) {};

  protected:
    // give the GridDefaultImplementation class access to the realImp
    friend class GridDefaultImplementation<
        GridImp::dimension, GridImp::dimensionworld,
        typename GridImp::ctype,
        typename GridImp::GridFamily> ;

    //! return reference to the real implementation
    EntityImp<dim,dim,GridImp> & getRealImp() { return realEntity; }
    //! return reference to the real implementation
    const EntityImp<dim,dim,GridImp> & getRealImp() const { return realEntity; }

  protected:
    /** hide copy constructor */
    Entity(const Entity& rhs) : realEntity(rhs.realEntity) {};
    /** hide assignment operator */
    Entity & operator = (const Entity& rhs) {
      realEntity = rhs.realEntity;
      return *this;
    }

  };

  /********************************************************************/
  /**
     @brief Interface Definition for EntityImp

     @note this is the general version, but there are specializations
     for codim=0 and codim=dim

     @ingroup GridDevel
   */


  //********************************************************************
  /**
     @brief Interface Definition for EntityImp (Elements [cd=0])

     @note
     this specialization has an extended interface compared to the general case

     @ingroup GridDevel
   */
  //********************************************************************
  /**
     @brief Default Implementations for EntityImp

     EntityDefaultImplementation provides default implementations for Entity which uses
     the implemented interface which has to be done by the user.

     @note this is the general version, but there are specializations
     for cd=0 and cd=dim

     @ingroup GridDevel
   */
  template<int cd, int dim, class GridImp, template<int,int,class> class EntityImp>
  class EntityDefaultImplementation
  {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;

  public:
    //! know your own codimension
    enum { codimension=cd };

    //! know your own dimension
    enum { dimension=dim };

    /** \brief Know dimension of the entity */
    enum { mydimension=dim-cd };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

  private:
    //!  Barton-Nackman trick
    EntityImp<cd,dim,GridImp>& asImp () { return static_cast<EntityImp<cd,dim,GridImp>&>(*this); }
    const EntityImp<cd,dim,GridImp>& asImp () const { return static_cast<const EntityImp<cd,dim,GridImp>&>(*this); }
  }; // end EntityDefaultImplementation

  //********************************************************************
  /**
     @brief Default Implementations for EntityImp (Elements [cd=0])

     EntityDefaultImplementation provides default implementations for Entity which uses
     the implemented interface which has to be done by the user.

     @note
     this specialization has an extended interface compared to the general case

     @ingroup GridDevel
   */
  template<int dim, class GridImp, template<int,int,class> class EntityImp>
  class EntityDefaultImplementation <0,dim,GridImp,EntityImp>
  {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  public:
    //! know your own codimension
    enum { codimension=0 };

    //! know your own dimension
    enum { dimension=dim };

    /** \brief Know dimension of the entity */
    enum { mydimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    /** \brief Default implementation for access to boundaryId of sub entities
     *
     * Default implementation for access to boundaryId via interface method
     * entity<codim>.boundaryId(), default is very slow, but works, can be
     * overloaded be the actual grid implementation.
     */
    template <int cc> int subBoundaryId  ( int i ) const
    {
      return (asImp().template entity<cc>(i))->boundaryId();
    }

    //! return whether entity could be coarsened (COARSEN) or was refined
    //! (REFINED) or nothing happend (NONE)
    //! @return The default implementation returns NONE for grid with no
    //! adaptation
    AdaptationState state () const { return NONE; }

  private:
    //  Barton-Nackman trick
    EntityImp<0,dim,GridImp>& asImp () { return static_cast<EntityImp<0,dim,GridImp>&>(*this); }
    const EntityImp<0,dim,GridImp>& asImp () const { return static_cast<const EntityImp<0,dim,GridImp>&>(*this); }
  };

  //********************************************************************
  /**
     @brief Default Implementations for EntityImp (Vertice [cd=dim])

     EntityDefaultImplementation provides default implementations for Entity which uses
     the implemented interface which has to be done by the user.

     @note
     this specialization has a reduced interface compared to the general case

     @ingroup GridDevel
   */
  template<int dim, class GridImp, template<int,int,class> class EntityImp>
  class EntityDefaultImplementation <dim,dim,GridImp,EntityImp>
  {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  public:
    //! know your own codimension
    enum { codimension=dim };

    //! know your own dimension
    enum { dimension=dim };

    /** \brief Know dimension of the entity */
    enum { mydimension=0 };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;
  private:
    //  Barton-Nackman trick
    EntityImp<dim,dim,GridImp>& asImp () {return static_cast<EntityImp<dim,dim,GridImp>&>(*this);}
    const EntityImp<dim,dim,GridImp>& asImp () const { return static_cast<const EntityImp<dim,dim,GridImp>&>(*this); }
  };

}

#endif // DUNE_GRID_ENTITY_HH
