// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ALU3DGRID_HH__
#define __DUNE_ALU3DGRID_HH__

#include <vector>
#include <assert.h>

#include <dune/common/misc.hh>
#include <dune/common/fmatrix.hh>

#include "common/grid.hh"
#include "common/defaultindexsets.hh"

#include "alu3dgrid/alu3dinclude.hh"

#include <dune/common/exceptions.hh>

namespace Dune
{

  /** @defgroup ALU3dGrid ALU3dGrid
     @ingroup GridCommon
     Adaptive parallel grid supporting dynamic load balancing, written by
     Bernard Schupp. This grid supports hexahedrons and tetrahedrons.

     (See Bernhard Schupp:  Entwicklung eines
     effizienten Verfahrens zur Simulation kompressibler Stroemungen
      in 3D auf Parallelrechnern. 1999
     http://www.freidok.uni-freiburg.de/volltexte/68/ )

     For partitioning two tools can be used:
      - Metis ( version 4.0 and higher, see http://www-users.cs.umn.edu/~karypis/metis/metis/ )
      - Party Lib ( version 1.1 and higher, see http://wwwcs.upb.de/fachbereich/AG/monien/RESEARCH/PART/party.html)

     @{
   */

  class ALU3dGridError : public Exception {};

  enum ALU3dGridElementType { tetra = 4, hexa = 7 };

  // i.e. double or float
  typedef double alu3d_ctype;

  template<int cd, int dim, class GridImp> class ALU3dGridEntity;
  template<int cd, PartitionIteratorType pitype, class GridImp > class ALU3dGridLevelIterator;
  template<int cd, PartitionIteratorType pitype, class GridImp > class ALU3dGridEntityPointer;

  template<int mydim, int coorddim, class GridImp>  class ALU3dGridGeometry;
  template<class GridImp>            class ALU3dGridBoundaryEntity;
  template<class GridImp>            class ALU3dGridHierarchicIterator;
  template<class GridImp>            class ALU3dGridIntersectionIterator;
  template<class GridImp>            class ALU3dGridLeafIterator;
  template<int dim, int dimworld>            class ALU3dGrid;

  // the hierarhic index set
  template<int dim, int dimworld> class ALU3dGridHierarchicIndexSet;

  // singleton holding reference elements
  template<int dim,class GridImp> struct ALU3dGridReferenceGeometry;

  //**********************************************************************
  //
  // --ALU3dGridGeometry
  // --Geometry
  /*!
     Defines the geometry part of a mesh entity. Works for all dimensions, element types and dime
     of world. Provides reference element and mapping between local and global coordinates.
     The element may have different implementations because the mapping can be
     done more efficient for structured meshes than for unstructured meshes.

     dim: An element is a polygonal in a hyperplane of dimension dim. 0 <= dim <= 3 is typically
     dim=0 is a point.

     dimworld: Each corner is a point with dimworld coordinates.
   */

#ifdef _ALU3DGRID_PARALLEL_
  static int __MyRank__ = -1;
#endif

  //! MakeableGeometry
  template<int mydim, int coorddim, class GridImp>
  class ALU3dGridMakeableGeometry : public Geometry<mydim, coorddim,
                                        GridImp, ALU3dGridGeometry>
  {
    typedef Geometry<mydim, coorddim, GridImp, ALU3dGridGeometry> GeometryType;
  public:
    ALU3dGridMakeableGeometry(bool makeRefelem=false) :
      GeometryType (ALU3dGridGeometry<mydim, coorddim,GridImp>(makeRefelem)) {}

    //! build geometry out of different ALU3dGrid Geometrys
    //! ItemType are HElementType, HFaceType, HEdgeType and VertexType
    template <class ItemType>
    bool buildGeom(const ItemType & item)
    {
      return this->realGeometry.buildGeom(item);
    }

    // call buildGhost of realGeometry
    bool buildGhost(const ALU3DSPACE PLLBndFaceType & ghost)
    {
      return this->realGeometry.buildGhost(ghost);
    }

    // print real entity for debugging
    void print (std::ostream& ss) const
    {
      this->realGeometry.print(ss);
    }

    // for changing the coordinates of one element
    FieldVector<alu3d_ctype, coorddim> & getCoordVec (int i)
    {
      return this->realGeometry.getCoordVec(i);
    }

  };

