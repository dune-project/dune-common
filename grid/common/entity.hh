// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_ENTITY_HH
#define DUNE_GRID_ENTITY_HH

#include <dune/common/typetraits.hh>

namespace Dune
{

  /** \brief The wrapper class for entities of arbitrary codimension
   * \ingroup GridCommon
   *
   */
  template<int codim, int dim, class GridImp, template<int,int,class> class EntityImp>
  class Entity {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  protected:
    EntityImp<codim,dim,GridImp> realEntity;
  public:

    //! The corresponding geometry type
    typedef typename GridImp::template codim<codim>::Geometry Geometry;
    //! know your own codimension
    enum { codimension=codim };
    //! know the grid dimension
    enum { dimension=dim };
    //! know dimension of the entity
    enum { mydimension=dim-codim };
    //! know the dimension of world
    enum { dimensionworld=dimworld };
    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! The level of this entity
    int level () const { return realEntity.level(); }
    //! Unique and consecutive index per level and codim
    int index () const { return realEntity.index(); }
    //! The global unique index
    int globalIndex () const { return realEntity.globalIndex(); }
    //! Return partition type attribute
    PartitionType partitionType () const { return realEntity.partitionType(); }
    /** \brief Id of the boundary which is associated with
        the entity, returns 0 for inner entities, arbitrary int otherwise */
    int boundaryId () const { return realEntity.boundaryId(); }
    //! Geometry of this entity
    const Geometry& geometry () const { return realEntity.geometry(); }

    //! Copy constructor from EntityImp
    explicit Entity(const EntityImp<codim,dim,GridImp> & e) : realEntity(e) {};

    typedef typename RemoveConst<GridImp>::Type mutableGridImp;

#ifdef __ICC
    // for icc
    friend EntityImp<codim,dim,GridImp>& mutableGridImp::template getRealEntity<>(typename GridImp::Traits::template codim<codim>::Entity& e );
    friend const EntityImp<codim,dim,GridImp>& mutableGridImp::template getRealEntity<>(const typename GridImp::Traits::template codim<codim>::Entity& e ) const;
#else
    // for g++
    template <int cd>
    friend EntityImp<cd,dim,GridImp>& mutableGridImp::getRealEntity(typename GridImp::Traits::template codim<cd>::Entity& e );
    template <int cd>
    friend const EntityImp<cd,dim,GridImp>& mutableGridImp::getRealEntity(const typename GridImp::Traits::template codim<cd>::Entity& e ) const;
#endif

  protected:
    /** hide copy constructor */
    Entity(const Entity& rhs) : realEntity(rhs.realEntity) {};
    /** hide assignement operator */
    Entity & operator = (const Entity& rhs) { realEntity = rhs.realEntity; };
  };

  /** \brief The wrapper class for entities of codimension zero, i.e. elements
   * \ingroup GridCommon
   */
  template<int dim, class GridImp, template<int,int,class> class EntityImp>
  class Entity <0,dim,GridImp,EntityImp>
  {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;

    typedef typename RemoveConst<GridImp>::Type mutableGridImp;

#ifdef __ICC
    // for icc
    friend EntityImp<0,dim,GridImp>& mutableGridImp::template getRealEntity<>(typename GridImp::Traits::template codim<0>::Entity& e );
    friend const EntityImp<0,dim,GridImp>& mutableGridImp::template getRealEntity<>(const typename GridImp::Traits::template codim<0>::Entity& e ) const;
#else
    // for g++
    template <int cd>
    friend EntityImp<cd,dim,GridImp>& mutableGridImp::getRealEntity(typename mutableGridImp::Traits::template codim<cd>::Entity& e );
    template <int cd>
    friend const EntityImp<cd,dim,GridImp>& mutableGridImp::getRealEntity(const typename mutableGridImp::Traits::template codim<cd>::Entity& e ) const;
#endif

  protected:
    EntityImp<0,dim,GridImp> realEntity;
  public:

    /** \brief The geometry type of this entity */
    typedef typename GridImp::template codim<0>::Geometry Geometry;

    /** \brief EntityPointer types of the different codimensions */
    template <int cd>
    struct codim
    {
      typedef typename GridImp::template codim<cd>::EntityPointer EntityPointer;
    };

    /** \brief The codim==0 EntityPointer type */
    typedef typename GridImp::template codim<0>::EntityPointer EntityPointer;

    /** \brief The IntersectionIterator type*/
    typedef typename GridImp::template codim<0>::IntersectionIterator IntersectionIterator;

