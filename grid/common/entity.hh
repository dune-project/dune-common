// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_ENTITY_HH
#define DUNE_GRID_ENTITY_HH

#include <dune/common/typetraits.hh>
#include "grid.hh"

namespace Dune
{

  /**
     @brief Wrapper class for entities


     Template parameters are:

     - <tt>cd</tt> Codimension of the entity
     - <tt>dim</tt> Dimension of the grid
     - <tt>GridImp</tt> Type that is a model of Dune::Grid
     - <tt>EntityImp</tt> Class template that is a model of Dune::Entity


     <H3>Engine Concept</H3>

     This class wraps a object of type EntityImp and forwards all member
     function calls to corresponding members of this class. In that sense Entity
     defines the interface and EntityImp supplies the implementation.
     For various reasons we do not use an inheritance hierarchy and the
     Barton-Nackman trick here.


     <H3>Specialization</H3>

     The Entity class template is specialized for <tt>cd=0</tt> (elements,
     Dune::Entity<0,dim,GridImp,EntityImp>)
     and <tt>cd=dim</tt> (vertices, Dune::Entity<dim,dim,GridImp,EntityImp>).
     These two cases have an extended interface,
     each adding different methods. The methods defined in the general template
     are provided by the two specializations as well. We did not use inheritance
     because different implementations for different codimensions may be required
     and virtual functions had to be avoided.

     This relation is shown in the following diagram:

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


     <H3>View concept</H3>

     Entities can not be created, assigned or otherwise modified outside
     the interface in the user code. They are only accessible by immutable
     iterators provided on the corresponding grid class.

     The only way to modify the entities of a grid is through grid adaptation which
     consists of tagging entities (of codimension 0) for refinement and then
     calling the adapt() method on the grid.


     \ingroup GIEntity
     \nosubgrouping
   */
  template<int cd, int dim, class GridImp, template<int,int,class> class EntityImp>
  class Entity {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  protected:
    EntityImp<cd,dim,GridImp> realEntity;
  public:

    //===========================================================
    /** @name Exported types and constants
     */
    //@{
    //===========================================================

    //! \brief The type of the wrapped implementation
    typedef EntityImp<cd,dim,GridImp> ImplementationType;

    //! \brief The corresponding geometry type
    typedef typename GridImp::template Codim<cd>::Geometry Geometry;

    enum {
      //! \brief Know your own codimension.
      codimension=cd
    };
    enum {
      //! \brief Know the grid dimension.
      dimension=dim
    };
    enum {
      //! \brief Dimensionality of the reference element of the entity.
      mydimension=dim-cd
    };
    enum {
      //! \brief Know the dimension of world.
      dimensionworld=dimworld
    };

    //! @brief coordinate type of the Grid
    typedef ct ctype;
    //@}



    //===========================================================
    /** @name Methods shared by entities of all codimensions
     */
    //@{
    //===========================================================

    //! The level of this entity
    int level () const { return realEntity.level(); }

    //! Partition type of this entity
    PartitionType partitionType () const { return realEntity.partitionType(); }

    /*! \brief Each entity encapsulates an object of type
          Dune::Geometry<dimension-codimension,dimensionworld,...> that
          gives (among other things) the map from a reference element to world coordinates.
          This method delivers a const reference to such a geometry.

          \note Be careful when storing such references. If the state
          of any object is changed, e.g. an iterator is advanced, there
          is no guarantee that the reference remains valid.
     */
    const Geometry& geometry () const { return realEntity.geometry(); }
    //@}



    //===========================================================
    /** @name Interface for the implementor
     */
    //@{
    //===========================================================

    //! Copy constructor from EntityImp
    explicit Entity(const EntityImp<cd,dim,GridImp> & e) : realEntity(e) {};

    /**
       \brief Id of the boundary which is associated with the entity,
           returns 0 for inner entities, arbitrary int otherwise
     */
    int boundaryId () const { return realEntity.boundaryId(); }
    //@}

    typedef typename RemoveConst<GridImp>::Type mutableGridImp;

  protected:
    //===========================================================
    /** @name Protected methods
     */
    //@{
    //===========================================================

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
    //@}
  };

