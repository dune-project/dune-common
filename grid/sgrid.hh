// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __SGRID_HH__
#define __SGRID_HH__

#include "../common/matvec.hh"
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

          \image html sgridclasses.png "Class diagram for classes in the grid interface"
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
  class SElement
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

    //! constructor, makes element from position and direction vectors
    SElement (const Vec<dimworld,sgrid_ctype>& s_, Vec<dimworld,sgrid_ctype> r_[dim]);

    //! constructor without arguments makes reference element
    SElement ();

  private:
    Vec<dimworld,sgrid_ctype> s;             //!< position of element
    Mat<dimworld,dim,sgrid_ctype> A;         //!< direction vectors as matrix
    Vec<dimworld,sgrid_ctype> c[1<<dim];     //!< coordinate vectors of corners
    Mat<dim,dim,sgrid_ctype> Jinv;           //!< storage for inverse of jacobian
    bool builtinverse;
  };

  // specialization for dim=1
  template<int dimworld>
  class SElement<1,dimworld>
  {
  public:
    //! know dimension
    enum { dimension=1 };

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

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned. A reference to a reference
       element is returned. Usually, the implementation will store the finite
       set of reference elements as global variables.
     */
    static SElement<1,1>& refelem ();

    //! maps a local coordinate within reference element to global coordinate in element
    Vec<dimworld,sgrid_ctype> global (Vec<1,sgrid_ctype> local);

    //! maps a global coordinate within the element to a local coordinate in its reference element
    Vec<1,sgrid_ctype> local (Vec<dimworld,sgrid_ctype> global);

    //! return integration element
    sgrid_ctype integration_element (const Vec<1,sgrid_ctype>& local);

    //! can only be called for dim=dimworld!
    Mat<1,1>& Jacobian_inverse (const Vec<1,sgrid_ctype>& local);

    //! constructor, makes element from position and direction vectors
    SElement (const Vec<dimworld,sgrid_ctype>& s_, Vec<dimworld,sgrid_ctype> r_[1]);

    //! constructor, makes element from position and one direction vector, asserts dim=1
    SElement (const Vec<dimworld,sgrid_ctype>& s_, const Vec<dimworld,sgrid_ctype>& r0);

    //! constructor without arguments makes reference element
    SElement ();

  private:
    Vec<dimworld,sgrid_ctype> s;           //!< position of element
    Mat<dimworld,1,sgrid_ctype> A;         //!< direction vectors as matrix
    Vec<dimworld,sgrid_ctype> c[1<<1];     //!< coordinate vectors of corners
    Mat<1,1,sgrid_ctype> Jinv;             //!< storage for inverse of jacobian
    bool builtinverse;
  };

  // specialization for dim=2
  template<int dimworld>
  class SElement<2,dimworld>
  {
  public:
    //! know dimension
    enum { dimension=2 };

    //! know dimension of world
    enum { dimensionworld=dimworld };

    //! return the element type identifier
    ElementType type ();

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners ();

    //! access to coordinates of corners. Index is the number of the corner
    Vec<dimworld,sgrid_ctype>& operator[] (int i);

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned. A reference to a reference
       element is returned. Usually, the implementation will store the finite
       set of reference elements as global variables.
     */
    static SElement<2,2>& refelem ();

    //! maps a local coordinate within reference element to global coordinate in element
    Vec<dimworld,sgrid_ctype> global (Vec<2,sgrid_ctype> local);

    //! maps a global coordinate within the element to a local coordinate in its reference element
    Vec<2,sgrid_ctype> local (Vec<dimworld,sgrid_ctype> global);

    //! return integration element
    sgrid_ctype integration_element (const Vec<2,sgrid_ctype>& local);

    //! can only be called for dim=dimworld!
    Mat<2,2>& Jacobian_inverse (const Vec<2,sgrid_ctype>& local);

    //! constructor, makes element from position and direction vectors
    SElement (const Vec<dimworld,sgrid_ctype>& s_, Vec<dimworld,sgrid_ctype> r_[2]);

    //! constructor, makes element from position and two direction vectors, asserts dim=2
    SElement (const Vec<dimworld,sgrid_ctype>& s_, const Vec<dimworld,sgrid_ctype>& r0, const Vec<dimworld,sgrid_ctype>& r1);

    //! constructor without arguments makes reference element
    SElement ();

  private:
    Vec<dimworld,sgrid_ctype> s;           //!< position of element
    Mat<dimworld,2,sgrid_ctype> A;         //!< direction vectors as matrix
    Vec<dimworld,sgrid_ctype> c[1<<2];     //!< coordinate vectors of corners
    Mat<2,2,sgrid_ctype> Jinv;             //!< storage for inverse of jacobian
    bool builtinverse;
  };

  // specialization for dim=3
  template<int dimworld>
  class SElement<3,dimworld>
  {
  public:
    //! know dimension
    enum { dimension=3 };

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

    /*! return reference element corresponding to this element. If this is
       a reference element then self is returned. A reference to a reference
       element is returned. Usually, the implementation will store the finite
       set of reference elements as global variables.
     */
    static SElement<3,3>& refelem ();

    //! maps a local coordinate within reference element to global coordinate in element
    Vec<dimworld,sgrid_ctype> global (Vec<3,sgrid_ctype> local);

    //! maps a global coordinate within the element to a local coordinate in its reference element
    Vec<3,sgrid_ctype> local (Vec<dimworld,sgrid_ctype> global);

    //! return integration element
    sgrid_ctype integration_element (const Vec<3,sgrid_ctype>& local);

    //! can only be called for dim=dimworld!
    Mat<3,3>& Jacobian_inverse (const Vec<3,sgrid_ctype>& local);

    //! constructor, makes element from position and direction vectors
    SElement (const Vec<dimworld,sgrid_ctype>& s_, Vec<dimworld,sgrid_ctype> r_[3]);

    //! constructor, makes element from position and three direction vectors, asserts dim=3
    SElement (const Vec<dimworld,sgrid_ctype>& s_, const Vec<dimworld,sgrid_ctype>& r0, const Vec<dimworld,sgrid_ctype>& r1, const Vec<dimworld,sgrid_ctype>& r2);

    //! constructor without arguments makes reference element
    SElement ();

  private:
    Vec<dimworld,sgrid_ctype> s;           //!< position of element
    Mat<dimworld,3,sgrid_ctype> A;         //!< direction vectors as matrix
    Vec<dimworld,sgrid_ctype> c[1<<3];     //!< coordinate vectors of corners
    Mat<3,3,sgrid_ctype> Jinv;             //!< storage for inverse of jacobian
    bool builtinverse;
  };

  //************************************************************************

  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Here: the general template
   */

  template<int codim, int dim, int dimworld>
  class SEntity
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

    //! level of this element
    int level ();

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index ();

    //! geometry of this entity
    SElement<dim-codim,dimworld> geometry ();

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
  class SEntity<0,dim,dimworld>
  {
  public:
    //! forward declaration of nested class
    class HierarchicIterator;

    //! forward declaration of nested class
    class NeighborIterator;

    //! define type used for coordinates in grid module
    typedef sgrid_ctype ctype;

    //! know your own codimension
    enum { codimension=0 };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! level of this element
    int level ();

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index ();

    //! geometry of this entity
    SElement<dim,dimworld> geometry ();

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
    SElement<dim,dim> father_relative_local ();

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
    HierarchicIterator operator ++();

    //! postfix increment
    HierarchicIterator operator ++(int i);

    //! equality
    bool operator== (const HierarchicIterator& i) const;

    //! inequality
    bool operator!= (const HierarchicIterator& i) const;

    //! dereferencing
    SEntity<0,dim,dimworld>& operator*();

    //! arrow
    SEntity<0,dim,dimworld>* operator->();

  private:
    //! implement with virtual element
    SEntity<0,dim,dimworld> virtual_element;
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
    NeighborIterator operator ++();

    //! postfix increment
    NeighborIterator operator ++(int i);

    //! equality
    bool operator== (const NeighborIterator& i) const;

    //! inequality
    bool operator!= (const NeighborIterator& i) const;

    //! access neighbor, dereferencing
    SEntity<0,dim,dimworld>& operator*();

    //! access neighbor, arrow
    SEntity<0,dim,dimworld>* operator->();

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of the element where iteration started.
     */
    SElement<dim-1,dim> intersection_self_local ();

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in GLOBAL coordinates of the element where iteration started.
     */
    SElement<dim-1,dimworld> intersection_self_global ();

    //! local number of codim 1 entity in self where intersection is contained in
    int number_in_self ();

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of neighbor
     */
    SElement<dim-1,dim> intersection_neighbor_local ();

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of neighbor
     */
    SElement<dim-1,dimworld> intersection_neighbor_global ();

    //! local number of codim 1 entity in neighbor where intersection is contained in
    int number_in_neighbor ();

  private:
    //! implement with virtual element
    SEntity<0,dim,dimworld> virtual_element;
  };


  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Entities of codimension=dimension ("vertices") are defined through template specialization. Note
     that this specialization has an extended interface compared to the general case
   */
  template<int dim, int dimworld>
  class SEntity<dim,dim,dimworld>
  {
  public:
    //! know your own codimension
    enum { codimension=dim };

    //! know your own dimension
    enum { dimension=dim };

    //! know your own dimension of world
    enum { dimensionworld=dimworld };

    //! level of this element
    int level ();

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index ();

    //! geometry of this entity, i.e. a single point
    SElement<0,dimworld> geometry ();

    /*! Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    SLevelIterator<0,dim,dimworld> father ();

    //! local coordinates within father
    Vec<dim,sgrid_ctype> local ();

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
    SLevelIterator<codim,dim,dimworld> operator ++();

    //! postfix increment
    SLevelIterator<codim,dim,dimworld> operator ++(int i);

    //! equality
    bool operator== (const SLevelIterator<codim,dim,dimworld>& i) const;

    //! inequality
    bool operator!= (const SLevelIterator<codim,dim,dimworld>& i) const;

    //! dereferencing
    SEntity<codim,dim,dimworld>& operator*() ;

    //! arrow
    SEntity<codim,dim,dimworld>* operator->() ;

  private:
    SEntity<codim,dim,dimworld> virtual_element;
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
    SGrid (double H_, int N0_, int L_);

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

  private:
    double H;      // length of cube in each direction
    int N0;     // number of elements in coarsest grid in one direction
    int L;      // number of levels in hierarchic mesh
  };

  /** @} end documentation group */

} // end namespace

#include "sgrid/sgrid.cc"

#endif
