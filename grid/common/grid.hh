// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __GRID_HH__
#define __GRID_HH__

namespace Dune {

  /** @defgroup GridCommon Dune Grid Module
          The Dune Grid module defines a general interface to a hierarchical finite element mesh.
          The interface is independent of dimension and element type. Various implementations
          of this interface exits:

          - Structured Grid (SGrid) : A structured mesh in d dimensions consisting of "cubes". The number
          of elements per dimension is variable.

          - Albert Grid (AlbertGrid) : Provides the simplicial meshes of the finite element tool box ALBERT
          written by Kunibert Siebert and Alfred Schmidt.

          - Quoc Mesh Grid  : Provides the meshes of the QuocMesh.

          - UG Grid (UGGrid) : Provides the meshes of the finite element toolbox UG.

          - Structured Parallel Grid (SPGrid) : Provides a distributed structured mesh.

          This Module contains only the description of compounds that are common to all implementations
          of the grid interface.

          For a detailed description of the interface itself please see the documentation
          of the "Structured Grid Module". Since Dune uses the Engine concept there is no abstract definition
          of the interface. As with STL containers, all implementations must implement the
          same classes with exactly the same members to be used in generic algorithms.


          @{
   */

  //************************************************************************
  /*! \enum ElementType
      Enum that declares identifiers for different element types. This
      list can be extended in the future. Not all meshes need to implement
      all element types.
   */

  enum ElementType {unknown,vertex,line, triangle, quadrilateral, tetrahedron, pyramid, prism, hexahedron,
                    iso_triangle, iso_quadrilateral};

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
    //! know dimension
    enum { dimension=dim };

    //! know dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! return the element type identifier
    ElementType type ();

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners ();

    //! access to coordinates of corners. Index is the number of the corner
    Vec<dimworld,ct>& operator[] (int i);

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned. A reference to a reference
       element is returned. Usually, the implementation will store the finite
       set of reference elements as global variables.
     */
    ElementImp<dim,dim>& refelem ();

    //! maps a local coordinate within reference element to global coordinate in element
    Vec<dimworld,ct> global (const Vec<dim,ct>& local);

    //! maps a global coordinate within the element to a local coordinate in its reference element
    Vec<dim,ct> local (const Vec<dimworld,ct>& global);

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
    ct integration_element (const Vec<dim,ct>& local);

    //! can only be called for dim=dimworld!
    Mat<dim,dim>& Jacobian_inverse (const Vec<dim,ct>& local);

    /*! Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //! Barton-Nackman trick
    ElementImp<dim,dimworld>& asImp () {return static_cast<ElementImp<dim,dimworld>&>(*this);}
  };


  //! Specialization of Element for dim=0 (vertices)
  template<int dimworld, class ct,template<int,int> class ElementImp>
  class Element<0,dimworld,ct,ElementImp> {
  public:
    //! know dimension
    enum { dimension=0 };

    //! know dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! return the element type identifier
    ElementType type ();

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners ();

    //! access to coordinates of corners. Index is the number of the corner
    Vec<dimworld,ct>& operator[] (int i);

    /*! Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //! Barton-Nackman trick
    ElementImp<0,dimworld>& asImp () {return static_cast<ElementImp<0,dimworld>&>(*this);}
  };


  //************************************************************************
  // N E I G H B O R I T E R A T O R
  //************************************************************************

  /*! Mesh entities of codimension 0 ("elements") allow to visit all neighbors, where
     a neighbor is an entity of codimension 0 which has a common entity of codimension 1 with the entity.
     These neighbors are accessed via a NeighborIterator. This allows the implementation of
     non-matching meshes. The number of neigbors may be different from the number of faces/edges
     of an element!
   */
  template<int dim, int dimworld, class ct,
      template<int,int> class NeighborIteratorImp,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp
      >
  class NeighborIterator
  {
  public:
    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! prefix increment
    NeighborIteratorImp<dim,dimworld>& operator++();

    //! equality
    bool operator== (const NeighborIteratorImp<dim,dimworld>& i) const;

    //! inequality
    bool operator!= (const NeighborIteratorImp<dim,dimworld>& i) const;

    //! access neighbor, dereferencing
    EntityImp<0,dim,dimworld>& operator*();

    //! access neighbor, arrow
    EntityImp<0,dim,dimworld>* operator->();

    //! return true if intersection is with boundary. \todo connection with boundary information, processor/outer boundary
    bool boundary ();

    //! return unit outer normal, this should be dependent on local coordinates for higher order boundary
    Vec<dimworld,ct>& unit_outer_normal (Vec<dim-1,ct>& local);

    //! return unit outer normal, if you know it is constant use this function instead
    Vec<dimworld,ct>& unit_outer_normal ();

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

