// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __SGRID_HH__
#define __SGRID_HH__

#include "../common/matvec.hh"
#include "../common/Stack.hh"
#include "common/grid.hh"
#include "sgrid/numbering.hh"

/*! \file sgrid.hh
   This file documents the DUNE grid interface. We use the special implementation for
   simple structured grid to illustrate the different classes and their members.
 */

namespace Dune {

  /** @defgroup SGrid Structured Grid Module

          This module describes the pilot implementation of the Dune grid interface.
          It implements the grid interface for simple structured meshes.

          The following class diagram shows how the classes are related with
          each other:

          \image html sgridclasses.gif "Class diagram for classes in the grid interface"
          \image latex sgridclasses.eps "Class diagram for classes in the grid interface" width=\textwidth

          Short description of the classes:

          - SElement is a class template providing the geometric part of a grid entity, i.e. a general polyhedron
          with a mapping from a reference polyhedron to the actual polyhedron.

          - SLevelIterator is a class template which allows to iterate over all grid entities of a given
          codimension and level.

          - SEntity is a class template realizing the grid entities. Grid entities are the constituents
          of a grid. Grid entities of codimension 0 and codimension dim are defines through specialization.
          Entities can be used as template parameters to generic algorithms. Each entity must therefore
          provide the nested classes Element, LevelIterator, HierarchicIterator and NeighborIterator.
          Element and LevelIterator are derived from the classes SELement and SLevelIterator.
          Note that entities of codimension 0 and dim have an extended interface.

          - SEntity::NeighborIterator provides access to all entities of codimension 0 sharing an object of codimension 1
          with the given entity of codimension 0. This interface covers nonmatching grids.

          - SEntity::HierarchicIterator provides access to the sons of an entity of codimension 0.

          - SGrid is conceptualized as a container of grid entities of various codimensions. Since grids
          are used as template parameters to generic algorithms they must include the nested classes
          LevelIterator and Entity which are derived from SLevelIterator and SEntity.

          @{
   */


  //************************************************************************
  /*! define name for floating point type used for coordinates in sgrid.
     You can change the type for coordinates by changing this single typedef.
   */
  typedef double sgrid_ctype;

  //************************************************************************
  // forward declaration of templates

  template<int dim, int dimworld> class SElement;
  template<int codim, int dim, int dimworld> class SEntity;
  template<int codim, int dim, int dimworld> class SLevelIterator;
  template<int dim, int dimworld> class SGrid;

  //! base class implementing parametrizable part of an SElement
  template<int dim, int dimworld>
  class SElementBase {
  public:
    // constructor
    SElementBase ();

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners ();

    //! access to coordinates of corners. Index is the number of the corner
    Vec<dimworld,sgrid_ctype>& operator[] (int i);

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned. A reference to a reference
       element is returned. Usually, the implementation will store the finite
       set of reference elements as global variables.
     */
    static SElement<dim,dim>& refelem ();

    //! maps a local coordinate within reference element to global coordinate in element
    Vec<dimworld,sgrid_ctype> global (const Vec<dim,sgrid_ctype>& local);

    //! maps a global coordinate within the element to a local coordinate in its reference element
    Vec<dim,sgrid_ctype> local (const Vec<dimworld,sgrid_ctype>& global);

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
    sgrid_ctype integration_element (const Vec<dim,sgrid_ctype>& local);

    //! can only be called for dim=dimworld!
    Mat<dim,dim>& Jacobian_inverse (const Vec<dim,sgrid_ctype>& local);

    //! print internal data
    void print (std::ostream& ss, int indent);

  protected:
    Vec<dimworld,sgrid_ctype> s;             //!< position of element
    Mat<dimworld,dim,sgrid_ctype> A;         //!< direction vectors as matrix
    Vec<dimworld,sgrid_ctype> c[1<<dim];     //!< coordinate vectors of corners
    Mat<dim,dim,sgrid_ctype> Jinv;           //!< storage for inverse of jacobian
    bool builtinverse;
  };

  template <int dim, int dimworld>
  inline std::ostream& operator<< (std::ostream& s, SElementBase<dim,dimworld>& e)
  {
    e.print(s,0);
    return s;
  }


