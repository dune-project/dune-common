// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ALBERTAGRID_HH__
#define __DUNE_ALBERTAGRID_HH__
#include <iostream>
#include <fstream>

#include <vector>
#include <assert.h>

#include <config.h>

// Dune includes
#include "../common/misc.hh"
#include "../common/matvec.hh"
#include "../common/array.hh"
#include "../common/capabilities.hh"
#include "common/grid.hh"

// some cpp defines and include of alberta.h
#include "albertagrid/albertaheader.hh"

// stack for index management
#include <dune/grid/common/indexstack.hh>

// IndexManager defined in indexstack.hh
typedef Dune::IndexStack<int,10000> IndexManagerType;

// some extra functions for handling the Albert Mesh
#include "albertagrid/albertaextra.hh"


#include <dune/common/exceptions.hh>

namespace Dune {
  // own exception classes
  class AlbertaError   : public Exception {};
  class AlbertaIOError : public IOError {};
}

// contains a simple memory management for some componds of this grid
#include "albertagrid/agmemory.hh"

// contains the communication for parallel computing for this grid

#include "albertagrid/agcommunicator.hh"

namespace Dune
{

  /** @defgroup AlbertaGrid AlbertaGrid
     @ingroup GridCommon

     This is one implementation of the grid interface using the
     the finite elemente tool box ALBERTA ( ALBERTA was written
     by Alfred Schmidt and Kunibert G. Siebert, see http://www.alberta-fem.de/).
     ALBERTA provides simplex meshes in 1d, 2d, and 3d space dimensions.
     Also the ALBERTA meshes can be dynamically adapted using
     bisection algorithms.

     The actual version of ALBERTA 1.2 can be
     downloaded at http://www.alberta-fem.de/.
     After installing the lib to a path of your choise (the PATH_TO_ALBERTA)
     you can use %Dune with this package and you have only to deliver
     the path with the --with-alberta option to %Dune.

     For using %Dune with ALBERTA you must tell %Dune where to find ALBERTA,
     which dimension to use and which dimension your world should have, i.e:

     <tt> ./autogen.sh [OPTIONS]
        --with-alberta=PATH_TO_ALBERTA and
        --with-problem-dim=DIM --with-world-dim=DIMWORLD
     </tt>

     Now you must use the AlbertaGrid with DIM and DIMWORLD, otherwise
     unpredictable results may occur.

     @{
   */

  class ObjectStream
  {
  public:
    template <class T>
    void readObject (T &) {}
    void readObject (int) {}
    void readObject (double) {}
    template <class T>
    void writeObject (T &) {};
    void writeObject (int) {} ;
    void writeObject (double) {};
  };


  // i.e. double or float
  typedef ALBERTA REAL albertCtype;

  // forward declarations
  class AlbertaMarkerVector;

  template<int codim, int dim, int dimworld> class AlbertaGridEntity;

  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  class AlbertaGridLevelIterator;

  template<int dim, int dimworld>            class AlbertaGridElement;
  template<int dim, int dimworld>            class AlbertaGridBoundaryEntity;
  template<int dim, int dimworld>            class AlbertaGridHierarchicIterator;
  template<int dim, int dimworld>            class AlbertaGridIntersectionIterator;
  template<int dim, int dimworld>            class AlbertaGrid;

  // singleton holding reference elements
  template<int dim> struct AlbertaGridReferenceElement;

  //**********************************************************************
  //
  // --AlbertaGridElement
  /*!
     Defines the geometry part of a mesh entity. Works for all dimensions, element types and dime
     of world. Provides reference element and mapping between local and global coordinates.
     The element may have different implementations because the mapping can be
     done more efficient for structured meshes than for unstructured meshes.

     dim: An element is a polygonal in a hyperplane of dimension dim. 0 <= dim <= 3 is typically
     dim=0 is a point.

     dimworld: Each corner is a point with dimworld coordinates.
   */

