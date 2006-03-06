// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTAGRID_IMP_HH
#define DUNE_ALBERTAGRID_IMP_HH
#include <iostream>
#include <fstream>

#include <vector>
#include <assert.h>
#include <algorithm>

#if defined GRIDDIM && defined GRIDDIMWORLD
  #define DIM GRIDDIM
  #define DIM_OF_WORLD GRIDDIMWORLD
#else
  #ifndef DUNE_PROBLEM_DIM
    #error "DUNE_PROBLEM_DIM needed to compile AlbertaGrid! \n"
  #endif

  #ifndef DUNE_WORLD_DIM
    #error "DUNE_WORLD_DIM needed to compile AlbertaGrid! \n"
  #endif

  #define DIM DUNE_PROBLEM_DIM
  #define DIM_OF_WORLD DUNE_WORLD_DIM
#endif

// Dune includes
#include <dune/common/misc.hh>
#include <dune/common/interfaces.hh>
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/array.hh>
#include <dune/common/capabilities.hh>
#include <dune/common/stdstreams.hh>
#include <dune/common/collectivecommunication.hh>
#include <dune/common/exceptions.hh>

#include <dune/grid/common/grid.hh>
#include <dune/grid/common/sizecache.hh>
#include <dune/grid/common/intersectioniteratorwrapper.hh>

// stack for index management
#include <dune/grid/common/indexstack.hh>

//- Local includes
// some cpp defines and include of alberta.h
#include "albertaheader.hh"

// grape data io
#include <dune/io/file/grapedataio.hh>

// IndexManager defined in indexstack.hh
// 10000 is the size of the finite stack used by IndexStack
typedef Dune::IndexStack<int,10000> IndexManagerType;

// some extra functions for handling the Albert Mesh
#include "albertaextra.hh"

namespace Dune {
  // own exception classes
  class AlbertaError   : public Exception {};
  class AlbertaIOError : public IOError {};
}

// contains a simple memory management for some componds of this grid
#include "agmemory.hh"

#include "referencetopo.hh"
#include "indexsets.hh"

// contains the communication for parallel computing for this grid
#include "agcommunicator.hh"

namespace Dune
{

  // i.e. double or float
  typedef ALBERTA REAL albertCtype;

  template<int codim, int dim, class GridImp> class AlbertaGridEntity;
  template<int codim, int dim, class GridImp> class AlbertaGridMakeableEntity;
  template<int codim, PartitionIteratorType pitype, class GridImp> class AlbertaGridTreeIterator;
  template<int codim, PartitionIteratorType pitype, class GridImp> class AlbertaGridLeafIterator;
  template<int cd, class GridImp> class AlbertaGridEntityPointer;

  template <int mydim, int cdim, class GridImp> class AlbertaGridGeometry;
  template<class GridImp>         class AlbertaGridHierarchicIterator;
  template<class GridImp>         class AlbertaGridIntersectionIterator;
  template<int dim, int dimworld> class AlbertaGrid;
  template<int dim, int dimworld> class AlbertaGridHierarchicIndexSet;

  template <int codim, int dim, class GridImp>
  struct SelectEntityImp
  {
    typedef AlbertaGridEntity<codim,dim,GridImp> EntityImp;
    typedef Dune::Entity<codim, dim, const GridImp, AlbertaGridEntity> Entity;
    typedef MakeableInterfaceObject<Entity> EntityObject;
  };

  //! Class to mark the Vertices on the leaf level
  //! to visit every vertex only once
  //! for the LevelIterator codim == dim
  class AlbertaMarkerVector
  {
    friend class AlbertaGrid<2,2>;
    friend class AlbertaGrid<2,3>;
    friend class AlbertaGrid<3,3>;

    enum { vxBufferSize_ = 10000 };
  public:
    //! create AlbertaMarkerVector for Level or Leaf Iterator, true == LevelIterator
    //! the vectors stored inside are empty first
    AlbertaMarkerVector (bool meLevel=true) : up2Date_(false), meLevel_(meLevel) {} ;

    //! return true if vertex is not watched on this element
    bool vertexNotOnElement(const int elIndex, const int vertex) const;

    //! return true if edge is not watched on this element
    bool edgeNotOnElement(const int elIndex, const int edge) const;

    //! return true if edge is not watched on this element
    bool faceNotOnElement(const int elIndex, const int face) const;

    //! mark vertices for LevelIterator and given level
    template <class GridType>
    void markNewVertices(GridType &grid, int level);

    //! mark vertices for LeafIterator , uses leaf level
    template <class GridType>
    void markNewLeafVertices(GridType &grid);

    //! return true if marking is up to date
    bool up2Date () const { return up2Date_; }

    //! unset up2date flag
    void unsetUp2Date () { up2Date_ = false; }

    //! print for debugin' only
    void print() const;

  private:
    // built in array to mark on which element a vertex is reached
    Array<int> vec_;
    Array<int> edgevec_;
    Array<int> facevec_;

    // number of vertices
    int numVertex_;

    // true is vertex marker is up to date
    bool up2Date_;
    bool meLevel_;
  };

  //**********************************************************************
  //
  // --AlbertaGridGeometry
  /*!
     Defines the geometry part of a mesh entity. Works for all dimensions, element types and dime
     of world. Provides reference element and mapping between local and global coordinates.
     The element may have different implementations because the mapping can be
     done more efficient for structured meshes than for unstructured meshes.

     dim: An element is a polygonal in a hyperplane of dimension dim. 0 <= dim <= 3 is typically
     dim=0 is a point.

     dimworld: Each corner is a point with dimworld coordinates.
   */