  //************************************************************************
  /*!
     SElement realizes the concept of the geometric part of a mesh entity.

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
     with respect to the standard Eucliden inner product.

     The \f$d\f$-dimensional reference polyhedron is given
     by the points \f$\{ (x_0,\ldots,x_{d-1}) \ | \ x_i\in\{0,1\}\ \}\f$.

     In order to invert the map for a point \f$p\f$, we have to find a local coordinate \f$l\f$ such
     that \f$g(l)=p\f$. Of course this is only possible if \f$d=w\f$. In the general case \f$d\leq w\f$
     we determine \f$l\f$ such that
     \f[(s,r^k) + \sum\limits_{i=0}^{d-1} l_i (r^i,r^k) = (p,r^k) \ \ \ \forall k=0,\ldots,d-1. \f]

     The resulting system is diagonal since the direction vectors are required to be orthogonal.
   */
  template<int dim, int dimworld>
  class SElement : public SElementBase<dim,dimworld>
  {
  public:
    //! know dimension
    enum { dimension=dim };

    //! know dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef sgrid_ctype ctype;

    //! return the element type identifier
    ElementType type ();

    /*! The first dim columns of As contain the dim direction vectors.
        Column dim is the position vector. This format allows a consistent
            treatement of all dimensions, including 0 (the vertex).
     */
    void make (Mat<dimworld,dim+1,sgrid_ctype>& As);

    //! constructor with bool argument makes reference element if true, uninitialized else
    SElement (bool b);
  };

  // specialization for dim=0, this is a vertex
  template<int dimworld>
  class SElement<0,dimworld>
  {
  public:
    //! know dimension
    enum { dimension=0 };

    //! know dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef sgrid_ctype ctype;

    //! return the element type identifier
    ElementType type ();

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners ();

    //! access to coordinates of corners. Index is the number of the corner
    Vec<dimworld,sgrid_ctype>& operator[] (int i);

    //! print internal data
    void print (std::ostream& ss, int indent);

    //! constructor, makes element from position and direction vectors
    void make (Mat<dimworld,1>& As);

    //! constructor with bool argument makes reference element if true, uninitialized else
    SElement (bool b);

  protected:
    Vec<dimworld,sgrid_ctype> s;             //!< position of element
  };

  template <int dimworld>
  inline std::ostream& operator<< (std::ostream& s, SElement<0,dimworld>& e)
  {
    e.print(s,0);
    return s;
  }

  //************************************************************************

  /*! SEntityBase contains the part of SEntity that can be defined
     without specialization. This is the base for all SEntity classes with dim>0.
   */
  template<int codim, int dim, int dimworld>
  class SEntityBase {
  public:
    //! level of this element
    int level ();

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index ();

    //! geometry of this entity
    SElement<dim-codim,dimworld>& geometry ();

    //! constructor
    SEntityBase (SGrid<dim,dimworld>& _grid, int _l, int _id);

    //! Reinitialization
    void make (int _l, int _id);

  protected:
    // this is how we implement our elements
    SGrid<dim,dimworld>& grid;            //!< grid containes mapper, geometry, etc.
    int l;                                //!< level where element is on
    int id;                               //!< my consecutive id
    Tupel<int,dim> z;                     //!< my coordinate, number of even components = codim
    SElement<dim-codim,dimworld> geo;     //!< geometry, is only built on demand
    bool builtgeometry;                   //!< true if geometry has been constructed
  };


  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Here: the general template
   */
  template<int codim, int dim, int dimworld>
  class SEntity : public SEntityBase<codim,dim,dimworld>
  {
  public:
    //! know your own codimension
    enum { codimension=codim };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef sgrid_ctype ctype;

    //! inlcude element in scope since it is used as a return type
    template<int d, int dd>
    class Element : public SElement<d,dd> {
    public:
      //! constructor without argument needed because copy constructor is explicitely defined
      Element () : SElement<d,dd>() {}

      //! copy constructor for initializing derived class object with base class object
      Element (const SElement<d,dd>& y) : SElement<d,dd>(y) {}

      //! assignement operator for assigning derived class object to base class object
      Element<d,dd>& operator= (const SElement<d,dd>& y)
      {
        asBase().operator=(y);
        return *this;
      }
    private:
      SElement<d,dd>& asBase() {
        return static_cast<SElement<d,dd>&>(*this);
      }
    };