  template<int dim, int dimworld>
  class AlbertaGridElement :
    public ElementDefault <dim,dimworld,albertCtype,AlbertaGridElement>
  {
    friend class AlbertaGridBoundaryEntity<dim,dimworld>;

    //! know dimension of barycentric coordinates
    enum { dimbary=dim+1};
  public:

    //! for makeRefElement == true a Element with the coordinates of the
    //! reference element is made
    AlbertaGridElement(bool makeRefElement=false);

    //! return the element type identifier
    //! line , triangle or tetrahedron, depends on dim
    ElementType type () const;

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const;

    //! access to coordinates of corners. Index is the number of the corner
    FieldVector<albertCtype, dimworld>& operator[] (int i) const;

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned.
     */
    AlbertaGridElement<dim,dim>& refelem () const;

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    FieldVector<albertCtype, dimworld> global (const FieldVector<albertCtype, dim>& local) const;

    //! maps a global coordinate within the element to a
    //! local coordinate in its reference element
    FieldVector<albertCtype, dim> local (const FieldVector<albertCtype, dimworld>& global) const;

    //! returns true if the point in local coordinates is inside reference element
    bool checkInside(const FieldVector<albertCtype, dim>& local) const;

    /*!
       Copy from sgrid.hh:

       Integration over a general element is done by integrating over the reference element
       and using the transformation from the reference element to the global element as follows:
       \f[\int\limits_{\Omega_e} f(x) dx = \int\limits_{\Omega_{ref}} f(g(l)) A(l) dl \f] where
       \f$g\f$ is the local to global mapping and \f$A(l)\f$ is the integration element.

       For a general map \f$g(l)\f$ involves partial derivatives of the map (surface element of
       the first kind if \f$d=2,w=3\f$, determinant of the Jacobian of the transformation for
       \f$d=w\f$, \f$\|dg/dl\|\f$ for \f$d=1\f$).

       For linear elements, the derivatives of the map with respect to local coordinates
       do not depend on the local coordinates and are the same over the whole element.

       For a structured mesh where all edges are parallel to the coordinate axes, the
       computation is the length, area or volume of the element is very simple to compute.

       Each grid module implements the integration element with optimal efficieny. This
       will directly translate in substantial savings in the computation of finite element
       stiffness matrices.
     */

    // A(l)
    albertCtype integration_element (const FieldVector<albertCtype, dim>& local) const;

    //! can only be called for dim=dimworld!
    //! Note that if both methods are called on the same element, then
    //! call Jacobian_inverse first because integration element is calculated
    //! during calculation of the Jacobian_inverse
    Mat<dim,dim>& Jacobian_inverse (const FieldVector<albertCtype, dim>& local) const;

    //***********************************************************************
    //!  Methods that not belong to the Interface, but have to be public
    //***********************************************************************
    //! generate the geometry for the ALBERTA EL_INFO
    //! no interface method
    bool builtGeom(ALBERTA EL_INFO *elInfo, int face, int edge, int vertex);
    // init geometry with zeros
    //! no interface method
    void initGeom();

    //! print internal data
    //! no interface method
    void print (std::ostream& ss, int indent) const;

    //! check if A * xref_i + P_2 == x_i
    bool checkMapping (int loc) const;

    //! check if A^-1 * x_i - A^-1 * P_2 == xref_i
    bool checkInverseMapping (int loc) const ;

  private:
    // calculate Matrix for Mapping from reference element to actual element
    void calcElMatrix () const;

    //! built the reference element
    void makeRefElemCoords();

    //! built the jacobian inverse and store the volume
    void buildJacobianInverse () const;

    mutable FieldVector<albertCtype, dim+1> tmpVec_;
    //! maps a global coordinate within the elements local barycentric
    //! koordinates
    //Vec<dim+1,albertCtype> localBary (const Vec<dimworld,albertCtype>& global);

    // template method for map the vertices of EL_INFO to the actual
    // coords with face_,edge_ and vertex_ , needes for operator []
    int mapVertices (int i) const;

    // calculates the volume of the element
    albertCtype elDeterminant () const;

    //! the vertex coordinates
    mutable Mat<dimworld,dim+1,albertCtype> coord_;

    //! storage for global coords
    mutable FieldVector<albertCtype, dimworld> globalCoord_;

    //! storage for local coords
    mutable FieldVector<albertCtype, dim> localCoord_;

    // make empty EL_INFO
    ALBERTA EL_INFO * makeEmptyElInfo();

    ALBERTA EL_INFO * elInfo_;

    //! Which Face of the Element 0...dim+1
    int face_;

    //! Which Edge of the Face of the Element 0...dim
    int edge_;

    //! Which Edge of the Face of the Element 0...dim-1
    int vertex_;

    //! is true if Jinv_ and volume_ is calced
    mutable bool builtinverse_;
    mutable Mat<dim,dim,albertCtype> Jinv_; //!< storage for inverse of jacobian

    //! is true if elMat_ was calced
    mutable bool builtElMat_;
    mutable Mat<dim,dim,albertCtype> elMat_; //!< storage for mapping matrix
    mutable albertCtype elDet_; //!< storage of element determinant

  };


