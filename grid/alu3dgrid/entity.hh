// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDENTITY_HH
#define DUNE_ALU3DGRIDENTITY_HH

// System includes

// Dune includes
#include "../common/entity.hh"
#include "../common/intersectioniteratorwrapper.hh"

// Local includes
#include "alu3dinclude.hh"
#include "iterator.hh"

namespace Dune {
  // Forward declarations
  template<int cd, int dim, class GridImp>
  class ALU3dGridEntity;
  template<int cd, PartitionIteratorType pitype, class GridImp >
  class ALU3dGridLevelIterator;
  template<int cd, class GridImp >
  class ALU3dGridEntityPointer;
  template<int mydim, int coorddim, class GridImp>
  class ALU3dGridGeometry;
  template<class GridImp>
  class ALU3dGridHierarchicIterator;
  template<class GridImp>
  class ALU3dGridIntersectionIterator;
  template<int codim, PartitionIteratorType, class GridImp>
  class ALU3dGridLeafIterator;
  template<int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGrid;

  //**********************************************************************
  //
  // --ALU3dGridEntity
  // --Entity
  // --Men
  //
  //**********************************************************************
  template<int cd, int dim, class GridImp>
  class ALU3dGridMakeableEntity :
    public GridImp::template Codim<cd>::Entity
  {
    // typedef typename GridImp::template Codim<codim>::Entity EntityType;
    friend class ALU3dGridEntity<cd, dim, GridImp>;
    typedef ALU3dImplTraits<GridImp::elementType> ImplTraitsType;

    typedef typename ImplTraitsType::PLLBndFaceType PLLBndFaceType;
    typedef typename ALU3dImplTraits<GridImp::elementType>::
    template Codim<cd>::ImplementationType IMPLElementType;
    typedef typename ALU3dImplTraits<GridImp::elementType>::
    template Codim<cd>::InterfaceType MyHElementType;

  public:

    // Constructor creating the realEntity
    ALU3dGridMakeableEntity(const GridImp & grid, int level) :
      GridImp::template Codim<cd>::
      Entity (ALU3dGridEntity<cd, dim, GridImp>(grid,level)) {}

    //! set element as normal entity
    //! ItemTypes are HElementType, HFaceType, HEdgeType and VertexType
    template <class ItemType>
    void setElement(ItemType & item, int twist = 0, int face = -1)
    {
      this->realEntity.setElement(item, twist,face);
    }

    //! set original element pointer to fake entity
    void setGhost(ALU3DSPACE HBndSegType &ghost)
    {
      this->realEntity.setGhost(ghost);
    }

    void reset ( int l )
    {
      this->realEntity.reset(l);
    }

    void removeElement ()
    {
      this->realEntity.removeElement();
    }

    bool equals ( const ALU3dGridMakeableEntity<cd,dim,GridImp> & org )
    {
      return this->realEntity.equals(org.realEntity);
    }

    void setEntity ( const ALU3dGridMakeableEntity<cd,dim,GridImp> & org )
    {
      this->realEntity.setEntity(org.realEntity);
    }

    // return reference to internal item
    // should be private, but the list of friends would be to long
    const MyHElementType & getItem () const { return this->realEntity.getItem(); }
  };

  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Here: the general template
   */
  template<int cd, int dim, class GridImp>
  class ALU3dGridEntity :
    public EntityDefaultImplementation <cd,dim,GridImp,ALU3dGridEntity>
  {
    enum { dimworld = GridImp::dimensionworld };

    friend class ALU3dGrid < dim , dimworld, GridImp::elementType >;
    friend class ALU3dGridEntity < 0, dim, GridImp >;
    friend class ALU3dGridLevelIterator < cd, All_Partition, GridImp >;
    friend class ALU3dGridMakeableEntity <cd, dim, GridImp >;

    friend class ALU3dGridHierarchicIndexSet<dim,dimworld,GridImp::elementType>;

  public:
    typedef typename ALU3dImplTraits<GridImp::elementType>::template Codim<cd>::InterfaceType ElementType;
    typedef typename ALU3dImplTraits<GridImp::elementType>::template Codim<cd>::ImplementationType IMPLElementType;

    typedef typename GridImp::template Codim<cd>::Entity Entity;
    typedef typename GridImp::template Codim<cd>::Geometry Geometry;
    typedef ALU3dGridMakeableGeometry<dim-cd,GridImp::dimensionworld,GridImp> GeometryImp;
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;

    //! level of this element
    int level () const;

    //! return partition type of this entity ( see grid.hh )
    PartitionType partitionType() const;

    //! Constructor
    ALU3dGridEntity(const GridImp &grid, int level);

    //! geometry of this entity
    const Geometry & geometry () const;

    /*! Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    EntityPointer ownersFather () const;

    //! my position in local coordinates of the owners father
    FieldVector<alu3d_ctype, dim>& positionInOwnersFather () const;

    // set element as normal entity
    void setElement(const ElementType & item, int twist=0, int face = -1);
    void setElement(const ALU3DSPACE HElementType & el, const ALU3DSPACE VertexType & vx);

    //! setGhost is not valid for this codim
    void setGhost(const ALU3DSPACE HBndSegType  &ghost);

    //! reset item pointer to NULL
    void removeElement ();

    //! reset item pointer to NULL
    void reset ( int l );

    //! compare 2 elements by comparing the item pointers
    bool equals ( const ALU3dGridEntity<cd,dim,GridImp> & org ) const;

    //! set item from other entity, mainly for copy constructor of entity pointer
    void setEntity ( const ALU3dGridEntity<cd,dim,GridImp> & org );

  private:
    // return reference to internal item
    const IMPLElementType & getItem () const { return *item_; }

    //! index is unique within the grid hierachy and per codim
    int getIndex () const;

    // the grid this entity belongs to
    const GridImp &grid_;

    int level_; //! level of entity
    int gIndex_; //! hierarchic index
    int twist_; //! twist of the underlying ALU element (with regard to the element that asked for it)
    int face_; //! for face, know on which face we are

    // corresponding ALU3dGridElement
    const IMPLElementType * item_;
    const ALU3DSPACE HElementType * father_;

    //! the cuurent geometry
    mutable GeometryImp geo_;
    mutable bool builtgeometry_;       //!< true if geometry has been constructed

    mutable bool localFCoordCalced_;
    mutable FieldVector<alu3d_ctype, dim> localFatherCoords_; //! coords of vertex in father
  };

  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Entities of codimension 0 ("elements") are defined through template specialization. Note
     that this specialization has an extended interface compared to the general case

     Entities of codimension 0  allow to visit all neighbors, where
     a neighbor is an entity of codimension 0 which has a common entity of codimension 1 with the
     These neighbors are accessed via an iterator. This allows the implementation of
     non-matching meshes. The number of neigbors may be different from the number of faces/edges
     of an element!
   */
  //***********************
  //
  //  --ALU3dGridEntity
  //  --0Entity
  //
  //***********************
  template<int dim, class GridImp>
  class ALU3dGridEntity<0,dim,GridImp>
    : public EntityDefaultImplementation<0,dim,GridImp,ALU3dGridEntity>
  {
    enum { dimworld = GridImp::dimensionworld };
    typedef typename ALU3dImplTraits<GridImp::elementType>::GEOElementType GEOElementType;
    typedef typename ALU3dImplTraits<GridImp::elementType>::PLLBndFaceType PLLBndFaceType;
    typedef typename ALU3dImplTraits<GridImp::elementType>::IMPLElementType IMPLElementType;

    enum { refine_element_t =
             ALU3dImplTraits<GridImp::elementType>::refine_element_t };
    enum { coarse_element_t =
             ALU3dImplTraits<GridImp::elementType>::coarse_element_t };
    enum { nosplit_element_t = ALU3dImplTraits<GridImp::elementType>::nosplit_element_t };

    friend class ALU3dGrid < dim , dimworld, GridImp::elementType>;
    friend class ALU3dGridIntersectionIterator < GridImp >;
    friend class ALU3dGridIntersectionIterator < const GridImp >;
    friend class ALU3dGridHierarchicIterator   < const GridImp >;
    friend class ALU3dGridHierarchicIterator   < GridImp >;
    friend class ALU3dGridLevelIterator <0,All_Partition,GridImp>;
    friend class ALU3dGridLevelIterator <1,All_Partition,GridImp>;
    friend class ALU3dGridLevelIterator <2,All_Partition,GridImp>;
    friend class ALU3dGridLevelIterator <3,All_Partition,GridImp>;
    friend class ALU3dGridLeafIterator <0, All_Partition,GridImp>;
    friend class ALU3dGridLeafIterator <1, All_Partition,GridImp>;
    friend class ALU3dGridLeafIterator <2, All_Partition,GridImp>;
    friend class ALU3dGridLeafIterator <3, All_Partition,GridImp>;
    friend class ALU3dGridMakeableEntity<0,dim,GridImp>;

    friend class ALU3dGridHierarchicIndexSet<dim,dimworld,GridImp::elementType>;

    typedef typename SelectType<
        SameType<GridImp,const ALU3dGrid<3, 3, tetra> >::value,
        ReferenceSimplex<alu3d_ctype, 3>,
        ReferenceCube<alu3d_ctype, 3>
        >::Type ReferenceElementType;

  public:
    typedef typename GridImp::template Codim<0>::Geometry Geometry;
    typedef ALU3dGridMakeableGeometry<dim,dimworld,GridImp> GeometryImp;
    typedef ALU3dGridIntersectionIterator<GridImp> IntersectionIteratorImp;
    typedef IntersectionIteratorWrapper<GridImp> ALU3dGridIntersectionIteratorType;

    typedef typename GridImp::template Codim<0>::Entity Entity;
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;

    template <int cd>
    struct Codim
    {
      typedef typename GridImp::template Codim<cd>::EntityPointer EntityPointer;
    };

    //! Constructor creating empty Entity
    ALU3dGridEntity(const GridImp &grid, int level);

    //! level of this element
    int level () const ;

    //! geometry of this entity
    const Geometry & geometry () const;

    //! return partition type of this entity ( see grid.hh )
    PartitionType partitionType() const;

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
        with codimension cc.
     */
    template<int cc> int count () const ;

    //! Provide access to mesh entity i of given codimension. Entities
    //!  are numbered 0 ... count<cc>()-1
    template <int cc>
    typename Codim<cc>::EntityPointer entity (int i) const;

    /*! Intra-level access to intersection with neighboring elements.
       A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor. */
    ALU3dGridIntersectionIteratorType ibegin () const;

    //! Reference to one past the last intersection with neighbor
    ALU3dGridIntersectionIteratorType iend () const;

    //! returns true if Entity is leaf (i.e. has no children)
    bool isLeaf () const;

    //! Inter-level access to father element on coarser grid.
    //! Assumes that meshes are nested.
    EntityPointer father () const;

    /*! Location of this element relative to the reference element
       of the father. This is sufficient to interpolate all
       dofs in conforming case. Nonconforming may require access to
       neighbors of father and computations with local coordinates.
       On the fly case is somewhat inefficient since dofs  are visited
       several times. If we store interpolation matrices, this is tolerable.
       We assume that on-the-fly implementation of numerical algorithms
       is only done for simple discretizations. Assumes that meshes are nested.
     */
    const Geometry & geometryInFather () const;

    /*! Inter-level access to son elements on higher levels<=maxlevel.
       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
     */
    ALU3dGridHierarchicIterator<GridImp> hbegin (int maxlevel) const;

    //! Returns iterator to one past the last son
    ALU3dGridHierarchicIterator<GridImp> hend (int maxlevel) const;

    //***************************************************************
    //  Interface for Adaptation
    //***************************************************************

    //! marks an element for refCount refines. if refCount is negative the
    //! element is coarsend -refCount times
    //! mark returns true if element was marked, otherwise false
    bool mark( int refCount ) const;

    //! return whether entity could be cosrsend (COARSEND) or was refined
    //! (REFINED) or nothing happend (NONE)
    AdaptationState state () const;

    /*! private methods, but public because of datahandle and template
        arguments of these methods
     */
    void setElement(ALU3DSPACE HElementType &element, int , int );

    //! set original element pointer to fake entity
    void setGhost(ALU3DSPACE HBndSegType  &ghost);

    //! set actual walk level
    void reset ( int l );

    //! set item pointer to NULL
    void removeElement();

    //! compare 2 entities, which means compare the item pointers
    bool equals ( const ALU3dGridEntity<0,dim,GridImp> & org ) const;

    void setEntity ( const ALU3dGridEntity<0,dim,GridImp> & org );

    //! return index of sub entity with codim = cc and local number i
    //! i.e. return global number of vertex i
    //! for use in hierarchical index set
    template<int cc> int getSubIndex (int i) const;

  private:
    // return reference to internal item
    const IMPLElementType & getItem () const { return *item_; }

    //! index is unique within the grid hierachie and per codim
    int getIndex () const;

    // corresponding grid
    const GridImp  & grid_;

    // the current element of grid
    mutable IMPLElementType *item_;

    mutable bool isGhost_; //! true if entity is ghost entity

    //! the cuurent geometry
    mutable GeometryImp geo_;
    mutable bool builtgeometry_; //!< true if geometry has been constructed

    int walkLevel_; //! tells the actual level of walk put to LevelIterator..

    int level_;  //!< level of element

    mutable GeometryImp geoInFather_;

    // is true if entity is leaf entity
    bool isLeaf_;

    static const ReferenceElementType refElem_;
  }; // end of ALU3dGridEntity codim = 0

  //**********************************************************************
  //
  // --ALU3dGridEntityPointer
  // --EntityPointer
  // --EnPointer
  /*!
     Enables iteration over all entities of a given codimension and level of a grid.
   */
  template<int cd, class GridImp>
  class ALU3dGridEntityPointerBase :
    public EntityPointerDefaultImplementation <cd, GridImp, ALU3dGridEntityPointer<cd,GridImp> >
  {
    typedef ALU3dGridEntityPointerBase <cd,GridImp> ThisType;
    enum { dim       = GridImp::dimension };
    enum { dimworld  = GridImp::dimensionworld };

    friend class ALU3dGridEntity<cd,dim,GridImp>;
    friend class ALU3dGridEntity< 0,dim,GridImp>;
    friend class ALU3dGrid < dim , dimworld, GridImp::elementType >;

    typedef typename ALU3dImplTraits<GridImp::elementType>::template Codim<cd>::InterfaceType MyHElementType;

    typedef ALU3DSPACE HBndSegType HBndSegType;
    typedef typename ALU3dImplTraits<GridImp::elementType>::BNDFaceType BNDFaceType;
  public:
    //! type of Entity
    typedef typename GridImp::template Codim<cd>::Entity Entity;
    //! underlying EntityImplementation
    typedef ALU3dGridMakeableEntity<cd,dim,GridImp> EntityImp;

    //! typedef of my type
    typedef ThisType ALU3dGridEntityPointerType;

    //! make base type available in derived types
    typedef ALU3dGridEntityPointerBase<cd,GridImp> Base;

    //! Constructor for EntityPointer that points to an element
    ALU3dGridEntityPointerBase(const GridImp & grid,
                               const MyHElementType & item);

    //! Constructor for EntityPointer that points to an ghost
    ALU3dGridEntityPointerBase(const GridImp & grid,
                               const HBndSegType & ghostFace );

    //! copy constructor
    ALU3dGridEntityPointerBase(const ALU3dGridEntityPointerType & org);

    //! Destructor
    ~ALU3dGridEntityPointerBase();

    //! equality
    bool equals (const ALU3dGridEntityPointerType& i) const;

    //! dereferencing
    Entity & dereference () const ;

    //! ask for level of entities
    int level () const ;

  protected:
    //! has to be called when iterator is finished
    void done ();

    //! Constructor for EntityPointer init of Level-, and Leaf-, and
    //! HierarchicIterator
    ALU3dGridEntityPointerBase(const GridImp & grid, int level );

    // update underlying item pointer and set ghost entity
    void updateGhostPointer( HBndSegType & ghostFace );
    // update underlying item pointer and set entity
    void updateEntityPointer( MyHElementType * item );

    // not allowed
    ThisType & operator = (const ALU3dGridEntityPointerType & org);

    // reference to grid
    const GridImp & grid_;

    // pointer to item
    MyHElementType * item_;

    // entity that this EntityPointer points to
    mutable EntityImp * entity_;
  };

  //! ALUGridEntityPointer points to an entity
  //! this class is the specialisation for codim 0,
  //! it has exactly the same functionality as the ALU3dGridEntityPointerBase
  template<class GridImp>
  class ALU3dGridEntityPointer<0,GridImp> :
    public ALU3dGridEntityPointerBase<0,GridImp>
  {
    enum { cd = 0 };
    typedef ALU3dGridEntityPointer <cd,GridImp> ThisType;
    enum { dim       = GridImp::dimension };
    enum { dimworld  = GridImp::dimensionworld };

    friend class ALU3dGridEntity<cd,dim,GridImp>;
    friend class ALU3dGridEntity< 0,dim,GridImp>;
    friend class ALU3dGrid < dim , dimworld, GridImp::elementType >;

    typedef typename ALU3dImplTraits<GridImp::elementType>::template Codim<cd>::InterfaceType MyHElementType;

    typedef ALU3DSPACE HBndSegType HBndSegType;
    typedef typename ALU3dImplTraits<GridImp::elementType>::BNDFaceType BNDFaceType;
  public:
    //! type of Entity
    typedef typename GridImp::template Codim<cd>::Entity Entity;
    //! underlying EntityImplementation
    typedef ALU3dGridMakeableEntity<cd,dim,GridImp> EntityImp;

    //! typedef of my type
    typedef ThisType ALU3dGridEntityPointerType;

    //! Constructor for EntityPointer that points to an element
    ALU3dGridEntityPointer(const GridImp & grid,
                           const MyHElementType & item)
      : ALU3dGridEntityPointerBase<cd,GridImp> (grid,item) {}

    //! Constructor for EntityPointer that points to an ghost
    ALU3dGridEntityPointer(const GridImp & grid,
                           const HBndSegType & ghostFace )
      : ALU3dGridEntityPointerBase<cd,GridImp> (grid,ghostFace) {}

    //! copy constructor
    ALU3dGridEntityPointer(const ALU3dGridEntityPointerType & org)
      : ALU3dGridEntityPointerBase<cd,GridImp> (org) {}

  protected:
    //! Constructor for EntityPointer init of Level-, and Leaf-, and
    //! HierarchicIterator
    ALU3dGridEntityPointer(const GridImp & grid, int level )
      : ALU3dGridEntityPointerBase<cd,GridImp> (grid,level) {}
  };


  template<int cd, class GridImp>
  class ALU3dGridEntityPointer :
    public ALU3dGridEntityPointerBase<cd,GridImp>
  {
    typedef ALU3dGridEntityPointer <cd,GridImp> ThisType;
    enum { dim       = GridImp::dimension };
    enum { dimworld  = GridImp::dimensionworld };

    friend class ALU3dGridEntity<cd,dim,GridImp>;
    friend class ALU3dGridEntity< 0,dim,GridImp>;
    friend class ALU3dGrid < dim , dimworld, GridImp::elementType >;

    typedef typename ALU3dImplTraits<GridImp::elementType>::template Codim<cd>::InterfaceType MyHElementType;

    typedef ALU3DSPACE HBndSegType HBndSegType;
    typedef typename ALU3dImplTraits<GridImp::elementType>::BNDFaceType BNDFaceType;
  public:
    //! type of Entity
    typedef typename GridImp::template Codim<cd>::Entity Entity;
    //! underlying EntityImplementation
    typedef ALU3dGridMakeableEntity<cd,dim,GridImp> EntityImp;

    //! typedef of my type
    typedef ALU3dGridEntityPointer<cd,GridImp> ALU3dGridEntityPointerType;

    //! Constructor for EntityPointer that points to an element
    ALU3dGridEntityPointer(const GridImp & grid,
                           const MyHElementType & item,
                           const int twist = 0,
                           const int duneFace = -1
                           );

    //! copy constructor
    ALU3dGridEntityPointer(const ALU3dGridEntityPointerType & org);

    //! dereferencing
    Entity & dereference () const ;

  protected:
    //! Constructor for EntityPointer init of Level-, and Leaf-, and
    //! HierarchicIterator
    ALU3dGridEntityPointer(const GridImp & grid, int level )
      : ALU3dGridEntityPointerBase<cd,GridImp> (grid,level)
        , twist_(0) , face_(-1) {}

    // not allowed
    ThisType & operator = (const ALU3dGridEntityPointerType & org);

    // twist of face, for codim 1 only
    const int twist_;
    // face number, for codim 1 only
    const int face_;
  };

} // end namespace Dune

#include "entity_imp.cc"

#endif