    //! constructor
    SEntity (SGrid<dim,dimworld>& _grid, int _l, int _id) : SEntityBase<codim,dim,dimworld>::SEntityBase(_grid,_l,_id) {};
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
  template<int dim, int dimworld>
  class SEntity<0,dim,dimworld> : public SEntityBase<0,dim,dimworld>
  {
  public:
    //! forward declaration of nested class
    class HierarchicIterator;

    //! make HierarchicIterator a friend
    friend class NeighborIterator;

    //! forward declaration of nested class
    class NeighborIterator;

    //! make NeighborIterator a friend
    friend class NeighborIterator;

    //! define type used for coordinates in grid module
    typedef sgrid_ctype ctype;

    //! know your own codimension
    enum { codimension=0 };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
       with codimension cc.
     */
    template<int cc> int count ();

    /*! Provide access to mesh entity i of given codimension. Entities
       are numbered 0 ... count<cc>()-1
     */
    template<int cc> SLevelIterator<cc,dim,dimworld> entity (int i);     // 0 <= i < count()

    /*! Intra-level access to neighboring elements. A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor.
     */
    NeighborIterator nbegin ();

    //! Reference to one past the last neighbor
    NeighborIterator nend ();

    //! Inter-level access to father element on coarser grid. Assumes that meshes are nested.
    SLevelIterator<0,dim,dimworld> father ();

    /*! Location of this element relative to the reference element element of the father.
       This is sufficient to interpolate all dofs in conforming case.
       Nonconforming may require access to neighbors of father and
       computations with local coordinates.
       On the fly case is somewhat inefficient since dofs  are visited several times.
       If we store interpolation matrices, this is tolerable. We assume that on-the-fly
       implementation of numerical algorithms is only done for simple discretizations.
       Assumes that meshes are nested.
     */
    SElement<dim,dim>& father_relative_local ();

    /*! Inter-level access to son elements on higher levels<=maxlevel.
       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
     */
    HierarchicIterator hbegin (int maxlevel);

    //! Returns iterator to one past the last son
    HierarchicIterator hend (int maxlevel);

    //! inlcude LevelIterator in scope since it is used as a return type
    template<int cc>
    class LevelIterator : public SLevelIterator<cc,dim,dimworld> {
    public:
      //! constructor without argument needed because copy constructor is explicitely defined
      LevelIterator () : SLevelIterator<cc,dim,dimworld>() {}

      //! copy constructor for initializing derived class object with base class object
      LevelIterator (const SLevelIterator<cc,dim,dimworld>& y) : SLevelIterator<cc,dim,dimworld>(y) {}

      //! assignement operator for assigning derived class object to base class object
      LevelIterator<cc>& operator= (const SLevelIterator<cc,dim,dimworld>& y)
      {
        asBase().operator=(y);
        return *this;
      }
    private:
      SLevelIterator<cc,dim,dimworld>& asBase() {
        return static_cast<SLevelIterator<cc,dim,dimworld>&>(*this);
      }
    };

    //! inlcude Element in scope since it is used as a return type
    template<int d, int dd>
    class Element : public SElement<d,dd> {
    public:
      //! constructor without argument needed because copy constructor is explicitely defined
      Element () : SElement<d,dd>() {}

      //! copy constructor for initializing derived class object with base class object
      Element (const SElement<d,dd>& y) : SElement<d,dd>(y) {}

      //! assignement operator for assigning derived class object to base class object
      Element<d,dd>& operator= (const SElement<d,dd>& y)
      {
        asBase().operator=(y);
        return *this;
      }
    private:
      SElement<d,dd>& asBase() {
        return static_cast<SElement<d,dd>&>(*this);
      }
    };

    //! constructor
    SEntity (SGrid<dim,dimworld>& _grid, int _l, int _id) :
      SEntityBase<0,dim,dimworld>::SEntityBase(_grid,_l,_id) , in_father_local(false)
    {
      built_father = false;
    }

    //! Reinitialization
    void make (int _l, int _id)
    {
      SEntityBase<0,dim,dimworld>::make(_l,_id);
      built_father = false;
    }

  private:
    bool built_father;
    int father_id;
    SElement<dim,dim> in_father_local;
    void make_father();
  };


  /*! Mesh entities of codimension 0 ("elements") allow to visit all neighbors, where
     a neighbor is an entity of codimension 0 which has a common entity of codimension 1 with the entity.
     These neighbors are accessed via a NeighborIterator. This allows the implementation of
     non-matching meshes. The number of neigbors may be different from the number of faces/edges
     of an element!
   */
  template<int dim, int dimworld>
  class SEntity<0,dim,dimworld>::NeighborIterator
  {
  public:
    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! prefix increment
    NeighborIterator& operator ++();