  //**********************************************************************
  //
  // --AlbertaGridEntity
  // --Entity
  //
  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Here: the general template
   */
  template<int codim, int dim, int dimworld>
  class AlbertaGridEntity :
    public EntityDefault <codim,dim,dimworld,albertCtype,
        AlbertaGridEntity,AlbertaGridElement,AlbertaGridLevelIterator,
        AlbertaGridIntersectionIterator,AlbertaGridHierarchicIterator>
  {
    friend class AlbertaGrid < dim , dimworld >;
    friend class AlbertaGridEntity < 0, dim, dimworld>;
    friend class AlbertaGridLevelIterator < codim, dim, dimworld,All_Partition>;
    //friend class AlbertaGridLevelIterator < dim, dim, dimworld>;
  public:

    //! level of this element
    int level () const;

    //! index is unique and consecutive per level and codim
    //! used for access to degrees of freedom
    int index () const;

    //! index of the boundary which is associated with the entity, 0 for inner entities
    int boundaryId () const ;

    //! return the global unique index in grid
    int global_index() const ;

    AlbertaGridEntity(const AlbertaGrid<dim,dimworld> &grid, int level,
                      ALBERTA TRAVERSE_STACK * travStack);

    AlbertaGridEntity(const AlbertaGrid<dim,dimworld> &grid, int level);

    //! geometry of this entity
    AlbertaGridElement<dim-codim,dimworld>& geometry () const;

    /*! Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    AlbertaGridLevelIterator<0,dim,dimworld,All_Partition> father () const;

    //! local coordinates within father
    FieldVector<albertCtype, dim>& local () const ;

    //! no interface method
    //! returns the global vertex number as default
    int el_index() const;

  private:
    // methods for setting the infos from the albert mesh
    void setTraverseStack (ALBERTA TRAVERSE_STACK *travStack);
    void setElInfo (ALBERTA EL_INFO *elInfo, int elNum, int face,
                    int edge, int vertex );
    // needed for the LevelIterator
    ALBERTA EL_INFO *getElInfo () const;

    // private Methods
    void makeDescription();

    const AlbertaGrid<dim,dimworld> &grid_;

    // private Members
    ALBERTA EL_INFO *elInfo_;
    ALBERTA TRAVERSE_STACK * travStack_;

    //! level
    int level_;

    //! the cuurent geometry
    mutable AlbertaGridElement<dim-codim,dimworld> geo_;
    mutable bool builtgeometry_;       //!< true if geometry has been constructed

    mutable FieldVector<albertCtype, dim> localFatherCoords_;

    //! element number
    int elNum_;

    //! Which Face of the Element
    int face_;

    //! Which Edge of the Face of the Element
    int edge_;

    //! Which Vertex of the Edge of the Face of the Element
    int vertex_;
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
  //  --AlbertaGridEntity
  //  --0Entity
  //
  //***********************
  template<int dim, int dimworld>
  class AlbertaGridEntity<0,dim,dimworld> :
    public EntityDefault<0,dim,dimworld,albertCtype,AlbertaGridEntity,AlbertaGridElement,
        AlbertaGridLevelIterator,AlbertaGridIntersectionIterator,
        AlbertaGridHierarchicIterator>
  {
    friend class AlbertaGrid < dim , dimworld >;
    friend class AlbertaMarkerVector;
    friend class AlbertaGridIntersectionIterator < dim, dimworld>;
    friend class AlbertaGridHierarchicIterator < dim, dimworld>;
    friend class AlbertaGridLevelIterator <0,dim,dimworld,All_Partition>;

  public:
    typedef AlbertaGridIntersectionIterator<dim,dimworld> IntersectionIterator;
    typedef AlbertaGridHierarchicIterator<dim,dimworld> HierarchicIterator;

    enum { dimension = dim };

    //! Destructor, needed perhaps needed for deleteing faceEntity_ and
    //! edgeEntity_ , see below
    //! there are only implementations for dim==dimworld 2,3
    ~AlbertaGridEntity() {};

    //! Constructor, real information is set via setElInfo method
    AlbertaGridEntity(AlbertaGrid<dim,dimworld> &grid, int level);

    //! level of this element
    int level () const;

    //! index is unique and consecutive per level and codim used for access to degrees of freedo
    int index () const;

    //! index of the boundary which is associated with the entity, 0 for inner entities
    int boundaryId () const;

    //! geometry of this entity
    AlbertaGridElement<dim,dimworld>& geometry () const;

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
       with codimension cc.
     */
    //!< Default codim 1 Faces and codim == dim Vertices
    template<int cc> int count () const;

    //! return index of sub entity with codim = cc and local number i
    //! i.e. return global number of vertex i
    template<int cc> int subIndex (int i) const;

    //! Provide access to mesh entity i of given codimension. Entities
    //!  are numbered 0 ... count<cc>()-1
    template<int cc> AlbertaGridLevelIterator<cc,dim,dimworld,All_Partition> entity (int i) const;

    //! Provide access to mesh entity i of given codimension. Entities
    //!  are numbered 0 ... count<cc>()-1
    //template<int cc> void entity (AlbertaGridLevelIterator<cc,dim,dimworld>& it, int i);

    /*! Intra-level access to intersection with neighboring elements.
       A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor. */
    AlbertaGridIntersectionIterator<dim,dimworld> ibegin () const;
    //! same method for fast access
    void ibegin (AlbertaGridIntersectionIterator<dim,dimworld> &it) const;

    //! Reference to one past the last intersection with neighbor
    AlbertaGridIntersectionIterator<dim,dimworld> iend () const;
    //! same method for fast access
    void iend (AlbertaGridIntersectionIterator<dim,dimworld> &it) const;

    //! returns true if Entity has children
    bool hasChildren () const ;

    //! returns true if entity is leaf entity, i.e. has no children
    bool isLeaf () const ;

    //! Inter-level access to father element on coarser grid.
    //! Assumes that meshes are nested.
    AlbertaGridLevelIterator<0,dim,dimworld,All_Partition> father () const;
    void father (AlbertaGridEntity<0,dim,dimworld> & vati) const ;
    AlbertaGridEntity<0,dim,dimworld> newEntity();

    /*! Location of this element relative to the reference element
       of the father. This is sufficient to interpolate all
       dofs in conforming case. Nonconforming may require access to
       neighbors of father and computations with local coordinates.
       On the fly case is somewhat inefficient since dofs  are visited
       several times. If we store interpolation matrices, this is tolerable.
       We assume that on-the-fly implementation of numerical algorithms
       is only done for simple discretizations. Assumes that meshes are nested.
     */
    AlbertaGridElement<dim,dim>& father_relative_local () const;

    /*! Inter-level access to son elements on higher levels<=maxlevel.
       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
     */
    AlbertaGridHierarchicIterator<dim,dimworld> hbegin (int maxlevel) const;

    //! Returns iterator to one past the last son
    AlbertaGridHierarchicIterator<dim,dimworld> hend (int maxlevel) const;

    //***************************************************************
    //  Interface for Adaptation
    //***************************************************************
    //! marks an element for refCount refines. if refCount is negative the
    //! element is coarsend -refCount times
    //! mark returns true if element was marked, otherwise false
    bool mark( int refCount );

    //! return whether entity could be cosrsend (COARSEND) or was refined
    //! (REFIEND) or nothing happend (NONE)
    AdaptationState state () const;

    //***************************************************************
    //  Interface for parallelisation
    //***************************************************************

    //! AlbertaGrid internal method for partitioning
    //! set processor number of this entity
    bool partition( int proc );

    //! return partition type of this entity ( see grid.hh )
    PartitionType partitionType() const;

    //! return true if this entity belong to master set of this grid
    bool master() const;

    //! return processor number where entity is master
    int owner () const;

    //! return the global unique index in grid
    int el_index() const;

    //! return the global unique index in grid , same as el_index
    int global_index() const ;

  private:
    // called from HierarchicIterator, because only this
    // class changes the level of the entity, otherwise level is set by
    // Constructor
    void setLevel ( int actLevel );

    // face, edge and vertex only for codim > 0, in this
    // case just to supply the same interface
    void setTraverseStack (ALBERTA TRAVERSE_STACK *travStack);
    void setElInfo (ALBERTA EL_INFO *elInfo,
                    int elNum = 0,
                    int face = 0,
                    int edge = 0,
                    int vertex = 0 );

    // needed for LevelIterator to compare
    ALBERTA EL_INFO *getElInfo () const;

    //! make a new AlbertaGridEntity
    void makeDescription();

    //! the corresponding grid
    AlbertaGrid<dim,dimworld> & grid_;

    //! the level of the entity
    int level_;

    //! for vertex access, to be revised, filled on demand
    mutable AlbertaGridLevelIterator<dim,dim,dimworld,All_Partition> vxEntity_;

    //! pointer to the Albert TRAVERSE_STACK data
    ALBERTA TRAVERSE_STACK * travStack_;

    //! pointer to the real Albert element data
    ALBERTA EL_INFO *elInfo_;

    // local coordinates within father
    mutable AlbertaGridElement <dim,dim> fatherReLocal_;

    //! the cuurent geometry
    mutable AlbertaGridElement<dim,dimworld> geo_;
    mutable bool builtgeometry_; //!< true if geometry has been constructed

  }; // end of AlbertaGridEntity codim = 0

