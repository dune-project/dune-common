// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRID_HH
#define DUNE_UGGRID_HH


//#include <vector>
//#include <assert.h>

#include "../../UG/ug/gm/gm.h"
// for debugging
#include "../../UG/ug/dev/ugdevices.h"

// undef stuff defined by UG
#include "uggrid/ug_undefs.hh"


//#include "../common/misc.hh"
//#include "../common/matvec.hh"
#include "../common/array.hh"
#include "common/grid.hh"

//#include "albertgrid/agmemory.hh"

namespace Dune
{
  /** @defgroup UGGrid UGGrid Module

     This is one implementation of the grid interface providing 2d triangle
     and 3d tetrahedra grids by using the FE Box ALBERT ( ALBERT
     was written by Kunibert Siebert
     and Alfred Schmidt).

     @{
   */

  // i.e. double or float
  typedef double UGCtype;


  // forward declarations
  class UGMarkerVector;

  template<int codim, int dim, int dimworld> class UGGridEntity;
  template<int codim, int dim, int dimworld> class UGGridLevelIterator;

  template<int dim, int dimworld>            class UGGridElement;
  template<int dim, int dimworld>            class UGGridBoundaryEntity;
  template<int dim, int dimworld>            class UGGridHierarchicIterator;
  template<int dim, int dimworld>            class UGGridNeighborIterator;
  template<int dim, int dimworld>            class UGGrid;

  template <class Object> class UGGridMemory;

  // singleton holding reference elements
  template<int dim> struct UGGridReferenceElement;

  //**********************************************************************
  //
  // --UGGridElement
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
  class UGGridElement :
    public ElementDefault <dim,dimworld, UGCtype,UGGridElement>
  {
    friend class UGGridBoundaryEntity<dim,dimworld>;
  public:

    //! know dimension of world
    enum { dimbary=dim+1};

    //! for makeRefElement == true a Element with the coordinates of the
    //! reference element is made
    UGGridElement(bool makeRefElement=false);

    //! return the element type identifier
    //! line , triangle or tetrahedron, depends on dim
    ElementType type ();

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners ();

    //! access to coordinates of corners. Index is the number of the corner
    Vec<dimworld, UGCtype>& operator[] (int i);

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned.
     */
    static UGGridElement<dim,dim>& refelem ();

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    Vec<dimworld,UGCtype> global (const Vec<dim, UGCtype>& local);

    //! maps a global coordinate within the element to a
    //! local coordinate in its reference element
    Vec<dim, UGCtype>& local (const Vec<dimworld, UGCtype>& global);

    //! returns true if the point is in the current element
    bool checkInside(const Vec<dimworld, UGCtype> &global);

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
    UGCtype integration_element (const Vec<dim, UGCtype>& local);

    //! can only be called for dim=dimworld!
    Mat<dim,dim>& Jacobian_inverse (const Vec<dim, UGCtype>& local);

    //***********************************************************************
    //  Methods that not belong to the Interface, but have to be public
    //***********************************************************************
#if 0
    //! generate the geometry for the ALBERT EL_INFO
    bool builtGeom(ALBERT EL_INFO *elInfo, unsigned char face,
                   unsigned char edge, unsigned char vertex);
    // init geometry with zeros
    void initGeom();

    //! print internal data
    void print (std::ostream& ss, int indent);
#endif

  private:
    // calc the local barycentric coordinates
    template <int dimbary>
    Vec<dimbary, UGCtype>& localB (const Vec<dimworld, UGCtype>& global)
    {
      localBary_ = localBary(global);
      return localBary_;
    }

    //! built the reference element
    void makeRefElemCoords();

    //! built the jacobian inverse and store the volume
    void builtJacobianInverse (const Vec<dim, UGCtype>& local);

    //Vec<dim+1, UGCtype> tmpVec_;

    //! maps a global coordinate within the elements local barycentric
    //! koordinates
    Vec<dim+1, UGCtype> localBary (const Vec<dimworld, UGCtype>& global);

    // template method for map the vertices of EL_INFO to the actual
    // coords with face_,edge_ and vertex_ , needes for operator []
    int mapVertices (int i) const;

    // calculates the volume of the element
    UGCtype elVolume () const;

    //! the vertex coordinates
    Mat<dimworld,dim+1, UGCtype> coord_;

    //! storage for global coords
    Vec<dimworld, UGCtype> globalCoord_;

    //! storage for local coords
    Vec<dim, UGCtype> localCoord_;

    //! storage for barycentric coords
    Vec<dimbary, UGCtype> localBary_;

#if 0
    ALBERT EL_INFO * makeEmptyElInfo();

    ALBERT EL_INFO * elInfo_;
#endif

    //! Which Face of the Element 0...dim+1
    unsigned char face_;

    //! Which Edge of the Face of the Element 0...dim
    unsigned char edge_;

    //! Which Edge of the Face of the Element 0...dim-1
    unsigned char vertex_;

    //! is true if Jinv_ and volume_ is calced
    bool builtinverse_;
    Mat<dim,dim, UGCtype> Jinv_; //!< storage for inverse of jacobian
    UGCtype volume_; //!< storage of element volume

  };


