// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_GRID_HH__
#define __DUNE_GRID_HH__

#include <string>
#include <dune/common/matvec.hh>
#include <dune/common/exceptions.hh>

namespace Dune {


  /** @defgroup GridCommon Grid Interface

     The Dune Grid module defines a general interface to a hierarchical finite element mesh.
     The interface is independent of dimension and element type. Various implementations
     of this interface exits:

     - Structured Grid (SGrid) : A structured mesh in d dimensions consisting of "cubes". The number
     of elements per dimension is variable.

     - Albert Grid (AlbertGrid) : Provides the simplicial meshes of the finite element tool box ALBERT
     written by Kunibert Siebert and Alfred Schmidt.

     - UG Grid (UGGrid) : Provides the meshes of the finite element toolbox UG.

     This Module contains only the description of compounds that are common to all implementations
     of the grid interface.

     For a detailed description of the interface itself please see the documentation
     of the "Structured Grid Module". Since Dune uses the Engine concept there is no abstract definition
     of the interface. As with STL containers, all implementations must implement the
     same classes with exactly the same members to be used in generic algorithms.

     \sa AlbertGrid UGGrid SGrid YaspGrid


     @{
   */

  //************************************************************************
  /** \brief Identifiers for different element types.
   *
   * This list can be extended in the future. Not all meshes need to implement
   * all element types.
   */

  enum ElementType {vertex=0,line=1, triangle=2, quadrilateral=3, tetrahedron=4,
                    pyramid=5, prism=6, hexahedron=7,
                    iso_triangle=8, iso_quadrilateral=9, unknown=127};

  /*! \internal
        Used for grid I/O
   */
  enum GridIdentifier { SGrid_Id, AlbertGrid_Id , SimpleGrid_Id, UGGrid_Id,
                        YaspGrid_Id , BSGrid_Id, OneDGrid_Id};

  /*!
     Specify the format to store grid and vector data
   */
  enum FileFormatType { ascii , //!< store data in a human readable form
                        xdr ,   //!< store data in SUN's library routines
                                //!< for external data representation (xdr)
                        pgm };  //!< store data in portable graymap file format (pgm)

  enum AdaptationState {
    NONE ,            //!< notin' to do and notin' was done
    COARSEN,          //!< entity could be coarsend in adaptation step
    REFINED           //!< enity was refined in adaptation step
  };


  /*! Parameter to be used for the communication functions
   */
  enum InterfaceType {
    InteriorBorder_InteriorBorder_Interface=0,
    InteriorBorder_All_Interface=1,
    Overlap_OverlapFront_Interface=2,
    Overlap_All_Interface=3,
    All_All_Interface=4
  };

  /*! Parameter to be used for the parallel level iterators
   */
  enum PartitionIteratorType {
    Interior_Partition=0,
    InteriorBorder_Partition=1,
    Overlap_Partition=2,
    OverlapFront_Partition=3,
    All_Partition=4,
    Ghost_Partition=5
  };


  /*! Define a type for communication direction parameter
   */
  enum CommunicationDirection {
    ForwardCommunication,
    BackwardCommunication
  };

  /*! Attributes used in the generic overlap model
   */
  enum PartitionType {
    InteriorEntity=0,     //!< all interior entities
    BorderEntity=1  ,     //!< on boundary between interior and overlap
    OverlapEntity=2 ,     //!< all entites lying in the overlap zone
    FrontEntity=3  ,      //!< on boundary between overlap and ghost
    GhostEntity=4         //!< ghost entities
  };

  //! provide names for the partition types
  inline std::string PartitionName(PartitionType type)
  {
    switch(type) {
    case InteriorEntity :
      return "interior";
    case BorderEntity :
      return "border";
    case OverlapEntity :
      return "overlap";
    case FrontEntity :
      return "front";
    case GhostEntity :
      return "ghost";
    default :
      return "unknown";
    }
  }

  /*! GridIndexType specifies which Index of the Entities of the grid
        should be used, i.e. global_index() or index()
   */
  enum GridIndexType { GlobalIndex , //!< use global_index() of entity
                       LevelIndex                        //!< use index() of entity
  };

  //************************************************************************
  // G R I D E R R O R
  //************************************************************************

  /*!
     exceptions in Dune grid modules.
   */

  class GridError : public Exception {};

  //************************************************************************
  // R E F E R E N C E T O P O L O G Y
  //************************************************************************

  /*!
     The reference topology defines the numbering of all entities of an
     element as well as their position in the reference element.
   */
  template<int dim, class ct>
  class ReferenceTopology {
  public:
    //! default constructor will build up all the internal data
    ReferenceTopology ();

    //! return local coordinates of center in reference element
    FieldVector<ct, dim>& center_codim0_local (int elemtype);

    //! return local coordinates of center of ith codim 1 subentity
    FieldVector<ct, dim-1>& center_codim1_local (int elemtype, int i);
  };

  // Specialization dim=1
  template<class ct>
  class ReferenceTopology<1,ct> {
  public:
    //! default constructor will build up all the internal data
    ReferenceTopology ();

    //! return local coordinates of center in reference element
    FieldVector<ct, 1>& center_codim0_local (int elemtype);