  //**********************************************************************
  //
  // --AlbertaGridHierarchicIterator
  // --HierarchicIterator
  /*!
     Mesh entities of codimension 0 ("elements") allow to visit all entities of
     codimension 0 obtained through nested, hierarchic refinement of the entity.
     Iteration over this set of entities is provided by the HIerarchicIterator,
     starting from a given entity.
     This is redundant but important for memory efficient implementations of unstru
     hierarchically refined meshes.
   */

  template<int dim, int dimworld>
  class AlbertaGridHierarchicIterator :
    public HierarchicIteratorDefault <dim,dimworld,albertCtype,
        AlbertaGridHierarchicIterator,AlbertaGridEntity>
  {
  public:
    //! the normal Constructor
    AlbertaGridHierarchicIterator(AlbertaGrid<dim,dimworld> &grid,
                                  ALBERTA TRAVERSE_STACK *travStack, int actLevel, int maxLevel);

    //! the default Constructor
    AlbertaGridHierarchicIterator(AlbertaGrid<dim,dimworld> &grid,
                                  int actLevel,int maxLevel);

    //! prefix increment
    AlbertaGridHierarchicIterator& operator ++();

    //! equality
    bool operator== (const AlbertaGridHierarchicIterator& i) const;

    //! inequality
    bool operator!= (const AlbertaGridHierarchicIterator& i) const;

    //! dereferencing
    AlbertaGridEntity<0,dim,dimworld>& operator*();

    //! arrow
    AlbertaGridEntity<0,dim,dimworld>* operator->();

  private:
    //! know the grid were im comming from
    AlbertaGrid<dim,dimworld> &grid_;

    //! the actual Level of this Hierarichic Iterator
    int level_;

    //! max level to go down
    int maxlevel_;

    //! implement with virtual element
    AlbertaGridEntity<0,dim,dimworld> virtualEntity_;

    //! we need this for Albert traversal, and we need ManageTravStack, which
    //! does count References when copied
    ALBERTA ManageTravStack manageStack_;

    //! The nessesary things for Albert
    ALBERTA EL_INFO * recursiveTraverse(ALBERTA TRAVERSE_STACK * stack);

    //! make empty HierarchicIterator
    void makeIterator();
  };


  /** \todo Please doc me! */
  template<int dim, int dimworld>
  class AlbertaGridBoundaryEntity
    : public BoundaryEntityDefault <dim,dimworld,albertCtype,
          AlbertaGridElement,AlbertaGridBoundaryEntity>
  {
    friend class AlbertaGridIntersectionIterator<dim,dimworld>;
  public:
    //! Constructor
    AlbertaGridBoundaryEntity ();

    //! return identifier of boundary segment, number
    int id () const;

    //! return true if geometry of ghost cells was filled
    bool hasGeometry () const ;

    //! return geometry of the ghost cell
    AlbertaGridElement<dim,dimworld>& geometry () const;

  private:
    // set elInfo
    void setElInfo(ALBERTA EL_INFO * elInfo, int nb);

    // ghost cell
    mutable AlbertaGridElement<dim,dimworld> _geom;

    // cooresponding el_info
    ALBERTA EL_INFO * _elInfo;

    int _neigh;
  };