  //! ALU3dGridGeometry
  template<int mydim, int cdim, class GridImp>
  class ALU3dGridGeometry :
    public GeometryDefault <mydim,cdim,GridImp,ALU3dGridGeometry>
  {
    friend class ALU3dGridBoundaryEntity<GridImp>;

    //! know dimension of barycentric coordinates
    enum { dimbary=mydim+1};
  public:
    //! for makeRefGeometry == true a Geometry with the coordinates of the
    //! reference element is made
    ALU3dGridGeometry(bool makeRefGeometry=false);

    //! return the element type identifier
    //! line , triangle or tetrahedron, depends on dim
    GeometryType type () const;

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const;

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<alu3d_ctype, cdim>& operator[] (int i) const;

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned.
     */
    static const Dune::Geometry<mydim,mydim,GridImp,Dune::ALU3dGridGeometry> & refelem ();

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    FieldVector<alu3d_ctype, cdim> global (const FieldVector<alu3d_ctype, mydim>& local) const;

    //! maps a global coordinate within the element to a
    //! local coordinate in its reference element
    FieldVector<alu3d_ctype,  mydim> local (const FieldVector<alu3d_ctype, cdim>& global) const;

    //! returns true if the point in local coordinates is inside reference element
    bool checkInside(const FieldVector<alu3d_ctype, mydim>& local) const;

    //! A(l) , see grid.hh
    alu3d_ctype integrationElement (const FieldVector<alu3d_ctype, mydim>& local) const;

    //! can only be called for dim=dimworld!
    const FieldMatrix<alu3d_ctype,mydim,mydim>& jacobianInverse (const FieldVector<alu3d_ctype, cdim>& local) const;

    //***********************************************************************
    //!  Methods that not belong to the Interface, but have to be public
    //***********************************************************************
    //! generate the geometry for out of given ALU3dGridElement
    bool buildGeom(const ALU3DSPACE IMPLElementType & item);
    bool buildGeom(const ALU3DSPACE HFaceType & item);
    bool buildGeom(const ALU3DSPACE HEdgeType & item);
    bool buildGeom(const ALU3DSPACE VertexType & item);

    //! build ghost out of internal boundary segment
    bool buildGhost(const ALU3DSPACE PLLBndFaceType & ghost);

    //! print internal data
    //! no interface method
    void print (std::ostream& ss) const;

    // for changing the coordinates of one element
    FieldVector<alu3d_ctype, cdim> & getCoordVec (int i);

  private:
    // generate Jacobian Inverse and calculate integration_element
    void buildJacobianInverse() const;

    // calculates the element matrix for calculation of the jacobian inverse
    void calcElMatrix () const;

    //! the vertex coordinates
    mutable FieldMatrix<alu3d_ctype,mydim+1,cdim> coord_;

    //! is true if Jinv_, A and detDF_ is calced
    mutable bool builtinverse_;
    mutable bool builtA_;
    mutable bool builtDetDF_;

    enum { matdim = (mydim > 0) ? mydim : 1 };
    mutable FieldMatrix<alu3d_ctype,matdim,matdim> Jinv_; //!< storage for inverse of jacobian
    mutable alu3d_ctype detDF_;                         //!< storage of integration_element
    mutable FieldMatrix<alu3d_ctype,matdim,matdim> A_;  //!< transformation matrix

    mutable FieldVector<alu3d_ctype, mydim> localCoord_;
    mutable FieldVector<alu3d_ctype, cdim>  globalCoord_;

    mutable FieldVector<alu3d_ctype,cdim> tmpV_; //! temporary memory
    mutable FieldVector<alu3d_ctype,cdim> tmpU_; //! temporary memory
  };