    //! return local coordinates of center of ith codim 1 subentity
    FieldVector<ct, 0>& center_codim1_local (int elemtype, int i);

  private:
    FieldVector<ct, 1> center0_local[1];    // ElementType
    FieldVector<ct, 0> center1_local[1];    // ElementType x faces
  };

  // Specialization dim=2
  template<class ct>
  class ReferenceTopology<2,ct> {
  public:
    //! default constructor will build up all the internal data
    ReferenceTopology ();

    //! return local coordinates of center in reference element
    FieldVector<ct, 2>& center_codim0_local (int elemtype);

    //! return local coordinates of center of ith codim 1 subentity
    FieldVector<ct, 1>& center_codim1_local (int elemtype, int i);

  private:
    FieldVector<ct, 2> center0_local[2];    // ElementType
    FieldVector<ct, 1> center1_local[2];    // ElementType x faces
  };


  // Specialization dim=3
  template<class ct>
  class ReferenceTopology<3,ct> {
  public:
    //! default constructor will build up all the internal data
    ReferenceTopology ();

    //! return local coordinates of center in reference element
    FieldVector<ct, 3>& center_codim0_local (int elemtype);

    //! return local coordinates of center of ith codim 1 subentity
    FieldVector<ct, 2>& center_codim1_local (int elemtype, int i);

  private:
    FieldVector<ct, 3> center0_local[4];      // ElementType
    FieldVector<ct, 2> center1_local[4][6];   // ElementType x faces
  };


  //************************************************************************
  // E L E M E N T
  //************************************************************************

  /*!
     Element realizes the concept of the geometric part of a mesh entity.

     The geometric part of a mesh entity is a \f$d\f$-dimensional object in \f$\mathbf{R}^w\f$
     where \f$d\f$ corresponds the template parameter dim and \f$w\f$ corresponds to the
     template parameter dimworld.

     The \f$d\f$-dimensional object is a polyhedron given by a certain number of corners, which
     are vectors in \f$\mathbf{R}^w\f$.

     The member function global provides a map from a topologically equivalent polyhedron ("reference element")
     in \f$\mathbf{R}^d\f$ to the given polyhedron. This map can be inverted by the member function local, where
     an appropriate projection is applied first, when \f$d\neq w\f$.

     In the case of a structured mesh discretizing a generalized cube this map is linear
     and can be described as \f[ g(l) = s + \sum\limits_{i=0}^{d-1} l_ir^i\f] where \f$s\in\mathbf{R}^w\f$
     is a given position vector, the \f$r^i\in\mathbf{R}^w\f$ are given direction vectors and \f$l\in\mathbf{R}^d\f$
     is a local coordinate within the reference polyhedron. The direction vectors are assumed to be orthogonal
     with respect to the standard Euclidean inner product.

     The \f$d\f$-dimensional reference polyhedron is given
     by the points \f$\{ (x_0,\ldots,x_{d-1}) \ | \ x_i\in\{0,1\}\ \}\f$.

     In order to invert the map for a point \f$p\f$, we have to find a local coordinate \f$l\f$ such
     that \f$g(l)=p\f$. Of course this is only possible if \f$d=w\f$. In the general case \f$d\leq w\f$
     we determine \f$l\f$ such that
     \f[(s,r^k) + \sum\limits_{i=0}^{d-1} l_i (r^i,r^k) = (p,r^k) \ \ \ \forall k=0,\ldots,d-1. \f]

     The resulting system is diagonal since the direction vectors are required to be orthogonal.

     Elements with dimension 0 (i.e. vertices) are defined through specialization.
   */
  template<int dim, int dimworld, class ct,template<int,int> class ElementImp>
  class Element {
  public:
    //! remeber the template types
    struct Traits
    {
      typedef ct CoordType;
      typedef ElementImp<dim,dimworld>  Element;
    };

    //! know dimension
    enum { dimension=dim };

    //! know dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! return the element type identifier
    ElementType type () const;

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const;

    //! access to coordinates of corners. Index is the number of the corner
    FieldVector<ct, dimworld>& operator[] (int i) const;

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned. A reference to a reference
       element is returned. Usually, the implementation will store the finite
       set of reference elements as global variables.
     */
    ElementImp<dim,dim>& refelem () const;

    //! maps a local coordinate within reference element to global coordinate in element
    FieldVector<ct, dimworld> global (const FieldVector<ct, dim>& local) const;

    //! maps a global coordinate within the element to a local coordinate in its reference element
    FieldVector<ct, dim> local (const FieldVector<ct, dimworld>& global) const;

    //! return true if the point in local coordinates lies inside the reference element
    bool checkInside (const FieldVector<ct, dim>& local) const;