    /** \brief The HierarchicIterator type*/
    typedef typename GridImp::template codim<0>::HierarchicIterator HierarchicIterator;

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
    //! Unique and consecutive index per level and codim
    int index () const { return realEntity.index(); }
    //! Global unique index per grid
    int globalIndex () const { return realEntity.globalIndex(); }
    //! Partition type attribute
    PartitionType partitionType () const { return realEntity.partitionType(); }
    /** \brief Id of the boundary which is associated with
          the entity, returns 0 for inner entities, arbitrary int otherwise */
    int boundaryId () const { return realEntity.boundaryId(); }
    //! geometry of this entity
    const Geometry& geometry () const { return realEntity.geometry(); }

    /** \brief Number of subentities with codimension <tt>cc</tt>.
     */
    template<int cc> int count () const { return realEntity.count<cc>(); }

    /** \brief Access to subentity <tt>i</tt> of codimension <tt>cc</tt>.
     */
    template<int cc> typename codim<cc>::EntityPointer entity (int i) const
    {
      return realEntity.entity<cc>(i);
    }
    /** \brief Intra-level access to intersections with neighboring elements.

       A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in common with this entity. Access to neighbors
       is provided using the IntersectionIterator. This allows meshes to be nonmatching.
       This method returns an iterator referencing the first neighbor.
     */
    IntersectionIterator ibegin () const
    {
      return realEntity.ibegin();
    }

    //! Reference to an IntersectionIterator  past the last intersection
    IntersectionIterator iend () const
    {
      return realEntity.iend();
    }
    //! Inter-level access to father entity on the next-coarser grid. Assumes that meshes are nested.
    EntityPointer father () const
    {
      return realEntity.father();
    }

    //! Returns true if the entity is a leaf in the grid hierarchy
    bool isLeaf ()
    {
      return realEntity.isLeaf();
    }

    /** \brief Location of this element relative to the reference element of the father.

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

    /** \brief Inter-level access to son elements on higher levels<=maxlevel.

       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
       \todo Does this point to the first son or the entity itself?
     */
    HierarchicIterator hbegin (int maxlevel) const
    {
      return realEntity.hbegin(maxlevel);
    }

    //! Returns iterator to one past the last son
    HierarchicIterator hend (int maxlevel) const
    {
      return realEntity.hend(maxlevel);
    }

    /** \brief The index of the i-th subentity of codimension <tt>cc</tt>
     *
     * This does the same as <code>entity<cc>(i).index()</code>, but it is
     * usually a lot faster.
     */
    template <int cc> int subIndex ( int i ) const
    {
      return realEntity.subIndex<cc>(i);
    }

    /** \brief The boundaryId of the i-th subentity of codimension <tt>cc</tt>
     *
     * This does the same as <code>entity<cc>(i).boundaryId()</code>, but it is
     * usually a lot faster.
     */
    template <int cc> int subBoundaryId  ( int i ) const
    {
      return realEntity.subBoundaryId<cc>(i);
    }

    //***************************************************************
    //  Interface for Adaptation
    //***************************************************************
    /* \brief Return whether entity could be coarsened (COARSEN) or was refined
       (REFINED) or nothing happened (NONE)

       @return The default implementation returns NONE for grid with no
       adaptation
     */
    AdaptationState state () const { return realEntity.state(); }

    //! Copy constructor from EntityImp
    explicit Entity(const EntityImp<0,dim,GridImp> & e) : realEntity(e) {};

  protected:
    /** hide copy constructor */
    Entity(const Entity& rhs) : realEntity(rhs.realEntity) {};
    /** hide assignement operator */
    Entity & operator = (const Entity& rhs) { realEntity = rhs.realEntity; };
  };

  /** \brief The wrapper class for entities representing vertices
   * \ingroup GridCommon
   */
  template<int dim, class GridImp, template<int,int,class> class EntityImp>
  class Entity <dim,dim,GridImp,EntityImp>
  {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;

    typedef typename RemoveConst<GridImp>::Type mutableGridImp;

#ifdef __ICC
    // for icc
    friend EntityImp<dim,dim,GridImp>& mutableGridImp::template getRealEntity<>(typename GridImp::Traits::template codim<dim>::Entity& e );
    friend const EntityImp<dim,dim,GridImp>& mutableGridImp::template getRealEntity<>(const typename GridImp::Traits::template codim<dim>::Entity& e ) const;
#else
    // for g++
    template <int cd>
    friend EntityImp<cd,dim,GridImp>& mutableGridImp::getRealEntity(typename GridImp::Traits::template codim<cd>::Entity& e );
    template <int cd>
    friend const EntityImp<cd,dim,GridImp>& mutableGridImp::getRealEntity(const typename GridImp::Traits::template codim<cd>::Entity& e ) const;
#endif