  //**********************************************************************
  //
  // --UGGridEntity
  // --Entity
  //
  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Here: the general template
   */
  template<int codim, int dim, int dimworld>
  class UGGridEntity :
    public EntityDefault <codim,dim,dimworld, UGCtype,
        UGGridEntity,UGGridElement,UGGridLevelIterator,
        UGGridNeighborIterator,UGGridHierarchicIterator>
  {
    friend class UGGrid < dim , dimworld >;
    //friend class UGGridEntity < 0, dim, dimworld>;
    friend class UGGridLevelIterator < codim, dim, dimworld>;
    //friend class UGGridLevelIterator < dim, dim, dimworld>;
  public:
    //! know your own codimension
    //enum { codimension=codim };

    //! know your own dimension
    //enum { dimension=dim };

    //! know your own dimension of world
    //enum { dimensionworld=dimworld };

    //! level of this element
    int level ();

    //! index is unique and consecutive per level and codim
    //! used for access to degrees of freedom
    int index ();


    UGGridEntity(UGGrid<dim,dimworld> &grid, int level);

    //! geometry of this entity
    UGGridElement<dim-codim,dimworld>& geometry ();

    /*! Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    UGGridLevelIterator<0,dim,dimworld> father ();

    //! local coordinates within father
    Vec<dim, UGCtype>& local ();
  private:

#if 0
    // methods for setting the infos from the albert mesh
    void setTraverseStack (ALBERT TRAVERSE_STACK *travStack);
    void setElInfo (ALBERT EL_INFO *elInfo, int elNum, unsigned char face,
                    unsigned char edge, unsigned char vertex );
    // needed for the LevelIterator
    ALBERT EL_INFO *getElInfo () const;
#endif

    // returns the global vertex number as default
    int globalIndex() { return elInfo_->el->dof[vertex_][0]; }

    // private Methods
    void makeDescription();

    UGGrid<dim,dimworld> &grid_;

#if 0
    // private Members
    ALBERT EL_INFO *elInfo_;
    ALBERT TRAVERSE_STACK * travStack_;
#endif

    //! the cuurent geometry
    UGGridElement<dim-codim,dimworld> geo_;
    bool builtgeometry_;       //!< true if geometry has been constructed

    Vec<dim,UGCtype> localFatherCoords_;

    //! element number
    int elNum_;

    //! level
    int level_;

    //! Which Face of the Element
    unsigned char face_;

    //! Which Edge of the Face of the Element
    unsigned char edge_;

    //! Which Vertex of the Edge of the Face of the Element
    unsigned char vertex_;
  };

#if 0
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
  //  --UGGridEntity
  //  --0Entity
  //
  //***********************
  template<int dim, int dimworld>
  class UGGridEntity<0,dim,dimworld> :
    public EntityDefault<0,dim,dimworld, UGCtype,UGGridEntity,UGGridElement,
        UGGridLevelIterator,UGGridNeighborIterator,
        UGGridHierarchicIterator>
  {
    friend class UGGrid < dim , dimworld >;
    friend class UGMarkerVector;
    friend class UGGridNeighborIterator < dim, dimworld>;
    friend class UGGridHierarchicIterator < dim, dimworld>;
    friend class UGGridLevelIterator <0,dim,dimworld>;
  public:
    typedef UGGridNeighborIterator<dim,dimworld> NeighborIterator;
    typedef UGGridHierarchicIterator<dim,dimworld> HierarchicIterator;

    //! know your own codimension
    //enum { codimension=0 };

    //! know your own dimension
    //enum { dimension=dim };

    //! know your own dimension of world
    //enum { dimensionworld=dimworld };

    //! Destructor, needed perhaps needed for deleteing faceEntity_ and
    //! edgeEntity_ , see below
    //! there are only implementations for dim==dimworld 2,3
    ~UGGridEntity() {};

    //! Constructor, real information is set via setElInfo method
    UGGridEntity(UGGrid<dim,dimworld> &grid, int level);

    //! level of this element
    int level ();

    //! index is unique and consecutive per level and codim used for access to degrees of freedo
    int index ();

    //! geometry of this entity
    UGGridElement<dim,dimworld>& geometry ();

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
       with codimension cc.
     */
    //!< Default codim 1 Faces and codim == dim Vertices
    template<int cc> int count ();

    //! return index of sub entity with codim = cc and local number i
    //! i.e. return global number of vertex i
    template<int cc> int subIndex (int i);

    //! Provide access to mesh entity i of given codimension. Entities
    //!  are numbered 0 ... count<cc>()-1
    template<int cc> UGGridLevelIterator<cc,dim,dimworld> entity (int i);

    /*! Intra-level access to neighboring elements. A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor. */
    UGGridNeighborIterator<dim,dimworld> nbegin ();

    //! Reference to one past the last neighbor
    UGGridNeighborIterator<dim,dimworld> nend ();

    //! returns true if Entity has children
    bool hasChildren ();

    //! return number of layers far from refined elements of this level
    int refDistance ();

    //! Inter-level access to father element on coarser grid.
    //! Assumes that meshes are nested.
    UGGridLevelIterator<0,dim,dimworld> father ();

    /*! Location of this element relative to the reference element
       of the father. This is sufficient to interpolate all
       dofs in conforming case. Nonconforming may require access to
       neighbors of father and computations with local coordinates.
       On the fly case is somewhat inefficient since dofs  are visited
       several times. If we store interpolation matrices, this is tolerable.
       We assume that on-the-fly implementation of numerical algorithms
       is only done for simple discretizations. Assumes that meshes are nested.
     */
    UGGridElement<dim,dim>& father_relative_local ();

    /*! Inter-level access to son elements on higher levels<=maxlevel.
       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
     */
    UGGridHierarchicIterator<dim,dimworld> hbegin (int maxlevel);

    //! Returns iterator to one past the last son
    UGGridHierarchicIterator<dim,dimworld> hend (int maxlevel);

    //***************************************************************
    //  Interface for Adaptation
    //***************************************************************

    //! marks an element for refCount refines. if refCount is negative the
    //! element is coarsend -refCount times
    //! mark returns true if element was marked, otherwise false
    bool mark( int refCount );

  private:
    // called from HierarchicIterator, because only this
    // class changes the level of the entity, otherwise level is set by
    // Constructor
    void setLevel ( int actLevel );

#if 0
    // face, edge and vertex only for codim > 0, in this
    // case just to supply the same interface
    void setTraverseStack (ALBERT TRAVERSE_STACK *travStack);
    void setElInfo (ALBERT EL_INFO *elInfo,
                    int elNum = 0,
                    unsigned char face = 0,
                    unsigned char edge = 0,
                    unsigned char vertex = 0 );

    // needed for LevelIterator to compare
    ALBERT EL_INFO *getElInfo () const;
#endif

    // return the global unique index in mesh
    int globalIndex() { return elInfo_->el->index; }

    //! make a new UGGridEntity
    void makeDescription();

    //! the corresponding grid
    UGGrid<dim,dimworld> &grid_;

    //! for vertex access, to be revised, filled on demand
    UGGridLevelIterator<dim,dim,dimworld> vxEntity_;

    //! the cuurent geometry
    UGGridElement<dim,dimworld> geo_;
    bool builtgeometry_; //!< true if geometry has been constructed

#if 0
    //! pointer to the real UG element data
    ALBERT EL_INFO *elInfo_;
#endif

    //! the level of the entity
    int level_;

#if 0
    //! pointer to the UG TRAVERSE_STACK data
    ALBERT TRAVERSE_STACK * travStack_;
#endif

    UGGridElement <dim,dim> fatherReLocal_;
  }; // end of UGGridEntity codim = 0


#endif //#if 0

