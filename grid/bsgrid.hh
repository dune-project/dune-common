// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_BSGRID_HH__
#define __DUNE_BSGRID_HH__

#include <vector>
#include <assert.h>

#include <dune/common/misc.hh>
#include <dune/common/matvec.hh>

#include "common/grid.hh"

#include "bsgrid/bsinclude.hh"
#include "bsgrid/myautoptr.hh"

namespace Dune
{

  /** @defgroup BSGrid BSGrid Module
     Adaptive parallel grid supporting dynamic load balancing, written by
     Bernard Schupp.
     @{
   */

  // i.e. double or float
  typedef double bs_ctype;

  template<int codim, int dim, int dimworld> class BSGridEntity;
  template<int codim, int dim, int dimworld,PartitionIteratorType pitype> class BSGridLevelIterator;

  template<int dim, int dimworld>            class BSGridElement;
  template<int dim, int dimworld>            class BSGridBoundaryEntity;
  template<int dim, int dimworld>            class BSGridHierarchicIterator;
  template<int dim, int dimworld>            class BSGridIntersectionIterator;
  template<int dim, int dimworld>            class BSGrid;


  // singleton holding reference elements
  template<int dim> struct BSGridReferenceElement;

  //**********************************************************************
  //
  // --BSGridElement
  // --Element
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
  class BSGridElement :
    public ElementDefault <dim,dimworld,bs_ctype,BSGridElement>
  {
    friend class BSGridBoundaryEntity<dim,dimworld>;

    //! know dimension of barycentric coordinates
    enum { dimbary=dim+1};
  public:

    //! for makeRefElement == true a Element with the coordinates of the
    //! reference element is made
    BSGridElement(bool makeRefElement=false);

    //! return the element type identifier
    //! line , triangle or tetrahedron, depends on dim
    ElementType type ();

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners ();

    //! access to coordinates of corners. Index is the number of the corner
    FieldVector<bs_ctype, dimworld>& operator[] (int i);

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned.
     */
    static BSGridElement<dim,dim>& refelem ();

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    FieldVector<bs_ctype, dimworld> global (const FieldVector<bs_ctype, dim>& local);

    //! maps a global coordinate within the element to a
    //! local coordinate in its reference element
    FieldVector<bs_ctype, dim> local (const FieldVector<bs_ctype, dimworld>& global);

    //! returns true if the point in local coordinates is inside reference element
    bool checkInside(const FieldVector<bs_ctype, dim>& local);

    /*!
       Copy from grid.hh:

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

    //! A(l)
    bs_ctype integration_element (const FieldVector<bs_ctype, dim>& local);

    //! can only be called for dim=dimworld!
    Mat<dim,dim>& Jacobian_inverse (const FieldVector<bs_ctype, dim>& local);

    //***********************************************************************
    //!  Methods that not belong to the Interface, but have to be public
    //***********************************************************************
    //! generate the geometry for the ALBERT EL_INFO
    //! no interface method
    bool builtGeom(const BSSPACE HElementType & item);
    bool builtGeom(const BSSPACE HFaceType & item);

    //! print internal data
    //! no interface method
    void print (std::ostream& ss, int indent);

  private:
    // generate Jacobian Inverse and calculate integration_element
    void buildJacobianInverse();

    // calculates the element matrix for calculation of the jacobian inverse
    void calcElMatrix ();

    // element type of element
    ElementType eltype_;

    //! the vertex coordinates
    Mat<dimworld,dim+1,bs_ctype> coord_;

    //! is true if Jinv_, A and detDF_ is calced
    bool builtinverse_;
    bool builtA_;
    bool builtDetDF_;

    Mat<dim,dim,bs_ctype> Jinv_; //!< storage for inverse of jacobian
    bs_ctype detDF_;             //!< storage of integration_element
    Mat<dimworld,dim,bs_ctype> A_; //!< for map2world NORMALE Nr.(ZEILE/SPALTE)

    FieldVector<bs_ctype, dimworld> tmpVec_;
  };


  //**********************************************************************
  //
  // --BSGridEntity
  // --Entity
  //
  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Here: the general template
   */
  template<int codim, int dim, int dimworld>
  class BSGridEntity :
    public EntityDefault <codim,dim,dimworld,bs_ctype,
        BSGridEntity,BSGridElement,BSGridLevelIterator,
        BSGridIntersectionIterator,BSGridHierarchicIterator>
  {
    friend class BSGrid < dim , dimworld >;
    friend class BSGridEntity < 0, dim, dimworld>;
    friend class BSGridLevelIterator < codim, dim, dimworld, All_Partition>;
    //friend class BSGridLevelIterator < dim, dim, dimworld>;
  public:

    //! level of this element
    int level ();

    //! index is unique and consecutive per level and codim
    //! used for access to degrees of freedom
    int index ();

    //! index is unique within the grid hierachie and per codim
    int global_index ();

    //! Constructor
    BSGridEntity(BSGrid<dim,dimworld> &grid, int level);

    //! geometry of this entity
    BSGridElement<dim-codim,dimworld>& geometry ();

    /*! Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    BSGridLevelIterator<0,dim,dimworld,All_Partition> father ();

    //! local coordinates within father
    FieldVector<bs_ctype, dim>& local ();
  private:

    BSGrid<dim,dimworld> &grid_;

    Mat<dimworld,dim+1,bs_ctype> coord_; //

    //! the cuurent geometry
    BSGridElement<dim-codim,dimworld> geo_;
    bool builtgeometry_;       //!< true if geometry has been constructed

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
  //  --BSGridEntity
  //  --0Entity
  //
  //***********************
  template<int dim, int dimworld>
  class BSGridEntity<0,dim,dimworld> :
    public EntityDefault<0,dim,dimworld,bs_ctype,BSGridEntity,BSGridElement,
        BSGridLevelIterator,BSGridIntersectionIterator,
        BSGridHierarchicIterator>
  {
    friend class BSGrid < dim , dimworld >;
    friend class BaumMarkerVector;
    friend class BSGridIntersectionIterator < dim, dimworld>;
    friend class BSGridHierarchicIterator < dim, dimworld>;
    friend class BSGridLevelIterator <0,dim,dimworld,All_Partition>;
  public:
    typedef BSGridIntersectionIterator<dim,dimworld> IntersectionIterator;
    typedef BSGridHierarchicIterator<dim,dimworld> HierarchicIterator;

    //! Destructor, needed perhaps needed for deleteing faceEntity_ and
    //! edgeEntity_ , see below
    //! there are only implementations for dim==dimworld 2,3
    ~BSGridEntity() {};

    //! Constructor, real information is set via setElInfo method
    BSGridEntity(BSGrid<dim,dimworld> &grid,
                 BSSPACE HElementType &element,int index, int wLevel);

    //! Default-Constructor
    BSGridEntity(BSGrid<dim,dimworld> &grid) : grid_(grid) { }

    //! set original element pointer to fake entity
    void setelement(BSSPACE HElementType &element,int index);

    //! level of this element
    int level ();

    //! index is unique and consecutive per level and codim used for access to degrees of freedo
    int index ();

    //! index is unique within the grid hierachie and per codim
    int global_index ();

    //! geometry of this entity
    BSGridElement<dim,dimworld>& geometry ();

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
       with codimension cc.
     */
    //!< Default codim 1 Faces and codim == dim Vertices
    template<int cc> int count () { return cc+1; }

    //! return index of sub entity with codim = cc and local number i
    //! i.e. return global number of vertex i
    template<int cc> int subIndex (int i)  {  return 0; }

    //! Provide access to mesh entity i of given codimension. Entities
    //!  are numbered 0 ... count<cc>()-1
    template<int cc> BSGridLevelIterator<cc,dim,dimworld,All_Partition> entity (int i);

    /*! Intra-level access to intersection with neighboring elements.
       A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor. */
    BSGridIntersectionIterator<dim,dimworld> ibegin ();

    //! Reference to one past the last intersection with neighbor
    BSGridIntersectionIterator<dim,dimworld> iend ();

    void ibegin (BSGridIntersectionIterator<dim,dimworld> & it);
    void iend   (BSGridIntersectionIterator<dim,dimworld> & it);

    //! returns true if Entity has children
    bool hasChildren ();

    //! return number of layers far from refined elements of this level
    int refDistance ();

    //! Inter-level access to father element on coarser grid.
    //! Assumes that meshes are nested.
    BSGridLevelIterator<0,dim,dimworld,All_Partition> father ();

    BSGridEntity<0,dim,dimworld> newEntity();
    void father ( BSGridEntity<0,dim,dimworld> &vati);

    /*! Location of this element relative to the reference element
       of the father. This is sufficient to interpolate all
       dofs in conforming case. Nonconforming may require access to
       neighbors of father and computations with local coordinates.
       On the fly case is somewhat inefficient since dofs  are visited
       several times. If we store interpolation matrices, this is tolerable.
       We assume that on-the-fly implementation of numerical algorithms
       is only done for simple discretizations. Assumes that meshes are nested.
     */
    BSGridElement<dim,dim>& father_relative_local ();

    /*! Inter-level access to son elements on higher levels<=maxlevel.
       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
     */
    BSGridHierarchicIterator<dim,dimworld> hbegin (int maxlevel);

    //! Returns iterator to one past the last son
    BSGridHierarchicIterator<dim,dimworld> hend (int maxlevel);

    //***************************************************************
    //  Interface for Adaptation
    //***************************************************************

    //! marks an element for refCount refines. if refCount is negative the
    //! element is coarsend -refCount times
    //! mark returns true if element was marked, otherwise false
    bool mark( int refCount );

    //! return whether entity could be cosrsend (COARSEND) or was refined
    //! (REFINED) or nothing happend (NONE)
    AdaptationState state () const;

  private:
    BSGrid<dim,dimworld> &grid_;
    BSSPACE HElementType *item_;

    //! the cuurent geometry
    BSGridElement<dim,dimworld> geo_;

    int walkLevel_; //! tells the actual level of walk put to LevelIterator..

    bool builtgeometry_; //!< true if geometry has been constructed
    int index_;

    //BSGridElement <dim,dim> fatherReLocal_;
  }; // end of BSGridEntity codim = 0

  //**********************************************************************
  //
  // --BSGridHierarchicIterator
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
  class BSGridHierarchicIterator :
    public HierarchicIteratorDefault <dim,dimworld,bs_ctype,
        BSGridHierarchicIterator,BSGridEntity>
  {
  public:
    //! the normal Constructor
    BSGridHierarchicIterator(BSGrid<dim,dimworld> &grid,
                             BSSPACE HElementType & elem,int maxlevel, bool end=false);

    //! prefix increment
    BSGridHierarchicIterator& operator ++();

    //! equality
    bool operator== (const BSGridHierarchicIterator& i) const;

    //! inequality
    bool operator!= (const BSGridHierarchicIterator& i) const;

    //! dereferencing
    BSGridEntity<0,dim,dimworld>& operator*();

    //! arrow
    BSGridEntity<0,dim,dimworld>* operator->();

  private:
    // go to next valid element
    BSSPACE HElementType * goNextElement (BSSPACE HElementType * oldEl);

    BSGrid<dim,dimworld> &grid_; //!< the corresponding BSGrid
    BSSPACE HElementType & elem_; //!< the start  element of this iterator
    BSSPACE HElementType * item_; //!< the actual element of this iterator
    int maxlevel_; //!< maxlevel

    // holds the entity, copy pointer and delete if no refcount is left
    AutoPointer< BSGridEntity<0,dim,dimworld> > objEntity_;
  };


  /** \todo Please doc me! */
  template<int dim, int dimworld>
  class BSGridBoundaryEntity
    : public BoundaryEntityDefault <dim,dimworld,bs_ctype,
          BSGridElement,BSGridBoundaryEntity>
  {
    friend class BSGridIntersectionIterator<dim,dimworld>;
  public:
    BSGridBoundaryEntity () : _geom (false) ,
                              _neigh (-1) {};

    //! return type of boundary , i.e. Neumann, Dirichlet ...
    BoundaryType type ()
    {
      return Dirichlet ;
    }

    int id () { return -1; }

    //! return true if geometry of ghost cells was filled
    bool hasGeometry () { return false; }

    //! return geometry of the ghost cell
    BSGridElement<dim,dimworld> geometry ()
    {
      return _geom;
    }

  private:
    int _neigh;

    // BSGrid<dim,dimworld> & _grid;
    BSGridElement<dim,dimworld> _geom;
  };

  //**********************************************************************
  //
  // --BSGridIntersectionIterator
  // --IntersectionIterator
  /*!
     Mesh entities of codimension 0 ("elements") allow to visit all neighbors, wh
     a neighbor is an entity of codimension 0 which has a common entity of codimens
     These neighbors are accessed via a IntersectionIterator. This allows the implement
     non-matching meshes. The number of neigbors may be different from the number o
     of an element!
   */
  template<int dim, int dimworld>
  class BSGridIntersectionIterator :
    public IntersectionIteratorDefault <dim,dimworld,bs_ctype,
        BSGridIntersectionIterator,BSGridEntity,
        BSGridElement, BSGridBoundaryEntity>
  {
    friend class BSGridEntity<0,dim,dimworld>;

  public:
    //! prefix increment
    BSGridIntersectionIterator& operator ++();

    //! The default Constructor , level tells on which level we want
    //! neighbours
    BSGridIntersectionIterator(BSGrid<dim,dimworld> &grid,BSSPACE HElementType *el,
                               int wLevel,bool end=false);

    //! The Destructor
    ~BSGridIntersectionIterator();

    //! equality
    bool operator== (const BSGridIntersectionIterator& i) const;

    //! inequality
    bool operator!= (const BSGridIntersectionIterator& i) const;

    //! access neighbor, dereferencing
    BSGridEntity<0,dim,dimworld>& operator*();

    //! access neighbor, arrow
    BSGridEntity<0,dim,dimworld>* operator->();

    //! return true if intersection is with boundary. \todo connection with
    //! boundary information, processor/outer boundary
    bool boundary ();

    //! return true if across the edge an neighbor on this level exists
    bool neighbor ();

    //! return information about the Boundary
    BSGridBoundaryEntity<dim,dimworld> & boundaryEntity ();

    //! return unit outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    FieldVector<bs_ctype, dimworld>& unit_outer_normal (FieldVector<bs_ctype, dim-1>& local);

    //! return unit outer normal, if you know it is constant use this function instead
    FieldVector<bs_ctype, dimworld>& unit_outer_normal ();

    //! intersection of codimension 1 of this neighbor with element where
    //! iteration started.
    //! Here returned element is in LOCAL coordinates of the element
    //! where iteration started.
    BSGridElement<dim-1,dim>& intersection_self_local ();

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in GLOBAL coordinates of the element where iteration started.
    BSGridElement<dim-1,dimworld>& intersection_self_global ();

    //! local number of codim 1 entity in self where intersection is contained in
    int number_in_self ();

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    BSGridElement<dim-1,dim>& intersection_neighbor_local ();

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    BSGridElement<dim-1,dimworld>& intersection_neighbor_global ();

    //! local number of codim 1 entity in neighbor where intersection is contained
    int number_in_neighbor ();

    //! return outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    FieldVector<bs_ctype, dimworld>& outer_normal (FieldVector<bs_ctype, dim-1>& local);

    //! return unit outer normal, if you know it is constant use this function instead
    FieldVector<bs_ctype, dimworld>& outer_normal ();

  private:
    // if neighbour exists , do setup of new neighbour
    void setNeighbor ();

    // reset IntersectionIterator to first neighbour
    void first(BSSPACE HElementType & elem, int wLevel);

    // set behind last neighbour
    void done ();

    BSGridEntity<0,dim,dimworld> entity_; //! neighbour entity

    // current element from which we started the intersection iterator
    BSSPACE HElementType *item_;

    int index_; //! internal count of faces

    int count_; //! index of intersection

    bool theSituation_; //! true if the "situation" occurs :-)

    FieldVector<bs_ctype, dimworld> outerNormal_; //! outerNormal ro current intersection
    FieldVector<bs_ctype, dimworld> unitOuterNormal_;

    bool needSetup_; //! true if setup is needed
    bool needNormal_;

    // pair holding pointer to face and twist
    BSSPACE NeighbourPairType neighpair_;

    BSGridElement<dim-1,dimworld> interSelfGlobal_; //! intersection_self_global
  };



  //**********************************************************************
  //
  // --BSGridLevelIterator
  // --LevelIterator
  /*!
     Enables iteration over all entities of a given codimension and level of a grid.
   */
  template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
  class BSGridLevelIterator :
    public LevelIteratorDefault <codim,dim,dimworld,pitype,bs_ctype,
        BSGridLevelIterator,BSGridEntity>
  {
    friend class BSGridEntity<3,dim,dimworld>;
    friend class BSGridEntity<2,dim,dimworld>;
    friend class BSGridEntity<1,dim,dimworld>;
    friend class BSGridEntity<0,dim,dimworld>;
    friend class BSGrid < dim , dimworld >;
  public:
    //! typedef of my type
    typedef BSGridLevelIterator<codim,dim,dimworld,pitype> BSGridLevelIteratorType;

    //! Constructor
    BSGridLevelIterator(BSGrid<dim,dimworld> &grid, int level , bool end=false);

    //! Constructor for father
    BSGridLevelIterator(BSGrid<dim,dimworld> &grid, BSSPACE HElementType & item);

    //! prefix increment
    BSGridLevelIteratorType& operator ++();

    //! equality
    bool operator== (const BSGridLevelIteratorType& i) const;

    //! inequality
    bool operator!= (const BSGridLevelIteratorType& i) const;

    //! dereferencing
    BSGridEntity<codim,dim,dimworld>& operator*();

    //! arrow
    BSGridEntity<codim,dim,dimworld>* operator->();

    //! ask for level of entities
    int level ();

  private:
    // reference to grid
    BSGrid<dim,dimworld> &grid_;

    // element index, -1 for end
    int index_;

    // actual level
    int level_;

    // the original iterator of the BSGrid
    typename BSSPACE BSLevelIterator<codim>::IteratorType iter_;

    // holds the entity, copy pointer and delete if no refcount is left
    AutoPointer< BSGridEntity<codim,dim,dimworld> > objEntity_;
  };



  //**********************************************************************
  //
  // --BSGrid
  // --Grid
  //
  //**********************************************************************

  /** \brief The Baum %Grid class
   *
   * \todo Please doc me!
   */
  template <int dim, int dimworld>
  class BSGrid : public GridDefault  < dim, dimworld,
                     bs_ctype,BSGrid,
                     BSGridLevelIterator,BSGridEntity>
  {
    CompileTimeChecker<dim      == 3>   BSGrid_only_implemented_for_3dp;
    CompileTimeChecker<dimworld == 3>   BSGrid_only_implemented_for_3dw;

    friend class BSGridEntity <0,dim,dimworld>;
#if 0
    //friend class BSGridEntity <1,dim,dimworld>;
    //friend class BSGridEntity <1 << dim-1 ,dim,dimworld>;
    friend class BSGridEntity <dim,dim,dimworld>;

    friend class BaumMarkerVector;

    // friends because of fillElInfo
    friend class BSGridLevelIterator<0,dim,dimworld>;
    friend class BSGridLevelIterator<1,dim,dimworld>;
    friend class BSGridLevelIterator<2,dim,dimworld>;
    friend class BSGridLevelIterator<3,dim,dimworld>;
    friend class BSGridHierarchicIterator<dim,dimworld>;

    friend class BSGridIntersectionIterator<dim,dimworld>;
#endif

    //! BSGrid is only implemented for 2 and 3 dimension
    //! for 1d use SGrid or SimpleGrid
    //CompileTimeChecker<dimworld != 1>   Do_not_use_BSGrid_for_1d_Grids;

    //**********************************************************
    // The Interface Methods
    //**********************************************************
  public:
    class BSGridLeafIterator;

    typedef BSGridReferenceElement<dim> ReferenceElement;
    typedef BSGridLeafIterator LeafIterator;

    /** \todo Please doc me! */
    enum { numCodim = dim+1 };

    enum { MAXL = 64 };

    //! Constructor which reads an Baum Macro Triang file
    //! or given GridFile
    BSGrid(const char* macroTriangFilename);

    //! empty Constructor
    BSGrid();

    //! Desctructor
    ~BSGrid();

    //! for type identification
    GridIdentifier type ();

    //! Return maximum level defined in this grid. Levels are numbered
    //! 0 ... maxlevel with 0 the coarsest level.
    int maxlevel() const;

    //! Iterator to first entity of given codim on level
    template<int codim, PartitionIteratorType pitype>
    BSGridLevelIterator<codim,dim,dimworld,pitype> lbegin (int level);

    //! one past the end on this level
    template<int codim, PartitionIteratorType pitype>
    BSGridLevelIterator<codim,dim,dimworld,pitype> lend (int level);

    //! Iterator to first entity of given codim on level
    template<int codim>
    BSGridLevelIterator<codim,dim,dimworld,All_Partition> lbegin (int level);

    //! one past the end on this level
    template<int codim>
    BSGridLevelIterator<codim,dim,dimworld,All_Partition> lend (int level);

    //! Iterator to first entity of given codim on leaf level
    LeafIterator leafbegin (int level);

    //! one past the end on this leaf level
    LeafIterator leafend (int level);

    //! number of grid entities per level and codim
    int size (int level, int codim);
    int size (int level, int codim) const;

    //! number of grid entities on all levels for given codim
    int global_size (int codim);

    /** \brief Number of grid entities per level and codim
     * \todo Why is there a non-const version of this method?
     */
    //int size (int level, int codim) const;

    //! refine all positive marked leaf entities
    //! return true if the grid was changed
    bool adapt ( );

    //! coarsen all negative marked leaf entities
    //! return true if the grid was changed
    bool preAdapt ( );
    void postAdapt ( );

    /** \brief Please doc me! */
    //GridIdentifier type () { return BSGrid_Id; };

    //**********************************************************
    // End of Interface Methods
    //**********************************************************
    //! uses the interface, mark on entity and refineLocal
    bool globalRefine(int refCount);

    /** \brief write Grid to file in specified FileFormatType
     */
    template <FileFormatType ftype>
    bool writeGrid( const char * filename, bs_ctype time );

    /** \brief read Grid from file filename and store time of mesh in time
     */
    template <FileFormatType ftype>
    bool readGrid( const char * filename, bs_ctype & time );

    //! return current time of grid
    //! not an interface method yet
    bs_ctype getTime () const { return time_; };

    BSSPACE GitterBasisImpl *mygrid();

  private:
    void calcExtras();

    void calcMaxlevel();

    // set _coarsenMark to true
    void setCoarsenMark();

    BSSPACE GitterBasisImpl *mygrid_;

    // save size of grid
    mutable int size_[MAXL][dim+1];

    // save global_size of grid
    mutable int globalSize_[dim+1];

    // max level of grid
    int maxlevel_;

    // true if at least one element is marked for coarsening
    bool coarsenMark_;
  public:
    //template<int codim, int dim, int dimworld,PartitionIteratorType pitype>
    class BSGridLeafIterator
    {
      friend class BSGridEntity<0,dim,dimworld>;
      friend class BSGrid < dim , dimworld >;
    public:
      enum { codim = 0 };

      typedef BSGridLeafIterator BSGridLeafIteratorType;

      //! Constructor
      BSGridLeafIterator(BSGrid<dim,dimworld> &grid, int level , bool end=false);

      //! prefix increment
      BSGridLeafIteratorType& operator ++();

      //! equality
      bool operator== (const BSGridLeafIteratorType& i) const;

      //! inequality
      bool operator!= (const BSGridLeafIteratorType& i) const;

      //! dereferencing
      BSGridEntity<codim,dim,dimworld>& operator*();

      //! arrow
      BSGridEntity<codim,dim,dimworld>* operator->();
    private:
      //! ask for level of entities
      int level ();

      // element index, -1 for end
      int index_;

      // actual level
      int level_;

      // the original iterator of the BSGrid
      typename BSSPACE BSLeafIterator<codim>::IteratorType iter_;

      // holds the entity, copy pointer and delete if no refcount is left
      AutoPointer< BSGridEntity<codim,dim,dimworld> > objEntity_;
    };



  }; // end Class BSGridGrid


  /** @} end documentation group */

}; // namespace Dune

#include "bsgrid/bsgrid.cc"

#endif