  //**********************************************************************
  //
  // --ALU3dGridEntity
  // --Entity
  //
  //**********************************************************************
  template<int codim, int dim, class GridImp>
  class ALU3dGridMakeableEntity :
    public GridImp::template codim<codim>::Entity
  {
    // typedef typename GridImp::template codim<codim>::Entity EntityType;
    friend class ALU3dGridEntity<codim, dim, GridImp>;
  public:

    // Constructor creating the realEntity
    ALU3dGridMakeableEntity(const GridImp & grid, int level) :
      GridImp::template codim<codim>::
      Entity (ALU3dGridEntity<codim, dim, GridImp>(grid,level)) {}

    //! set element as normal entity
    //! ItemTypes are HElementType, HFaceType, HEdgeType and VertexType
    template <class ItemType>
    void setElement(ItemType & item)
    {
      this->realEntity.setElement(item);
    }

    // set element as ghost
    void setGhost(ALU3DSPACE HElementType &ghost)
    {
      this->realEntity.setGhost(ghost);
    }

    //! set original element pointer to fake entity
    void setGhost(ALU3DSPACE PLLBndFaceType  &ghost)
    {
      this->realEntity.setGhost(ghost);
    }

    void reset ( int l )
    {
      this->realEntity.reset(l);
    }
  };

  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Here: the general template
   */
  template<int cd, int dim, class GridImp>
  class ALU3dGridEntity :
    public EntityDefault <cd,dim,GridImp,ALU3dGridEntity>
  {
    enum { dimworld = GridImp::dimensionworld };

    friend class ALU3dGrid < dim , dimworld >;
    friend class ALU3dGridEntity < 0, dim, GridImp >;
    friend class ALU3dGridLevelIterator < cd, All_Partition, GridImp >;

    friend class ALU3dGridHierarchicIndexSet<dim,dimworld>;

  public:
    typedef typename ALU3DSPACE ALUHElementType<cd>::ElementType BSElementType;
    typedef typename ALU3DSPACE BSIMPLElementType<cd>::ElementType BSIMPLElementType;

    typedef typename GridImp::template codim<cd>::Entity Entity;
    typedef typename GridImp::template codim<cd>::Geometry Geometry;
    typedef ALU3dGridMakeableGeometry<dim-cd,GridImp::dimensionworld,GridImp> GeometryImp;
    typedef typename GridImp::template codim<0>::EntityPointer EntityPointer;

    //! level of this element
    int level () const;

    //! index is unique and consecutive per level and codim
    //! used for access to degrees of freedom
    int index () const;

    //! index is unique within the grid hierachie and per codim
    int globalIndex () const;

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
    void setElement(const BSElementType & item);
    void setElement(const ALU3DSPACE HElementType & el, const ALU3DSPACE VertexType & vx);

  private:
    //! index is unique within the grid hierachie and per codim
    int getIndex () const;

    // the grid this entity belongs to
    const GridImp &grid_;

    int gIndex_; //! hierarchic index

    // corresponding ALU3dGridElement
    const BSIMPLElementType * item_;
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
    : public EntityDefault<0,dim,GridImp,ALU3dGridEntity>
  {
    enum { dimworld = GridImp::dimensionworld };

    friend class ALU3dGrid < dim , dimworld >;
    friend class ALU3dGridIntersectionIterator < GridImp >;
    friend class ALU3dGridHierarchicIterator   < const GridImp >;
    friend class ALU3dGridHierarchicIterator   < GridImp >;
    friend class ALU3dGridLevelIterator <0,All_Partition,GridImp>;
    friend class ALU3dGridLevelIterator <1,All_Partition,GridImp>;
    friend class ALU3dGridLevelIterator <2,All_Partition,GridImp>;
    friend class ALU3dGridLevelIterator <3,All_Partition,GridImp>;
    friend class ALU3dGridLeafIterator <GridImp>;

    friend class ALU3dGridHierarchicIndexSet<dim,dimworld>;

    // partial specialisation of subIndex
    template <int codim>
    struct IndexWrapper
    {
      static inline int subIndex(ALU3DSPACE GEOElementType &elem, int i)
      {
        return elem.myvertex(i)->getIndex();
      }
    };

    /*
       // partial specialisation of subIndex for codim == dim
       template <>
       struct IndexWrapper<dim>
       {
       static int subIndex(ALU3DSPACE GEOElementType &elem, int i) const
       {
        return elem.myvertex(i)->getIndex();
       }
       };
     */

  public:
    typedef typename GridImp::template codim<0>::Geometry Geometry;
    typedef  ALU3dGridMakeableGeometry<dim,dimworld,GridImp> GeometryImp;

    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef typename GridImp::template codim<0>::EntityPointer EntityPointer;

    template <int cd>
    struct codim
    {
      typedef typename GridImp::template codim<cd>::EntityPointer EntityPointer;
    };

    //! Constructor creating empty Entity
    ALU3dGridEntity(const GridImp &grid, int level);

    //! level of this element
    int level () const ;

    //! index is unique and consecutive per level and codim used for access to degrees of freedo
    int index () const;

    //! index is unique within the grid hierachie and per codim
    int globalIndex () const;

    //! geometry of this entity
    const Geometry & geometry () const;

    //! return partition type of this entity ( see grid.hh )
    PartitionType partitionType() const;

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
        with codimension cc.
     */
    template<int cc> int count () const ;

    //! return index of sub entity with codim = cc and local number i
    //! i.e. return global number of vertex i
    template<int cc> int subIndex (int i) const;

    //! Provide access to mesh entity i of given codimension. Entities
    //!  are numbered 0 ... count<cc>()-1
    template <int cc>
    typename codim<cc>::EntityPointer entity (int i) const;

    /*! Intra-level access to intersection with neighboring elements.
       A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor. */
    ALU3dGridIntersectionIterator<GridImp> ibegin () const;

    //! Reference to one past the last intersection with neighbor
    ALU3dGridIntersectionIterator<GridImp> iend () const;

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
    void setElement(ALU3DSPACE HElementType &element);

    /*! private methods, but public because of datahandle and template
        arguments of these methods
        set original element pointer to fake entity
     */
    void setGhost(ALU3DSPACE HElementType &ghost);

    //! set original element pointer to fake entity
    void setGhost(ALU3DSPACE PLLBndFaceType  &ghost);

    //! set actual walk level
    void reset ( int l );
  private:

    //! index is unique within the grid hierachie and per codim
    int getIndex () const;

    // corresponding grid
    const GridImp  & grid_;

    // the current element of grid
    ALU3DSPACE IMPLElementType *item_;

    // the current ghost, if element is ghost
    ALU3DSPACE PLLBndFaceType * ghost_;
    mutable bool isGhost_; //! true if entity is ghost entity

    //! the cuurent geometry
    mutable GeometryImp geo_;
    mutable bool builtgeometry_; //!< true if geometry has been constructed


    int index_; //! level index of entity

    int walkLevel_; //! tells the actual level of walk put to LevelIterator..

    int glIndex_; //!< global index of element
    int level_;  //!< level of element

    mutable GeometryImp geoInFather_;
  }; // end of ALU3dGridEntity codim = 0

  //**********************************************************************
  //
  // --ALU3dGridHierarchicIterator
  // --HierarchicIterator
  /*!
     Mesh entities of codimension 0 ("elements") allow to visit all entities of
     codimension 0 obtained through nested, hierarchic refinement of the entity.
     Iteration over this set of entities is provided by the HIerarchicIterator,
     starting from a given entity.
     This is redundant but important for memory efficient implementations of unstru
     hierarchically refined meshes.
   */

  template<class GridImp>
  class ALU3dGridHierarchicIterator :
    public HierarchicIteratorDefault <GridImp,ALU3dGridHierarchicIterator>
  {
    enum { dim = GridImp::dimension };
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef typename GridImp::ctype ctype;
    typedef ALU3dGridMakeableEntity<0,dim,GridImp> EntityImp;

    //! the normal Constructor
    ALU3dGridHierarchicIterator(const GridImp &grid,
                                const ALU3DSPACE HElementType & elem, int maxlevel, bool end=false);

    //! the normal Constructor
    ALU3dGridHierarchicIterator(const ALU3dGridHierarchicIterator<GridImp> &org);

    //! the Destructor
    ~ALU3dGridHierarchicIterator();

    //! increment
    void increment();

    //! equality
    bool equals (const ALU3dGridHierarchicIterator<GridImp>& i) const;

    //! dereferencing
    Entity & dereference() const;

  private:
    // go to next valid element
    ALU3DSPACE HElementType * goNextElement (ALU3DSPACE HElementType * oldEl);

    const GridImp & grid_; //!< the corresponding ALU3dGrid
    const ALU3DSPACE HElementType & elem_; //!< the start  element of this iterator
    ALU3DSPACE HElementType * item_; //!< the actual element of this iterator
    int maxlevel_; //!< maxlevel

    // holds the entity, copy pointer and delete if no refcount is left
    EntityImp * entity_;
  };

  //*******************************************************************
  //
  //  --ALU3dGridBoundaryEntity
  //  --BoundaryEntity
  //
  //*******************************************************************
  template<class GridImp>
  class ALU3dGridMakeableBoundaryEntity :
    public GridImp::template codim<0>::BoundaryEntity
  {
  public:
    ALU3dGridMakeableBoundaryEntity () :
      GridImp::template codim<0>::BoundaryEntity (ALU3dGridBoundaryEntity<GridImp>()) {};

    // set boundary Id, done by IntersectionIterator
    void setId ( int id )
    {
      this->realBoundaryEntity.setId(id);
    }
  };

  /** BoundaryEntity of the ALU3dGrid module */
  template<class GridImp>
  class ALU3dGridBoundaryEntity
    : public BoundaryEntityDefault <GridImp,ALU3dGridBoundaryEntity>
  {
    enum {dim = GridImp::dimension };
    friend class ALU3dGridIntersectionIterator<GridImp>;
    friend class ALU3dGridIntersectionIterator<const GridImp>;
  public:
    typedef typename GridImp::template codim<0>::Geometry Geometry;
    typedef ALU3dGridMakeableGeometry<dim,dim,GridImp> GeometryImp;

    //! Constructor
    ALU3dGridBoundaryEntity ();

    /*! \brief return identifier of boundary segment which is an
        abitrary integer not zero */
    int id () const ;

    //! return true if geometry of ghost cells was filled
    bool hasGeometry () const ;

    //! return geometry of the ghost cell
    const Geometry & geometry () const ;

    void setId ( int id ) ;

  private:
    mutable GeometryImp _geom;
    int _id;
  };

  //**********************************************************************
  //
  // --ALU3dGridIntersectionIterator
  // --IntersectionIterator
  /*!
     Mesh entities of codimension 0 ("elements") allow to visit all neighbors, wh
     a neighbor is an entity of codimension 0 which has a common entity of codimens
     These neighbors are accessed via a IntersectionIterator. This allows the implement
     non-matching meshes. The number of neigbors may be different from the number o
     of an element!
   */
  template<class GridImp>
  class ALU3dGridIntersectionIterator :
    public IntersectionIteratorDefault <GridImp,ALU3dGridIntersectionIterator>
  {
    enum { dim       = GridImp::dimension };
    enum { dimworld  = GridImp::dimensionworld };

    friend class ALU3dGridEntity<0,dim,GridImp>;

  public:
    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef typename GridImp::template codim<0>::BoundaryEntity BoundaryEntity;
    typedef ALU3dGridMakeableBoundaryEntity<GridImp> MakeableBndEntityImp;
    typedef typename GridImp::template codim<1>::Geometry Geometry;
    typedef typename GridImp::template codim<1>::LocalGeometry LocalGeometry;
    typedef ALU3dGridMakeableEntity<0,dim,GridImp> EntityImp;
    typedef ALU3dGridMakeableGeometry<dim-1,dimworld,GridImp> GeometryImp;
    typedef ALU3dGridMakeableGeometry<dim-1,dimworld,GridImp> LocalGeometryImp;


    //! The default Constructor , level tells on which level we want
    //! neighbours
    ALU3dGridIntersectionIterator(const GridImp & grid, ALU3DSPACE HElementType *el,
                                  int wLevel,bool end=false);

    //! The copy constructor
    ALU3dGridIntersectionIterator(const ALU3dGridIntersectionIterator<GridImp> & org);

    //! The Destructor
    ~ALU3dGridIntersectionIterator();

    //! increment iterator
    void increment ();

    //! equality
    bool equals(const ALU3dGridIntersectionIterator<GridImp> & i) const;

    //! access neighbor, dereferencing
    Entity & dereference () const;

    //! return true if intersection is with boundary. \todo connection with
    //! boundary information, processor/outer boundary
    bool boundary () const;

    //! return true if across the edge an neighbor on this level exists
    bool neighbor () const;

    //! return information about the Boundary
    const BoundaryEntity & boundaryEntity () const;

    //! intersection of codimension 1 of this neighbor with element where
    //! iteration started.
    //! Here returned element is in LOCAL coordinates of the element
    //! where iteration started.
    const LocalGeometry & intersectionSelfLocal () const;

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in GLOBAL coordinates of the element where iteration started.
    const Geometry & intersectionGlobal () const;

    //! local number of codim 1 entity in self where intersection is contained in
    int numberInSelf () const;

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    const LocalGeometry & intersectionNeighborLocal () const;

    //! local number of codim 1 entity in neighbor where intersection is contained
    int numberInNeighbor () const;

    //! return unit outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    typedef FieldVector<alu3d_ctype, dimworld> NormalType;

    const NormalType & unitOuterNormal (const FieldVector<alu3d_ctype, dim-1>& local) const ;

    //! return outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    const NormalType & outerNormal (const FieldVector<alu3d_ctype, dim-1>& local) const;

    //! return outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    const NormalType & integrationOuterNormal (const FieldVector<alu3d_ctype, dim-1>& local) const;

  private:
    // if neighbour exists , do setup of new neighbour
    void setNeighbor () const ;

    // check wether we at ghost boundary, only parallel grid
    void checkGhost () const ;

    // reset bool flags
    void resetBools () const ;

    // reset IntersectionIterator to first neighbour
    void first(ALU3DSPACE HElementType & elem, int wLevel);

    // set behind last neighbour
    void done ();

    //! the grid
    const GridImp & grid_;
    int walkLevel_;

    //ALU3DSPACE MemoryPointer < EntityImp , ALU3DSPACE MemoryProvider<EntityImp> > entity_;
    EntityImp * entity_; //! neighbour entity
    //EntityImp fEntity_; //! neighbour entity

    // current element from which we started the intersection iterator
    mutable ALU3DSPACE GEOElementType *item_;

    //! current neighbour
    mutable ALU3DSPACE GEOElementType *neigh_;

    //! current ghost if we have one
    mutable ALU3DSPACE PLLBndFaceType *ghost_;

    mutable int index_;       //! internal index of intersection
    mutable int numberInNeigh_; //! index of intersection in neighbor

    mutable bool theSituation_;   //! true if the "situation" occurs :-)
    mutable bool daOtherSituation_; //! true if the "da other situation" occurs :-)
    //! see bsgrid.cc for descritption

    mutable bool isBoundary_; //! true if intersection is with boundary
    mutable bool isGhost_; //! true if intersection is with internal boundary (only parallel grid)

    mutable FieldVector<alu3d_ctype, dimworld> outNormal_; //! outerNormal of current intersection
    mutable FieldVector<alu3d_ctype, dimworld> unitOuterNormal_; //! unitOuterNormal of current intersection

    mutable bool needSetup_; //! true if setup is needed
    mutable bool needNormal_; //! true if normal has to be calculated

    // pair holding pointer to face and twist
    mutable ALU3DSPACE NeighbourFaceType neighpair_;

    mutable bool initInterGl_; //! true if interSelfGlobal_ was initialized
    mutable GeometryImp interSelfGlobal_; //! intersection_self_global

    mutable MakeableBndEntityImp bndEntity_; //! boundaryEntity
  };

  //**********************************************************************
  //
  // --ALU3dGridLevelIterator
  // --LevelIterator
  /*!
     Enables iteration over all entities of a given codimension and level of a grid.
   */
  template<int cd, PartitionIteratorType pitype, class GridImp>
  class ALU3dGridLevelIterator :
    public LevelIteratorDefault <cd,pitype,GridImp,ALU3dGridLevelIterator>
  {
    enum { dim       = GridImp::dimension };
    enum { dimworld  = GridImp::dimensionworld };

    friend class ALU3dGridEntity<3,dim,GridImp>;
    friend class ALU3dGridEntity<2,dim,GridImp>;
    friend class ALU3dGridEntity<1,dim,GridImp>;
    friend class ALU3dGridEntity<0,dim,GridImp>;
    friend class ALU3dGrid < dim , dimworld >;

  public:
    typedef typename GridImp::template codim<cd>::Entity Entity;

    typedef ALU3dGridMakeableEntity<cd,dim,GridImp> EntityImp;

    //! typedef of my type
    typedef ALU3dGridLevelIterator<cd,pitype,GridImp> ALU3dGridLevelIteratorType;

    //! Constructor
    ALU3dGridLevelIterator(const GridImp & grid, int level , bool end=false);

    //! Constructor for father
    ALU3dGridLevelIterator(const GridImp & grid, const ALU3DSPACE HElementType & item);

    //! prefix increment
    void increment ();

    //! equality
    bool equals (const ALU3dGridLevelIteratorType& i) const;

    //! dereferencing
    Entity & dereference () const ;

    //! ask for level of entities
    int level () const ;

  private:
    // reference to grid
    const GridImp & grid_;

    // element index, -1 for end
    int index_;

    // actual level
    int level_;

    // the wrapper for the original iterator of the ALU3dGrid
    typedef typename ALU3DSPACE ALU3dGridLevelIteratorWrapper<cd> IteratorType;
    ALU3DSPACE AutoPointer< IteratorType > iter_;

    // holds the entity, copy pointer and delete if no refcount is left
    ALU3DSPACE AutoPointer< EntityImp > objEntity_;
  };

  //********************************************************************
  //
  //  --ALU3dGridLeafIterator
  //  --LeafIterator
  //
  //********************************************************************
  template<class GridImp>
  class ALU3dGridLeafIterator
  {
    enum { dim = GridImp :: dimension };

    friend class ALU3dGridEntity<0,dim,GridImp>;
    //friend class ALU3dGrid < dim , dimworld >;
    enum { codim = 0 };

  public:
    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef ALU3dGridMakeableEntity<0,dim,GridImp> EntityImp;

    typedef ALU3dGridLeafIterator<GridImp> ALU3dGridLeafIteratorType;

    //! Constructor
    ALU3dGridLeafIterator(const GridImp & grid, int level , bool end,
                          PartitionIteratorType pitype );

    //! prefix increment
    void increment ();

    //! equality
    bool equals (const ALU3dGridLeafIteratorType & i) const;

    //! dereferencing
    Entity & dereference () const ;

    //! ask for level of entities
    int level () const;

  private:
    //! the current grid
    const GridImp & grid_;

    // element index, -1 for end
    int index_;

    // actual level
    int level_;

    // the wrapper for the original iterator of the ALU3dGrid
    typedef typename ALU3DSPACE ALU3dGridLeafIteratorWrapper<codim> IteratorType;
    ALU3DSPACE AutoPointer < IteratorType > iter_;

    // holds the entity, copy pointer and delete if no refcount is left
    ALU3DSPACE AutoPointer< EntityImp > objEntity_;

    //! my partition tpye
    const PartitionIteratorType pitype_;
  };

  //**********************************************************************
  //
  // --ALU3dGridEntityPointer
  // --EntityPointer
  // --EnPointer
  /*!
     Enables iteration over all entities of a given codimension and level of a grid.
   */
  template<int cd, PartitionIteratorType pitype, class GridImp>
  class ALU3dGridEntityPointer :
    public LevelIteratorDefault <cd,pitype,GridImp,ALU3dGridEntityPointer>
  {
    enum { dim       = GridImp::dimension };
    enum { dimworld  = GridImp::dimensionworld };

    friend class ALU3dGridEntity<3,dim,GridImp>;
    friend class ALU3dGridEntity<2,dim,GridImp>;
    friend class ALU3dGridEntity<1,dim,GridImp>;
    friend class ALU3dGridEntity<0,dim,GridImp>;
    friend class ALU3dGrid < dim , dimworld >;

  public:
    typedef typename GridImp::template codim<cd>::Entity Entity;

    typedef ALU3dGridMakeableEntity<cd,dim,GridImp> EntityImp;

    //! typedef of my type
    typedef ALU3dGridEntityPointer<cd,pitype,GridImp> ALU3dGridEntityPointerType;

    //! Constructor for father
    ALU3dGridEntityPointer(const GridImp & grid, const ALU3DSPACE HElementType & item);
    //ALU3dGridEntityPointer(const GridImp & grid, const ALU3DSPACE HFaceType & item);
    //ALU3dGridEntityPointer(const GridImp & grid, const ALU3DSPACE HEdgeType & item);
    //ALU3dGridEntityPointer(const GridImp & grid, const ALU3DSPACE VertexType & item);

    //! make empty entity pointer (to be revised)
    //ALU3dGridEntityPointer(const GridImp & grid);

    //! make empty entity pointer (to be revised)
    ALU3dGridEntityPointer(const ALU3dGridEntityPointerType & org);

    //! Destructor
    ~ALU3dGridEntityPointer();

    //! prefix increment
    void increment ();

    //! equality
    bool equals (const ALU3dGridEntityPointerType& i) const;

    //! dereferencing
    Entity & dereference () const ;

    //! ask for level of entities
    int level () const ;

  private:
    // reference to grid
    const GridImp & grid_;

    //! item that Pointer points to
    ALU3DSPACE HElementType & item_;

    // entity the this EntityPointer points to
    EntityImp * entity_;
  };

  //**********************************************************************
  //
  // --ALU3dGrid
  // --Grid
  //
  //**********************************************************************

  /*!
   *  The ALU3dGrid implements the Dune GridInterface for 3d tetrahedral
   *  meshes. This grid can be locally adapted and used in parallel
   *  computations using dynamcic load balancing.
   */
  template <int dim, int dimworld>
  class ALU3dGrid : public GridDefault  < dim, dimworld, alu3d_ctype,ALU3dGrid<dim,dimworld> >
  {
    //CompileTimeChecker<dim      == 3>   ALU3dGrid_only_implemented_for_3dp;
    //CompileTimeChecker<dimworld == 3>   ALU3dGrid_only_implemented_for_3dw;
    //CompileTimeChecker< (eltype == ALU3DSPACE tetra_t) || (eltype == ALU3DSPACE hexa_t ) > ALU3dGrid_only_implemented_for_tetra_or_hexa;

    typedef ALU3dGrid<dim,dimworld> MyType;
    friend class ALU3dGridEntity <0,dim,MyType>;
    friend class ALU3dGridEntity <0,dim,const MyType>;
    friend class ALU3dGridIntersectionIterator<MyType>;

    friend class ALU3dGridEntityPointer<0,All_Partition,const MyType >;
    friend class ALU3dGridEntityPointer<1,All_Partition,const MyType >;
    friend class ALU3dGridEntityPointer<2,All_Partition,const MyType >;
    friend class ALU3dGridEntityPointer<3,All_Partition,const MyType >;

    friend class ALU3dGridIntersectionIterator<const MyType>;
    friend class ALU3dGridHierarchicIterator<const MyType>;

    //**********************************************************
    // The Interface Methods
    //**********************************************************
  public:
    enum { myElementType = tetra };
    typedef GridTraits<dim,dimworld, MyType ,
        ALU3dGridGeometry,ALU3dGridEntity,
        ALU3dGridBoundaryEntity,
        ALU3dGridEntityPointer,
        ALU3dGridLevelIterator,
        ALU3dGridIntersectionIterator,
        ALU3dGridHierarchicIterator,
        ALU3dGridLeafIterator>  Traits;


    typedef ALU3dGridLeafIterator<MyType>       LeafIteratorImp;
    typedef ALU3dGridHierarchicIterator<MyType> HierarchicIteratorImp;

    typedef typename Traits::LeafIterator LeafIteratorType;
    //typedef ALU3dGridReferenceGeometry<dim> ReferenceGeometry;

    typedef ALU3DSPACE ObjectStream ObjectStreamType;

    //typedef typename std::pair < ObjectStreamType * , ALU3dGridEntity<0,dim,dimworld> * >
    //              DataCollectorParamType;

    typedef ALU3dGridHierarchicIndexSet<dim,dimworld> HierarchicIndexSetType;
    typedef DefaultLevelIndexSet<MyType>           LevelIndexSetType;

    typedef typename Traits::LeafIterator LeafIterator;

    /** \todo Please doc me! */

    //! maximal number of levels
    enum { MAXL = 64 };

    //! Constructor which reads an ALU3dGrid Macro Triang file
    //! or given GridFile
#ifdef _ALU3DGRID_PARALLEL_
    ALU3dGrid(const char* macroTriangFilename , MPI_Comm mpiComm);
    ALU3dGrid(MPI_Comm mpiComm);
#else
    ALU3dGrid(const char* macroTriangFilename );
    //! empty Constructor
    ALU3dGrid(int myrank = -1);
#endif

    //! Desctructor
    ~ALU3dGrid();

    //! for type identification
    GridIdentifier type  () const;

    //! Return maximum level defined in this grid. Levels are numbered
    //! 0 ... maxlevel with 0 the coarsest level.
    int maxlevel() const;

    //! Iterator to first entity of given codim on level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template codim<cd>::template partition<pitype>::LevelIterator
    lbegin (int level) const;

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template codim<cd>::template partition<pitype>::LevelIterator
    lend (int level) const;

    //! Iterator to first entity of given codim on level
    template<int cd>
    typename Traits::template codim<cd>::
    template partition<All_Partition>::LevelIterator
    lbegin (int level) const;

    //! one past the end on this level
    template<int cd>
    typename Traits::template codim<cd>::
    template partition<All_Partition>::LevelIterator
    lend (int level) const;

    //! Iterator to first entity of given codim on leaf level
    LeafIteratorType leafbegin (int level,
                                PartitionIteratorType pitype=InteriorBorder_Partition ) const;

    //! one past the end on this leaf level
    LeafIteratorType leafend (int level,
                              PartitionIteratorType pitype=InteriorBorder_Partition ) const;

    //! number of grid entities per level and codim
    int size (int level, int cd) const;

    //! number of grid entities on all levels for given codim
    int global_size (int cd) const ;

    const HierarchicIndexSetType & hierarchicIndexSet () const { return hIndexSet_; }
    const LevelIndexSetType & levelIndexSet () const
    {
      if(!levelIndexSet_) levelIndexSet_ = new LevelIndexSetType (*this);
      return *levelIndexSet_;
    }


    //! calculate load of each proc and repartition if neccessary
    bool loadBalance ();

    //! calculate load of each proc and repartition if neccessary
    template <class DofManagerType>
    bool loadBalance (DofManagerType & dm);

    //! calculate load of each proc and repartition if neccessary
    template <class DofManagerType>
    bool communicate (DofManagerType & dm);

    //! returns if a least one entity was marked for coarsening
    bool preAdapt ( );

    //! clear all entity new markers
    void postAdapt ( );

    /**! refine all positive marked leaf entities,
       return true if a least one entity was refined */
    bool adapt ( );


    //**********************************************************
    // End of Interface Methods
    //**********************************************************
    //! uses the interface, mark on entity and refineLocal
    bool globalRefine(int refCount);

    /** \brief write Grid to file in specified FileFormatType
     */
    template <FileFormatType ftype>
    bool writeGrid( const char * filename, alu3d_ctype time ) const ;

    /** \brief read Grid from file filename and store time of mesh in time
     */
    template <FileFormatType ftype>
    bool readGrid( const char * filename, alu3d_ctype & time );

    //! return pointer to org ALU3dGrid
    //! private method, but otherwise we have to friend class all possible
    //! types of LevelIterator ==> later
    ALU3DSPACE GitterImplType & myGrid();

    //! return my rank (only parallel)
    int myRank () const { return myRank_; }

    double communicateValue (double val) const ;
    double communicateSum  (double val) const ;
    int communicateInt (int val) const ;

    //! no interface method, but has to be public
    void updateStatus ();

    bool mark( int refCount , typename Traits::template codim<0>::EntityPointer & ep )
    {
      return this->mark(*ep);
    }
    bool mark( int refCount , const typename Traits::template codim<0>::Entity & en ) const;

    template <int cd>
    ALU3dGridEntity<cd,dim,const ALU3dGrid<dim,dimworld> >&
    getRealEntity(typename Traits::template codim<cd>::Entity& entity)
    {
      return entity.realEntity;
    }

    //private:
    template <int cd>
    const ALU3dGridEntity<cd,dim,const ALU3dGrid<dim,dimworld> >&
    getRealEntity(const typename Traits::template codim<cd>::Entity& entity) const
    {
      return entity.realEntity;
    }

  private:
    //! Copy constructor should not be used
    ALU3dGrid( const ALU3dGrid<dim,dimworld> & g);

    //! assignment operator should not be used
    ALU3dGrid<dim,dimworld> & operator = (const ALU3dGrid<dim,dimworld> & g);

    // reset size and global size
    void calcExtras();

    // calculate maxlevel
    void calcMaxlevel();

    // make grid walkthrough and calc global size
    void recalcGlobalSize();

    // set _coarsenMark to true
    void setCoarsenMark() const;

    // the real grid
    ALU3DSPACE GitterImplType * mygrid_;
#ifdef _BSGRID_PARALLEL_
    ALU3DSPACE MpAccessMPI mpAccess_;
#endif
    // save global_size of grid
    mutable int globalSize_[dim+1];

    // max level of grid
    int maxlevel_;

    // true if at least one element is marked for coarsening
    mutable bool coarsenMark_;

    const int myRank_;

    // our hierarchic index set
    HierarchicIndexSetType hIndexSet_;

    // the level index set ( default type )
    mutable LevelIndexSetType * levelIndexSet_;

    // the entity codim 0
    typedef ALU3dGridMakeableEntity<0,dim,const MyType> EntityImp;
    typedef ALU3DSPACE MemoryProvider< EntityImp > EntityProvider;

    //typedef ALU3dGridMakeableEntity<3,dim,const MyType> VertexImp;
    //typedef ALU3DSPACE MemoryProvider< VertexImp > VertexProvider;

    mutable EntityProvider entityProvider_;
    //mutable VertexProvider vertexProvider_;

  }; // end Class ALU3dGridGrid

  //! hierarchic index set of ALU3dGrid
  template <int dim, int dimworld>
  class ALU3dGridHierarchicIndexSet
  {
    typedef ALU3dGrid<dim,dimworld> GridType;
    enum { numCodim = 4 };

  public:
    typedef typename GridType::Traits::template codim<0>::Entity EntityCodim0Type;

    ALU3dGridHierarchicIndexSet(const GridType & grid, const int (& s)[numCodim]) : grid_(grid) , size_(s) {}

    //! return hierarchic index of given entity
    template <class EntityType>
    int index (const EntityType & ep) const
    {
      enum { cd = EntityType :: codimension };
      return (grid_.template getRealEntity<cd>(ep)).getIndex();
    }

    //! return subIndex of given entity
    template <int cd>
    int subIndex (const EntityCodim0Type & ep, int i) const
    {
      assert(cd == dim);
      const ALU3dGridEntity<0,dim,const GridType> & en = (grid_.template getRealEntity<0>(ep));
      int idx = en.template getSubIndex<cd>(i);
      std::cout << idx << " index \n";
      return idx;
    }

    //! return size of indexset, i.e. maxindex+1
    int size ( int level, int codim ) const
    {
      assert(size_[codim] >= 0);
      return size_[codim];
    }

  private:
    // our Grid
    const GridType & grid_;
    // size of indexset, managed by grid
    const int (& size_)[numCodim];
  };

  /** @} end documentation group */

}; // namespace Dune

#include "alu3dgrid/datahandle.hh"
#include "alu3dgrid/alu3dgrid.cc"

#endif