    /*! Integration over a general element is done by integrating over the reference element
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
    ct integration_element (const FieldVector<ct, dim>& local) const;

    //! can only be called for dim=dimworld!
    Mat<dim,dim,ct>& Jacobian_inverse (const FieldVector<ct, dim>& local) const;

    /*! \internal
       Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF () const DUNE_DEPRECATED;

  private:
    //!  Barton-Nackman trick
    ElementImp<dim,dimworld>& asImp () {return static_cast<ElementImp<dim,dimworld>&>(*this);}
  };

  //*****************************************************************************
  //
  //
  //
  //
  //*****************************************************************************
  template<int dim, int dimworld, class ct,template<int,int> class ElementImp>
  class ElementDefault : public Element <dim,dimworld,ct,ElementImp>
  {
  public:

  protected:

  private:
    //!  Barton-Nackman trick
    ElementImp<dim,dimworld>& asImp () {return static_cast<ElementImp<dim,dimworld>&>(*this);}
  }; // end ElementDefault
     //************************************************************

  //! Specialization of Element for dim=0 (vertices)
  template<int dimworld, class ct,template<int,int> class ElementImp>
  class Element<0,dimworld,ct,ElementImp> {
  public:
    //! remeber the template types
    struct Traits
    {
      typedef ct CoordType;
      typedef ElementImp<0,dimworld>  Element;
    };

    //! know dimension
    enum { dimension=0 };

    //! know dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! return the element type identifier
    ElementType type () const;

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const;

    //! access to coordinates of corners. Index is the number of the corner
    FieldVector<ct, dimworld>& operator[] (int i) const;

    /*! \internal Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF () const DUNE_DEPRECATED;

  private:
    //!  Barton-Nackman trick
    ElementImp<0,dimworld>& asImp () {return static_cast<ElementImp<0,dimworld>&>(*this);}
  };


  //*************************************************************************
  //
  // --ElementDefault
  //
  //! Default implementation for class Element with dim = 0 (vertices)
  //!
  //
  //*************************************************************************
  template<int dimworld, class ct,template<int,int> class ElementImp>
  class ElementDefault <0,dimworld,ct,ElementImp>
    : public Element<0,dimworld,ct,ElementImp>
  {
  public:
    //! remeber the template types
    struct Traits
    {
      typedef ct CoordType;
      typedef ElementImp<0,dimworld>  Element;
    };

    //! know dimension
    enum { dimension=0 };

    //! know dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;
  private:
    //!  Barton-Nackman trick
    ElementImp<0,dimworld>& asImp () {return static_cast<ElementImp<0,dimworld>&>(*this);}
  }; // end ElementDefault, dim = 0
     //****************************************************************************


  //********************************************************************
  //  --BoundaryEntity
  //
  /*!
     First Version of a BoundaryEntity which holds some information about
     the boundary on an intersection with the boundary or and ghost boundary
     cell.
   */
  //********************************************************************
  template<int dim , int dimworld, class ct,
      template<int,int> class ElementImp ,
      template<int,int> class BoundaryEntityImp >
  class BoundaryEntity
  {
  public:
    //! return id of booundary segment, any integer but != 0
    int id () const;

    //! return true if ghost boundary cell was generated
    bool hasGeometry () const;

    //! return geometry of ghost boundary cell
    ElementImp<dim,dimworld> & geometry ();

  private:
    //!  Barton-Nackman trick
    BoundaryEntityImp<dim,dimworld> & asImp ()
    {return static_cast<BoundaryEntityImp<dim,dimworld>&>(*this);}
  };

  //********************************************************************
  //
  // --BoundaryEntityDefault
  //
  //! Default implementations for the BoundaryEntity class.
  //********************************************************************
  template<int dim , int dimworld, class ct,
      template<int,int> class ElementImp  ,
      template<int,int> class BoundaryEntityImp>
  class BoundaryEntityDefault
    : public BoundaryEntity<dim,dimworld,ct,ElementImp,BoundaryEntityImp>
  {
  public:

  private:
    //!  Barton-Nackman trick
    BoundaryEntityImp<dim,dimworld> & asImp ()
    {return static_cast<BoundaryEntityImp<dim,dimworld>&>(*this);}
  };


  //************************************************************************
  // N E I G H B O R I T E R A T O R
  //************************************************************************

  /*! Mesh entities of codimension 0 ("elements") allow to visit all neighbors, where
     a neighbor is an entity of codimension 0 which has a common entity of codimension 1 with the entity.
     These neighbors are accessed via a IntersectionIterator. This allows the implementation of
     non-matching meshes. The number of neigbors may be different from the number of faces/edges
     of an element!
   */
  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp ,
      template<int,int> class BoundaryEntityImp
      >
  class IntersectionIterator
  {
  public:

    //! rember the template types
    struct Traits
    {
      typedef ct CoordType;
      typedef EntityImp<0,dim,dimworld>         Entity;
      typedef ElementImp<dim,dimworld>          Element;
      typedef IntersectionIteratorImp<dim,dimworld> IntersectionIterator;
      typedef BoundaryEntityImp<dim,dimworld>   BoundaryEntity;
    };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! prefix increment
    IntersectionIteratorImp<dim,dimworld>& operator++();

    //! equality
    bool operator== (const IntersectionIteratorImp<dim,dimworld>& i) const;

    //! inequality
    bool operator!= (const IntersectionIteratorImp<dim,dimworld>& i) const;

    //! access neighbor, dereferencing
    EntityImp<0,dim,dimworld>& operator*();

    //! access neighbor, arrow
    EntityImp<0,dim,dimworld>* operator->();

    //! return true if intersection is with boundary. \todo connection with boundary information, processor/outer boundary
    bool boundary ();

    //! return true if intersection is with neighbor on this level.
    bool neighbor ();