    /*! Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //! Barton-Nackman trick
    NeighborIteratorImp<dim,dimworld>& asImp ()
    {return static_cast<NeighborIteratorImp<dim,dimworld>&>(*this);}
    const NeighborIteratorImp<dim,dimworld>& asImp () const
    {return static_cast<const NeighborIteratorImp<dim,dimworld>&>(*this);}
  };


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

    /*! Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //! Barton-Nackman trick
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
      template<int,int,int> class LevelIteratorImp,
      template<int,int> class NeighborIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  class Entity {
  public:
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

    //! geometry of this entity
    ElementImp<dim-codim,dimworld>& geometry ();

    /*! Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //! Barton-Nackman trick
    EntityImp<codim,dim,dimworld>& asImp () {return static_cast<EntityImp<codim,dim,dimworld>&>(*this);}
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
  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int> class LevelIteratorImp,
      template<int,int> class NeighborIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  class Entity<0,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,NeighborIteratorImp,HierarchicIteratorImp> {
  public:
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

    //! geometry of this entity
    ElementImp<dim,dimworld>& geometry ();

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
       with codimension cc.
     */
    template<int cc> int count ();

    /*! Provide access to mesh entity i of given codimension. Entities
       are numbered 0 ... count<cc>()-1
     */
    template<int cc> LevelIteratorImp<cc,dim,dimworld> entity (int i);     // 0 <= i < count()

    /*! Intra-level access to neighboring elements. A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor.
     */
    NeighborIteratorImp<dim,dimworld> nbegin ();

    //! Reference to one past the last neighbor
    NeighborIteratorImp<dim,dimworld> nend ();

    //! Inter-level access to father element on coarser grid. Assumes that meshes are nested.
    LevelIteratorImp<0,dim,dimworld> father ();

    /*! Location of this element relative to the reference element element of the father.
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

    /*! Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //! Barton-Nackman trick
    EntityImp<0,dim,dimworld>& asImp () {return static_cast<EntityImp<0,dim,dimworld>&>(*this);}
  };

  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Entities of codimension=dimension ("vertices") are defined through template specialization. Note
     that this specialization has an extended interface compared to the general case
   */
  template<int dim, int dimworld, class ct,
      template<int,int,int> class EntityImp,
      template<int,int> class ElementImp,
      template<int,int,int> class LevelIteratorImp,
      template<int,int> class NeighborIteratorImp,
      template<int,int> class HierarchicIteratorImp
      >
  class Entity<dim,dim,dimworld,ct,EntityImp,ElementImp,LevelIteratorImp,NeighborIteratorImp,HierarchicIteratorImp> {
  public:
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

    //! geometry of this entity
    ElementImp<0,dimworld>& geometry ();

    /*! Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    LevelIteratorImp<0,dim,dimworld> father ();

    //! local coordinates within father
    Vec<dim,ct>& local ();

    /*! Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //! Barton-Nackman trick
    EntityImp<dim,dim,dimworld>& asImp () {return static_cast<EntityImp<dim,dim,dimworld>&>(*this);}
  };


  //************************************************************************
  // L E V E L I T E R A T O R
  //************************************************************************

  /*! Enables iteration over all entities of a given codimension and level of a grid.
   */
  template<int codim, int dim, int dimworld, class ct,
      template<int,int,int> class LevelIteratorImp,
      template<int,int,int> class EntityImp
      >
  class LevelIterator
  {
  public:
    //! know your own codimension
    enum { codimension=dim };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! prefix increment
    LevelIteratorImp<codim,dim,dimworld>& operator++();

    //! equality
    bool operator== (const LevelIteratorImp<codim,dim,dimworld>& i) const;

    //! inequality
    bool operator!= (const LevelIteratorImp<codim,dim,dimworld>& i) const;

    //! dereferencing
    EntityImp<codim,dim,dimworld>& operator*() ;

    //! arrow
    EntityImp<codim,dim,dimworld>* operator->() ;

    //! ask for level of entity
    int level ();

    /*! Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //! Barton-Nackman trick
    LevelIteratorImp<codim,dim,dimworld>& asImp ()
    {return static_cast<LevelIteratorImp<codim,dim,dimworld>&>(*this);}
    const LevelIteratorImp<codim,dim,dimworld>& asImp () const
    {return static_cast<const LevelIteratorImp<codim,dim,dimworld>&>(*this);}
  };


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
      template<int,int,int> class LevelIteratorImp, template<int,int,int> class EntityImp>
  class Grid {
  public:
    //! A grid exports its dimension
    enum { dimension=dim };

    //! A grid knowns the dimension of the world
    enum { dimensionworld=dimworld };

    //! Define type used for coordinates in grid module
    typedef ct ctype;

    /*! Return maximum level defined in this grid. Levels are numbered
       0 ... maxlevel with 0 the coarsest level.
     */
    int maxlevel();

    //! Return number of grid entities of a given codim on a given level
    int size (int level, int codim);

    //! Iterator to first entity of given codim on level
    template<int codim>
    LevelIteratorImp<codim,dim,dimworld> lbegin (int level);

    //! one past the end on this level
    template<int codim>
    LevelIteratorImp<codim,dim,dimworld> lend (int level);

    /*! Checking presence and format of all interface functions. With
       this method all derived classes can check their correct definition.
     */
    void checkIF ();

  private:
    //! Barton-Nackman trick
    GridImp<dim,dimworld>& asImp () {return static_cast<GridImp<dim,dimworld>&>(*this);}
  };


  /** @} */

}

#include "grid.cc"

#endif