  //**********************************************************************
  //
  // --UGGridHierarchicIterator
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
  class UGGridHierarchicIterator :
    public HierarchicIteratorDefault <dim,dimworld, UGCtype,
        UGGridHierarchicIterator,UGGridEntity>
  {
  public:
    //! know your own dimension
    //enum { dimension=dim };

    //! know your own dimension of world
    //enum { dimensionworld=dimworld };

#if 1

#if 0
    //! the normal Constructor
    UGGridHierarchicIterator(UGGrid<dim,dimworld> &grid,
                             ALBERT TRAVERSE_STACK *travStack, int actLevel, int maxLevel);
#endif

    //! the default Constructor
    UGGridHierarchicIterator(UGGrid<dim,dimworld> &grid,
                             int actLevel,int maxLevel);
#else
    //! the normal Constructor
    UGGridHierarchicIterator(UGGrid<dim,dimworld> &grid,
                             ALBERT TRAVERSE_STACK *travStack, int travLevel);

    //! the default Constructor
    UGGridHierarchicIterator(UGGrid<dim,dimworld> &grid);
#endif

    //! prefix increment
    UGGridHierarchicIterator& operator ++();

    //! postfix increment
    UGGridHierarchicIterator& operator ++(int i);

    //! equality
    bool operator== (const UGGridHierarchicIterator& i) const;

    //! inequality
    bool operator!= (const UGGridHierarchicIterator& i) const;

    //! dereferencing
    UGGridEntity<0,dim,dimworld>& operator*();

    //! arrow
    UGGridEntity<0,dim,dimworld>* operator->();

  private:
    //! implement with virtual element
    UGGridEntity<0,dim,dimworld> virtualEntity_;

    //! know the grid were im comming from
    UGGrid<dim,dimworld> &grid_;

    //! the actual Level of this Hierarichic Iterator
    int level_;

    //! max level to go down
    int maxlevel_;

#if 0
    //! we need this for Albert traversal, and we need ManageTravStack, which
    //! does count References when copied
    ALBERT ManageTravStack manageStack_;

    //! The nessesary things for Albert
    ALBERT EL_INFO * recursiveTraverse(ALBERT TRAVERSE_STACK * stack);
#endif

    //! make empty HierarchicIterator
    void makeIterator();
  };


#define NEIGH_DEBUG