    BoundaryEntityImp<dim,dimworld> & boundaryEntity ();

    //! return unit outer normal, this should be dependent on local coordinates for higher order boundary
    FieldVector<ct, dimworld>& unit_outer_normal (FieldVector<ct, dim-1>& local);

    //! return unit outer normal, if you know it is constant use this function instead
    FieldVector<ct, dimworld>& unit_outer_normal ();

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of the element where iteration started.
     */
    ElementImp<dim-1,dim>& intersection_self_local ();

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in GLOBAL coordinates of the element where iteration started.
     */
    ElementImp<dim-1,dimworld>& intersection_self_global ();

    //! local number of codim 1 entity in self where intersection is contained in
    int number_in_self ();

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of neighbor
     */
    ElementImp<dim-1,dim>& intersection_neighbor_local ();

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of neighbor
     */
    ElementImp<dim-1,dimworld>& intersection_neighbor_global ();

    //! local number of codim 1 entity in neighbor where intersection is contained in
    int number_in_neighbor ();

    /*! \internal Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //!  Barton-Nackman trick
    IntersectionIteratorImp<dim,dimworld>& asImp ()
    {return static_cast<IntersectionIteratorImp<dim,dimworld>&>(*this);}
    const IntersectionIteratorImp<dim,dimworld>& asImp () const
    {return static_cast<const IntersectionIteratorImp<dim,dimworld>&>(*this);}
  };

  //**************************************************************************
  //
  // --IntersectionIteratorDefault
  //
  //! Default implementation for IntersectionIterator.
  //
  //**************************************************************************
  template<int dim, int dimworld, class ct,
      template<int,int> class IntersectionIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp ,
      template<int,int> class BoundaryEntityImp
      >
  class IntersectionIteratorDefault
    : public IntersectionIterator <dim,dimworld,ct,IntersectionIteratorImp,EntityImp,ElementImp,BoundaryEntityImp>
  {
  public:
    //! return outer normal, which is the unit_outer_normal() scaled with the
    //! volume of the intersection_self_global ()
    FieldVector<ct, dimworld>& outer_normal ();

    //! return unit outer normal, this should be dependent on
    //! local coordinates for higher order boundary
    //! the normal is scaled with the integration element
    FieldVector<ct, dimworld>& outer_normal (FieldVector<ct, dim-1>& local);

  protected:
    //! the outer normal
    FieldVector<ct, dimworld> outerNormal_;

    //! tmp Vec for integration_element
    FieldVector<ct, dim-1> tmp_;

  private:
    //!  Barton-Nackman trick
    IntersectionIteratorImp<dim,dimworld>& asImp ()
    {return static_cast<IntersectionIteratorImp<dim,dimworld>&>(*this);}
    const IntersectionIteratorImp<dim,dimworld>& asImp () const
    {return static_cast<const IntersectionIteratorImp<dim,dimworld>&>(*this);}
  }; // end IntersectionIteratorDefault

  //************************************************************************
  // H I E R A R C H I C I T E R A T O R
  //************************************************************************

  /*! Mesh entities of codimension 0 ("elements") allow to visit all entities of
     codimension 0 obtained through nested, hierarchic refinement of the entity.
     Iteration over this set of entities is provided by the HIerarchicIterator,
     starting from a given entity.
     This is redundant but important for memory efficient implementations of unstructured
     hierarchically refined meshes.
   */
  template<int dim, int dimworld, class ct,
      template<int,int> class HierarchicIteratorImp,
      template<int,int,int> class EntityImp
      >
  class HierarchicIterator
  {
  public:

    //! remember the template types
    struct Traits
    {
      typedef ct CoordType;
      typedef EntityImp<0,dim,dimworld>           Entity;
      typedef HierarchicIteratorImp<dim,dimworld> HierarchicIterator;
    };
    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! prefix increment
    HierarchicIteratorImp<dim,dimworld>& operator++();

    //! equality
    bool operator== (const HierarchicIteratorImp<dim,dimworld>& i) const;

    //! inequality
    bool operator!= (const HierarchicIteratorImp<dim,dimworld>& i) const;

    //! dereferencing
    EntityImp<0,dim,dimworld>& operator*();

    //! arrow
    EntityImp<0,dim,dimworld>* operator->();