  protected:
    EntityImp<dim,dim,GridImp> realEntity;
  public:

    /** \brief Geometry type of this entity */
    typedef typename GridImp::template codim<dim>::Geometry Geometry;

    /** \brief Codim 0 EntityPointer type*/
    typedef typename GridImp::template codim<0>::EntityPointer EntityPointer;
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
    //! Unique and consecutive index per level and codim
    int index () const { return realEntity.index(); }
    //! Global unique index per grid
    int globalIndex () const { return realEntity.globalIndex(); }
    //! Partition type attribute
    PartitionType partitionType () const { return realEntity.partitionType(); }
    /** \brief Id of the boundary which is associated with
          the entity, returns 0 for inner entities, arbitrary int otherwise */
    int boundaryId () const { return realEntity.boundaryId(); }
    //! Geometry of this entity
    const Geometry& geometry () const { return realEntity.geometry(); }

    /** \todo Please doc me! */
    EntityPointer ownersFather () const { return realEntity.ownersFather(); }

    //! This entity's position in local coordinates of the owners father
    FieldVector<ct, dim>& positionInOwnersFather () const { return realEntity.positionInOwnersFather(); }

    //! Copy constructor from EntityImp
    explicit Entity(const EntityImp<dim,dim,GridImp> & e) : realEntity(e) {};

  protected:
    /** hide copy constructor */
    Entity(const Entity& rhs) : realEntity(rhs.realEntity) {};
    /** hide assignement operator */
    Entity & operator = (const Entity& rhs) { realEntity = rhs.realEntity; };
  };

  //********************************************************************
  //
  // --EntityInterface
  //
  //********************************************************************

  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Here: the general template
   */
  template<int codim, int dim, class GridImp, template<int,int,class> class EntityImp>
  class EntityInterface {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  public:

    typedef typename GridImp::template codim<codim>::Geometry Geometry;

    //! know your own codimension
    enum { codimension=codim };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! level of this entity
    int level () const { return asImp().level(); }

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index () const { return asImp().index(); }


    //! return partition type attribute
    PartitionType partitionType () const { return asImp().partitionType(); }

    //! id of the boundary which is associated with
    //! the entity, returns 0 for inner entities, arbitrary int otherwise
    int boundaryId () const { return asImp().boundaryId(); }

    //! geometry of this entity
    const Geometry& geometry () const { return asImp().geometry(); }

  private:
    //!  Barton-Nackman trick
    EntityImp<codim,dim,GridImp>& asImp () {return static_cast<EntityImp<codim,dim,GridImp>&>(*this);}
    const EntityImp<codim,dim,GridImp>& asImp () const { return static_cast<const EntityImp<codim,dim,GridImp>&>(*this); }
  };


  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Entities of codimension 0 ("elements") are defined through template specialization. Note
     that this specialization has an extended interface compared to the general case

     Entities of codimension 0  allow to visit all neighbors, where
     a neighbor is an entity of codimension 0 which has a common entity of codimension 1 with the entity.
     These neighbors are accessed via an iterator. This allows the implementation of
     non-matching meshes. The number of neigbors may be different from the number of faces/edges
     of an element!
   */
  template<int dim, class GridImp, template<int,int,class> class EntityImp>
  class EntityInterface <0,dim,GridImp,EntityImp>
  {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  public:

    typedef typename GridImp::template codim<0>::Geometry Geometry;
    typedef typename GridImp::template codim<0>::EntityPointer EntityPointer;
    typedef typename GridImp::template codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename GridImp::template codim<0>::HierarchicIterator HierarchicIterator;

    //! know your own codimension
    enum { codimension=0 };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! level of this entity
    int level () const { return asImp().level(); }

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index () const { return asImp().index(); }

    //! return partition type attribute
    PartitionType partitionType () const { return asImp().partitionType(); }

    //! geometry of this entity
    const Geometry& geometry () const { return asImp().geometry(); }

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
       with codimension cc.
     */
    template<int cc> int count () const { return asImp().count<cc>(); }

    /*! Provide access to mesh entity i of given codimension. Entities
       are numbered 0 ... count<cc>()-1
     */
    template<int cc> EntityPointer entity (int i) const
    {
      return asImp().entity<cc>(i);
    }

    /*! Intra-level access to intersections with neighboring elements.
       A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor.
     */
    IntersectionIterator ibegin () const
    {
      return asImp().ibegin();
    }

    //! Reference to one past the last intersection
    IntersectionIterator iend () const
    {
      return asImp().iend();
    }

    //! Inter-level access to father element on coarser grid. Assumes that meshes are nested.
    EntityPointer father () const
    {
      return asImp().father();
    }