  //**********************************************************************
  //
  // --AlbertaGridIntersectionIterator
  // --IntersectionIterator
  /*!
     Mesh entities of codimension 0 ("elements") allow to visit all neighbors, wh
     a neighbor is an entity of codimension 0 which has a common entity of codimens
     These neighbors are accessed via a IntersectionIterator. This allows the implement
     non-matching meshes. The number of neigbors may be different from the number o
     of an element!
   */
  template<int dim, int dimworld>
  class AlbertaGridIntersectionIterator :
    public IntersectionIteratorDefault <dim,dimworld,albertCtype,
        AlbertaGridIntersectionIterator,AlbertaGridEntity,
        AlbertaGridElement, AlbertaGridBoundaryEntity>
  {
    friend class AlbertaGridEntity<0,dim,dimworld>;
  public:
    //! prefix increment
    AlbertaGridIntersectionIterator& operator ++();

    //! The default Constructor makes empty Iterator
    AlbertaGridIntersectionIterator();

    //! The default Constructor
    AlbertaGridIntersectionIterator(AlbertaGrid<dim,dimworld> &grid,int level);

    //! The Constructor
    AlbertaGridIntersectionIterator(AlbertaGrid<dim,dimworld> &grid,int level,
                                    ALBERTA EL_INFO *elInfo);

    //! The Destructor
    ~AlbertaGridIntersectionIterator();

    //! equality
    bool operator== (const AlbertaGridIntersectionIterator& i) const;

    //! inequality
    bool operator!= (const AlbertaGridIntersectionIterator& i) const;

    //! access neighbor, dereferencing
    AlbertaGridEntity<0,dim,dimworld>& operator*();

    //! access neighbor, arrow
    AlbertaGridEntity<0,dim,dimworld>* operator->();

    //! return true if intersection is with boundary. \todo connection with
    //! boundary information, processor/outer boundary
    bool boundary () const;

    //! return true if across the edge an neighbor on this level exists
    bool neighbor () const;

    //! return information about the Boundary
    AlbertaGridBoundaryEntity<dim,dimworld> & boundaryEntity () const;

    //! return unit outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    FieldVector<albertCtype, dimworld>& unit_outer_normal (FieldVector<albertCtype, dim-1>& local) const;

    //! return unit outer normal, if you know it is constant use this function instead
    FieldVector<albertCtype, dimworld>& unit_outer_normal () const;

    //! intersection of codimension 1 of this neighbor with element where
    //! iteration started.
    //! Here returned element is in LOCAL coordinates of the element
    //! where iteration started.
    AlbertaGridElement<dim-1,dim>& intersection_self_local () const;

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in GLOBAL coordinates of the element where iteration started.
    AlbertaGridElement<dim-1,dimworld>& intersection_self_global () const;

    //! local number of codim 1 entity in self where intersection is contained in
    int number_in_self () const;

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    AlbertaGridElement<dim-1,dim>& intersection_neighbor_local () const;

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    AlbertaGridElement<dim-1,dimworld>& intersection_neighbor_global () const;

    //! local number of codim 1 entity in neighbor where intersection is contained
    int number_in_neighbor () const;

    //! return outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    FieldVector<albertCtype, dimworld>& outer_normal (FieldVector<albertCtype, dim-1>& local) const;

    //! return unit outer normal, if you know it is constant use this function instead
    FieldVector<albertCtype, dimworld>& outer_normal () const;

  private:
    //**********************************************************
    //  private methods
    //**********************************************************
    //! make Iterator set to begin of actual entitys intersection Iterator
    void makeBegin (AlbertaGrid<dim,dimworld> &grid,int level, ALBERTA EL_INFO * elInfo );

    //! set Iterator to end of actual entitys intersection Iterator
    void makeEnd (AlbertaGrid<dim,dimworld> &grid,int level );

    // put objects on stack
    void freeObjects ();

    //! setup the virtual neighbor
    void setupVirtEn ();

    //! know the grid were im comming from
    AlbertaGrid<dim,dimworld> *grid_;

    //! the actual level
    int level_;

    //! count on which neighbor we are lookin' at
    int neighborCount_;

    //! implement with virtual element
    //! Most of the information can be generated from the ALBERTA EL_INFO
    //! therefore this element is only created on demand.
    mutable bool builtNeigh_;
    mutable AlbertaGridEntity<0,dim,dimworld> *virtualEntity_;

    //! pointer to the EL_INFO struct storing the real element information
    ALBERTA EL_INFO * elInfo_;

    // for memory management
    mutable typename AlbertaGrid<dim,dimworld>::EntityProvider::ObjectEntity *manageObj_;
    mutable typename AlbertaGrid<dim,dimworld>::IntersectionSelfProvider::ObjectEntity  *manageInterEl_;
    mutable typename AlbertaGrid<dim,dimworld>::IntersectionNeighProvider::ObjectEntity *manageNeighEl_;

    //! pointer to element holding the self_local and self_global information.
    //! This element is created on demand.
    mutable AlbertaGridElement<dim-1,dim> *fakeNeigh_;

    //! pointer to element holding the neighbor_global and neighbor_local
    //! information. This element is created on demand.
    mutable AlbertaGridElement<dim-1,dimworld> *neighGlob_;

    //! BoundaryEntity
    mutable AlbertaGridBoundaryEntity<dim,dimworld> *boundaryEntity_;

    //! defined in agmemory.hh
    mutable typename ElInfoProvider::ObjectEntity *manageNeighInfo_;

    //! EL_INFO th store the information of the neighbor if needed
    mutable ALBERTA EL_INFO * neighElInfo_;

    mutable FieldVector<albertCtype, dimworld> outNormal_;

    // tmp memory for normal calculation
    mutable FieldVector<albertCtype, dimworld> tmpU_;
    mutable FieldVector<albertCtype, dimworld> tmpV_;
  };



  //**********************************************************************
  //
  // --AlbertaGridLevelIterator
  // --LevelIterator
  /*!
     Enables iteration over all entities of a given codimension and level of a grid.
   */
  template<int codim, int dim, int dimworld, PartitionIteratorType pitype>
  class AlbertaGridLevelIterator :
    public LevelIteratorDefault <codim,dim,dimworld,pitype,albertCtype,
        AlbertaGridLevelIterator,AlbertaGridEntity>
  {
    friend class AlbertaGridEntity<2,dim,dimworld>;
    friend class AlbertaGridEntity<1,dim,dimworld>;
    friend class AlbertaGridEntity<0,dim,dimworld>;
    friend class AlbertaGrid < dim , dimworld >;

    typedef AlbertaGridLevelIterator<codim,dim,dimworld,pitype>  AlbertaGridLevelIteratorType;
  public:

    enum { dimension = dim };

    //! Constructor
    AlbertaGridLevelIterator(AlbertaGrid<dim,dimworld> &grid, int
                             travLevel, int proc, bool leafIt=false );

    //! Constructor
    AlbertaGridLevelIterator(AlbertaGrid<dim,dimworld> &grid, int travLevel,
                             ALBERTA EL_INFO *elInfo,int elNum = 0 , int face=0, int edge=0,int vertex=0);

    //! Constructor
    AlbertaGridLevelIterator(AlbertaGrid<dim,dimworld> &grid,
                             AlbertaMarkerVector * vec ,int travLevel,
                             int proc ,bool leafIt=false);

    //! prefix increment
    AlbertaGridLevelIteratorType& operator ++();

    //! equality
    bool operator== (const AlbertaGridLevelIteratorType & i) const;

    //! inequality
    bool operator!= (const AlbertaGridLevelIteratorType & i) const;

    //! dereferencing
    AlbertaGridEntity<codim,dim,dimworld>& operator *() ;

    //! arrow
    AlbertaGridEntity<codim,dim,dimworld>* operator->() ;

    //! dereferencing
    const AlbertaGridEntity<codim,dim,dimworld>& operator *() const ;

    //! arrow
    const AlbertaGridEntity<codim,dim,dimworld>* operator->() const ;

    //! ask for level of entity
    int level () const;

  private:
    // private Methods
    void makeIterator();

    ALBERTA EL_INFO * goFirstElement(ALBERTA TRAVERSE_STACK *stack,
                                     ALBERTA MESH *mesh,
                                     int level, ALBERTA FLAGS fill_flag);
    ALBERTA EL_INFO * traverseElLevel(ALBERTA TRAVERSE_STACK * stack);
    ALBERTA EL_INFO * traverseElLevelInteriorBorder(ALBERTA TRAVERSE_STACK * stack);
    ALBERTA EL_INFO * traverseElLevelGhosts(ALBERTA TRAVERSE_STACK * stack);

    // the default is, go to next elInfo
    //template <int cc>
    ALBERTA EL_INFO * goNextEntity(ALBERTA TRAVERSE_STACK *stack,ALBERTA EL_INFO *elinfo_old);

    // the real go next methods
    ALBERTA EL_INFO * goNextElInfo(ALBERTA TRAVERSE_STACK *stack,ALBERTA EL_INFO *elinfo_old);
    ALBERTA EL_INFO * goNextFace(ALBERTA TRAVERSE_STACK *stack,
                                 ALBERTA EL_INFO *elInfo);
    ALBERTA EL_INFO * goNextEdge(ALBERTA TRAVERSE_STACK *stack,
                                 ALBERTA EL_INFO *elInfo);
    ALBERTA EL_INFO * goNextVertex(ALBERTA TRAVERSE_STACK *stack,
                                   ALBERTA EL_INFO *elInfo);

    // search next macro el
    ALBERTA MACRO_EL * nextGhostMacro(ALBERTA MACRO_EL *mel);

    //! the grid were it all comes from
    AlbertaGrid<dim,dimworld> & grid_;

    //! level :)
    int level_;

    // private Members
    AlbertaGridEntity<codim,dim,dimworld> virtualEntity_;

    // contains ALBERTA traversal stack
    ALBERTA ManageTravStack manageStack_;

    //! element number
    int elNum_;

    //! which face, edge and vertex are we watching of an elInfo
    int face_;
    int edge_;
    int vertex_;

    // knows on which element a point is viewed
    AlbertaMarkerVector * vertexMarker_;

    // variable for operator++
    bool okReturn_;

    // true if we use LeafIterator
    bool leafIt_;

    // store processor number of elements
    // for ghost walktrough, i.e. walk over ghosts which belong
    // tp processor 2
    const int proc_;
  };