    //! equality
    bool operator== (const NeighborIterator& i) const;

    //! inequality
    bool operator!= (const NeighborIterator& i) const;

    //! return true if intersection is with boundary. \todo connection with boundary information, processor/outer boundary
    bool boundary ();

    //! access neighbor, dereferencing
    SEntity<0,dim,dimworld>& operator*();

    //! access neighbor, arrow
    SEntity<0,dim,dimworld>* operator->();

    //! return unit outer normal, this should be dependent on local coordinates for higher order boundary
    Vec<dimworld,sgrid_ctype>& unit_outer_normal (Vec<dim-1,sgrid_ctype>& local);

    //! return unit outer normal, if you know it is constant use this function instead
    Vec<dimworld,sgrid_ctype>& unit_outer_normal ();

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of the element where iteration started.
     */
    SElement<dim-1,dim>& intersection_self_local ();

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in GLOBAL coordinates of the element where iteration started.
     */
    SElement<dim-1,dimworld>& intersection_self_global ();

    //! local number of codim 1 entity in self where intersection is contained in
    int number_in_self ();

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of neighbor
     */
    SElement<dim-1,dim>& intersection_neighbor_local ();

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of neighbor
     */
    SElement<dim-1,dimworld>& intersection_neighbor_global ();

    //! local number of codim 1 entity in neighbor where intersection is contained in
    int number_in_neighbor ();

    //! constructor
    NeighborIterator (SGrid<dim,dimworld>& _grid, SEntity<0,dim,dimworld>& _self, int _count);

  private:
    void make (int _count);                     //!< reinitialze iterator with given neighbor
    void makeintersections ();                  //!< compute intersections
    SGrid<dim,dimworld>& grid;                  //!< my grid
    SEntity<0,dim,dimworld>& self;              //!< myself, SEntity is a friend class
    int partition;                              //!< partition number of self, needed for coordinate expansion
    Tupel<int,dim> zred;                        //!< reduced coordinates of myself, allows easy computation of neighbors
    int count;                                  //!< number of neighbor
    bool valid_count;                           //!< true if count is in range
    bool is_on_boundary;                        //!< true if neighbor is otside the domain
    SEntity<0,dim,dimworld> e;                  //!< virtual neighbor entity
    Vec<dimworld,sgrid_ctype> normal;           //!< outer unit normal direction
    bool built_intersections;                   //!< true if all intersections have been built
    SElement<dim-1,dim> is_self_local;          //!< intersection in own local coordinates
    SElement<dim-1,dim> is_nb_local;            //!< intersection in neighbors local coordinates
    SElement<dim-1,dimworld> is_global;         //!< intersection in global coordinates, map consistent with is_self_local
  };


  /*! Mesh entities of codimension 0 ("elements") allow to visit all entities of
     codimension 0 obtained through nested, hierarchic refinement of the entity.
     Iteration over this set of entities is provided by the HIerarchicIterator,
     starting from a given entity.
     This is redundant but important for memory efficient implementations of unstructured
     hierarchically refined meshes.
   */
  template<int dim, int dimworld>
  class SEntity<0,dim,dimworld>::HierarchicIterator
  {
  public:
    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! prefix increment
    HierarchicIterator& operator ++();

    //! equality
    bool operator== (const HierarchicIterator& i) const;

    //! inequality
    bool operator!= (const HierarchicIterator& i) const;

    //! dereferencing
    SEntity<0,dim,dimworld>& operator*();

    //! arrow
    SEntity<0,dim,dimworld>* operator->();

    /*! constructor. Here is how it works: If with_sons is true, push start
       element and all its sons on the stack, so the initial element is popped
            last. For an end iterator, push the starting element and no sons. Then
            the iteration will stop when both iterators have the same id AND the
            stack is empty
     */
    HierarchicIterator (SGrid<dim,dimworld>& _grid, SEntity<0,dim,dimworld>& _e, int _maxlevel, bool makeend);

  private:
    SGrid<dim,dimworld>& grid;       //!< my grid
    SEntity<0,dim,dimworld> e;       //!< virtual son entity
    int maxlevel;                    //!< maximum level of elements to be processed
    int orig_l, orig_id;             //!< element where begin was called (the root of the tree to be processed)