    //! return true if the entity is leaf
    bool isLeaf () const
    {
      return asImp().isLeaf();
    }

    /*! Location of this element relative to the reference element of the father.
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
      return asImp().geometryInFather();
    }

    /*! Inter-level access to son elements on higher levels<=maxlevel.
       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
     */
    HierarchicIterator hbegin (int maxlevel) const
    {
      return asImp().hbegin(maxlevel);
    }

    //! Returns iterator to one past the last son
    HierarchicIterator hend (int maxlevel) const
    {
      return asImp().hend(maxlevel);
    }

  private:
    //!  Barton-Nackman trick
    EntityImp<0,dim,GridImp>& asImp () { return static_cast<EntityImp<0,dim,GridImp>&>(*this); }
    const EntityImp<0,dim,GridImp>& asImp () const { return static_cast<const EntityImp<0,dim,GridImp>&>(*this); }
  };

  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Entities of codimension=dimension ("vertices") are defined through template specialization. Note
     that this specialization has an extended interface compared to the general case
   */
  template<int dim, class GridImp, template<int,int,class> class EntityImp>
  class EntityInterface <dim,dim,GridImp,EntityImp>
  {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  public:
    typedef typename GridImp::template codim<dim>::Geometry Geometry;
    typedef typename GridImp::template codim<dim>::EntityPointer EntityPointer;

    //! know your own codimension
    enum { codimension=dim };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! level of this entity
    int level () const { return asImp().level(); }

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index () const { return asImp().index(); }

    //! return partition type attribute
    PartitionType partitionType () const { return asImp().partitionType(); }

    //! geometry of this entity
    const Geometry& geometry () const { return asImp().geometry(); }

    /*! Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    EntityPointer father () const
    {
      return asImp().father();
    }

    //! local coordinates within father
    FieldVector<ct, dim>& local () const { return asImp().local(); }

  private:
    //!  Barton-Nackman trick
    EntityImp<dim,dim,GridImp>& asImp () {return static_cast<EntityImp<dim,dim,GridImp>&>(*this);}
    const EntityImp<dim,dim,GridImp>& asImp () const { return static_cast<const EntityImp<dim,dim,GridImp>&>(*this); }
  };

  //********************************************************************
  //
  // --EntityDefault
  //
  //! EntityDefault provides default implementations for Entity which uses
  //! the implemented interface which has to be done by the user.
  //
  //********************************************************************

  template<int codim, int dim, class GridImp, template<int,int,class> class EntityImp>
  class EntityDefault
    : public EntityInterface <codim,dim,GridImp,EntityImp>
  {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  private:
    //!  Barton-Nackman trick
    EntityImp<codim,dim,GridImp>& asImp () { return static_cast<EntityImp<codim,dim,GridImp>&>(*this); }
    const EntityImp<codim,dim,GridImp>& asImp () const { return static_cast<const EntityImp<codim,dim,GridImp>&>(*this); }
  }; // end EntityDefault

  template<int dim, class GridImp, template<int,int,class> class EntityImp>
  class EntityDefault <0,dim,GridImp,EntityImp> : public EntityInterface <0,dim,GridImp,EntityImp>
  {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  public:
    /** \brief Default implementation for access to subIndex
     *
     * Default implementation for access to subIndex via interface method entity
     * default is to return the index of the sub entity, is very slow, but works
     */
    template <int cc> int subIndex ( int i ) const
    {
      return (asImp().template entity<cc>(i))->index();
    }

    /** \brief Default implementation for access to boundaryId of sub entities
     *
     * Default implementation for access to boundaryId via interface method
     * entity<codim>.boundaryId(), default is very slow, but works, can be
     * overloaded be the actual grid implementation, works the same way as
     * subIndex
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
    //!  Barton-Nackman trick
    EntityImp<0,dim,GridImp>& asImp () { return static_cast<EntityImp<0,dim,GridImp>&>(*this); }
    const EntityImp<0,dim,GridImp>& asImp () const { return static_cast<const EntityImp<0,dim,GridImp>&>(*this); }
  };

  template<int dim, class GridImp, template<int,int,class> class EntityImp>
  class EntityDefault <dim,dim,GridImp,EntityImp> : public EntityInterface <dim,dim,GridImp,EntityImp>
  {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  private:
    //!  Barton-Nackman trick
    EntityImp<dim,dim,GridImp>& asImp () {return static_cast<EntityImp<dim,dim,GridImp>&>(*this);}
    const EntityImp<dim,dim,GridImp>& asImp () const { return static_cast<const EntityImp<dim,dim,GridImp>&>(*this); }
  };

}

#endif // DUNE_GRID_ENTITY_HH