  //**********************************************************************
  //
  // --AlbertaGrid
  // --Grid
  //
  //**********************************************************************

  /** \brief The Albert %Grid class
   *
   * This is the implementation of the grid interface class.
   */
  template <int dim, int dimworld>
  class AlbertaGrid : public GridDefault  < dim, dimworld,
                          albertCtype,AlbertaGrid,
                          AlbertaGridLevelIterator,AlbertaGridEntity>
  {
    friend class AlbertaGridEntity <0,dim,dimworld>;
    //friend class AlbertaGridEntity <1,dim,dimworld>;
    //friend class AlbertaGridEntity <1 << dim-1 ,dim,dimworld>;
    friend class AlbertaGridEntity <dim,dim,dimworld>;

    friend class AlbertaMarkerVector;

    // friends because of fillElInfo
    friend class AlbertaGridLevelIterator<0,dim,dimworld,All_Partition>;
    friend class AlbertaGridLevelIterator<1,dim,dimworld,All_Partition>;
    friend class AlbertaGridLevelIterator<2,dim,dimworld,All_Partition>;
    friend class AlbertaGridLevelIterator<3,dim,dimworld,All_Partition>;
    friend class AlbertaGridHierarchicIterator<dim,dimworld>;

    friend class AlbertaGridIntersectionIterator<dim,dimworld>;

    //! AlbertaGrid is only implemented for 2 and 3 dimension
    //! for 1d use SGrid or SimpleGrid
    CompileTimeChecker<dimworld != 1>   Do_not_use_AlbertaGrid_for_1d_Grids;

    //**********************************************************
    // The Interface Methods
    //**********************************************************
  public:
    //! remember the types of template parameters
    template <int codim>
    struct Traits
    {
      //! Please doc me!
      typedef albertCtype CoordType;

      //! Please doc me!
      typedef AlbertaGrid<dim,dimworld>           ImpGrid;

      //! Please doc me!
      typedef AlbertaGridLevelIterator<codim,dim,dimworld,All_Partition>  LevelIterator;

      //! Please doc me!
      typedef AlbertaGridLevelIterator<codim,dim,dimworld,Interior_Partition>        InteriorLevelIterator;

      //! Please doc me!
      typedef AlbertaGridLevelIterator<codim,dim,dimworld,InteriorBorder_Partition>  InteriorBorderLevelIterator;

      //! Please doc me!
      typedef AlbertaGridLevelIterator<codim,dim,dimworld,Overlap_Partition>         OverlapLevelIterator;

      //! Please doc me!
      typedef AlbertaGridLevelIterator<codim,dim,dimworld,OverlapFront_Partition>    OverlapFrontLevelIterator;

      //! Please doc me!
      typedef AlbertaGridLevelIterator<codim,dim,dimworld,Ghost_Partition>           GhostLevelIterator;

      //! Please doc me!
      typedef ConstLevelIteratorWrapper<AlbertaGridLevelIterator<codim,dim,dimworld,All_Partition> >  ConstLevelIterator;

      //! Please doc me!
      typedef ConstLevelIteratorWrapper<AlbertaGridLevelIterator<codim,dim,dimworld,Interior_Partition> > ConstInteriorLevelIterator;

      //! Please doc me!
      typedef ConstLevelIteratorWrapper<AlbertaGridLevelIterator<codim,dim,dimworld,InteriorBorder_Partition> > ConstInteriorBorderLevelIterator;

      //! Please doc me!
      typedef ConstLevelIteratorWrapper<AlbertaGridLevelIterator<codim,dim,dimworld,Overlap_Partition> > ConstOverlapLevelIterator;

      //! Please doc me!
      typedef ConstLevelIteratorWrapper<AlbertaGridLevelIterator<codim,dim,dimworld,OverlapFront_Partition> > ConstOverlapFrontLevelIterator;

      //! Please doc me!
      typedef ConstLevelIteratorWrapper<AlbertaGridLevelIterator<codim,dim,dimworld,Ghost_Partition> > ConstGhostLevelIterator;

      //! Please doc me!
      typedef AlbertaGridEntity<codim,dim,dimworld>         Entity;
    };

    typedef AlbertaGridLevelIterator<0,dim,dimworld,All_Partition> LeafIterator;
    typedef AlbertaGridReferenceElement<dim> ReferenceElement;

    typedef ObjectStream ObjectStreamType;
    typedef std::pair < ObjectStreamType * ,
        AlbertaGridEntity<0,dim,dimworld>  * > DataCollectorParamType;

    template<int codim, PartitionIteratorType pitype>
    struct ConstAlbertaGridLevelIterator
    {
      typedef ConstLevelIteratorWrapper<
          AlbertaGridLevelIterator<codim,dim,dimworld,pitype> > IteratorType;
    };

    typedef typename ConstAlbertaGridLevelIterator<0,All_Partition> :: IteratorType Const0LevelIteratorType;
    typedef typename ConstAlbertaGridLevelIterator<dim,All_Partition> :: IteratorType ConstDimLevelIteratorType;

    //! we always have dim+1 codimensions since we use only simplices
    enum { numCodim = dim+1 };

    //! Constructor which reads an Albert Macro Triang file
    //! or given GridFile
    //! levInd = true means that a consecutive level index is generated
    //! if levInd == true the the element number of first macro element is
    //! set to 1 so hasLevelIndex_ can be identified we grid is read from
    //! file
    AlbertaGrid(const char* macroTriangFilename, bool levInd = true);

    //! Constructor which reads an Albert Macro Triang file
    //! or given GridFile , proc is the number of domain ,
    //! levInd = true means that a consecutive level index is generated
    //! if levInd == true the the element number of first macro element is
    //! set to 1 so hasLevelIndex_ can be identified we grid is read from
    //! file
    AlbertaGrid(AlbertaGrid<dim,dimworld> & oldGrid, int proc,bool levInd = true);

    //! empty Constructor
    AlbertaGrid();

    //! Desctructor
    ~AlbertaGrid();

    //! Return maximum level defined in this grid. Levels are numbered
    //! 0 ... maxlevel with 0 the coarsest level.
    int maxlevel() const;

    //! Iterator to first entity of given codim on level
    template<int codim, PartitionIteratorType pitype>
    AlbertaGridLevelIterator<codim,dim,dimworld,pitype>
    lbegin (int level, int proc = -1 );

    //! one past the end on this level
    template<int codim, PartitionIteratorType pitype>
    AlbertaGridLevelIterator<codim,dim,dimworld,pitype>
    lend (int level, int proc = -1 );

    //! Iterator to first entity of given codim on level
    template<int codim> AlbertaGridLevelIterator<codim,dim,dimworld,All_Partition>
    lbegin (int level, int proc = -1 );

    //! one past the end on this level
    template<int codim> AlbertaGridLevelIterator<codim,dim,dimworld,All_Partition>
    lend (int level, int proc = -1 );

    // the const versions

    //! Iterator to first entity of given codim on level
    template<int codim, PartitionIteratorType pitype>
    typename ConstAlbertaGridLevelIterator <codim,pitype> :: IteratorType
    lbegin (int level, int proc = -1 ) const;

    //! one past the end on this level
    template<int codim, PartitionIteratorType pitype>
    typename ConstAlbertaGridLevelIterator <codim,pitype> :: IteratorType
    lend (int level, int proc = -1 ) const;

    //! Iterator to first entity of given codim on level
    template<int codim>
    typename ConstAlbertaGridLevelIterator <codim,All_Partition> :: IteratorType
    lbegin (int level, int proc = -1 ) const;

    //! one past the end on this level
    template<int codim>
    typename ConstAlbertaGridLevelIterator <codim,All_Partition> :: IteratorType
    lend (int level, int proc = -1 ) const;

    /** \brief Number of grid entities per level and codim
     * because lbegin and lend are none const, and we need this methods
     * counting the entities on each level, you know.
     */
    int size (int level, int codim) const;

    /*! \brief refine all positive marked leaf entities,
    *  coarsen all negative marked entities if possible,
    *  return true if a least one element was refined */
    bool adapt ( );

    //! returns true, if a least one element is marked for coarsening
    bool preAdapt ();

    //! clean up some markers
    bool postAdapt();

    /** \brief return type of grid, here AlbertaGrid_Id. */
    GridIdentifier type () const { return AlbertaGrid_Id; };

    //**********************************************************
    // End of Interface Methods
    //**********************************************************

    //! uses the interface, mark on entity and refineLocal
    bool globalRefine(int refCount);

    /** \brief write Grid to file in specified FileFormatType */
    template <FileFormatType ftype>
    bool writeGrid( const char * filename, albertCtype time ) const;

    /** \brief read Grid from file filename and store time of mesh in time */
    template <FileFormatType ftype>
    bool readGrid( const char * filename, albertCtype & time );

    //! return current time of grid
    //! not an interface method yet
    albertCtype getTime () const { return time_; };

    //! return LeafIterator which points to first leaf entity
    LeafIterator leafbegin ( int maxlevel, int proc = -1 );

    //! return LeafIterator which points behind last leaf entity
    LeafIterator leafend   ( int maxlevel, int proc = -1 );

    //! returns size of mesh include all levels
    //! max Index of grid entities with given codim
    //! for outside the min index is 0, the shift has to done inside
    //! the grid which is of minor cost
    int global_size (int codim) const;

    //! return number of my processor
    int myProcessor () const { return myProc_; };

    //! transform grid N = scalar * x + trans
    void setNewCoords(const FieldVector<albertCtype, dimworld> & trans, const albertCtype scalar);
  private:
    //! number of grid entities per level and codim, size is cached
    int calcLevelSize (int level, int codim);

    // initialize of some members
    void initGrid(int proc, bool swapEls = true );

    // max global index in Grid
    int maxHierIndex_[dim+1];

    // make the calculation of indexOnLevel and so on.
    // extra method because of Reihenfolge
    void calcExtras();

    // write ALBERTA mesh file
    bool writeGridXdr  ( const char * filename, albertCtype time ) const;

    //! reads ALBERTA mesh file
    bool readGridXdr   ( const char * filename, albertCtype & time );

    //! reads ALBERTA macro file
    bool readGridAscii ( const char * filename, albertCtype & time );

    //! access to mesh pointer, needed by some methods
    ALBERTA MESH* getMesh () const { return mesh_; };

    // delete mesh and all vectors
    void removeMesh();

    // pointer to an Albert Mesh, which contains the data
    ALBERTA MESH *mesh_;

    // number of maxlevel of the mesh
    int maxlevel_;

    // true if grid was refined or coarsend
    bool wasChanged_;

    // is true, if a least one entity is marked for coarsening
    bool isMarked_;

    // set isMarked, isMarked is true if at least one entity is marked for
    // coarsening
    void setMark ( bool isMarked );

    // number of entitys of each level an codim
    mutable Array<int> size_;

    // help vector for setNewCoords
    mutable Array<int> macroVertices_;

    // this method is new fill_elinfo from ALBERTA but here the neighbor
    // relations are calced diffrent, on ervery level there are neighbor
    // realtions ( in ALBERTA only on leaf level ), so we needed a new
    // fill_elinfo.
    void fillElInfo(int ichild, int actLevel ,const ALBERTA EL_INFO *elinfo_old,
                    ALBERTA EL_INFO *elinfo, bool hierachical, bool leaf=false ) const;

    // calc the neigh[0]
    void firstNeigh(const int ichild,const ALBERTA EL_INFO *elinfo_old,
                    ALBERTA EL_INFO *elinfo, const bool leafLevel) const;

    // calc the neigh[1]
    void secondNeigh(const int ichild, const ALBERTA EL_INFO *elinfo_old,
                     ALBERTA EL_INFO *elinfo, const bool leafLevel) const;

    // calc the neigh[2]
    void thirdNeigh(const int ichild, const ALBERTA EL_INFO *elinfo_old,
                    ALBERTA EL_INFO *elinfo, const bool leafLevel) const;

    // needed for VertexIterator, mark on which element a vertex is treated
    AlbertaMarkerVector * vertexMarker_;

    //*********************************************************
    // Methods for mapping the global Index to local on Level
    // contains the index on level for each unique el->index of Albert
    enum { AG_MAXLEVELS = 100 };
    Array<int> levelIndex_[dim+1][AG_MAXLEVELS];

    void makeNewSize(Array<int> &a, int newNumberOfEntries);
    void markNew();

    //! map the global index from the Albert Mesh to the local index on Level
    template <int codim>
    int indexOnLevel(int globalIndex, int level ) const ;

    // pointer to the real number of elements or vertices
    // i.e. points to mesh->n_hier_elements or mesh->n_vertices
    int numberOfEntitys_[dim+1];
    int oldNumberOfEntities_[dim+1];

    //! actual time of Grid
    albertCtype time_;

    // true if level index is porvided
    bool hasLevelIndex_;

    //***********************************************************************
    //  MemoryManagement for Entitys and Elements
    //**********************************************************************
    typedef MemoryProvider< AlbertaGridEntity<0,dim,dimworld > > EntityProvider;
    typedef MemoryProvider< AlbertaGridElement<dim-1,dimworld> > IntersectionSelfProvider;
    typedef MemoryProvider< AlbertaGridElement<dim-1,dim> >      IntersectionNeighProvider;

    EntityProvider entityProvider_;
    IntersectionSelfProvider interSelfProvider_;
    IntersectionNeighProvider interNeighProvider_;

    //*********************************************************************
    // organisation of the global index
    //*********************************************************************
    // provides the indices for the elements
    IndexManagerType indexStack_[dim+1];

    // the DOF_INT_VECs we need
    ALBERTA AlbertHelp::DOFVEC_STACK dofvecs_;

    const ALBERTA DOF_ADMIN * elAdmin_;
    // pointer to vec of elNumbers_
    const int * elNumVec_;
    const int * elNewVec_;

    const int nv_;
    const int dof_;

    // make some shortcuts
    void arrangeDofVec();

    // return true if el is new
    bool checkElNew ( ALBERTA EL * el ) const;

    // read global element number form elNumbers_
    int getElementNumber ( ALBERTA EL * el ) const;

    //********************************************************************
    //  organisation of the parallelisation
    //********************************************************************

    // pointer to vec  with processor number for each element,
    // access via setOwner and getOwner
    int * ownerVec_;

    // set owner of element, for partioning
    bool setOwner ( ALBERTA EL * el , int proc );

    // return the processor number of element
    int getOwner ( ALBERTA EL * el ) const;

    // PartitionType (InteriorEntity , BorderEntity, GhostEntity )
    PartitionType partitionType ( ALBERTA EL_INFO * elinfo) const;

    // rank of my thread, i.e. number of my processor
    const int myProc_;

  }; // end Class AlbertaGridGrid