  /**
     @brief Template specialization of Dune::Entity for Elements (codim==0)

     @see Dune::Entity (general version) for the full documentation

     \ingroup GIEntity
     \nosubgrouping
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

    //===========================================================
    /** @name Exported types and constants
     */
    //@{
    //===========================================================

    /** \brief the type of the wrapped implementation */
    typedef EntityImp<0,dim,GridImp> ImplementationType;

    /** \brief The geometry type of this entity */
    typedef typename GridImp::template Codim<0>::Geometry Geometry;

    /** \brief The geometry type of this entity when the geometry is expressed
       embedded in the father element.  This differs from Geometry only when
       dim != dimworld.*/
    typedef typename GridImp::template Codim<0>::LocalGeometry LocalGeometry;

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

    enum {
      //! Know your own codimension
      codimension=0
    };
    enum {
      //! Know the grid's dimension
      dimension=dim
    };
    enum {
      /** \brief Know dimension of the entity */
      mydimension=dim
    };
    enum {
      //! Know the world dimension
      dimensionworld=dimworld
    };
    //! Type used for coordinates in grid module
    typedef ct ctype;
    //@}


    //===========================================================
    /** @name Methods shared by entities of all codimensions
     */
    //@{
    //===========================================================

    //! @copydoc Dune::Entity::level()
    int level () const { return realEntity.level(); }

    //! @copydoc Dune::Entity::partitionType()
    PartitionType partitionType () const { return realEntity.partitionType(); }

    //! @copydoc Dune::Entity::geometry()
    const Geometry& geometry () const { return realEntity.geometry(); }
    //@}


    //===========================================================
    /** @name Extended interface of entities of codimension 0
     */
    //@{
    //===========================================================

    /**\brief Number of subentities with codimension <tt>cc</tt>. This method is in
           principle redundant because this information can be obtained via the reference
           element of the geometry. It is there for efficiency reasons and to make
           the interface self-contained.
     */
    template<int cc> int count () const { return realEntity.count<cc>(); }

    /** \brief Access to subentity <tt>i</tt> of codimension <tt>cc</tt>.
     */
    template<int cc> typename Codim<cc>::EntityPointer entity (int i) const
    {
      return realEntity.entity<cc>(i);
    }

    /**\brief Intra-level access to intersections with neighboring elements.
       A neighbor is an entity of codimension 0
       which has an intersection of codimension 1 in common with this entity.
           Access to those neighbors is provided using the IntersectionIterator.
       This method returns an iterator refering to the first neighbor.
     */
    IntersectionIterator ibegin () const
    {
      return realEntity.ibegin();
    }

    /**\brief Reference to an IntersectionIterator one past the last intersection
     */
    IntersectionIterator iend () const
    {
      return realEntity.iend();
    }

    /**\brief Inter-level access to father entity on the next-coarser grid.
           The given entity resulted directly from a subdivision of its father
           entity. For the macro elements dereferencing the EntityPointer is undefined.
     */
    EntityPointer father () const
    {
      return realEntity.father();
    }

    //! Returns true if the entity is contained in the leaf grid
    bool isLeaf () const
    {
      return realEntity.isLeaf();
    }

    /**\brief Provides information how this element has been subdivided from
           its father element.
           The returned LocalGeometry is a model of Dune::Geometry<dimension,dimension,...>
           mapping from the reference element of the given element to the reference
           element of the father element.
       This is sufficient to interpolate all degrees of freedom in the
           conforming case. Nonconforming may require access to neighbors of father and
       computations with local coordinates.
       On the fly case is somewhat inefficient since degrees of freedom
           may be visited several times.
       If we store interpolation matrices, this is tolerable. We assume that on-the-fly
       implementation of interpolation is only done for simple discretizations.
     */
    const LocalGeometry& geometryInFather () const
    {
      return realEntity.geometryInFather();
    }

    /**\brief Inter-level access to elements that resulted from (recursive)
           subdivision of this element.

           \param[in] maxlevel Iterator does not stop at elements with level greater than maxlevel.
           \return Iterator to the first son (level is not greater than maxlevel)
     */
    HierarchicIterator hbegin (int maxlevel) const
    {
      return realEntity.hbegin(maxlevel);
    }

    /** \brief Returns iterator to one past the last son element
     */
    HierarchicIterator hend (int maxlevel) const
    {
      return realEntity.hend(maxlevel);
    }

