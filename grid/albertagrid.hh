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
#include <dune/common/misc.hh>
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/array.hh>
#include <dune/common/capabilities.hh>
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

  template<int codim, int dim, class GridImp> class AlbertaGridEntity;
  template<int codim, PartitionIteratorType pitype, class GridImp> class AlbertaGridLevelIterator;
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
    FieldVector<albertCtype, cdim>& operator[] (int i) const;

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
    mutable albertCtype elDet_; //!< storage of element determinant

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

    //AlbertaGridMakeableEntity(const GridImp & grid, int level,
    //                 ALBERTA TRAVERSE_STACK * travStack) :
    //  EntityType (AlbertaGridEntity<codim, dim, GridImp>(grid,level,travStack)) {}

    AlbertaGridMakeableEntity(const GridImp & grid, int level) :
      GridImp::template codim<codim>::Entity (AlbertaGridEntity<codim, dim, GridImp>(grid,level)) {}

    // passing through mehtods
    void setTraverseStack (ALBERTA TRAVERSE_STACK *travStack)
    {
      this->realEntity.setTraverseStack(travStack);
    }

    void setElInfo (ALBERTA EL_INFO *elInfo, int elNum, int face, int edge, int vertex )
    {
      this->realEntity.setElInfo(elInfo,elNum,face,edge,vertex);
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

    int vx () const
    {
      return 0;
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
  template<int codim, int dim, class GridImp>
  class AlbertaGridEntity :
    public EntityDefault <codim,dim,GridImp,AlbertaGridEntity>
  {
    enum { dimworld = GridImp::dimensionworld };
    friend class AlbertaGrid < dim , dimworld >;
    friend class AlbertaGridEntity < 0, dim, GridImp>;
    friend class AlbertaGridLevelIterator < codim, All_Partition,GridImp>;
    //friend class AlbertaGridLevelIterator < dim, dim, dimworld>;
    friend class AlbertaGridMakeableEntity<codim,dim,GridImp>;
  public:

    typedef typename GridImp::template codim<codim>::Entity Entity;
    typedef typename GridImp::template codim<codim>::Geometry Geometry;
    typedef AlbertaGridMakeableGeometry<codim,dimworld,GridImp> GeometryImp;
    typedef typename GridImp::template codim<codim>::LevelIterator LevelIterator;

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

    //! local coordinates within father
    FieldVector<albertCtype, dim>& local () const ;

    // needed for the LevelIterator
    ALBERTA EL_INFO *getElInfo () const;
  private:
    // methods for setting the infos from the albert mesh
    void setTraverseStack (ALBERTA TRAVERSE_STACK *travStack);
    void setElInfo (ALBERTA EL_INFO *elInfo, int elNum, int face,
                    int edge, int vertex );
    // private Methods
    void makeDescription();

    const GridImp &grid_;

    // private Members
    ALBERTA EL_INFO *elInfo_;
    ALBERTA TRAVERSE_STACK * travStack_;

    //! level
    int level_;

    //! the current geometry
    mutable GeometryImp geo_;
    mutable bool builtgeometry_;       //!< true if geometry has been constructed

    mutable FieldVector<albertCtype, dim> localFatherCoords_;

    //! element number
    int elNum_;

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

    //! returns true if Entity has children
    bool hasChildren () const ;

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

    //! AlbertaGrid internal method for partitioning
    //! set processor number of this entity
    bool partition( int proc );

    //! return partition type of this entity ( see grid.hh )
    PartitionType partitionType() const;

    //! return true if this entity belong to master set of this grid
    bool master() const;

    //! return processor number where entity is master
    int owner () const;

    //! return the global unique index in grid , same as el_index
    int globalIndex() const ;

    int vx () const
    {
      return 0;
    }

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
                    int elNum = 0,
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
  template<int codim, PartitionIteratorType pitype, class GridImp>
  class AlbertaGridLevelIterator :
    public LevelIteratorDefault <codim,pitype,GridImp,AlbertaGridLevelIterator>
  {
    enum { dim = GridImp::dimension };
    friend class AlbertaGridEntity<2,dim,GridImp>;
    friend class AlbertaGridEntity<1,dim,GridImp>;
    friend class AlbertaGridEntity<0,dim,GridImp>;
    friend class AlbertaGrid < dim , GridImp::dimensionworld >;

    typedef AlbertaGridLevelIterator<codim,pitype,GridImp>  AlbertaGridLevelIteratorType;
  public:

    typedef typename GridImp::template codim<codim>::Entity Entity;
    typedef AlbertaGridMakeableEntity<codim,dim,GridImp> EntityImp;

    //! Constructor
    AlbertaGridLevelIterator(const GridImp & grid, int
                             travLevel, int proc, bool leafIt=false );

    //! Constructor
    AlbertaGridLevelIterator(const GridImp & grid,
                             int travLevel,
                             ALBERTA EL_INFO *elInfo,int elNum = 0,
                             int face=0,
                             int edge=0,
                             int vertex=0);

    //! Constructor
    AlbertaGridLevelIterator(const GridImp & grid,
                             AlbertaMarkerVector * vec,
                             int travLevel,
                             int proc,
                             bool leafIt=false);

    //! increment
    void increment();
    //! equality
    bool equals(const AlbertaGridLevelIterator<codim,pitype,GridImp>& i) const;
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
  class AlbertaGrid
    : public GridDefault <dim,dimworld,albertCtype, AlbertaGrid<dim,dimworld> >
  {
    friend class AlbertaGridEntity <0,dim,AlbertaGrid<dim,dimworld> >;
    //friend class AlbertaGridEntity <1,dim,dimworld>;
    //friend class AlbertaGridEntity <1 << dim-1 ,dim,dimworld>;
    friend class AlbertaGridEntity <dim,dim,AlbertaGrid<dim,dimworld> >;

    friend class AlbertaMarkerVector;

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
    //**********************************************************
    // The Interface Methods
    //**********************************************************
  public:
    typedef GridTraits<dim,dimworld,Dune::AlbertaGrid<dim,dimworld> ,AlbertaGridGeometry,AlbertaGridEntity,
        AlbertaGridBoundaryEntity,AlbertaGridLevelIterator,AlbertaGridIntersectionIterator,
        AlbertaGridHierarchicIterator>  Traits;

    typedef typename Traits::template codim<0>::template partition<All_Partition>::LevelIterator LeafIterator;

    typedef AlbertaGridReferenceGeometry<dim,AlbertaGrid<dim,dimworld> > ReferenceGeometry;
    typedef AlbertaGridHierarchicIndexSet<dim,dimworld> HierarchicIndexSetType;
    typedef DefaultLevelIndexSet< AlbertaGrid<dim,dimworld> > LevelIndexSetType;


    typedef ObjectStream ObjectStreamType;
    //typedef std::pair < ObjectStreamType * ,
    //  Traits::template codim<0>::Entity  * > DataCollectorParamType;


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

    /** \brief Number of grid entities per level and codim
     * because lbegin and lend are none const, and we need this methods
     * counting the entities on each level, you know.
     */
    int size (int level, int codim) const;

    //***************************************************************
    //  Interface for Adaptation
    //***************************************************************
    /*! \brief marks an element for refCount refines. if refCount is negative the
     *  element is coarsend -refCount times
     * mark returns true if element was marked, otherwise false */
    bool mark( int refCount , typename Traits::template codim<0>::EntityPointer & en );
    bool mark( int refCount , typename Traits::template codim<0>::Entity & en );

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
    LeafIterator leafbegin ( int maxlevel, int proc = -1 ) const;

    //! return LeafIterator which points behind last leaf entity
    LeafIterator leafend   ( int maxlevel, int proc = -1 ) const;

    //! returns size of mesh include all levels
    //! max Index of grid entities with given codim
    //! for outside the min index is 0, the shift has to done inside
    //! the grid which is of minor cost
    int global_size (int codim) const;

    //! return number of my processor
    int myProcessor () const { return myProc_; };

    //! transform grid N = scalar * x + trans
    void setNewCoords(const FieldVector<albertCtype, dimworld> & trans, const albertCtype scalar);

    const HierarchicIndexSetType & hierarchicIndexSet () const { return hIndexSet_; }
    const LevelIndexSetType & levelIndexSet () const
    {
      if(!levelIndexSet_) levelIndexSet_ = new LevelIndexSetType (*this);
      return *levelIndexSet_;
    }

    //private:
    //! access to mesh pointer, needed by some methods
    ALBERTA MESH* getMesh () const { return mesh_; };

    template<int cd>
    AlbertaGridEntity<cd,dim,const AlbertaGrid<dim,dimworld> >&
    getRealEntity(typename Traits::template codim<cd>::Entity& e )
    {
      return e.realEntity;
    }

    template<int cd>
    const AlbertaGridEntity<cd,dim,const AlbertaGrid<dim,dimworld> >&
    getRealEntity(const typename Traits::template codim<cd>::Entity& e )
    {
      return e.realEntity;
    }

    template<int codim>
    AlbertaGridEntity<codim,dim,const AlbertaGrid<dim,dimworld> >&
    getRealEntity(AlbertaGridEntity<codim,dim,const AlbertaGrid<dim,dimworld> > & e )
    {
      return e;
    }

    template<int codim>
    const AlbertaGridEntity<codim,dim,const AlbertaGrid<dim,dimworld> >&
    getRealEntity(const AlbertaGridEntity<codim,dim,const AlbertaGrid<dim,dimworld> > & e ) const
    {
      return e;
    }

    //! number of grid entities per level and codim, size is cached
    template <int codim> int calcLevelSize (int level);
    int calcLevelCodimSize (int level, int codim);

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
    //  MemoryManagement for Entitys and Geometrys
    //**********************************************************************
    typedef AlbertaGridMakeableEntity<0,dim,const MyType>            EntityImp;
    typedef AlbertaGridMakeableGeometry<dim-1,dimworld,const MyType> GeometryImp;

    typedef MemoryProvider< EntityImp > EntityProvider;
    typedef MemoryProvider< GeometryImp > IntersectionSelfProvider;
    typedef MemoryProvider< GeometryImp > IntersectionNeighProvider;

    mutable EntityProvider entityProvider_;
    mutable IntersectionSelfProvider interSelfProvider_;
    mutable IntersectionNeighProvider interNeighProvider_;

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

    // the hierarchical numbering of AlbertaGrid, unique per codim and processor
    AlbertaGridHierarchicIndexSet<dim,dimworld> hIndexSet_;

    // the level index set, is generated from the HierarchicIndexSet
    // is generated, when accessed
    mutable LevelIndexSetType * levelIndexSet_;

  }; // end Class AlbertaGridGrid

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

    friend class AlbertaGrid<dim,dimworld>;
  public:
    AlbertaGridHierarchicIndexSet( const GridType & grid , const int (& s)[numCodim])
      : grid_( grid ), size_(s) {}

    //template <class EntityType>
    template <int cd>
    int index (const typename Dune::Entity<cd,dim,const GridType,AlbertaGridEntity> & ep) const
    {
      typedef typename codim<cd>::EntityType EntityType;
      enum { cd = EntityType :: codimension };
      typedef typename codim<cd> :: RealEntityType REnType;
      const REnType & en = grid_.template getRealEntity<cd>(ep);
      return getIndex(
               en.getElInfo()->el,
               en.vx(),
               Int2Type<dim-cd>());
    }

    template <int cd>
    int index (typename Dune::Entity<cd,dim,const GridType,AlbertaGridEntity> & ep) const
    {
      typedef typename codim<cd>::EntityType EntityType;
      enum { cd = EntityType :: codimension };
      typedef typename codim<cd> :: RealEntityType REnType;
      const REnType & en = grid_.template getRealEntity<cd>(ep);
      return getIndex(
               en.getElInfo()->el,
               en.vx(),
               Int2Type<dim-cd>());
    }

    /*
       template <class EntityType>
       int index (const EntityType & ep) const
       {
       enum { cd = EntityType :: codimension };
       typedef typename codim<cd> :: RealEntityType REnType;
          REnType & en = grid_.template getRealEntity<cd>(ep);

       return getIndex(
                en.getElInfo()->el,
                en.vx(),
                Int2Type<dim-cd>()
                );
       }
     */

    template <int codim>
    int subIndex (const EntityCodim0Type & en, int i)
    {
      return 0;
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

    void initializePointers(ALBERTA AlbertHelp::DOFVEC_STACK & dofvecs)
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

      std::cout << "nv = " << nv_[cd] << "  dof = " << dof_[cd] << "\n";
    }

    // codim = 0 means we get from dim-cd = dim
    int getIndex ( const ALBERTA EL * el, int i , Int2Type<dim> fake ) const
    {
      enum { codim = 0 };
      assert(el);
      int idx = elNumVec_[0][ el->dof[dof_[codim]][nv_[codim]] ];
      std::cout << "Returning " << idx << "\n";
      return idx;
    }

    enum { cd1 = (dim > 1) ? 1 : 5 };
    // codim = 0 means we get from dim-cd = dim
    int getIndex ( const ALBERTA EL * el, int i , Int2Type<cd1> fake ) const
    {
      return 0;
    }

    enum { cd2 = (dim > 2) ? 2 : 6 };
    // codim = 0 means we get from dim-cd = dim
    int getIndex ( const ALBERTA EL * el, int i , Int2Type<cd2> fake ) const
    {
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
    struct hasEntity< AlbertaGrid<dim,dimw>, AlbertaGridEntity<cdim,dim,AlbertaGrid<dim,dimw> > >
    {
      static const bool v = true;
    };
  }

}; // namespace Dune

#include "albertagrid/agmemory.hh"
#include <dune/io/file/asciiparser.hh>
#include "albertagrid/albertagrid.cc"

#endif