  // Class to mark the Vertices on the leaf level
  // to visit every vertex only once
  // for the LevelIterator codim == dim
  class AlbertaMarkerVector
  {
    friend class AlbertaGrid<2,2>;
    friend class AlbertaGrid<3,3>;

    enum { MAXL = 64 };
    enum { vxBufferSize_ = 10000 };
  public:
    AlbertaMarkerVector () {} ;

    bool notOnThisElement(ALBERTA EL * el, int elIndex, int level , int vertex);

    template <class GridType>
    void markNewVertices(GridType &grid);

    void print();

  private:
    // built in array to mark on which element a vertex is reached
    Array<int> vec_[MAXL];
    // number of vertices
    int numVertex_;
  };


  /** @} end documentation group */
  namespace Capabilities
  {
    template<int dim,int dimw>
    struct hasLeafIterator< AlbertaGrid<dim,dimw> >
    {
      static const bool v = true;
    };

    template<int dim, int dimw, int cdim>
    struct hasEntity< AlbertaGrid<dim,dimw>, AlbertaGridEntity<cdim,dim,dimw> >
    {
      static const bool v = true;
    };
  }

}; // namespace Dune

#include "albertagrid/agmemory.hh"
#include <dune/io/file/asciiparser.hh>
#include "albertagrid/albertagrid.cc"

#endif