    /**\brief Return current adaptation state of entity. See explanation of AdaptationState.
     */
    AdaptationState state () const { return realEntity.state(); }
    //@}


    //===========================================================
    /** @name Interface for the implementor
     */
    //@{
    //===========================================================

    //! Copy constructor from EntityImp
    explicit Entity(const EntityImp<0,dim,GridImp> & e) : realEntity(e) {};

    //! @copydoc Dune::Entity::boundaryId()
    int boundaryId () const { return realEntity.boundaryId(); }

    /**
       \brief The boundaryId of the i-th subentity of codimension <tt>cc</tt>

       This does the same as <code>entity<cc>(i).boundaryId()</code>, but it is
       usually a lot faster.
     */
    template <int cc> int subBoundaryId  ( int i ) const
    {
      return realEntity.subBoundaryId<cc>(i);
    }
    //@}


  protected:
    //===========================================================
    /** @name Protected methods
     */
    //@{
    //===========================================================

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
    //@}
  };

  /**
     @brief Interface Definition for EntityImp
     \brief Template specialization of Dune::Entity for Vertices (codim==dim)

     @see Dune::Entity (general version) for the full documentation


     \ingroup GIEntity
     \nosubgrouping
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

    //===========================================================
    /** @name Exported types and constants
     */
    //@{
    //===========================================================

    /** \brief the type of the wrapped implementation */
    typedef EntityImp<dim,dim,GridImp> ImplementationType;

    /** \brief Geometry type of this entity */
    typedef typename GridImp::template Codim<dim>::Geometry Geometry;

    /** \brief Codim 0 EntityPointer type*/
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;

    enum {
      //! Know your own codimension
      codimension=dim
    };
    enum {
      //! Know the grid's dimension
      dimension=dim
    };
    enum {
      /** \brief Know dimension of the entity */
      mydimension=0
    };
    enum {
      //! Know the world dimension
      dimensionworld=dimworld
    };
    //! Type used for coordinates in grid module
    typedef ct ctype;
    //@}


    //===========================================================
    /** @name Methods shared by entities of all codimensions
     */
    //@{
    //===========================================================


    //!  @copydoc Dune::Entity::level()
    int level () const { return realEntity.level(); }

    //!  @copydoc Dune::Entity::partitionType()
    PartitionType partitionType () const { return realEntity.partitionType(); }

    //!  @copydoc Dune::Entity::geometry()
    const Geometry& geometry () const { return realEntity.geometry(); }
    //@}


    //===========================================================
    /** @name Extended interface of entities of codimension dim
     */
    //@{
    //===========================================================

    /** \brief Returns EntityPointer to an element on the next-coarser level
            that contains this vertex.
       This method is for fast implementations of interpolation for linear conforming elements.
       Of course, there may be more than one element on the coarser grid containing this
       vertex.  In that case it is not prescribed precisely which of those elements
       gets returned.
     */
    EntityPointer ownersFather () const { return realEntity.ownersFather(); }

    /** \brief This vertex' position in local coordinates of the element returned
            by the ownersFather() method. Thus both methods together allow
            the pointwise interpolation for conforming finite elements.
     */
    const FieldVector<ct, dim>& positionInOwnersFather () const
    { return realEntity.positionInOwnersFather(); }
    //@}



    //===========================================================
    /** @name Interface for the implementor
     */
    //@{
    //===========================================================

    //! Copy constructor from EntityImp
    explicit Entity(const EntityImp<dim,dim,GridImp> & e) : realEntity(e) {};

    /** \brief Id of the boundary which is associated with
          the entity, returns 0 for inner entities, arbitrary int otherwise */
    int boundaryId () const { return realEntity.boundaryId(); }
    //@}


  protected:

    //===========================================================
    /** @name Protected methods
     */
    //@{
    //===========================================================

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
    //@}

  };

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
    EntityImp<cd,dim,GridImp>& asImp ()
    {
      return static_cast<EntityImp<cd,dim,GridImp>&>(*this);
    }
    const EntityImp<cd,dim,GridImp>& asImp () const
    {
      return static_cast<const EntityImp<cd,dim,GridImp>&>(*this);
    }
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