    struct StackElem {
      int l;
      int id;
      StackElem (int _l, int _id) {l=_l; id=_id;}
      bool operator== (const StackElem& s) const {return operator!=(s);}
      bool operator!= (const StackElem& s) const {return l!=s.l || id!=s.id;}
    };
    Stack<StackElem> stack;          //!< stack holding elements to be processed

    void push_sons (int level, int fatherid);     //!< push all sons of this element on the stack
  };



  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Entities of codimension=dimension ("vertices") are defined through template specialization. Note
     that this specialization has an extended interface compared to the general case
   */
  template<int dim, int dimworld>
  class SEntity<dim,dim,dimworld> : public SEntityBase<dim,dim,dimworld>
  {
  public:
    //! know your own codimension
    enum { codimension=dim };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    /*! Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    SLevelIterator<0,dim,dimworld> father ();

    //! local coordinates within father
    Vec<dim,sgrid_ctype>& local ();

    //! inlcude element in scope
    template<int d, int dd>
    class Element : public SElement<d,dd> {
    public:
      //! constructor without argument needed because copy constructor is explicitely defined
      Element () : SElement<d,dd>() {}

      //! copy constructor for initializing derived class object with base class object
      Element (const SElement<d,dd>& y) : SElement<d,dd>(y) {}

      //! assignement operator for assigning derived class object to base class object
      Element<d,dd>& operator= (const SElement<d,dd>& y)
      {
        asBase().operator=(y);
        return *this;
      }
    private:
      SElement<d,dd>& asBase() {
        return static_cast<SElement<d,dd>&>(*this);
      }
    };

    //! include LevelIterator in scope since it is used as return type in this class
    template<int cc>
    class LevelIterator : public SLevelIterator<cc,dim,dimworld> {
    public:
      //! constructor without argument needed because copy constructor is explicitely defined
      LevelIterator () : SLevelIterator<cc,dim,dimworld>() {}

      //! copy constructor for initializing derived class object with base class object
      LevelIterator (const SLevelIterator<cc,dim,dimworld>& y) : SLevelIterator<cc,dim,dimworld>(y) {}

      //! assignement operator for assigning derived class object to base class object
      LevelIterator<cc>& operator= (const SLevelIterator<cc,dim,dimworld>& y)
      {
        asBase().operator=(y);
        return *this;
      }
    private:
      SLevelIterator<cc,dim,dimworld>& asBase() {
        return static_cast<SLevelIterator<cc,dim,dimworld>&>(*this);
      }
    };

    //! constructor
    SEntity (SGrid<dim,dimworld>& _grid, int _l, int _id) : SEntityBase<dim,dim,dimworld>::SEntityBase(_grid,_l,_id)
    {
      built_father = false;
    }

    //! Reinitialization
    void make (int _l, int _id)
    {
      SEntityBase<dim,dim,dimworld>::make(_l,_id);
      built_father = false;
    }

  private:
    bool built_father;
    int father_id;
    Vec<dim,sgrid_ctype> in_father_local;
    void make_father();
  };

  //************************************************************************
  /*! Enables iteration over all entities of a given codimension and level of a grid.
   */
  template<int codim, int dim, int dimworld>
  class SLevelIterator
  {
  public:
    //! know your own codimension
    enum { codimension=dim };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! prefix increment
    SLevelIterator<codim,dim,dimworld>& operator ++();

    //! equality
    bool operator== (const SLevelIterator<codim,dim,dimworld>& i) const;

    //! inequality
    bool operator!= (const SLevelIterator<codim,dim,dimworld>& i) const;

    //! dereferencing
    SEntity<codim,dim,dimworld>& operator*() ;

    //! arrow
    SEntity<codim,dim,dimworld>* operator->() ;

    //! ask for level of entity
    int level ();

    //! constructor
    SLevelIterator (SGrid<dim,dimworld>& _grid, int _l, int _id);

  private:
    SGrid<dim,dimworld>& grid;         //!< my grid
    int l;                             //!< level where element is on
    int id;                            //!< my consecutive id
    SEntity<codim,dim,dimworld> e;     //!< virtual entity
  };