    /*! \internal Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //!  Barton-Nackman trick
    HierarchicIteratorImp<dim,dimworld>& asImp ()
    {return static_cast<HierarchicIteratorImp<dim,dimworld>&>(*this);}
    const HierarchicIteratorImp<dim,dimworld>& asImp () const
    {return static_cast<const HierarchicIteratorImp<dim,dimworld>&>(*this);}
  };

  //***************************************************************************
  //
  // --HierarchicIteratorDefault
  //
  //! Default implementation of the HierarchicIterator.
  //! This class provides functionality which uses the interface of
  //! HierarchicIterator. For performance implementation the method of this
  //! class should be overloaded if a fast implementation can be done.
  //
  //***************************************************************************
  template<int dim, int dimworld, class ct,
      template<int,int> class HierarchicIteratorImp,
      template<int,int,int> class EntityImp
      >
  class HierarchicIteratorDefault
    : public HierarchicIterator <dim,dimworld,ct,HierarchicIteratorImp,EntityImp>
  {
  public:

    //! remember the template types
    struct Traits
    {
      typedef ct CoordType;
      typedef EntityImp<0,dim,dimworld>           Entity;
      typedef HierarchicIteratorImp<dim,dimworld> HierarchicIterator;
    };
    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

  private:
    //!  Barton-Nackman trick
    HierarchicIteratorImp<dim,dimworld>& asImp ()
    {return static_cast<HierarchicIteratorImp<dim,dimworld>&>(*this);}
    const HierarchicIteratorImp<dim,dimworld>& asImp () const
    {return static_cast<const HierarchicIteratorImp<dim,dimworld>&>(*this);}
  };

  //************************************************************************
  // E N T I T Y
  //************************************************************************

  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Here: the general template
   */
  template<int codim, int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  class Entity {
  public:

    //! remeber the template types
    struct Traits
    {
      typedef ct CoordType;
      typedef ElementImp<dim,dimworld>             Element;
      typedef LevelIteratorImp<codim,dim,dimworld,All_Partition> LevelIterator;
      typedef IntersectionIteratorImp<dim,dimworld>    IntersectionIterator;
      typedef HierarchicIteratorImp<dim,dimworld>  HierarchicIterator;
    };

    //! know your own codimension
    enum { codimension=codim };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! level of this element
    int level ();

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index ();

    //! return partition type attribute
    PartitionType partition_type ();

    //! id of the boundary which is associated with
    //! the entity, returns 0 for inner entities, arbitrary int otherwise
    int boundaryId () const ;

    //! geometry of this entity
    ElementImp<dim-codim,dimworld>& geometry ();

    /*! \internal Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //!  Barton-Nackman trick
    EntityImp<codim,dim,dimworld>& asImp () {return static_cast<EntityImp<codim,dim,dimworld>&>(*this);}
  };


  //********************************************************************
  //
  // --EntityDefault
  //
  //! EntityDefault provides default implementations for Entity which uses
  //! the implemented interface which has to be done by the user.
  //
  //********************************************************************
  template<int codim, int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  class EntityDefault
    : public Entity <codim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>
  {
  public:
    // at this moment no default implementation
  private:
    //!  Barton-Nackman trick
    EntityImp<codim,dim,dimworld>& asImp () {return static_cast<EntityImp<codim,dim,dimworld>&>(*this);}

  }; // end EntityDefault


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
  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  class Entity<0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp> {
  public:


    //! remeber the template types
    struct Traits
    {
      typedef ct CoordType;
      typedef ElementImp<dim,dimworld>             Element;
      typedef EntityImp<0,dim,dimworld>            Entity;
      typedef LevelIteratorImp<0,dim,dimworld,All_Partition>     LevelIterator;
      typedef IntersectionIteratorImp<dim,dimworld>    IntersectionIterator;
      typedef HierarchicIteratorImp<dim,dimworld>  HierarchicIterator;
    };


    //! know your own codimension
    enum { codimension=0 };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! level of this element
    int level ();

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index ();

    //! return partition type attribute
    PartitionType partition_type ();

    //! geometry of this entity
    ElementImp<dim,dimworld>& geometry ();

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
       with codimension cc.
     */
    template<int cc> int count ();

    /*! Provide access to mesh entity i of given codimension. Entities
       are numbered 0 ... count<cc>()-1
     */
    template<int cc> LevelIteratorImp<cc,dim,dimworld,All_Partition> entity (int i); // 0 <= i < count()

    /*! Intra-level access to intersections with neighboring elements.
       A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor.
     */
    IntersectionIteratorImp<dim,dimworld> ibegin ();

    //! Reference to one past the last intersection
    IntersectionIteratorImp<dim,dimworld> iend ();

    //! Inter-level access to father element on coarser grid. Assumes that meshes are nested.
    LevelIteratorImp<0,dim,dimworld,All_Partition> father ();

    /*! Location of this element relative to the reference element of the father.
       This is sufficient to interpolate all dofs in conforming case.
       Nonconforming may require access to neighbors of father and
       computations with local coordinates.
       On the fly case is somewhat inefficient since dofs  are visited several times.
       If we store interpolation matrices, this is tolerable. We assume that on-the-fly
       implementation of numerical algorithms is only done for simple discretizations.
       Assumes that meshes are nested.
     */
    ElementImp<dim,dim>& father_relative_local ();

    /*! Inter-level access to son elements on higher levels<=maxlevel.
       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
     */
    HierarchicIteratorImp<dim,dimworld> hbegin (int maxlevel);

    //! Returns iterator to one past the last son
    HierarchicIteratorImp<dim,dimworld> hend (int maxlevel);

    /*! \internal Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //!  Barton-Nackman trick
    EntityImp<0,dim,dimworld>& asImp () {return static_cast<EntityImp<0,dim,dimworld>&>(*this);}
  };


  //********************************************************************
  //
  // --EntityDefault
  //
  //! EntityDefault provides default implementations for Entity which uses
  //! the implemented interface which has to be done by the user.
  //
  //********************************************************************
  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  class EntityDefault
  <0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>
    : public Entity <0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,
          IntersectionIteratorImp,HierarchicIteratorImp>
  {
  public:
    //! Remember the template types
    struct Traits
    {
      //! The coordinate type
      typedef ct CoordType;
      //! The element type
      typedef ElementImp<dim,dimworld>             Element;
      //! The entity type
      typedef EntityImp<0,dim,dimworld>            Entity;
      typedef LevelIteratorImp<0,dim,dimworld,All_Partition>     LevelIterator;
      typedef IntersectionIteratorImp<dim,dimworld>    IntersectionIterator;
      typedef HierarchicIteratorImp<dim,dimworld>  HierarchicIterator;
    };

    //! know your own codimension
    enum { codimension=0 };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    /** \brief Default implementation for access to subIndex
     *
     * Default implementation for access to subIndex via interface method entity
     * default is to return the index of the sub entity, is very slow, but works
     */
    template <int cc> int subIndex ( int i ) const ;

    /** \brief Default implementation for access to boundaryId of sub entities
     *
     * Default implementation for access to boundaryId via interface method
     * entity<codim>.boundaryId(), default is very slow, but works, can be
     * overloaded be the actual grid implementation, works the same way as
     * subIndex
     */
    template <int cc> int subBoundaryId  ( int i ) const ;

    //***************************************************************
    //  Interface for Adaptation
    //***************************************************************
    /** \brief Marks an element for refCount refines.
     * If refCount is negative the element is coarsened -refCount times.
     * \return  True if element was marked, otherwise false
     */
    bool mark( int refCount ) { return false; }

    //! return whether entity could be coarsened (COARSEN) or was refined
    //! (REFINED) or nothing happend (NONE)
    //! @return The default implementation returns NONE for grid with no
    //! adaptation
    AdaptationState state () { return NONE; }

  private:
    //!  Barton-Nackman trick
    EntityImp<0,dim,dimworld>& asImp () {return static_cast<EntityImp<0,dim,dimworld>&>(*this);}
  };
  // end EntityDefault
  //******************************************************************************

  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Entities of codimension=dimension ("vertices") are defined through template specialization. Note
     that this specialization has an extended interface compared to the general case
   */
  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  class Entity<dim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp> {
  public:
    //! remeber the template types
    struct Traits
    {
      typedef ct CoordType;
      typedef ElementImp<dim,dimworld>               Element;
      typedef EntityImp<dim,dim,dimworld>            Entity;
      typedef LevelIteratorImp<dim,dim,dimworld,All_Partition>     LevelIterator;
      typedef IntersectionIteratorImp<dim,dimworld>  IntersectionIterator;
      typedef HierarchicIteratorImp<dim,dimworld>    HierarchicIterator;
    };

    //! know your own codimension
    enum { codimension=dim };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! level of this element
    int level ();

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index ();

    //! return partition type attribute
    PartitionType partition_type ();

    //! geometry of this entity
    ElementImp<0,dimworld>& geometry ();

    /*! Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    LevelIteratorImp<0,dim,dimworld,All_Partition> father ();

    //! local coordinates within father
    FieldVector<ct, dim>& local ();

    /*! \internal Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //!  Barton-Nackman trick
    EntityImp<dim,dim,dimworld>& asImp () {return static_cast<EntityImp<dim,dim,dimworld>&>(*this);}
  };

  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int> class IntersectionIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  class EntityDefault <dim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>
    : public Entity <dim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,IntersectionIteratorImp,HierarchicIteratorImp>
  {
  public:
    // no default implementation at the moment
  private:
    //!  Barton-Nackman trick
    EntityImp<dim,dim,dimworld>& asImp () {return static_cast<EntityImp<dim,dim,dimworld>&>(*this);}
  };



  //************************************************************************
  // L E V E L I T E R A T O R
  //************************************************************************

  /** \brief Enables iteration over all entities of a given codimension and level of a grid.
   */
  template<int codim, int dim, int dimworld, PartitionIteratorType pitype, class ct,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int,int> class EntityImp
      >
  class LevelIterator
  {
  public:
    /** \brief Remember the template types */
    struct Traits
    {
      /** \todo Please doc me! */
      typedef ct CoordType;

      /** \todo Please doc me! */
      typedef EntityImp<codim,dim,dimworld>        Entity;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,All_Partition>             LevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,Interior_Partition>        InteriorLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,InteriorBorder_Partition>  InteriorBorderLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,Overlap_Partition>         OverlapLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,OverlapFront_Partition>    OverlapFrontLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,Ghost_Partition>           GhostLevelIterator;
    };

    //! know your own codimension
    enum { codimension=dim };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! prefix increment
    LevelIteratorImp<codim,dim,dimworld,pitype>& operator++();

    //! equality
    bool operator== (const LevelIteratorImp<codim,dim,dimworld,pitype>& i) const;

    //! inequality
    bool operator!= (const LevelIteratorImp<codim,dim,dimworld,pitype>& i) const;

    //! dereferencing
    EntityImp<codim,dim,dimworld>& operator*() ;

    //! arrow
    EntityImp<codim,dim,dimworld>* operator->() ;

    //! ask for level of entity
    int level ();

    /*! \internal Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //!  Barton-Nackman trick
    LevelIteratorImp<codim,dim,dimworld,pitype>& asImp ()
    {return static_cast<LevelIteratorImp<codim,dim,dimworld,pitype>&>(*this);}
    const LevelIteratorImp<codim,dim,dimworld,pitype>& asImp () const
    {return static_cast<const LevelIteratorImp<codim,dim,dimworld,pitype>&>(*this);}
  };

  //**********************************************************************
  //
  //  --LevelIteratorDefault
  //
  //! Default implementation of LevelIterator.
  //
  //**********************************************************************
  template<int codim, int dim, int dimworld, PartitionIteratorType pitype, class ct,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int,int> class EntityImp
      >
  class LevelIteratorDefault
    : public LevelIterator <codim,dim,dimworld,pitype,ct,LevelIteratorImp,EntityImp>
  {
  public:

  private:
    //!  Barton-Nackman trick
    LevelIteratorImp<codim,dim,dimworld,pitype>& asImp () {
      return static_cast<LevelIteratorImp<codim,dim,dimworld,pitype>&>(*this);
    }

    const LevelIteratorImp<codim,dim,dimworld,pitype>& asImp () const {
      return static_cast<const LevelIteratorImp<codim,dim,dimworld,pitype>&>(*this);
    }

  }; // end LevelIteratorDefault
     //**************************************************************************


  //************************************************************************
  // G R I D
  //************************************************************************

  /*!
     A Grid is a container of grid entities. Given a dimension dim these entities have a
     codimension codim with 0 <= codim <= dim.

     The Grid is assumed to be hierachically refined and nested. It enables iteration over
     entities of a given level and codimension.

     The grid can be non-matching.

     All information is provided to allocate degrees of freedom in appropriate vector
     data structures (which are not part of this module).

     Template class Grid defines a "base class" for all grids.
   */
  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  class Grid {
  public:

    //! remember the types of template parameters
    template <int codim>
    struct Traits
    {
      /** \todo Please doc me! */
      typedef ct CoordType;

      /** \todo Please doc me! */
      typedef GridImp<dim,dimworld>                 ImpGrid;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,All_Partition>             LevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,Interior_Partition>        InteriorLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,InteriorBorder_Partition>  InteriorBorderLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,Overlap_Partition>         OverlapLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,OverlapFront_Partition>    OverlapFrontLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,Ghost_Partition>           GhostLevelIterator;

      /** \todo Please doc me! */
      typedef EntityImp<codim,dim,dimworld>         Entity;

    };

    //! A grid exports its dimension
    enum { dimension=dim };

    //! A grid knowns the dimension of the world
    enum { dimensionworld=dimworld };

    //! Define type used for coordinates in grid module
    typedef ct ctype;

    /*! Return maximum level defined in this grid. Levels are numbered
       0 ... maxlevel with 0 the coarsest level.
     */
    int maxlevel() const;

    //! Return number of grid entities of a given codim on a given level
    int size (int level, int codim) const;

    //! return size (= distance in graph) of overlap region
    int overlap_size (int level, int codim);

    //! return size (= distance in graph) of ghost region
    int ghost_size (int level, int codim);

    //! Iterator to first entity of given codim on level
    template<int codim, PartitionIteratorType pitype>
    LevelIteratorImp<codim,dim,dimworld,pitype> lbegin (int level);

    //! one past the end on this level
    template<int codim, PartitionIteratorType pitype>
    LevelIteratorImp<codim,dim,dimworld,pitype> lend (int level);

    //! version without second template parameter to run sequential code
    template<int codim>
    LevelIteratorImp<codim,dim,dimworld,All_Partition> lbegin (int level);

    //! version without second template parameter to run sequential code
    template<int codim>
    LevelIteratorImp<codim,dim,dimworld,All_Partition> lend (int level);

    //! the generic communication function
    template<class T, template<class> class P, int codim>
    void communicate (T& t, InterfaceType iftype, CommunicationDirection dir, int level);

    //! return GridIdentifierType of Grid, i.e. SGrid_Id or AlbertGrid_Id ...
    GridIdentifier type();

    /*! \internal Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //!  Barton-Nackman trick
    GridImp<dim,dimworld>& asImp () {return static_cast<GridImp<dim,dimworld>&>(*this);}
    //!  Barton-Nackman trick
    const GridImp<dim,dimworld>& asImp () const {return static_cast<const GridImp<dim,dimworld>&>(*this);}
  };


  //************************************************************************
  //
  //  Default Methods of Grid
  //
  //************************************************************************
  //

  /** \brief The Default Methods of Grid
   *
   * \todo Please doc me!
   */
  template<int dim,
      int dimworld,
      class ct,
      template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp,
      template<int,int,int> class EntityImp>
  class GridDefault : public Grid <dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>
  {
  public:

    //! remember the types of template parameters
    template <int codim>
    struct Traits
    {

      //! Please doc me!
      typedef ct CoordType;

      //! Please doc me!
      typedef GridImp<dim,dimworld>                 ImpGrid;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,All_Partition>             LevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,Interior_Partition>        InteriorLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,InteriorBorder_Partition>  InteriorBorderLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,Overlap_Partition>         OverlapLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,OverlapFront_Partition>    OverlapFrontLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<codim,dim,dimworld,Ghost_Partition>           GhostLevelIterator;

      //! Please doc me!
      typedef EntityImp<codim,dim,dimworld>         Entity;
    };

    //! Please doc me!
    class LeafIterator;

    /*
       //! return LeafIterator which points to the first entity in maxLevel
       LeafIterator leafbegin(int maxLevel);

       //! return LeafIterator which points behind the last entity in maxLevel
       LeafIterator leafend(int maxLevel);
     */

    /** \brief Refine all positive marked leaf entities
        coarsen all negative marked entities if possible
        \return true if a least one element was refined

       **Note**: this default implementation always returns false
        so grid with no adaptation doesn't need to implement these methods
     */
    bool adapt ()    { return false; }

    //! returns true, if a least one element is marked for coarsening
    bool preAdapt () { return false; }

    //! clean up some markers
    void postAdapt() {}

    /** Write Grid with GridType file filename and time
     *
     * This method uses the Grid Interface Method writeGrid
     * to actually write the grid, within this method the real file name is
     * generated out of filename and timestep
     */
    bool write (const FileFormatType ftype, const char * fnprefix , ct time=0.0, int timestep=0);

    //! get Grid from file with time and timestep , return true if ok
    bool read ( const char * fnprefix , ct & time , int timestep);

    //! write Grid to file filename and store time
    template <FileFormatType ftype>
    bool writeGrid ( const char * filename , ct time );

    //! read Grid from file filename and also read time of grid
    template <FileFormatType ftype>
    bool readGrid ( const char * filename , ct &time );

  protected:
    //! Barton-Nackman trick
    GridImp<dim,dimworld>& asImp () {return static_cast<GridImp<dim,dimworld>&>(*this);}
  };


  /** \brief Iterates over the leaves of a hierarchical grid. */
  template< int dim, int dimworld, class ct, template<int,int> class GridImp,
      template<int,int,int,PartitionIteratorType> class LevelIteratorImp, template<int,int,int> class EntityImp>
  class GridDefault<dim,dimworld,ct,GridImp,LevelIteratorImp,EntityImp>::LeafIterator
  {
    // some typedefs
    typedef GridImp<dim,dimworld> GridType;
    typedef typename Traits<0>::LevelIterator LevelIterator;
    typedef typename Traits<0>::Entity EntityType;
    typedef typename EntityType::Traits::HierarchicIterator HierIterator;

  public:
    //! remember the types of template parameters
    //! LeafIterator only on Entitys of codim=0 , because we use the
    //! HierarchicIterator
    struct Traits
    {
      //! Please doc me!
      typedef ct CoordType;

      //! Please doc me!
      typedef GridImp<dim,dimworld>             ImpGrid;

      //! Please doc me!
      typedef LevelIteratorImp<0,dim,dimworld,All_Partition>             LevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<0,dim,dimworld,Interior_Partition>        InteriorLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<0,dim,dimworld,InteriorBorder_Partition>  InteriorBorderLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<0,dim,dimworld,Overlap_Partition>         OverlapLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<0,dim,dimworld,OverlapFront_Partition>    OverlapFrontLevelIterator;

      //! Please doc me!
      typedef LevelIteratorImp<0,dim,dimworld,Ghost_Partition>           GhostLevelIterator;

      //! Please doc me!
      typedef EntityImp<0,dim,dimworld>         Entity;

    };

    //! Constructor making new LeafIterator
    LeafIterator (GridType &grid, int maxlevel, bool end);

    //! Desctructor removing underlying iterators
    ~LeafIterator ();

    //! prefix increment
    LeafIterator& operator ++();

    //! prefix increment i times
    LeafIterator& operator ++(int i);

    //! equality
    bool operator == (const LeafIterator& i) const;

    //! inequality
    bool operator != (const LeafIterator& i) const;

    // didn't work with implementation in .cc file, so what
    //! dereferencing
    EntityImp<0,dim,dimworld>& operator*() { return (*en_); };

    //! arrow
    EntityImp<0,dim,dimworld>* operator->() { return (en_); };

    //! ask for level of entity
    int level ();

  private:
    EntityImp<0,dim,dimworld> * goNextEntity();

    // macro grid iterator
    LevelIterator *it_;
    // end of macro grid iterator
    LevelIterator *endit_;

    // hierarchical iterator
    HierIterator *hierit_;

    // end of hierarchical iterator
    HierIterator *endhierit_;

    // pointer to actual Entity
    EntityType        *en_;

    // true if we must go to next macro element
    bool goNextMacroEntity_;
    bool built_;
    bool useHierarchic_;
    bool end_;

    // go down until max level
    int maxLev_;
  };

  //! provide names for the partition types
  inline std::string GridName(GridIdentifier type)
  {
    switch(type) {
    case SGrid_Id :
      return "SGrid";
    case AlbertGrid_Id :
      return "AlbertGrid";
    case SimpleGrid_Id :
      return "SimpleGrid";
    case UGGrid_Id :
      return "UGGrid";
    case YaspGrid_Id :
      return "YaspGrid";
    case BSGrid_Id :
      return "BSGrid";
    case OneDGrid_Id :
      return "OneDGrid";
    default :
      return "unknown";
    }
  }

  /** @} */

}

#include "grid.cc"

#endif