  /** \todo Please doc me! */
  template<int dim, int dimworld>
  class UGGridBoundaryEntity
    : public BoundaryEntityDefault <dim,dimworld, UGCtype,
          UGGridElement,UGGridBoundaryEntity>
  {
    friend class UGGridNeighborIterator<dim,dimworld>;
  public:
    UGGridBoundaryEntity () : _geom (false) , _elInfo ( NULL ),
                              _neigh (-1) {};

    //! return type of boundary , i.e. Neumann, Dirichlet ...
    BoundaryType type ()
    {
#ifdef NEIGH_DEBUG
      if(_elInfo->boundary[_neigh] == NULL)
      {
        std::cerr << "No Boundary, fella! \n";
        abort();
      }
#endif
      return (( _elInfo->boundary[_neigh]->bound < 0 ) ? Neumann : Dirichlet );
    }

    //! return identifier of boundary segment, number
    int id ()
    {
#ifdef NEIGH_DEBUG
      if(_elInfo->boundary[_neigh] == NULL)
      {
        std::cerr << "No Boundary, fella! \n";
        abort();
      }
#endif
      return _elInfo->boundary[_neigh]->bound;
    }

    //! return true if geometry of ghost cells was filled
    bool hasGeometry () { return _geom.builtGeom(_elInfo,0,0,0); }

    //! return geometry of the ghost cell
    UGGridElement<dim,dimworld> geometry ()
    {
      return _geom;
    }

  private:
#if 0
    // set elInfo
    void setElInfo(ALBERT EL_INFO * elInfo, int nb)
    {
      _neigh = nb;
      if(elInfo)
        _elInfo = elInfo;
      else
        _elInfo = NULL;
    };
#endif

    int _neigh;

    // UGGrid<dim,dimworld> & _grid;
    UGGridElement<dim,dimworld> _geom;

#if 0
    // cooresponding el_info
    ALBERT EL_INFO * _elInfo;
#endif
  };
  //**********************************************************************
  //
  // --UGGridNeighborIterator
  // --NeighborIterator
  /*!
     Mesh entities of codimension 0 ("elements") allow to visit all neighbors, wh
     a neighbor is an entity of codimension 0 which has a common entity of codimens
     These neighbors are accessed via a NeighborIterator. This allows the implement
     non-matching meshes. The number of neigbors may be different from the number o
     of an element!
   */
  template<int dim, int dimworld>
  class UGGridNeighborIterator :
    public NeighborIteratorDefault <dim,dimworld, UGCtype,
        UGGridNeighborIterator,UGGridEntity,
        UGGridElement, UGGridBoundaryEntity>
  {
  public:
    //! know your own dimension
    //enum { dimension=dim };

    //! know your own dimension of world
    //enum { dimensionworld=dimworld };

    //! prefix increment
    UGGridNeighborIterator& operator ++();

    //! postfix increment
    UGGridNeighborIterator& operator ++(int i);

    //! The default Constructor
    UGGridNeighborIterator(UGGrid<dim,dimworld> &grid,int level);

#if 0
    //! The Constructor
    UGGridNeighborIterator(UGGrid<dim,dimworld> &grid,int level,
                           ALBERT EL_INFO *elInfo);
#endif

    //! The Destructor
    ~UGGridNeighborIterator();

    //! equality
    bool operator== (const UGGridNeighborIterator& i) const;

    //! inequality
    bool operator!= (const UGGridNeighborIterator& i) const;

    //! access neighbor, dereferencing
    UGGridEntity<0,dim,dimworld>& operator*();

    //! access neighbor, arrow
    UGGridEntity<0,dim,dimworld>* operator->();

    //! return true if intersection is with boundary. \todo connection with
    //! boundary information, processor/outer boundary
    bool boundary ();

    //! return true if across the edge an neighbor on this level exists
    bool neighbor ();

    //! return information about the Boundary
    UGGridBoundaryEntity<dim,dimworld> & boundaryEntity ();

    //! return unit outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    Vec<dimworld,UGCtype>& unit_outer_normal (Vec<dim-1,UGCtype>& local);

    //! return unit outer normal, if you know it is constant use this function instead
    Vec<dimworld,UGCtype>& unit_outer_normal ();

    //! intersection of codimension 1 of this neighbor with element where
    //! iteration started.
    //! Here returned element is in LOCAL coordinates of the element
    //! where iteration started.
    UGGridElement<dim-1,dim>& intersection_self_local ();

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in GLOBAL coordinates of the element where iteration started.
    UGGridElement<dim-1,dimworld>& intersection_self_global ();

    //! local number of codim 1 entity in self where intersection is contained in
    int number_in_self ();

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    UGGridElement<dim-1,dim>& intersection_neighbor_local ();

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    UGGridElement<dim-1,dimworld>& intersection_neighbor_global ();

    //! local number of codim 1 entity in neighbor where intersection is contained
    int number_in_neighbor ();

    //! return outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    Vec<dimworld,UGCtype>& outer_normal (Vec<dim-1,UGCtype>& local);

    //! return unit outer normal, if you know it is constant use this function instead
    Vec<dimworld,UGCtype>& outer_normal ();

  private:
    //**********************************************************
    //  private methods
    //**********************************************************

    //! setup the virtual neighbor
    void setupVirtEn ();

    //! know the grid were im comming from
    UGGrid<dim,dimworld> &grid_;

    //! the actual level
    int level_;

    //! implement with virtual element
    //! Most of the information can be generated from the ALBERT EL_INFO
    //! therefore this element is only created on demand.
    bool builtNeigh_;
    UGGridEntity<0,dim,dimworld> *virtualEntity_;

    // for memory management
    typename UGGrid<dim,dimworld>::EntityProvider::ObjectEntity *manageObj_;
    typename UGGrid<dim,dimworld>::IntersectionSelfProvider::ObjectEntity  *manageInterEl_;
    typename UGGrid<dim,dimworld>::IntersectionNeighProvider::ObjectEntity *manageNeighEl_;

    //! defined in agmemory.hh
    typename ElInfoProvider::ObjectEntity *manageNeighInfo_;

    //! vector storing the outer normal
    //Vec<dimworld,albertCtype> outerNormal_;

    //! pointer to element holding the self_local and self_global information.
    //! This element is created on demand.
    UGGridElement<dim-1,dim> *fakeNeigh_;

    //! pointer to element holding the neighbor_global and neighbor_local
    //! information. This element is created on demand.
    UGGridElement<dim-1,dimworld> *neighGlob_;

    //! BoundaryEntity
    UGGridBoundaryEntity<dim,dimworld> *boundaryEntity_;

#if 0
    //! pointer to the EL_INFO struct storing the real element information
    ALBERT EL_INFO * elInfo_;

    //! EL_INFO th store the information of the neighbor if needed
    ALBERT EL_INFO * neighElInfo_;
#endif

    Vec<dimworld,UGCtype> outNormal_;

    //! count on which neighbor we are lookin' at
    int neighborCount_;
  };

#include "uggrid/uggridleveliterator.hh"