  //************************************************************************
  /*!
     A Grid is a container of grid entities. Given a dimension dim these entities have a
     codimension codim with 0 <= codim <= dim.

     The Grid is assumed to hierachically refined and nested. It enables iteration over
     entities of a given level and codimension.

     The grid can consist of several subdomains and it can be non-matching.

     All information is provided to allocate degrees of freedom in appropriate vector
     data structures (which are not part of this module).
   */
  template<int dim, int dimworld>
  class SGrid
  {
  public:
    //! maximum number of levels allowed
    enum { MAXL=32 };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! define type used for coordinates in grid module
    typedef sgrid_ctype ctype;

    /*! constructor, subject to change!
       H_: size of domain
       N_: coarse grid size, #elements in one direction
       L_: number of levels 0,...,L_-1, maxlevel = L_-1
     */
    SGrid (Tupel<int,dim> N_, Tupel<sgrid_ctype,dim> H_, int L_);

    /*! Return maximum level defined in this grid. Levels are numbered
       0 ... maxlevel with 0 the coarsest level.
     */
    int maxlevel();

    //! Iterator to first entity of given codim on level
    template<int codim>
    SLevelIterator<codim,dim,dimworld> lbegin (int level);

    //! one past the end on this level
    template<int codim>
    SLevelIterator<codim,dim,dimworld> lend (int level);

    //! number of grid entities per level and codim
    int size (int level, int codim);

    //! inlcude level iterator in scope
    template<int codim>
    class LevelIterator : public SLevelIterator<codim,dim,dimworld> {
    public:
      //! constructor without argument needed because copy constructor is explicitely defined
      LevelIterator () : SLevelIterator<codim,dim,dimworld>() {}

      //! copy constructor for initializing derived class object with base class object
      LevelIterator (const SLevelIterator<codim,dim,dimworld>& y) :
        SLevelIterator<codim,dim,dimworld>(y) {}

      //! assignement operator for assigning derived class object to base class object
      LevelIterator<codim>& operator= (const SLevelIterator<codim,dim,dimworld>& y)
      {
        asBase().operator=(y);
        return *this;
      }
    private:
      SLevelIterator<codim,dim,dimworld>& asBase() {
        return static_cast<SLevelIterator<codim,dim,dimworld>&>(*this);
      }
    };

    // include entity in scope
    template<int codim>
    class Entity : public SEntity<codim,dim,dimworld> {
    public:
      //! constructor without argument needed because copy constructor is explicitely defined
      Entity () : SEntity<codim,dim,dimworld>() {}

      //! copy constructor for initializing derived class object with base class object
      Entity (const SEntity<codim,dim,dimworld>& y) : SEntity<codim,dim,dimworld>(y) {}

      //! assignement operator for assigning derived class object to base class object
      Entity<codim>& operator= (const SEntity<codim,dim,dimworld>& y)
      {
        asBase().operator=(y);
        return *this;
      }
    private:
      SEntity<codim,dim,dimworld>& asBase() {
        return static_cast<SEntity<codim,dim,dimworld>&>(*this);
      }
    };

    //! map expanded coordinates to position
    Vec<dim,sgrid_ctype> pos (int level, Tupel<int,dim>& z);

    //! compute codim from coordinate
    int codim (int level, Tupel<int,dim>& z);

    //! compute number from expanded coordinate
    int n (int level, Tupel<int,dim> z);

    //! compute coordinates from number and codimension
    Tupel<int,dim> z (int level, int i, int codim);

    //! compress from expanded coordinates to grid for a single partition number
    Tupel<int,dim> compress (int level, Tupel<int,dim>& z);

    //! expand with respect to partition number
    Tupel<int,dim> expand (int level, Tupel<int,dim>& r, int b);

    /*! There are \f$2^d\f$ possibilities of having even/odd coordinates.
        The binary representation is called partition number.
     */
    int partition (int level, Tupel<int,dim>& z);

    //! given reduced coordinates of an element, determine if element is in the grid
    bool exists (int level, Tupel<int,dim>& zred);

  private:
    int L;                              // number of levels in hierarchic mesh 0<=level<L
    Tupel<sgrid_ctype,dim> H;           // length of cube per direction
    Tupel<int,dim> N[MAXL];             // number of elements per direction
    Vec<dim,sgrid_ctype> h[MAXL];       // mesh size per direction
    CubeMapper<dim> mapper[MAXL];       // a mapper for each level
  };

  /** @} end documentation group */

} // end namespace

#include "sgrid/sgrid.cc"

#endif