  //******************************************************
  //
  //  --Geometry
  //
  //******************************************************
  template <int mydim, int cdim, class GridImp>
  class AlbertaGridGeometry :
    public GeometryDefaultImplementation<mydim,cdim,GridImp,AlbertaGridGeometry>
  {

    typedef AlbertaGridGeometry<mydim,cdim,GridImp> ThisType;
    //! know dimension of barycentric coordinates
    enum { dimbary=mydim+1};
  public:
    //! Default constructor
    AlbertaGridGeometry();

    //! constructor building geometry in father
    AlbertaGridGeometry(const int child, const int orientation );

    //! return the element type identifier
    //! line , triangle or tetrahedron, depends on dim
    const GeometryType & type () const;

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const;

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<albertCtype, cdim> & operator[] (int i) const;

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    FieldVector<albertCtype, cdim> global (const FieldVector<albertCtype, mydim>& local) const;

    //! maps a global coordinate within the element to a
    //! local coordinate in its reference element
    FieldVector<albertCtype, mydim> local (const FieldVector<albertCtype, cdim>& global) const;

    //! returns true if the point in local coordinates is inside reference element
    bool checkInside(const FieldVector<albertCtype, mydim>& local) const;

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
    albertCtype integrationElement (const FieldVector<albertCtype, mydim>& local) const;

    //! can only be called for dim=dimworld!
    //! Note that if both methods are called on the same element, then
    //! call jacobianInverseTransposed first because integration element is calculated
    //! during calculation of the transposed of the jacobianInverse
    const FieldMatrix<albertCtype,mydim,mydim>& jacobianInverseTransposed (const FieldVector<albertCtype, mydim>& local) const;

    //***********************************************************************
    //!  Methods that not belong to the Interface, but have to be public
    //***********************************************************************
    //! generate the geometry for the ALBERTA EL_INFO
    //! no interface method
    bool builtGeom(ALBERTA EL_INFO *elInfo, int face, int edge, int vertex);

    //! build geometry for intersectionSelfLocal and
    //! intersectionNeighborLocal
    template <class GeometryType, class LocalGeomType >
    bool builtLocalGeom(const GeometryType & geo , const LocalGeomType & lg,
                        ALBERTA EL_INFO *elInfo, int face);

    // init geometry with zeros
    //! no interface method
    void initGeom();
    FieldVector<albertCtype, cdim>& getCoordVec (int i);

    //! print internal data
    //! no interface method
    void print (std::ostream& ss) const;

  private:
    // build geometry with local coords of child in reference element
    void buildGeomInFather(const int child, const int orientation);

    // calculate Matrix for Mapping from reference element to actual element
    void calcElMatrix () const;

    //! build the transposed of the jacobian inverse and store the volume
    void buildJacobianInverseTransposed () const;

    // template method for map the vertices of EL_INFO to the actual
    // coords with face_,edge_ and vertex_ , needes for operator []
    int mapVertices (int i) const;

    // calculates the volume of the element
    albertCtype elDeterminant () const;

    // temporary need vector
    mutable FieldVector<albertCtype, mydim+1> tmpVec_;

    //! the vertex coordinates
    mutable FieldMatrix<albertCtype,mydim+1,cdim> coord_;

    //! storage for global coords
    mutable FieldVector<albertCtype, cdim> globalCoord_;

    //! storage for local coords
    mutable FieldVector<albertCtype, mydim> localCoord_;

    // make empty EL_INFO
    ALBERTA EL_INFO * makeEmptyElInfo();

    ALBERTA EL_INFO * elInfo_;

    //! Which Face of the Geometry 0...dim+1
    int face_;

    //! Which Edge of the Face of the Geometry 0...dim
    int edge_;

    //! Which Edge of the Face of the Geometry 0...dim-1
    int vertex_;

    enum { matdim = (mydim > 0) ? mydim : 1 };
    mutable FieldMatrix<albertCtype,matdim,matdim> Jinv_; //!< storage for inverse of jacobian
    mutable FieldMatrix<albertCtype,matdim,matdim> Mtmp_;    //!< storage for inverse of jacobian

    mutable FieldMatrix<albertCtype,cdim,mydim> elMat_; //!< storage for mapping matrix
    mutable FieldMatrix<albertCtype,matdim,matdim> elMatT_elMat_; //!< storage for mapping matrix

    //! is true if elMat_ was calced
    mutable bool builtElMat_;
    //! is true if Jinv_ and volume_ is calced
    mutable bool builtinverse_;


    mutable bool calcedDet_; //! true if determinant was calculated
    mutable albertCtype elDet_; //!< storage of element determinant

    // temporary mem for integrationElement with mydim < cdim
    mutable FieldVector<albertCtype,cdim> tmpV_;
    mutable FieldVector<albertCtype,cdim> tmpU_;
    mutable FieldVector<albertCtype,cdim> tmpZ_;

    mutable FieldVector<albertCtype,mydim> AT_x_;
    const GeometryType myGeomType_;
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
  template<int cd, int dim, class GridImp>
  class AlbertaGridEntity :
    public EntityDefaultImplementation <cd,dim,GridImp,AlbertaGridEntity>
  {
    enum { dimworld = GridImp::dimensionworld };
    friend class AlbertaGrid < dim , dimworld >;
    friend class AlbertaGridEntity < 0, dim, GridImp>;
    friend class AlbertaGridTreeIterator < cd, All_Partition,GridImp>;
    friend class AlbertaGridMakeableEntity<cd,dim,GridImp>;
    friend class AlbertaGridEntityPointer<cd,GridImp>;

    typedef AlbertaGridGeometry<dim-cd,dimworld,GridImp> GeometryImp;
  public:
    template <int cc>
    struct Codim
    {
      typedef typename GridImp::template Codim<cc>::EntityPointer EntityPointer;
    };

    typedef typename GridImp::template Codim<cd>::Entity Entity;
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;
    typedef typename GridImp::template Codim<cd>::Geometry Geometry;
    typedef typename GridImp::template Codim<cd>::LevelIterator LevelIterator;

    //! level of this element
    int level () const;

    //! index of the boundary which is associated with the entity, 0 for inner entities
    int boundaryId () const ;

    //! contructor takeing traverse stack
    AlbertaGridEntity(const GridImp &grid, int level,
                      ALBERTA TRAVERSE_STACK * travStack);

    //! cosntructor
    AlbertaGridEntity(const GridImp &grid, int level, bool);

    //! copy constructor
    AlbertaGridEntity(const AlbertaGridEntity & org);

    //! return partition type of this entity ( see grid.hh )
    PartitionType partitionType() const;

    //! geometry of this entity
    const Geometry & geometry () const;

    EntityPointer ownersFather () const;

    //! my position in local coordinates of the owners father
    const FieldVector<albertCtype, dim>& positionInOwnersFather () const;

    //***********************************************
    //  End of Interface methods
    //***********************************************
    //! needed for the LevelIterator and LeafIterator
    ALBERTA EL_INFO *getElInfo () const;
    //! return element for equaltiy in EntityPointer
    ALBERTA EL *getElement () const;

    //! set elInfo and Element and builtgeometry to zero
    void removeElInfo();

    //! return the current face/edge or vertex number
    //! no interface method
    int getFEVnum () const;

    //! equality of entities
    bool equals ( const AlbertaGridEntity<cd,dim,GridImp> & i) const;

    // dummy function, only needed for codim 0
    bool leafIt () const { return false; }

    // methods for setting the infos from the albert mesh
    void setTraverseStack (ALBERTA TRAVERSE_STACK *travStack);
    void setElInfo (ALBERTA EL_INFO *elInfo, int face,
                    int edge, int vertex );

    // same as setElInfo just with a entity given
    void setEntity (const AlbertaGridEntity<cd,dim,GridImp> & org);

    // set level of entity
    void setLevel ( int newLevel );
    void setNewLevel ( int newLevel , bool ) { setLevel(level); }

  private:
    // the grid this entity belong to
    const GridImp &grid_;

    // Alberta element info
    ALBERTA EL_INFO *elInfo_;

    // Alberta element
    ALBERTA EL * element_;

    // current traverse stack this entity belongs too
    ALBERTA TRAVERSE_STACK * travStack_;

    //! level
    int level_;

    // type of createable object, just derived from Geometry class
    typedef MakeableInterfaceObject<Geometry> GeometryObject;

    //! the current geometry
    GeometryObject geo_;
    //! the reference to the real imp object inside of GeometryObject
    mutable GeometryImp & geoImp_;

    //! true if geometry has been constructed
    mutable bool builtgeometry_;

    //! local coord within father
    mutable FieldVector<albertCtype, dim> localFatherCoords_;
    mutable bool localFCoordCalced_;

    //! Which Face of the Geometry
    int face_;

    //! Which Edge of the Face of the Geometry
    int edge_;

    //! Which Vertex of the Edge of the Face of the Geometry
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
  template<int dim, class GridImp>
  class AlbertaGridEntity<0,dim,GridImp> :
    public EntityDefaultImplementation <0,dim,GridImp,AlbertaGridEntity>
  {
    enum { dimworld = GridImp::dimensionworld };
    friend class AlbertaGrid < dim , GridImp::dimensionworld >;
    friend class AlbertaMarkerVector;
    friend class AlbertaGridIntersectionIterator <GridImp>;
    friend class AlbertaGridHierarchicIterator <GridImp>;
    friend class AlbertaGridTreeIterator <0,All_Partition,GridImp>;
    friend class AlbertaGridMakeableEntity<0,dim,GridImp>;
    friend class AlbertaGridEntityPointer<0,GridImp>;
  public:
    template <int cd>
    struct Codim
    {
      typedef typename GridImp::template Codim<cd>::EntityPointer EntityPointer;
    };

    typedef typename GridImp::template Codim<0>::Entity Entity;
    typedef typename GridImp::template Codim<0>::Geometry Geometry;
    typedef AlbertaGridGeometry<dim,dimworld,GridImp> GeometryImp;

    typedef typename GridImp::template Codim<0>::LevelIterator LevelIterator;
    typedef typename GridImp::template Codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename GridImp::template Codim<0>::HierarchicIterator HierarchicIterator;
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;

    typedef IntersectionIteratorWrapper<GridImp> AlbertaGridIntersectionIteratorType;

    enum { dimension = dim };

    //! Destructor, needed perhaps needed for deleteing faceEntity_ and
    //! edgeEntity_ , see below
    //! there are only implementations for dim==dimworld 2,3
    ~AlbertaGridEntity() {};

    //! Constructor, real information is set via setElInfo method
    AlbertaGridEntity(const GridImp &grid, int level, bool leafIt );

    AlbertaGridEntity(const AlbertaGridEntity & org);

    //! level of this element
    int level () const;

    //! index of the boundary which is associated with the entity, 0 for inner entities
    int boundaryId () const;

    //! geometry of this entity
    const Geometry & geometry () const;

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
       with codimension cc.
     */
    //!< Default codim 1 Faces and codim == dim Vertices
    template<int cc> int count () const;

    //! Provide access to mesh entity i of given codimension. Entities
    //!  are numbered 0 ... count<cc>()-1
    template<int cc> typename Codim<cc>::EntityPointer entity (int i) const;

    //! Provide access to mesh entity i of given codimension. Entities
    //!  are numbered 0 ... count<cc>()-1
    //template<int cc> void entity (AlbertaGridTreeIterator<cc,dim,dimworld>& it, int i);

    /*! Intra-level access to intersection with neighboring elements.
       A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor. */
    AlbertaGridIntersectionIteratorType ibegin () const;

    //! Reference to one past the last intersection with neighbor
    AlbertaGridIntersectionIteratorType iend () const;

    //! returns true if entity is leaf entity, i.e. has no children
    bool isLeaf () const ;

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
       NOTE: the imeplementation at the moment is very inefficient.
     */
    const Geometry & geometryInFather () const;

    /*! Inter-level access to son elements on higher levels<=maxlevel.
       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
     */
    AlbertaGridHierarchicIterator<GridImp> hbegin (int maxlevel) const;

    //! Returns iterator to one past the last son
    AlbertaGridHierarchicIterator<GridImp> hend (int maxlevel) const;

    //! return whether entity could be cosrsend (COARSEND) or was refined
    //! (REFIEND) or nothing happend (NONE)
    AdaptationState state () const;

    //***************************************************************
    //  Interface for parallelisation
    //***************************************************************
    //! \todo brief me
    void setLeafData( int proc );

    //! return partition type of this entity ( see grid.hh )
    PartitionType partitionType() const;

    //! return true if this entity belong to master set of this grid
    bool master() const;

    //! return 0 for elements
    int getFEVnum () const { return 0; }

    //! needed for LevelIterator to compare
    ALBERTA EL_INFO *getElInfo () const;

    //! return element for equaltiy in EntityPointer
    ALBERTA EL *getElement () const;

    //! set elInfo and Element to nil
    void removeElInfo();

    //! equality of entities
    bool equals ( const AlbertaGridEntity<0,dim,GridImp> & i) const;

    // returns true if entity comes from LeafIterator
    bool leafIt () const { return leafIt_; }

    // called from HierarchicIterator, because only this
    // class changes the level of the entity, otherwise level is set by
    // Constructor
    void setLevel ( int actLevel );
    void setNewLevel( int level, bool leafIt );

    // face, edge and vertex only for codim > 0, in this
    // case just to supply the same interface
    void setTraverseStack (ALBERTA TRAVERSE_STACK *travStack);
    void setElInfo (ALBERTA EL_INFO *elInfo,
                    int face = 0,
                    int edge = 0,
                    int vertex = 0 );

    //! same as setElInfo just with a entity given
    void setEntity (const AlbertaGridEntity<0,dim,GridImp> & org);

  private:
    //! return which number of child we are, i.e. 0 or 1
    int nChild () const;

    //! make a new AlbertaGridEntity
    void makeDescription();

    //! the corresponding grid
    const GridImp & grid_;

    //! the level of the entity
    int level_;

    //! pointer to the Albert TRAVERSE_STACK data
    ALBERTA TRAVERSE_STACK * travStack_;

    //! pointer to the real Albert element data
    ALBERTA EL_INFO *elInfo_;

    //! pointer to the real Albert element
    ALBERTA EL *element_;

    // local coordinates within father
    typedef MakeableInterfaceObject<Geometry> GeometryObject;

    //! the cuurent geometry
    mutable GeometryObject geoObj_;
    mutable GeometryImp & geo_;
    mutable bool builtgeometry_;  //!< true if geometry has been constructed

    // is true if entity comes from leaf iterator
    bool leafIt_;

  }; // end of AlbertaGridEntity codim = 0


  //**********************************************************************
  //
  // --AlbertaGridEntityPointer
  // --EntityPointer
  // --EnPointer
  /*!
     Enables iteration over all entities of a given codimension and level of a grid.
   */
  template<int cd, class GridImp>
  class AlbertaGridEntityPointer :
    public EntityPointerDefaultImplementation <cd, GridImp, AlbertaGridEntityPointer<cd,GridImp> >
  {
    enum { dim       = GridImp::dimension };
    enum { dimworld  = GridImp::dimensionworld };

    friend class AlbertaGridEntity<cd,dim,GridImp>;
    friend class AlbertaGridEntity< 0,dim,GridImp>;
    friend class AlbertaGrid < dim , dimworld >;

  public:
    typedef typename GridImp::template Codim<cd>::Entity Entity;
    typedef typename SelectEntityImp<cd,dim,GridImp>::EntityImp EntityImp;
    typedef typename SelectEntityImp<cd,dim,GridImp>::EntityObject EntityObject;

    //! typedef of my type
    typedef AlbertaGridEntityPointer<cd,GridImp> AlbertaGridEntityPointerType;

    //! Constructor for EntityPointer that points to an element
    AlbertaGridEntityPointer(const GridImp & grid,
                             int level,  ALBERTA EL_INFO *elInfo,int face,int edge,int vertex);

    //! Constructor for EntityPointer that points to an element
    AlbertaGridEntityPointer(const GridImp & grid, ALBERTA TRAVERSE_STACK * stack,
                             int level,  ALBERTA EL_INFO *elInfo,int face,int edge,int vertex);

    //! Constructor for EntityPointer init of Level- and LeafIterator
    AlbertaGridEntityPointer(const GridImp & grid, int level , bool isLeaf, bool done);

    //! make empty entity pointer (to be revised)
    AlbertaGridEntityPointer(const AlbertaGridEntityPointerType & org);

    //! make empty entity pointer (to be revised)
    AlbertaGridEntityPointer(const GridImp & , const EntityImp & en);

    //! assignment operator
    AlbertaGridEntityPointer& operator= (const AlbertaGridEntityPointer& org);

    //! Destructor
    ~AlbertaGridEntityPointer();

    //! equality
    bool equals (const AlbertaGridEntityPointerType& i) const;

    //! dereferencing
    Entity & dereference () const ;

    //! ask for level of entities
    int level () const ;

    //! has to be called when iterator is finished
    void done ();

  protected:
    //! returns true if entity comes from LeafIterator
    bool leafIt () const { return isLeaf_; }

    //! return reference to internal entity imp
    EntityImp & entityImp ();
    //! return const reference to internal entity imp
    const EntityImp & entityImp () const;

    // reference to grid
    const GridImp & grid_;

    //! flag for leaf iterators
    bool isLeaf_;

    // entity that this EntityPointer points to
    EntityObject * entity_;

    // pointer to internal realEntity of Entity Object
    EntityImp * entityImp_;
  };


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

  template<class GridImp>
  class AlbertaGridHierarchicIterator :
    public AlbertaGridEntityPointer<0,GridImp> ,
    public HierarchicIteratorDefaultImplementation <GridImp,AlbertaGridHierarchicIterator>
  {
  public:
    typedef typename GridImp::template Codim<0>::Entity Entity;
    typedef typename GridImp::ctype ctype;

    //typedef AlbertaGridMakeableEntity<0,GridImp::dimension,GridImp> EntityImp;
    typedef typename SelectEntityImp<0,GridImp::dimension,GridImp>::EntityImp EntityImp;

    //! the normal Constructor
    AlbertaGridHierarchicIterator(const GridImp &grid,
                                  ALBERTA TRAVERSE_STACK *travStack,
                                  int actLevel, int maxLevel, bool leafIt );

    //! the default Constructor
    AlbertaGridHierarchicIterator(const GridImp &grid,
                                  int actLevel,int maxLevel);

    //! the default Constructor
    AlbertaGridHierarchicIterator(const AlbertaGridHierarchicIterator<GridImp> &org);

    //! the default Constructor
    AlbertaGridHierarchicIterator<GridImp> & operator = (const AlbertaGridHierarchicIterator<GridImp> &org);

    //! increment
    void increment();

  private:
    //! the actual Level of this Hierarichic Iterator
    int level_;

    //! max level to go down
    int maxlevel_;

    //! reference to entity of entity pointer class
    EntityImp & virtualEntity_;

    //! we need this for Albert traversal, and we need ManageTravStack, which
    //! does count References when copied
    ALBERTA ManageTravStack manageStack_;

    //! true if iterator is end iterator
    bool end_;

    //! The nessesary things for Albert
    ALBERTA EL_INFO * recursiveTraverse(ALBERTA TRAVERSE_STACK * stack);

    //! make empty HierarchicIterator
    void makeIterator();
  };


  //**********************************************************************
  //
  // --AlbertaGridIntersectionIterator
  // --IntersectionIterator
  /*!
     Mesh entities of codimension 0 ("elements") allow to visit all neighbors, where
     a neighbor is an entity of codimension 0 which has a common entity of codimension 1
     These neighbors are accessed via a IntersectionIterator. This allows the implementation of
     non-matching meshes. The number of neigbors may be different from the number of faces
     of an element!
   */
  template<class GridImp>
  class AlbertaGridIntersectionIterator :
    //public AlbertaGridEntityPointer<0,GridImp> ,
    public IntersectionIteratorDefaultImplementation <GridImp,AlbertaGridIntersectionIterator>
  {
    enum { dim      = GridImp::dimension };
    enum { dimworld = GridImp::dimensionworld };

    friend class AlbertaGridEntity<0,dim,GridImp>;
  public:
    typedef typename GridImp::template Codim<0>::Entity Entity;
    typedef typename GridImp::template Codim<1>::Geometry Geometry;
    typedef typename GridImp::template Codim<1>::LocalGeometry LocalGeometry;

    typedef typename SelectEntityImp<0,dim,GridImp>::EntityImp EntityImp;

    //typedef AlbertaGridMakeableGeometry<dim-1,dimworld,GridImp> LocalGeometryImp;
    typedef AlbertaGridGeometry<dim-1,dimworld,GridImp> LocalGeometryImp;
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;

    //! know your own dimension
    enum { dimension=dim };
    //! know your own dimension of world
    enum { dimensionworld=dimworld };
    //! define type used for coordinates in grid module
    typedef typename GridImp::ctype ctype;

    //! equality
    bool equals (const AlbertaGridIntersectionIterator<GridImp> & i) const;

    //! increment
    void increment();

    //! equality
    bool operator==(const AlbertaGridIntersectionIterator<GridImp>& i) const;

    //! access neighbor
    EntityPointer outside() const;

    //! access element where IntersectionIterator started
    EntityPointer inside() const;

    //! The default Constructor
    AlbertaGridIntersectionIterator(const GridImp & grid,
                                    int level);

    //! The Constructor
    AlbertaGridIntersectionIterator(const GridImp & grid,
                                    int level,
                                    ALBERTA EL_INFO *elInfo,
                                    bool leafIt );
    //! The copy constructor
    AlbertaGridIntersectionIterator(const AlbertaGridIntersectionIterator<GridImp> & org);

    //! assignment operator, implemented because default does not the right thing
    void assign (const AlbertaGridIntersectionIterator<GridImp> & org);

    //! The Destructor
    //~AlbertaGridIntersectionIterator();

    //! return true if intersection is with boundary. \todo connection with
    //! boundary information, processor/outer boundary
    bool boundary () const;

    //! return true if across the edge an neighbor on this level exists
    bool neighbor () const;

    //! return information about the Boundary
    int boundaryId () const;

    //! intersection of codimension 1 of this neighbor with element where
    //! iteration started.
    //! Here returned element is in LOCAL coordinates of the element
    //! where iteration started.
    const LocalGeometry& intersectionSelfLocal () const;
    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of neighbor
     */
    const LocalGeometry& intersectionNeighborLocal () const;
    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in GLOBAL coordinates of the element where iteration started.
     */
    const Geometry& intersectionGlobal () const;

    //! local number of codim 1 entity in self where intersection is contained in
    int numberInSelf () const;
    //! local number of codim 1 entity in neighbor where intersection is contained in
    int numberInNeighbor () const;

    //! twist of the face seen from the inner element
    int twistInSelf() const;

    //! twist of the face seen from the outer element
    int twistInNeighbor() const;

    //! return unit outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    typedef FieldVector<albertCtype, GridImp::dimensionworld> NormalVecType;
    typedef FieldVector<albertCtype, GridImp::dimension-1> LocalCoordType;

    const NormalVecType & unitOuterNormal (const LocalCoordType & local) const;

    //! return outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    const NormalVecType & outerNormal (const LocalCoordType & local) const;

    //! return outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    const NormalVecType & integrationOuterNormal (const LocalCoordType & local) const;

    //! return level of inside entity
    int level () const;

    //! reset IntersectionIterator
    template <class EntityType>
    void first(const EntityType & en, int level );

    //**********************************************************
    //  private methods
    //**********************************************************
    // calls EntityPointer done and sets done_ to true
    void done ();

  private:
    // returns true if actual neighbor has same level
    bool neighborHasSameLevel () const;

    //! make Iterator set to begin of actual entitys intersection Iterator
    void makeBegin (const GridImp & grid,
                    int level,
                    ALBERTA EL_INFO * elInfo ) const;

    //! set Iterator to end of actual entitys intersection Iterator
    void makeEnd (const GridImp & grid,int level ) const;

    // put objects on stack
    void freeObjects () const;

    //! setup the virtual neighbor
    void setupVirtEn () const;

    //! calculate normal to current face
    void calcOuterNormal () const;

    // return whether the iterator was called from a LeafIterator entity or
    // LevelIterator entity
    bool leafIt () const { return leafIt_; }
    ////////////////////////////////////////////////
    // private member variables
    ////////////////////////////////////////////////

    //! know the grid were im coming from
    const GridImp& grid_;

    //! the actual level
    mutable int level_;

    //! count on which neighbor we are lookin' at
    mutable int neighborCount_;

    //! implement with virtual element
    //! Most of the information can be generated from the ALBERTA EL_INFO
    //! therefore this element is only created on demand.
    mutable bool builtNeigh_;

    bool leafIt_;

    //! pointer to the EL_INFO struct storing the real element information
    mutable ALBERTA EL_INFO * elInfo_;

    typedef MakeableInterfaceObject<LocalGeometry> LocalGeometryObject;

    // the objects holding the real implementations
    mutable LocalGeometryObject fakeNeighObj_;
    mutable LocalGeometryObject fakeSelfObj_;
    mutable LocalGeometryObject neighGlobObj_;

    //! pointer to element holding the intersectionNeighbourLocal information.
    //! This element is created on demand.
    mutable LocalGeometryImp & fakeNeigh_;
    //! pointer to element holding the intersectionSelfLocal information.
    //! This element is created on demand.
    mutable LocalGeometryImp & fakeSelf_;
    //! pointer to element holding the neighbor_global and neighbor_local
    //! information. This element is created on demand.
    mutable LocalGeometryImp & neighGlob_;

    //! EL_INFO th store the information of the neighbor if needed
    mutable ALBERTA EL_INFO neighElInfo_;

    mutable NormalVecType outNormal_;
    mutable NormalVecType unitNormal_;

    // tmp memory for normal calculation
    mutable FieldVector<albertCtype, dimworld> tmpU_;
    mutable FieldVector<albertCtype, dimworld> tmpV_;

    // twist seen from the neighbor
    mutable int twist_;

    //! is true when iterator finished
    bool done_;
  };





  //**********************************************************************
  //
  // --AlbertaGridTreeIterator
  // --LevelIterator
  // --TreeIterator
  //

  namespace AlbertaTreeIteratorHelp {
    template <class IteratorImp, int dim, int codim>
    struct GoNextEntity;
  }

  /*!
     Enables iteration over all entities of a given codimension and level of a grid.
   */
  template<int cd, PartitionIteratorType pitype, class GridImp>
  class AlbertaGridTreeIterator :
    public AlbertaGridEntityPointer<cd,GridImp>
  {
    enum { dim = GridImp::dimension };
    friend class AlbertaGridEntity<2,dim,GridImp>;
    friend class AlbertaGridEntity<1,dim,GridImp>;
    friend class AlbertaGridEntity<0,dim,GridImp>;
    friend class AlbertaGrid < dim , GridImp::dimensionworld >;


    typedef AlbertaGridTreeIterator<cd,pitype,GridImp>  AlbertaGridTreeIteratorType;
    typedef AlbertaGridTreeIteratorType ThisType;
    friend class AlbertaTreeIteratorHelp::GoNextEntity<ThisType,dim,cd>;
  public:

    typedef typename GridImp::template Codim<cd>::Entity Entity;
    typedef typename SelectEntityImp<cd,dim,GridImp>::EntityImp EntityImp;
    //typedef AlbertaGridMakeableEntity<cd,dim,GridImp> EntityImp;

    //! Constructor making end iterator
    AlbertaGridTreeIterator(const AlbertaGridTreeIterator<cd,pitype,GridImp> & org );

    //! Constructor making end iterator
    AlbertaGridTreeIterator<cd,pitype,GridImp> & operator = (const AlbertaGridTreeIterator<cd,pitype,GridImp> & org );

    //! Constructor making end iterator
    AlbertaGridTreeIterator(const GridImp & grid, int
                            travLevel, int proc, bool leafIt=false );

    //! Constructor making begin iterator
    AlbertaGridTreeIterator(const GridImp & grid,
                            const AlbertaMarkerVector * vec,
                            int travLevel,
                            int proc,
                            bool leafIt=false);

    //! increment
    void increment();
    //! equality

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

    //! level :)
    int level_;

    //! level :)
    int enLevel_;

    //! reference to entity of entity pointer class
    EntityImp & virtualEntity_;

    // contains ALBERTA traversal stack
    ALBERTA ManageTravStack manageStack_;

    //! which face, edge and vertex are we watching of an elInfo
    int face_;
    int edge_;
    int vertex_;

    // knows on which element a point,edge,face is viewed
    const AlbertaMarkerVector * vertexMarker_;

    // variable for operator++
    bool okReturn_;

    // store processor number of elements
    // for ghost walktrough, i.e. walk over ghosts which belong
    // tp processor 2
    const int proc_;
  };

  //! --LevelIterator
  //! the same as TreeIterator
  template<int cd, PartitionIteratorType pitype, class GridImp>
  class AlbertaGridLevelIterator :
    public AlbertaGridTreeIterator<cd,pitype,GridImp> ,
    public LevelIteratorDefaultImplementation <cd,pitype,GridImp,AlbertaGridLevelIterator>
  {
  public:
    typedef typename GridImp::template Codim<cd>::Entity Entity;

    //! Constructor making end iterator
    AlbertaGridLevelIterator(const GridImp & grid, int level, int proc) :
      AlbertaGridTreeIterator<cd,pitype,GridImp> (grid,level,proc)
    {}

    //! Constructor making begin iterator
    AlbertaGridLevelIterator(const GridImp & grid,
                             const AlbertaMarkerVector * vec, int level, int proc) :
      AlbertaGridTreeIterator<cd,pitype,GridImp> (grid,vec,level,proc)
    {}

    //! increment the iterator
    void increment ()
    {
      AlbertaGridTreeIterator<cd,pitype,GridImp>::increment();
    }
  };

  //**********************************************************************
  //
  //  AlbertaGridLeafIterator
  //  --LeafIterator
  //
  //**********************************************************************
  //! LeafIterator which is just a hull for the LevelIterator
  template<int codim, PartitionIteratorType pitype, class GridImp>
  class AlbertaGridLeafIterator :
    public AlbertaGridTreeIterator<codim, pitype, GridImp>,
    public LeafIteratorDefaultImplementation<codim, pitype, GridImp, AlbertaGridLeafIterator>
  {
  public:
    typedef typename GridImp::template Codim<codim>::Entity Entity;

    //! Constructor making end iterator
    AlbertaGridLeafIterator(const GridImp & grid, int level, int proc) :
      AlbertaGridTreeIterator<codim,pitype,GridImp> (grid,level,proc,true)
    {}

    //! Constructor making begin iterator
    AlbertaGridLeafIterator(const GridImp & grid,
                            const AlbertaMarkerVector * vec, int level, int proc) :
      AlbertaGridTreeIterator<codim, pitype, GridImp> (grid,vec,level,proc,true)
    {}

    //! increment the iterator
    void increment ()
    {
      AlbertaGridTreeIterator<codim, pitype, GridImp>::increment();
    }
  };

  //**********************************************************************
  //
  // --AlbertaGrid
  // --Grid
  //
  //**********************************************************************

  template <int dim, int dimworld>
  struct AlbertaGridFamily
  {
    typedef AlbertaGrid<dim,dimworld> GridImp;

    typedef DefaultLevelIndexSet< AlbertaGrid<dim,dimworld> > LevelIndexSetImp;
    //typedef AdaptiveLeafIndexSet< AlbertaGrid<dim,dimworld> > LeafIndexSetImp;
    typedef DefaultLeafIndexSet< AlbertaGrid<dim,dimworld> > LeafIndexSetImp;

    typedef AlbertaGridIdSet<dim,dimworld> IdSetImp;
    typedef int IdType;

    struct Traits
    {
      typedef GridImp Grid;

      typedef Dune::IntersectionIterator<const GridImp, IntersectionIteratorWrapper > IntersectionIterator;

      typedef Dune::HierarchicIterator<const GridImp, AlbertaGridHierarchicIterator> HierarchicIterator;

      typedef IdType GlobalIdType;
      typedef IdType LocalIdType;

      template <int cd>
      struct Codim
      {
        // IMPORTANT: Codim<codim>::Geometry == Geometry<dim-codim,dimw>
        typedef Dune::Geometry<dim-cd, dimworld, const GridImp, AlbertaGridGeometry> Geometry;
        typedef Dune::Geometry<dim-cd, dim, const GridImp, AlbertaGridGeometry> LocalGeometry;
        // we could - if needed - introduce an other struct for dimglobal of Geometry

        typedef typename SelectEntityImp<cd,dim,GridImp>::Entity Entity;

        //typedef Dune::Entity<cd, dim, const GridImp, AlbertaGridEntity> Entity;
        //typedef Dune::EntityDefaultImplementation<cd,dim,const GridImp> Entity;

        typedef Dune::LevelIterator<cd,All_Partition,const GridImp,AlbertaGridLevelIterator> LevelIterator;

        typedef Dune::LeafIterator<cd,All_Partition,const GridImp,AlbertaGridLeafIterator> LeafIterator;

        typedef Dune::EntityPointer<const GridImp,AlbertaGridEntityPointer<cd,const GridImp> > EntityPointer;

        template <PartitionIteratorType pitype>
        struct Partition
        {
          typedef Dune::LevelIterator<cd,pitype,const GridImp,AlbertaGridLevelIterator> LevelIterator;
          typedef Dune::LeafIterator<cd,pitype,const GridImp,AlbertaGridLeafIterator> LeafIterator;
        };

      };

      typedef IndexSet<GridImp,LevelIndexSetImp,DefaultLevelIteratorTypes<GridImp> > LevelIndexSet;
      typedef IndexSet<GridImp,LeafIndexSetImp,DefaultLeafIteratorTypes<GridImp> > LeafIndexSet;
      //typedef LeafIndexSetImp LeafIndexSet;
      typedef IdSet<GridImp,IdSetImp,IdType> GlobalIdSet;
      typedef IdSet<GridImp,IdSetImp,IdType> LocalIdSet;
    };
  };

  /**
     \class AlbertaGrid

     \brief [<em> provides \ref Dune::Grid </em>]
     \brief Provides the simplicial meshes of the finite element tool box
     \brief ALBERTA (http://www.alberta-fem.de/)
     \brief written by Kunibert Siebert and Alfred Schmidt.
     \ingroup GridImplementations

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
   */
  template <int dim, int dimworld>
  class AlbertaGrid :
    public GridDefaultImplementation <dim,dimworld,albertCtype, AlbertaGridFamily<dim,dimworld> >,
    public HasObjectStream
  {
    friend class AlbertaGridEntity <0,dim,const AlbertaGrid<dim,dimworld> >;
    friend class AlbertaGridEntity <1,dim,const AlbertaGrid<dim,dimworld> >;
    friend class AlbertaGridEntity <2,dim,const AlbertaGrid<dim,dimworld> >;
    friend class AlbertaGridEntity <dim,dim,const AlbertaGrid<dim,dimworld> >;

    friend class AlbertaGridEntityPointer <0,const AlbertaGrid<dim,dimworld> >;
    friend class AlbertaGridEntityPointer <1,const AlbertaGrid<dim,dimworld> >;
    friend class AlbertaGridEntityPointer <2,const AlbertaGrid<dim,dimworld> >;
    friend class AlbertaGridEntityPointer <3,const AlbertaGrid<dim,dimworld> >;

    // friends because of fillElInfo
    friend class AlbertaGridTreeIterator<0,All_Partition,AlbertaGrid<dim,dimworld> >;
    friend class AlbertaGridTreeIterator<1,All_Partition,AlbertaGrid<dim,dimworld> >;
    friend class AlbertaGridTreeIterator<2,All_Partition,AlbertaGrid<dim,dimworld> >;
    friend class AlbertaGridTreeIterator<3,All_Partition,AlbertaGrid<dim,dimworld> >;

    friend class AlbertaGridHierarchicIterator<AlbertaGrid<dim,dimworld> >;

    friend class AlbertaGridIntersectionIterator<AlbertaGrid<dim,dimworld> >;
    friend class AlbertaGridIntersectionIterator<const AlbertaGrid<dim,dimworld> >;

    //! AlbertaGrid is only implemented for 2 and 3 dimension
    //! for 1d use SGrid or SimpleGrid
    //CompileTimeChecker<dimworld != 1>   Do_not_use_AlbertaGrid_for_1d_Grids;

    typedef AlbertaGrid<dim,dimworld> MyType;

    friend class AlbertaMarkerVector;
    friend class AlbertaGridHierarchicIndexSet<dim,dimworld>;

    //**********************************************************
    // The Interface Methods
    //**********************************************************
  public:
    //! the grid family of AlbertaGrid
    typedef AlbertaGridFamily<dim,dimworld> GridFamily;
    typedef GridDefaultImplementation <dim,dimworld,albertCtype,
        AlbertaGridFamily<dim,dimworld> > BaseType;


    //! the Traits
    typedef typename AlbertaGridFamily<dim,dimworld> :: Traits Traits;

    typedef typename Traits::template Codim<0>::LeafIterator LeafIterator;

    typedef AlbertaGridHierarchicIndexSet<dim,dimworld> HierarchicIndexSet;
    typedef typename GridFamily:: LevelIndexSetImp LevelIndexSetImp;
    typedef typename GridFamily:: LeafIndexSetImp LeafIndexSetImp;
    typedef typename Traits :: LeafIndexSet LeafIndexSet;

    typedef AlbertaGridIdSet<dim,dimworld> IdSetImp;
    typedef typename Traits :: GlobalIdSet GlobalIdSet;
    typedef typename Traits :: LocalIdSet LocalIdSet;

    typedef typename ALBERTA AlbertHelp::AlbertLeafData<dimworld,dim+1> LeafDataType;

    typedef ObjectStream ObjectStreamType;

    //! we always have dim+1 codimensions
    enum { numCodim = dim+1 };

    //! max number of allowed levels
    enum { MAXL = 64 };

    //! Constructor which reads an Albert Macro Triang file
    //! or given GridFile
    //! levInd = true means that a consecutive level index is generated
    //! if levInd == true the the element number of first macro element is
    //! set to 1 so hasLevelIndex_ can be identified we grid is read from
    //! file
    AlbertaGrid(const std::string macroTriangFilename);

    //! Constructor which reads an Albert Macro Triang file
    //! or given GridFile , proc is the number of domain ,
    //! levInd = true means that a consecutive level index is generated
    //! if levInd == true the the element number of first macro element is
    //! set to 1 so hasLevelIndex_ can be identified we grid is read from
    //! file
    AlbertaGrid(AlbertaGrid<dim,dimworld> & oldGrid, int proc);

    //! empty Constructor
    AlbertaGrid();

    //! Desctructor
    ~AlbertaGrid();

    //! Return maximum level defined in this grid. Levels are numbered
    //! 0 ... maxLevel with 0 the coarsest level.
    int maxLevel() const;

    //! Iterator to first entity of given codim on level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template Codim<cd>::template Partition<pitype>::LevelIterator
    lbegin (int level) const;

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template Codim<cd>::template Partition<pitype>::LevelIterator
    lend (int level) const;

    //! Iterator to first entity of given codim on level
    template<int cd>  typename Traits::template Codim<cd>::
    template Partition<All_Partition>::LevelIterator
    lbegin (int level) const;

    //! one past the end on this level
    template<int cd>  typename Traits::template Codim<cd>::
    template Partition<All_Partition>::LevelIterator
    lend (int level) const;

    //! return LeafIterator which points to first leaf entity
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    leafbegin ( int maxlevel, int proc = -1 ) const;

    //! return LeafIterator which points to first leaf entity
    template <int codim>
    typename Traits::template Codim<codim>::LeafIterator
    leafbegin ( int maxlevel, int proc = -1 ) const;

    //! return LeafIterator which points behind last leaf entity
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    leafend   ( int maxlevel, int proc = -1 ) const;

    //! return LeafIterator which points behind last leaf entity
    template <int codim>
    typename Traits::template Codim<codim>::LeafIterator
    leafend   ( int maxlevel, int proc = -1 ) const;

    //! return LeafIterator which points to first leaf entity
    LeafIterator leafbegin ( int maxlevel, int proc = -1 ) const;

    //! return LeafIterator which points behind last leaf entity
    LeafIterator leafend   ( int maxlevel, int proc = -1 ) const;

    //! return LeafIterator which points to first leaf entity
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    leafbegin () const;

    //! return LeafIterator which points to first leaf entity
    template <int codim>
    typename Traits::template Codim<codim>::LeafIterator
    leafbegin () const;

    //! return LeafIterator which points behind last leaf entity
    template <int codim, PartitionIteratorType pitype>
    typename Traits::template Codim<codim>::template Partition<pitype>::LeafIterator
    leafend   () const;

    //! return LeafIterator which points behind last leaf entity
    template <int codim>
    typename Traits::template Codim<codim>::LeafIterator
    leafend   () const;

    //! return LeafIterator which points to first leaf entity
    LeafIterator leafbegin () const;

    //! return LeafIterator which points behind last leaf entity
    LeafIterator leafend   () const;


    /** \brief Number of grid entities per level and codim
     * because lbegin and lend are none const, and we need this methods
     * counting the entities on each level, you know.
     */
    int size (int level, int codim) const;

    //! number of entities per level and geometry type in this process
    int size (int level, GeometryType type) const;

    //! number of leaf entities per codim in this process
    int size (int codim) const;

    //! number of leaf entities per geometry type in this process
    int size (GeometryType type) const;

    /** dummy collective communication */
    const CollectiveCommunication<AlbertaGrid>& comm () const
    {
      return ccobj;
    }

  private:
    CollectiveCommunication<AlbertaGrid> ccobj;

  public:
    //***************************************************************
    //  Interface for Adaptation
    //***************************************************************
    /*! \brief marks an element for refCount refines. if refCount is negative the
     *  element is coarsend -refCount times
     *  NOTE: if element was already marked for refinement then nothing
     *  happens if element is again marked for coarsen, refinement always
     *  counts more then coarsening
     *  mark returns true if element was marked, otherwise false */
    bool mark( int refCount , const typename Traits::template Codim<0>::EntityPointer & en ) const;

  private:
    bool mark( int refCount , const typename Traits::template Codim<0>::Entity & en ) const;

  public:
    //! uses the interface, mark on entity and refineLocal
    bool globalRefine(int refCount);

    /*! \brief refine all positive marked leaf entities,
    *  coarsen all negative marked entities if possible,
    *  return true if a least one element was refined */
    bool adapt ( );

    //! adapt method with DofManager
    template <class DofManagerType, class RestrictProlongOperatorType>
    bool adapt (DofManagerType &, RestrictProlongOperatorType &, bool verbose=false );

    //! returns true, if a least one element is marked for coarsening
    bool preAdapt ();

    //! clean up some markers
    bool postAdapt();

    //! fake implementation of load-balance
    template <class DofManagerType>
    bool loadBalance (DofManagerType & dm) { return false; }

    /*
       //! fake implementation of communicate
       template <class DofManagerType>
       bool communicate (DofManagerType & dm) { return false; }
     */

    /** \brief return type of grid, here AlbertaGrid_Id. */
    GridIdentifier type () const { return AlbertaGrid_Id; };

    //**********************************************************
    // End of Interface Methods
    //**********************************************************
    /** \brief write Grid to file in specified GrapeIOFileFormatType */
    template <GrapeIOFileFormatType ftype>
    bool writeGrid( const std::basic_string<char> filename, albertCtype time ) const;

    /** \brief read Grid from file filename and store time of mesh in time */
    template <GrapeIOFileFormatType ftype>
    bool readGrid( const std::basic_string<char> filename, albertCtype & time );

    //! returns size of mesh include all levels
    //! max Index of grid entities with given codim
    //! for outside the min index is 0, the shift has to done inside
    //! the grid which is of minor cost
    int global_size (int codim) const;

    //! return number of my processor
    int myRank () const { return myRank_; };

    //! transform grid N = scalar * x + trans
    void setNewCoords(const FieldVector<albertCtype, dimworld> & trans, const albertCtype scalar);

    //! return hierarchic index set
    const HierarchicIndexSet & hierarchicIndexSet () const { return hIndexSet_; }

    //! return level index set for given level
    const typename Traits :: LevelIndexSet & levelIndexSet (int level) const;

    //! return leaf index set
    const typename Traits :: LeafIndexSet & leafIndexSet () const;

    //! return global IdSet
    const GlobalIdSet & globalIdSet () const { return globalIdSet_; }

    //! return local IdSet
    const LocalIdSet & localIdSet () const { return globalIdSet_; }

    //! access to mesh pointer, needed by some methods
    ALBERTA MESH* getMesh () const { return mesh_; };

    //! return real entity implementation
    template <int cd>
    AlbertaGridEntity<cd,dim,const AlbertaGrid<dim,dimworld> >&
    getRealEntity(typename Traits::template Codim<cd>::Entity& entity)
    {
      return this->getRealImplementation(entity);
    }

  private:
    //! return real entity implementation
    template <int cd>
    const AlbertaGridEntity<cd,dim,const AlbertaGrid<dim,dimworld> >&
    getRealEntity(const typename Traits::template Codim<cd>::Entity& entity) const
    {
      return this->getRealImplementation(entity);
    }

  public:
    //! create ghost cells
    void createGhosts ();

    //! get adaptation mark
    template <class EntityType>
    int getMark(const EntityType & ) const;

    //! return processor number where entity is master
    template <class EntityType>
    int owner (const EntityType & ) const;

    //! AlbertaGrid internal method for partitioning
    //! set processor number of this entity
    template <class EntityType>
    bool partition( int proc , EntityType & );

    //! unpack recieved ObjectStream
    void unpackAll ( ObjectStreamType & os );

    //! pack this entity and all chilcren to ObjectStream
    template <class EntityType>
    void packAll ( ObjectStreamType & os, EntityType & en );

    //! pack this entity and all chilcren to ObjectStream
    template <class EntityType>
    void packBorder ( ObjectStreamType & os, EntityType & en );

    // return true if macro element is ghost
    bool isGhost( const ALBERTA MACRO_EL * mel) const;

    // return true if element is neihter interior nor ghost
    bool isNoElement( const ALBERTA MACRO_EL * mel) const;

    //! returns geometry type vector for codimension
    const std::vector < GeometryType > & geomTypes (int codim) const { return geomTypes_[codim]; }

  private:
    friend class Conversion<AlbertaGrid<dim, dimworld>, HasObjectStream>;
    friend class Conversion<const AlbertaGrid<dim, dimworld>, HasObjectStream>;

    AlbertaGrid(const MyType& other);
    MyType& operator=(const MyType& other);

  private:
    Array<int> ghostFlag_; // store ghost information

    // initialize of some members
    void initGrid(int proc);

    // make the calculation of indexOnLevel and so on.
    // extra method because of Reihenfolge
    void calcExtras();

    // write ALBERTA mesh file
    bool writeGridXdr  ( const std::basic_string<char> filename, albertCtype time ) const;

    //! reads ALBERTA mesh file
    bool readGridXdr   ( const std::basic_string<char> filename, albertCtype & time );

    //! reads ALBERTA macro file
    bool readGridAscii ( const std::basic_string<char> filename, albertCtype & time );

    // delete mesh and all vectors
    void removeMesh();

    // pointer to an Albert Mesh, which contains the data
    ALBERTA MESH *mesh_;

    // number of maxlevel of the mesh
    int maxlevel_;

    // true if grid was refined or coarsend
    bool wasChanged_;

    // is true, if a least one entity is marked for coarsening
    mutable bool isMarked_;

    // set isMarked, isMarked is true if at least one entity is marked for
    // coarsening
    void setMark ( bool isMarked ) const;

    // help vector for setNewCoords
    mutable Array<int> macroVertices_;

  public:
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

  private:
    // needed for VertexIterator, mark on which element a vertex is treated
    mutable AlbertaMarkerVector vertexMarkerLeaf_;

    // needed for VertexIterator, mark on which element a vertex is treated
    mutable AlbertaMarkerVector vertexMarkerLevel_[MAXL];

    //***********************************************************************
    //  MemoryManagement for Entitys and Geometrys
    //**********************************************************************
    typedef typename SelectEntityImp<0,dim,const MyType>::EntityObject EntityObject;

  public:
    typedef AGMemoryProvider< EntityObject > EntityProvider;
    mutable EntityProvider entityProvider_;

    typedef AlbertaGridIntersectionIterator< const MyType > IntersectionIteratorImp;
    typedef AGMemoryProvider< IntersectionIteratorImp > IntersectionIteratorProviderType;
    friend class IntersectionIteratorWrapper< const MyType > ;

    typedef IntersectionIteratorWrapper<const MyType >
    AlbertaGridIntersectionIteratorType;

    IntersectionIteratorProviderType & intersetionIteratorProvider() const { return interItProvider_; }
    mutable IntersectionIteratorProviderType interItProvider_;

    AlbertaGridIntersectionIteratorType &
    getRealIntersectionIterator(typename Traits::IntersectionIterator& it)
    {
      return this->getRealImplementation(it);
    }

    AlbertaGridIntersectionIteratorType &
    getRealIntersectionIterator(const typename Traits::IntersectionIterator& it) const
    {
      return this->getRealImplementation(it);
    }

    //! return obj pointer to EntityImp
    template <int codim>
    typename SelectEntityImp<codim,dim,const MyType>::EntityObject *
    getNewEntity (int level, bool leafIt ) const;

    //! free obj pointer of EntityImp
    template <int codim>
    void freeEntity (typename SelectEntityImp<codim,dim,const MyType>::EntityObject * en) const;

  private:
    //*********************************************************************
    // organisation of the global index
    //*********************************************************************
    // provides the indices for the elements
    IndexManagerType indexStack_[AlbertHelp::numOfElNumVec];

    // the DOF_INT_VECs we need
    // * change to mutable here
    mutable ALBERTA AlbertHelp::DOFVEC_STACK dofvecs_;

    const ALBERTA DOF_ADMIN * elAdmin_;
    // pointer to vec of elNumbers_
    const int * elNewVec_;

    // for access in the elNewVec and ownerVec
    const int nv_;
    const int dof_;

    // make some shortcuts
    void arrangeDofVec();

  public:
    // return true if el is new
    bool checkElNew ( const ALBERTA EL * el ) const;

    // read level from elNewCehck vector
    int getLevelOfElement ( const ALBERTA EL * el ) const;

    // read global element number from elNumbers_
    int getElementNumber ( const ALBERTA EL * el ) const;

    // read global element number from elNumbers_
    int getEdgeNumber ( const ALBERTA EL * el, int edge ) const;

    // read global element number from elNumbers_
    int getFaceNumber ( const ALBERTA EL * el, int face ) const;

    // read global element number from elNumbers_
    int getVertexNumber ( const ALBERTA EL * el, int vx ) const;

    //********************************************************************
    //  organisation of the parallelisation
    //********************************************************************

    // set owner of element, for partioning
    bool setOwner ( const ALBERTA EL * el , int proc );

    // return the processor number of element
    int getOwner ( const ALBERTA EL * el ) const;

    // PartitionType (InteriorEntity , BorderEntity, GhostEntity )
    PartitionType partitionType ( ALBERTA EL_INFO * elinfo) const;

  private:

    // pointer to vec  with processor number for each element,
    // access via setOwner and getOwner
    int * ownerVec_;

    // rank of my thread, i.e. number of my processor
    const int myRank_;

    // the hierarchical numbering of AlbertaGrid, unique per codim and processor
    AlbertaGridHierarchicIndexSet<dim,dimworld> hIndexSet_;

    // the id set of this grid
    IdSetImp globalIdSet_;

    // the level index set, is generated from the HierarchicIndexSet
    // is generated, when accessed
    mutable std::vector < LevelIndexSetImp * > levelIndexVec_;

    // the leaf index set, is generated from the HierarchicIndexSet
    // is generated, when accessed
    mutable LeafIndexSetImp* leafIndexSet_;

    //! stores geometry types of this grid
    std::vector < std::vector< GeometryType > > geomTypes_;

    // stack for storing BOUNDARY objects created during mesh creation
    std::stack < BOUNDARY * > bndStack_;

    typedef SingleTypeSizeCache<MyType> SizeCacheType;
    SizeCacheType * sizeCache_;
  }; // end class AlbertaGrid


  namespace Capabilities
  {
    template<int dim,int dimw>
    struct hasLeafIterator< AlbertaGrid<dim,dimw> >
    {
      static const bool v = true;
    };

    template<int dim, int dimw, int cdim>
    struct hasEntity<AlbertaGrid<dim,dimw>, cdim >
    {
      static const bool v = true;
    };

    template<int dim, int dimw>
    struct isLevelwiseConforming< AlbertaGrid<dim,dimw> >
    {
      static const bool v = true;
    };

    template<int dim, int dimw>
    struct hasHangingNodes< AlbertaGrid<dim,dimw> >
    {
      static const bool v = false;
    };
  } // end namespace Capabilities

} // namespace Dune

#include "agmemory.hh"
#include <dune/io/file/asciiparser.hh>
#include "albertagrid.cc"

// undef all dangerous defines
#undef DIM
#undef DIM_OF_WORLD
#include "alberta_undefs.hh"
#endif
