// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ALBERTAGRID_HH__
#define __DUNE_ALBERTAGRID_HH__
#include <iostream>
#include <fstream>

#include <vector>
#include <assert.h>
#include <algorithm>

#include <config.h>

// Dune includes
#include <dune/common/misc.hh>
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/array.hh>
#include <dune/common/capabilities.hh>
#include <dune/common/stdstreams.hh>
#include "common/grid.hh"

#include "dune/common/exceptions.hh"

// some cpp defines and include of alberta.h
#include "albertagrid/albertaheader.hh"

// stack for index management
#include <dune/grid/common/indexstack.hh>

// IndexManager defined in indexstack.hh
// 10000 is the size of the finite stack used by IndexStack
typedef Dune::IndexStack<int,10000> IndexManagerType;

// some extra functions for handling the Albert Mesh
#include "albertagrid/albertaextra.hh"

//#define DUNE_THROW(E, m) assert(false)

namespace Dune {
  // own exception classes
  class AlbertaError   : public Exception {};
  class AlbertaIOError : public IOError {};
}

// contains a simple memory management for some componds of this grid
#include "albertagrid/agmemory.hh"

// contains the communication for parallel computing for this grid

#include "albertagrid/agcommunicator.hh"
#include "common/defaultindexsets.hh"

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

  // i.e. double or float
  typedef ALBERTA REAL albertCtype;

  // forward declarations
  class AlbertaMarkerVector;

  template<int codim, int dim, class GridImp> class AlbertaGridEntity;
  template<int codim, PartitionIteratorType pitype, class GridImp> class AlbertaGridLevelIterator;
  template<class GridImp> class AlbertaGridLeafIterator;

  template <int mydim, int cdim, class GridImp> class AlbertaGridGeometry;
  template<class GridImp>         class AlbertaGridBoundaryEntity;
  template<class GridImp>         class AlbertaGridHierarchicIterator;
  template<class GridImp>         class AlbertaGridIntersectionIterator;
  template<int dim, int dimworld> class AlbertaGrid;
  template<int dim, int dimworld> class AlbertaGridHierarchicIndexSet;

  // singleton holding reference elements
  template<int dim, class GridImp> struct AlbertaGridReferenceGeometry;

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

  template<int mydim, int coorddim, class GridImp>
  class AlbertaGridMakeableGeometry : public Geometry<mydim, coorddim, GridImp, AlbertaGridGeometry>
  {
    typedef Geometry<mydim, coorddim, GridImp, AlbertaGridGeometry> GeometryType;
  public:
    AlbertaGridMakeableGeometry(bool makeRefelem=false) :
      GeometryType (AlbertaGridGeometry<mydim, coorddim, GridImp>(makeRefelem)) {};

    // just a wrapper call
    bool builtGeom(ALBERTA EL_INFO *elInfo, int face, int edge, int vertex)
    {
      return this->realGeometry.builtGeom(elInfo,face,edge,vertex);
    }

    // just a wrapper call
    void initGeom()
    {
      this->realGeometry.initGeom();
      return ;
    }

    // for changing the coordinates of one element
    FieldVector<albertCtype, coorddim>& getCoordVec (int i)
    {
      return this->realGeometry.getCoordVec(i);
    }

  };



  //******************************************************
  //
  //  --Geometry
  //
  //******************************************************
  template <int mydim, int cdim, class GridImp>
  class AlbertaGridGeometry :
    public GeometryDefault<mydim,cdim,GridImp,AlbertaGridGeometry>
  {
    friend class AlbertaGridBoundaryEntity<GridImp>;

    //! know dimension of barycentric coordinates
    enum { dimbary=mydim+1};
  public:
    //! for makeRefGeometry == true a Geometry with the coordinates of the
    //! reference element is made
    AlbertaGridGeometry(bool makeRefGeometry=false);

    //! return the element type identifier
    //! line , triangle or tetrahedron, depends on dim
    GeometryType type () const;

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const;

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<albertCtype, cdim> & operator[] (int i) const;

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned.
     */
    static const Dune::Geometry<mydim,mydim,GridImp,Dune::AlbertaGridGeometry> & refelem ();

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
    //! call jacobianInverse first because integration element is calculated
    //! during calculation of the jacobianInverse
    const FieldMatrix<albertCtype,mydim,mydim>& jacobianInverse (const FieldVector<albertCtype, cdim>& global) const;

    //***********************************************************************
    //!  Methods that not belong to the Interface, but have to be public
    //***********************************************************************
    //! generate the geometry for the ALBERTA EL_INFO
    //! no interface method
    bool builtGeom(ALBERTA EL_INFO *elInfo, int face, int edge, int vertex);
    // init geometry with zeros
    //! no interface method
    void initGeom();
    FieldVector<albertCtype, cdim>& getCoordVec (int i);

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

    //! is true if Jinv_ and volume_ is calced
    mutable bool builtinverse_;
    enum { matdim = (mydim > 0) ? mydim : 1 };
    mutable FieldMatrix<albertCtype,matdim,matdim> Jinv_; //!< storage for inverse of jacobian

    //! is true if elMat_ was calced
    mutable bool builtElMat_;
    mutable FieldMatrix<albertCtype,matdim,matdim> elMat_; //!< storage for mapping matrix

    mutable bool calcedDet_; //! true if determinant was calculated
    mutable albertCtype elDet_; //!< storage of element determinant

    // temporary mem for integrationElement with mydim < cdim
    mutable FieldVector<albertCtype,cdim> tmpV_;
    mutable FieldVector<albertCtype,cdim> tmpU_;
    mutable FieldVector<albertCtype,cdim> tmpZ_;
  };

  //******************************************************************
  // --Mentity
  template<int codim, int dim, class GridImp>
  class AlbertaGridMakeableEntity :
    public GridImp::template codim<codim>::Entity
  {
    typedef typename GridImp::template codim<codim>::Entity EntityType;
    friend class AlbertaGridEntity<codim, dim, GridImp>;
  public:

    AlbertaGridMakeableEntity(const GridImp & grid, int level) :
      GridImp::template codim<codim>::Entity (AlbertaGridEntity<codim, dim, GridImp>(grid,level)) {}

    // passing through mehtods
    void setTraverseStack (ALBERTA TRAVERSE_STACK *travStack)
    {
      this->realEntity.setTraverseStack(travStack);
    }

    void setElInfo (ALBERTA EL_INFO *elInfo, int face, int edge, int vertex )
    {
      this->realEntity.setElInfo(elInfo,face,edge,vertex);
    }

    void setElInfo (ALBERTA EL_INFO *elInfo)
    {
      this->realEntity.setElInfo(elInfo);
    }
    // needed for the LevelIterator
    ALBERTA EL_INFO * getElInfo () const
    {
      return this->realEntity.getElInfo();
    }

    void setLevel ( int level )
    {
      this->realEntity.setLevel(level);
    }
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
    public EntityDefault <cd,dim,GridImp,AlbertaGridEntity>
  {
    enum { dimworld = GridImp::dimensionworld };
    friend class AlbertaGrid < dim , dimworld >;
    friend class AlbertaGridEntity < 0, dim, GridImp>;
    friend class AlbertaGridLevelIterator < cd, All_Partition,GridImp>;
    friend class AlbertaGridMakeableEntity<cd,dim,GridImp>;

    typedef AlbertaGridMakeableGeometry<dim-cd,dimworld,GridImp> GeometryImp;
  public:
    typedef typename GridImp::template codim<cd>::Entity Entity;
    typedef typename GridImp::template codim<0>::EntityPointer EntityPointer;
    typedef typename GridImp::template codim<cd>::Geometry Geometry;
    typedef typename GridImp::template codim<cd>::LevelIterator LevelIterator;

    //! level of this element
    int level () const;

    //! index is unique and consecutive per level and codim
    //! used for access to degrees of freedom
    int index () const;

    //! index of the boundary which is associated with the entity, 0 for inner entities
    int boundaryId () const ;

    //! return the global unique index in grid
    int globalIndex() const ;

    AlbertaGridEntity(const GridImp &grid, int level,
                      ALBERTA TRAVERSE_STACK * travStack);

    AlbertaGridEntity(const GridImp &grid, int level);

    //! geometry of this entity
    const Geometry & geometry () const;

    EntityPointer ownersFather () const;

    //! my position in local coordinates of the owners father
    FieldVector<albertCtype, dim>& positionInOwnersFather () const;

    //***********************************************
    //  End of Interface methods
    //***********************************************
    // needed for the LevelIterator and LeafIterator
    ALBERTA EL_INFO *getElInfo () const;

    //! return the current face/edge or vertex number
    //! no interface method
    int getFEVnum () const;
  private:
    // methods for setting the infos from the albert mesh
    void setTraverseStack (ALBERTA TRAVERSE_STACK *travStack);
    void setElInfo (ALBERTA EL_INFO *elInfo, int face,
                    int edge, int vertex );
    // private Methods
    void makeDescription();

    // the grid this entity belong to
    const GridImp &grid_;

    // Alberta element
    ALBERTA EL_INFO *elInfo_;

    // current traverse stack this entity belongs too
    ALBERTA TRAVERSE_STACK * travStack_;

    //! level
    int level_;

    //! the current geometry
    mutable GeometryImp geo_;

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
    public EntityDefault <0,dim,GridImp,AlbertaGridEntity>
  {
    enum { dimworld = GridImp::dimensionworld };
    friend class AlbertaGrid < dim , GridImp::dimensionworld >;
    friend class AlbertaMarkerVector;
    friend class AlbertaGridIntersectionIterator <GridImp>;
    friend class AlbertaGridHierarchicIterator <GridImp>;
    friend class AlbertaGridLevelIterator <0,All_Partition,GridImp>;
    friend class AlbertaGridMakeableEntity<0,dim,GridImp>;
  public:
    template <int cd>
    struct codim
    {
      typedef typename GridImp::template codim<cd>::EntityPointer EntityPointer;
    };

    typedef AlbertaGridMakeableEntity<0,GridImp::dimension,GridImp> EntityImp;

    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef typename GridImp::template codim<0>::Geometry Geometry;
    typedef AlbertaGridMakeableGeometry<dim,dimworld,GridImp> GeometryImp;

    typedef typename GridImp::template codim<0>::LevelIterator LevelIterator;
    typedef typename GridImp::template codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename GridImp::template codim<0>::HierarchicIterator HierarchicIterator;
    typedef typename GridImp::template codim<0>::EntityPointer EntityPointer;

    enum { dimension = dim };

    //! Destructor, needed perhaps needed for deleteing faceEntity_ and
    //! edgeEntity_ , see below
    //! there are only implementations for dim==dimworld 2,3
    ~AlbertaGridEntity() {};

    //! Constructor, real information is set via setElInfo method
    AlbertaGridEntity(const GridImp &grid, int level);

    //! level of this element
    int level () const;

    //! index is unique and consecutive per level and codim used for access to degrees of freedo
    int index () const;

    //! return the global unique index in grid , same as el_index
    int globalIndex() const ;

    //! index of the boundary which is associated with the entity, 0 for inner entities
    int boundaryId () const;

    //! geometry of this entity
    const Geometry & geometry () const;

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
    template<int cc> typename codim<cc>::EntityPointer entity (int i) const;

    //! Provide access to mesh entity i of given codimension. Entities
    //!  are numbered 0 ... count<cc>()-1
    //template<int cc> void entity (AlbertaGridLevelIterator<cc,dim,dimworld>& it, int i);

    /*! Intra-level access to intersection with neighboring elements.
       A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor. */
    AlbertaGridIntersectionIterator<GridImp> ibegin () const;

    //! Reference to one past the last intersection with neighbor
    AlbertaGridIntersectionIterator<GridImp> iend () const;

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
    void setLeafData( int proc );

    //! return partition type of this entity ( see grid.hh )
    PartitionType partitionType() const;

    //! return true if this entity belong to master set of this grid
    bool master() const;

    // return 0 for elements
    int getFEVnum () const { return 0; }

    // needed for LevelIterator to compare
    ALBERTA EL_INFO *getElInfo () const;
  private:
    // called from HierarchicIterator, because only this
    // class changes the level of the entity, otherwise level is set by
    // Constructor
    void setLevel ( int actLevel );

    // face, edge and vertex only for codim > 0, in this
    // case just to supply the same interface
    void setTraverseStack (ALBERTA TRAVERSE_STACK *travStack);
    void setElInfo (ALBERTA EL_INFO *elInfo,
                    int face = 0,
                    int edge = 0,
                    int vertex = 0 );

    //! make a new AlbertaGridEntity
    void makeDescription();

    //! the corresponding grid
    const GridImp & grid_;

    //! the level of the entity
    int level_;

    //! for vertex access, to be revised, filled on demand
    //mutable typename codim<dim>::EntityPointer vxEntity_;

    //! pointer to the Albert TRAVERSE_STACK data
    ALBERTA TRAVERSE_STACK * travStack_;

    //! pointer to the real Albert element data
    ALBERTA EL_INFO *elInfo_;

    // local coordinates within father
    mutable GeometryImp fatherReLocal_;

    //! the cuurent geometry
    mutable GeometryImp geo_;
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

  template<class GridImp>
  class AlbertaGridHierarchicIterator :
    public HierarchicIteratorDefault <GridImp,AlbertaGridHierarchicIterator>
  {
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef typename GridImp::ctype ctype;
    typedef AlbertaGridMakeableEntity<0,GridImp::dimension,GridImp> EntityImp;

    //! the normal Constructor
    AlbertaGridHierarchicIterator(const GridImp &grid,
                                  ALBERTA TRAVERSE_STACK *travStack, int actLevel, int maxLevel);

    //! the default Constructor
    AlbertaGridHierarchicIterator(const GridImp &grid,
                                  int actLevel,int maxLevel);

    //! increment
    void increment();

    //! equality
    bool equals (const AlbertaGridHierarchicIterator<GridImp>& i) const;

    //! dereferencing
    Entity& dereference() const;

  private:
    //! know the grid were im comming from
    const GridImp &grid_;

    //! the actual Level of this Hierarichic Iterator
    int level_;

    //! max level to go down
    int maxlevel_;

    //! implement with virtual element
    mutable EntityImp virtualEntity_;

    //! we need this for Albert traversal, and we need ManageTravStack, which
    //! does count References when copied
    ALBERTA ManageTravStack manageStack_;

    //! The nessesary things for Albert
    ALBERTA EL_INFO * recursiveTraverse(ALBERTA TRAVERSE_STACK * stack);

    //! make empty HierarchicIterator
    void makeIterator();
  };


  //********************************************************************
  //
  //  --BoundaryEntity
  //
  //********************************************************************
  template<class GridImp>
  class AlbertaGridMakeableBoundaryEntity :
    public GridImp::template codim<0>::BoundaryEntity
  {
  public:
    AlbertaGridMakeableBoundaryEntity () :
      GridImp::template codim<0>::BoundaryEntity (AlbertaGridBoundaryEntity<GridImp>()) {};
    // set elInfo
    void setElInfo(ALBERTA EL_INFO * elInfo, int nb)
    {
      this->realBoundaryEntity.setElInfo(elInfo,nb);
    }
  };

  /** \todo Please doc me! */
  template <class GridImp>
  class AlbertaGridBoundaryEntity :
    public BoundaryEntityDefault <GridImp,AlbertaGridBoundaryEntity>
  {
    friend class AlbertaGridIntersectionIterator<GridImp>;
    friend class AlbertaGridMakeableBoundaryEntity<GridImp>;
  public:
    typedef typename GridImp::template codim<0>::Geometry Geometry;

    //! Constructor
    AlbertaGridBoundaryEntity ();

    //! return identifier of boundary segment, number
    int id () const;

    //! return true if geometry of ghost cells was filled
    bool hasGeometry () const ;

    //! return geometry of the ghost cell
    const Geometry & geometry () const;

  private:
    // set elInfo
    void setElInfo(ALBERTA EL_INFO * elInfo, int nb);

    // ghost cell
    mutable Geometry _geom;

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
  template<class GridImp>
  class AlbertaGridIntersectionIterator :
    public IntersectionIteratorDefault <GridImp,AlbertaGridIntersectionIterator>
  {
    enum { dim      = GridImp::dimension };
    enum { dimworld = GridImp::dimensionworld };

    friend class AlbertaGridEntity<0,dim,GridImp>;
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef typename GridImp::template codim<0>::BoundaryEntity BoundaryEntity;
    typedef AlbertaGridMakeableBoundaryEntity<GridImp> MakeableBndEntityType;
    typedef typename GridImp::template codim<1>::Geometry Geometry;
    typedef typename GridImp::template codim<1>::LocalGeometry LocalGeometry;
    typedef AlbertaGridMakeableEntity<0,dim,GridImp> EntityImp;
    typedef AlbertaGridMakeableGeometry<dim-1,dimworld,GridImp> LocalGeometryImp;

    //! know your own dimension
    enum { dimension=dim };
    //! know your own dimension of world
    enum { dimensionworld=dimworld };
    //! define type used for coordinates in grid module
    typedef typename GridImp::ctype ctype;

    //! increment
    void increment();
    //! equality
    bool equals (const AlbertaGridIntersectionIterator<GridImp>& i) const;
    //! access neighbor, dereferencing
    Entity & dereference() const;

    //! The default Constructor
    AlbertaGridIntersectionIterator(const GridImp & grid,
                                    int level);

    //! The Constructor
    AlbertaGridIntersectionIterator(const GridImp & grid,
                                    int level,
                                    ALBERTA EL_INFO *elInfo);

    //! The Destructor
    ~AlbertaGridIntersectionIterator();

    //! return true if intersection is with boundary. \todo connection with
    //! boundary information, processor/outer boundary
    bool boundary () const;

    //! return true if across the edge an neighbor on this level exists
    bool neighbor () const;

    //! return information about the Boundary
    BoundaryEntity & boundaryEntity () const;

    //! intersection of codimension 1 of this neighbor with element where
    //! iteration started.
    //! Here returned element is in LOCAL coordinates of the element
    //! where iteration started.
    LocalGeometry& intersectionSelfLocal () const;
    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of neighbor
     */
    LocalGeometry& intersectionNeighborLocal () const;
    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in GLOBAL coordinates of the element where iteration started.
     */
    Geometry& intersectionGlobal () const;

    //! local number of codim 1 entity in self where intersection is contained in
    int numberInSelf () const;
    //! local number of codim 1 entity in neighbor where intersection is contained in
    int numberInNeighbor () const;

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

  private:
    //**********************************************************
    //  private methods
    //**********************************************************
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

    //! know the grid were im comming from
    const GridImp *grid_;

    //! the actual level
    mutable int level_;

    //! count on which neighbor we are lookin' at
    mutable int neighborCount_;

    //! implement with virtual element
    //! Most of the information can be generated from the ALBERTA EL_INFO
    //! therefore this element is only created on demand.
    mutable bool builtNeigh_;
    mutable EntityImp *virtualEntity_;

    //! pointer to the EL_INFO struct storing the real element information
    mutable ALBERTA EL_INFO * elInfo_;

    // for memory management
    mutable typename GridImp::EntityProvider::ObjectEntity *manageObj_;
    mutable typename GridImp::IntersectionSelfProvider::ObjectEntity  *manageInterEl_;
    mutable typename GridImp::IntersectionNeighProvider::ObjectEntity *manageNeighEl_;

    //! pointer to element holding the self_local and self_global information.
    //! This element is created on demand.
    mutable LocalGeometryImp *fakeNeigh_;

    //! pointer to element holding the neighbor_global and neighbor_local
    //! information. This element is created on demand.
    mutable LocalGeometryImp *neighGlob_;

    //! BoundaryEntity
    mutable MakeableBndEntityType * boundaryEntity_;

    //! defined in agmemory.hh
    mutable typename ElInfoProvider::ObjectEntity *manageNeighInfo_;

    //! EL_INFO th store the information of the neighbor if needed
    mutable ALBERTA EL_INFO * neighElInfo_;

    mutable NormalVecType outNormal_;
    mutable NormalVecType unitNormal_;

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
  template<int cd, PartitionIteratorType pitype, class GridImp>
  class AlbertaGridLevelIterator :
    public LevelIteratorDefault <cd,pitype,GridImp,AlbertaGridLevelIterator>
  {
    enum { dim = GridImp::dimension };
    friend class AlbertaGridEntity<2,dim,GridImp>;
    friend class AlbertaGridEntity<1,dim,GridImp>;
    friend class AlbertaGridEntity<0,dim,GridImp>;
    friend class AlbertaGrid < dim , GridImp::dimensionworld >;

    typedef AlbertaGridLevelIterator<cd,pitype,GridImp>  AlbertaGridLevelIteratorType;
  public:

    typedef typename GridImp::template codim<cd>::Entity Entity;
    typedef AlbertaGridMakeableEntity<cd,dim,GridImp> EntityImp;

    //! Constructor making end iterator
    AlbertaGridLevelIterator(const GridImp & grid, int
                             travLevel, int proc, bool leafIt=false );

    //! Constructor making EntityPointer
    AlbertaGridLevelIterator(const GridImp & grid,
                             ALBERTA TRAVERSE_STACK * stack,
                             int travLevel,
                             ALBERTA EL_INFO *elInfo,
                             int face=0,
                             int edge=0,
                             int vertex=0);

    //! Constructor making begin iterator
    AlbertaGridLevelIterator(const GridImp & grid,
                             AlbertaMarkerVector * vec,
                             int travLevel,
                             int proc,
                             bool leafIt=false);

    //! increment
    void increment();
    //! equality
    bool equals(const AlbertaGridLevelIterator<cd,pitype,GridImp>& i) const;
    //! dereferencing
    Entity& dereference() const;

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
    const GridImp & grid_;

    //! level :)
    int level_;

    mutable EntityImp virtualEntity_;

    // contains ALBERTA traversal stack
    ALBERTA ManageTravStack manageStack_;

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
  //  AlbertaGridLeafIterator
  //  --LeafIterator
  //
  //**********************************************************************
  //! LeafIterator which is just a hull for the LevelIterator
  template<class GridImp>
  class AlbertaGridLeafIterator :
    public LeafIteratorDefault <GridImp,AlbertaGridLeafIterator>
  {
    AlbertaGridLevelIterator<0,All_Partition,GridImp> it;
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;

    //! Constructor making end iterator
    AlbertaGridLeafIterator(const GridImp & grid, int level, int proc) :
      it(grid,level,proc,true) {}

    //! Constructor making begin iterator
    AlbertaGridLeafIterator(const GridImp & grid,
                            AlbertaMarkerVector * vec,
                            int level,
                            int proc) :
      it(grid,vec,level,proc,true) {}

    //! increment
    void increment() { it.increment(); }
    //! equality
    bool equals(const AlbertaGridLeafIterator<GridImp>& i) const
    {
      return it.equals(i.it);
    }

    //! dereferencing
    Entity& dereference() const
    {
      return it.dereference();
    }

    //! ask for level of entity
    int level () const
    {
      return it.level();
    }
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
  class AlbertaGrid
    : public GridDefault <dim,dimworld,albertCtype, AlbertaGrid<dim,dimworld> >
  {
    friend class AlbertaGridEntity <0,dim,const AlbertaGrid<dim,dimworld> >;
    //friend class AlbertaGridEntity <1,dim,dimworld>;
    //friend class AlbertaGridEntity <1 << dim-1 ,dim,dimworld>;
    friend class AlbertaGridEntity <dim,dim,const AlbertaGrid<dim,dimworld> >;


    // friends because of fillElInfo
    friend class AlbertaGridLevelIterator<0,All_Partition,AlbertaGrid<dim,dimworld> >;
    friend class AlbertaGridLevelIterator<1,All_Partition,AlbertaGrid<dim,dimworld> >;
    friend class AlbertaGridLevelIterator<2,All_Partition,AlbertaGrid<dim,dimworld> >;
    friend class AlbertaGridLevelIterator<3,All_Partition,AlbertaGrid<dim,dimworld> >;
    friend class AlbertaGridHierarchicIterator<AlbertaGrid<dim,dimworld> >;

    friend class AlbertaGridIntersectionIterator<AlbertaGrid<dim,dimworld> >;

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
    typedef GridTraits<dim,dimworld,Dune::AlbertaGrid<dim,dimworld> ,
        AlbertaGridGeometry,AlbertaGridEntity,
        AlbertaGridBoundaryEntity,AlbertaGridLevelIterator,
        AlbertaGridIntersectionIterator,AlbertaGridHierarchicIterator,
        AlbertaGridLeafIterator>  Traits;

    typedef typename Traits::LeafIterator LeafIterator;

    typedef AlbertaGridReferenceGeometry<dim,AlbertaGrid<dim,dimworld> > ReferenceGeometry;
    typedef AlbertaGridHierarchicIndexSet<dim,dimworld> HierarchicIndexSetType;
    typedef DefaultLevelIndexSet<AlbertaGrid<dim,dimworld> > LevelIndexSetType;

    typedef ObjectStream ObjectStreamType;
    //typedef AlbertaObjectStream ObjectStreamType;

    //! we always have dim+1 codimensions
    enum { numCodim = dim+1 };

    //! Constructor which reads an Albert Macro Triang file
    //! or given GridFile
    //! levInd = true means that a consecutive level index is generated
    //! if levInd == true the the element number of first macro element is
    //! set to 1 so hasLevelIndex_ can be identified we grid is read from
    //! file
    AlbertaGrid(const char* macroTriangFilename);

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
    //! 0 ... maxlevel with 0 the coarsest level.
    int maxlevel() const;

    //! Iterator to first entity of given codim on level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template codim<cd>::template partition<pitype>::LevelIterator
    lbegin (int level, int proc=-1) const;

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template codim<cd>::template partition<pitype>::LevelIterator
    lend (int level, int proc=-1) const;

    //! Iterator to first entity of given codim on level
    template<int cd>  typename Traits::template codim<cd>::
    template partition<All_Partition>::LevelIterator
    lbegin (int level, int proc=-1) const;

    //! one past the end on this level
    template<int cd>  typename Traits::template codim<cd>::
    template partition<All_Partition>::LevelIterator
    lend (int level, int proc=-1) const;

    //! return LeafIterator which points to first leaf entity
    template <PartitionIteratorType pitype>
    LeafIterator leafbegin ( int maxlevel, int proc = -1 ) const;

    //! return LeafIterator which points behind last leaf entity
    template <PartitionIteratorType pitype>
    LeafIterator leafend   ( int maxlevel, int proc = -1 ) const;

    //! return LeafIterator which points to first leaf entity
    LeafIterator leafbegin ( int maxlevel, int proc = -1 ) const;

    //! return LeafIterator which points behind last leaf entity
    LeafIterator leafend   ( int maxlevel, int proc = -1 ) const;

    /** \brief Number of grid entities per level and codim
     * because lbegin and lend are none const, and we need this methods
     * counting the entities on each level, you know.
     */
    int size (int level, int codim) const;

    /** \brief ghostSize is zero for this grid  */
    int ghostSize (int level, int codim) const { return 0; }

    /** \brief overlapSize is zero for this grid  */
    int overlapSize (int level, int codim) const { return 0; }

    //***************************************************************
    //  Interface for Adaptation
    //***************************************************************
    /*! \brief marks an element for refCount refines. if refCount is negative the
     *  element is coarsend -refCount times
     *  NOTE: if element was already marked for refinement then nothing
     *  happens if element is again marked for coarsen, refinement alsway
     *  counts more then coarsening
     *  mark returns true if element was marked, otherwise false */
    bool mark( int refCount , typename Traits::template codim<0>::EntityPointer & en );
    bool mark( int refCount , typename Traits::template codim<0>::Entity & en );

    //! uses the interface, mark on entity and refineLocal
    bool globalRefine(int refCount);

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
    /** \brief write Grid to file in specified FileFormatType */
    template <FileFormatType ftype>
    bool writeGrid( const char * filename, albertCtype time ) const;

    /** \brief read Grid from file filename and store time of mesh in time */
    template <FileFormatType ftype>
    bool readGrid( const char * filename, albertCtype & time );

    //! returns size of mesh include all levels
    //! max Index of grid entities with given codim
    //! for outside the min index is 0, the shift has to done inside
    //! the grid which is of minor cost
    int global_size (int codim) const;

    //! return number of my processor
    int myRank () const { return myRank_; };

    //! transform grid N = scalar * x + trans
    void setNewCoords(const FieldVector<albertCtype, dimworld> & trans, const albertCtype scalar);

    const HierarchicIndexSetType & hierarchicIndexSet () const { return hIndexSet_; }
    const LevelIndexSetType & levelIndexSet () const
    {
      if(!levelIndexSet_) levelIndexSet_ = new LevelIndexSetType (*this);
      return *levelIndexSet_;
    }

    //! access to mesh pointer, needed by some methods
    ALBERTA MESH* getMesh () const { return mesh_; };

    template <int cd>
    AlbertaGridEntity<cd,dim,const AlbertaGrid<dim,dimworld> >&
    getRealEntity(typename Traits::template codim<cd>::Entity& entity)
    {
      return entity.realEntity;
    }

  private:
    template <int cd>
    const AlbertaGridEntity<cd,dim,const AlbertaGrid<dim,dimworld> >&
    getRealEntity(const typename Traits::template codim<cd>::Entity& entity) const
    {
      return entity.realEntity;
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

  private:
    Array<int> ghostFlag_; // store ghost information

    // initialize of some members
    void initGrid(int proc);

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

    // needed for VertexIterator, mark on which element a vertex is treated
    AlbertaMarkerVector * vertexMarker_;

  private:
    //***********************************************************************
    //  MemoryManagement for Entitys and Geometrys
    //**********************************************************************
    typedef AlbertaGridMakeableEntity<0,dim,const MyType>            EntityImp;
    typedef AlbertaGridMakeableGeometry<dim-1,dimworld,const MyType> GeometryImp;

  public:
    typedef MemoryProvider< EntityImp > EntityProvider;
    typedef MemoryProvider< GeometryImp > IntersectionSelfProvider;
    typedef MemoryProvider< GeometryImp > IntersectionNeighProvider;

    mutable EntityProvider entityProvider_;
    mutable IntersectionSelfProvider interSelfProvider_;
    mutable IntersectionNeighProvider interNeighProvider_;

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
    bool checkElNew ( ALBERTA EL * el ) const;

    // read global element number from elNumbers_
    int getElementNumber ( ALBERTA EL * el ) const;

    // read global element number from elNumbers_
    int getEdgeNumber ( ALBERTA EL * el, int edge ) const;

    // read global element number from elNumbers_
    int getVertexNumber ( ALBERTA EL * el, int vx ) const;

    //********************************************************************
    //  organisation of the parallelisation
    //********************************************************************

    // set owner of element, for partioning
    bool setOwner ( ALBERTA EL * el , int proc );

    // return the processor number of element
    int getOwner ( ALBERTA EL * el ) const;

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

    // the level index set, is generated from the HierarchicIndexSet
    // is generated, when accessed
    mutable LevelIndexSetType * levelIndexSet_;

  }; // end Class AlbertaGridGrid

  template <class GridType, int dim> struct MarkEdges;

  template <int dim, int dimworld>
  class AlbertaGridHierarchicIndexSet
  {
    typedef AlbertaGrid<dim,dimworld> GridType;
    typedef typename GridType :: Traits :: template codim<0>::Entity EntityCodim0Type;
    enum { numVecs  = AlbertHelp::numOfElNumVec };
    enum { numCodim = dim + 1 };

    template <int cd>
    struct codim
    {
      typedef AlbertaGridEntity<cd,dim,const GridType> RealEntityType;
      typedef typename Dune::Entity<cd,dim,const GridType,AlbertaGridEntity> EntityType;
    };

    // all classes that are allowed to call private functions
    friend class AlbertaGrid<dim,dimworld>;
    friend class MarkEdges<GridType,3>;
    friend class MarkEdges<const GridType,3>;

  public:
    AlbertaGridHierarchicIndexSet(const GridType & grid , const int (& s)[numCodim])
      : grid_( grid ), size_(s) {}

    template <class EntityType>
    int index (const EntityType & ep) const
    {
      enum { cd = EntityType :: codimension };
      const AlbertaGridEntity<cd,dim,const GridType> & en = (grid_.template getRealEntity<cd>(ep));
      return getIndex(en.getElInfo()->el, en.getFEVnum(),Int2Type<dim-cd>());
    }

    template <int cd>
    int subIndex (const EntityCodim0Type & en, int i) const
    {
      assert(cd == dim);
      return getIndex((grid_.template getRealEntity<0>(en)).getElInfo()->el
                      ,i,Int2Type<dim-cd>());
    }

    int size ( int level, int codim ) const
    {
      assert(size_[codim] >= 0);
      return size_[codim];
    }

  private:
    const GridType & grid_;
    const int * elNumVec_[numVecs];
    const int (& size_)[numCodim];
    int nv_[numVecs];
    int dof_[numVecs];

    // update vec pointer of the DOF_INT_VECs, which can change during resize
    void updatePointers(ALBERTA AlbertHelp::DOFVEC_STACK & dofvecs)
    {
      for(int i=0; i<numVecs; i++)
      {
        elNumVec_[i] = (dofvecs.elNumbers[i])->vec;
        assert(elNumVec_[i]);
      }

      setDofIdentifier<0> (dofvecs);
      if(numVecs > 1) setDofIdentifier<1> (dofvecs);
      if(numVecs > 2) setDofIdentifier<2> (dofvecs);
      if(numVecs > 3) setDofIdentifier<3> (dofvecs);
    }

    template <int cd>
    void setDofIdentifier (ALBERTA AlbertHelp::DOFVEC_STACK & dofvecs)
    {
      const ALBERTA DOF_ADMIN * elAdmin_ = dofvecs.elNumbers[cd]->fe_space->admin;
      // see Albert Doc. , should stay the same

      nv_ [cd] = elAdmin_->n0_dof    [ALBERTA AlbertHelp::AlbertaDofType<cd>::type];
      dof_[cd] = elAdmin_->mesh->node[ALBERTA AlbertHelp::AlbertaDofType<cd>::type];
    }

    // codim = 0 means we get from dim-cd = dim
    int getIndex ( const ALBERTA EL * el, int i , Int2Type<dim> fake ) const
    {
      enum { cd = 0 };
      assert(el);
      return elNumVec_[cd][ el->dof[ dof_[cd] ][nv_[cd]] ];
    }

    enum { cd1 = (dim > 1) ? 1 : 5 };
    // codim = 0 means we get from dim-cd = dim
    int getIndex ( const ALBERTA EL * el, int i , Int2Type<cd1> fake ) const
    {
      enum { cd = 1 };
      assert(el);
      // dof_[cd] marks the insertion point form which this dofs start
      // then i is the i-th dof
      return elNumVec_[cd][ el->dof[ dof_[cd] + i ][ nv_[cd] ] ];

      //int idx = elNumVec_[cd][ el->dof[ dof_[cd]+i ][nv_[cd]] ];
      //return idx;
    }

    enum { cd2 = (dim > 2) ? 2 : 6 };
    // codim = 0 means we get from dim-cd = dim
    // this method we have only in 3d
    int getIndex ( const ALBERTA EL * el, int i , Int2Type<cd2> fake ) const
    {
      enum { cd = 2 };
      assert(el);
      // dof_[cd] marks the insertion point form which this dofs start
      // then i is the i-th dof
      //return elNumVec_[cd][ el->dof[ dof_[cd] + i ][ nv_[cd] ] ];
      return 0;
    }

    // codim = dim  means we get from dim-cd = 0
    int getIndex ( const ALBERTA EL * el, int i , Int2Type<0> fake ) const
    {
      assert(el);
      return (el->dof[i][0]);
    }
  };


  // Class to mark the Vertices on the leaf level
  // to visit every vertex only once
  // for the LevelIterator codim == dim
  class AlbertaMarkerVector
  {
    friend class AlbertaGrid<2,2>;
    friend class AlbertaGrid<2,3>;
    friend class AlbertaGrid<3,3>;

    enum { MAXL = 64 };
    enum { vxBufferSize_ = 10000 };
  public:
    AlbertaMarkerVector () : up2Date_(false) {} ;

    bool notOnThisElement(ALBERTA EL * el, int elIndex, int level , int vertex);
    bool edgeNotOnElement(ALBERTA EL * el, int elIndex, int level , int edgenum);

    template <class GridType>
    void markNewVertices(GridType &grid);

    bool up2Date () const { return up2Date_; }
    void unsetUp2Date () { up2Date_ = false; }

    void print();

  private:
    // built in array to mark on which element a vertex is reached
    Array<int> vec_[MAXL];
    Array<int> edgevec_[MAXL];
    // number of vertices
    int numVertex_;

    // true is vertex marker is up to date
    bool up2Date_;
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
    struct hasEntity< AlbertaGrid<dim,dimw>, AlbertaGridEntity<cdim,dim,AlbertaGrid<dim,dimw> > >
    {
      static const bool v = true;
    };
  }

}; // namespace Dune

#include "albertagrid/agmemory.hh"
#include <dune/io/file/asciiparser.hh>
#include <dune/io/file/grapedataio.hh>
#include "albertagrid/albertagrid.cc"

#endif