  //**********************************************************************
  //
  // --UGGrid
  //
  //**********************************************************************

  /** \brief The UG %Grid class
   *
   * \todo Please doc me!
   */
  template <int dim, int dimworld>
  class UGGrid : public GridDefault  < dim, dimworld,
                     UGCtype, UGGrid,
                     UGGridLevelIterator,UGGridEntity>
  {

    friend class UGGridEntity <0,dim,dimworld>;
    //friend class UGGridEntity <1,dim,dimworld>;
    //friend class UGGridEntity <1 << dim-1 ,dim,dimworld>;
    friend class UGGridEntity <dim,dim,dimworld>;

    friend class UGMarkerVector;

    // friends because of fillElInfo
    friend class UGGridLevelIterator<0,dim,dimworld>;
    friend class UGGridLevelIterator<1,dim,dimworld>;
    friend class UGGridLevelIterator<2,dim,dimworld>;
    friend class UGGridLevelIterator<3,dim,dimworld>;
    friend class UGGridHierarchicIterator<dim,dimworld>;

    friend class UGGridNeighborIterator<dim,dimworld>;


    //! UGGrid is only implemented for 2 and 3 dimension
    //! for 1d use SGrid or SimpleGrid
    CompileTimeChecker< (dimworld==dim) && ((dim==2) || (dim==3)) >   Use_UGGrid_only_for_2d_and_3d;

    //**********************************************************
    // The Interface Methods
    //**********************************************************
  public:

    typedef UGGridReferenceElement<dim> ReferenceElement;

    /** \todo Please doc me! */
    enum { numCodim = dim+1 };

    //! empty Constructor
    UGGrid();

    //! Desctructor
    ~UGGrid();

    //! Return maximum level defined in this grid. Levels are numbered
    //! 0 ... maxlevel with 0 the coarsest level.
    int maxlevel() const;

    //! Iterator to first entity of given codim on level
    template<int codim>
    UGGridLevelIterator<codim,dim,dimworld> lbegin (int level) const;

    //! one past the end on this level
    template<int codim>
    UGGridLevelIterator<codim,dim,dimworld> lend (int level) const;


    /** \brief Number of grid entities per level and codim
     */
    int size (int level, int codim) const;
#if 0
    //! refine all positive marked leaf entities
    //! return true if the grid was changed
    bool refine  ( );

    //! coarsen all negative marked leaf entities
    //! return true if the grid was changed
    bool coarsen ( );

    /** \brief Please doc me! */
    GridIdentifier type () { return UGGrid_Id; };

    //**********************************************************
    // End of Interface Methods
    //**********************************************************

    //! uses the interface, mark on entity and refineLocal
    bool globalRefine(int refCount);

    // write Grid to file
    //void writeGrid(int level=-1);

    /** \brief write Grid to file in specified FileFormatType
     */
    template <FileFormatType ftype>
    bool writeGrid( const char * filename, UGCtype time );

    /** \brief read Grid from file filename and store time of mesh in time
     */
    template <FileFormatType ftype>
    bool readGrid( const char * filename, UGCtype & time );
#endif
    UGCtype getTime () const { return time_; };

    //private:
  public:
    // make the calculation of indexOnLevel and so on.
    // extra method because of Reihenfolge
    void calcExtras();

#if 0
    //! access to mesh pointer, needed by some methods
    ALBERT MESH* getMesh () const { return mesh_; };
#endif

    // pointer to a UG multigrid, which contains the data
    UG3d::multigrid *mesh_;

    // number of maxlevel of the mesh
    int maxlevel_;

    // true if grid was refined
    bool wasChanged_;

    // number of entitys of each level an codim
    Array<int> size_;

#if 0
    // remember on which level an ALBERT EL lives, is needed for the new
    // fillElInfo method that takes the level of an element into account
    // for calculation of the neighbor realtions
    std::vector<int> neighOnLevel_;

    // this method is new fill_elinfo from ALBERT but here the neighbor
    // relations are calced diffrent, on ervery level there are neighbor
    // realtions ( in ALBERT only on leaf level ), so we needed a new
    // fill_elinfo.
    void fillElInfo(int ichild, int actLevel ,const ALBERT EL_INFO *elinfo_old,
                    ALBERT EL_INFO *elinfo, bool hierachical ) const;

    // needed for VertexIterator, mark on which element a vertex is treated
    UGMarkerVector * vertexMarker_;

    //*********************************************************
    // Methods for mapping the global Index to local on Level
    // contains the index on level for each unique el->index of UG
    Array<int> levelIndex_[dim+1];
    void makeNewSize(Array<int> &a, int newNumberOfEntries);
    void markNew();
    //**********************************************************

    //! map the global index from the UG Mesh to the local index on Level
    template <int codim>
    int indexOnLevel(int globalIndex, int level ) ;

    // pointer to the real number of elements or vertices
    // i.e. points to mesh->n_hier_elements or mesh->n_vertices
    typename std::vector<int *> numberOfEntitys_;

    //! actual time of Grid
    UGCtype time_;

    //***********************************************************************
    //  MemoryManagement for Entitys and Elements
    //**********************************************************************
    typedef MemoryProvider< UGGridEntity<0,dim,dimworld > > EntityProvider;
    typedef MemoryProvider< UGGridElement<dim-1,dimworld> > IntersectionSelfProvider;
    typedef MemoryProvider< UGGridElement<dim-1,dim> >      IntersectionNeighProvider;

    EntityProvider entityProvider_;
    IntersectionSelfProvider interSelfProvider_;
    IntersectionNeighProvider interNeighProvider_;


#endif
  }; // end Class UGGrid

  // Class to mark the Vertices on the leaf level
  // to visit every vertex only once
  // for the LevelIterator codim == dim
  class UGMarkerVector
  {
    friend class UGGrid<2,2>;
    friend class UGGrid<3,3>;
  public:
    UGMarkerVector () {} ;

#if 0
    bool notOnThisElement(ALBERT EL * el, int level , int vertex);
#endif

    template <class GridType>
    void markNewVertices(GridType &grid);

    void print();

  private:
    // built in array to mark on which element a vertex is reached
    Array<int> vec_;
    // number of vertices
    int numVertex_;
  };

  /** @} end documentation group */


}; // namespace Dune

#include "uggrid/uggrid.cc"

#endif
