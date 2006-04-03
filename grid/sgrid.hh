// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_SGRID_HH
#define DUNE_SGRID_HH

#include <limits>
#include <vector>

#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include "../common/stack.hh"
#include "../common/capabilities.hh"
#include "common/grid.hh"
#include "sgrid/numbering.hh"
#include "../common/bigunsignedint.hh"
#include "../common/collectivecommunication.hh"
#include "common/indexidset.hh"

/*! \file sgrid.hh
   This file documents the DUNE grid interface. We use the special implementation for
   simple structured grid to illustrate the different classes and their members.
 */

namespace Dune {

  //************************************************************************
  /*! define name for floating point type used for coordinates in sgrid.
     You can change the type for coordinates by changing this single typedef.
   */
  typedef double sgrid_ctype;

  // globally define the persistent index type
  const int sgrid_dim_bits = 24;   // bits for encoding each dimension
  const int sgrid_level_bits = 6;  // bits for encoding level number
  const int sgrid_codim_bits = 4;  // bits for encoding codimension

  //************************************************************************
  // forward declaration of templates

  template<int dim, int dimworld, class GridImp> class SGeometry;
  template<int codim, int dim, class GridImp> class SEntity;
  template<int codim, class GridImp> class SEntityPointer;
  template<int codim, PartitionIteratorType, class GridImp> class SLevelIterator;
  template<int dim, int dimworld> class SGrid;
  template<class GridImp> class SIntersectionIterator;
  template<class GridImp> class SHierarchicIterator;

  //************************************************************************
  /*!
     SGeometry realizes the concept of the geometric part of a mesh entity.

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
  template<int mydim, int cdim, class GridImp>
  class SGeometry : public GeometryDefaultImplementation<mydim,cdim,GridImp,SGeometry>
  {
  public:
    //! define type used for coordinates in grid module
    typedef sgrid_ctype ctype;

    //! return the element type identifier
    GeometryType type () const;

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const;

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<sgrid_ctype, cdim>& operator[] (int i) const;

    //! maps a local coordinate within reference element to global coordinate in element
    FieldVector<sgrid_ctype, cdim> global (const FieldVector<sgrid_ctype, mydim>& local) const;

    //! maps a global coordinate within the element to a local coordinate in its reference element
    FieldVector<sgrid_ctype, mydim> local (const FieldVector<sgrid_ctype, cdim>& global) const;

    //! returns true if the point in local coordinates is located within the refelem
    bool checkInside (const FieldVector<sgrid_ctype, mydim>& local) const;

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
    sgrid_ctype integrationElement (const FieldVector<sgrid_ctype, mydim>& local) const;

    //! can only be called for dim=dimworld!
    const FieldMatrix<sgrid_ctype,mydim,mydim>& jacobianInverseTransposed (const FieldVector<sgrid_ctype, mydim>& local) const;

    //! print internal data
    void print (std::ostream& ss, int indent) const;

    /*! The first dim columns of As contain the dim direction vectors.
       Column dim is the position vector. This format allows a consistent
       treatement of all dimensions, including 0 (the vertex).
     */
    void make (FieldMatrix<sgrid_ctype,mydim+1,cdim>& __As);

    //! constructor
    SGeometry () : builtinverse(false) {};

  private:
    FieldVector<sgrid_ctype, cdim> s;             //!< position of element
    FieldMatrix<sgrid_ctype,mydim,cdim> A;         //!< direction vectors as matrix
    FixedArray<FieldVector<sgrid_ctype, cdim>, 1<<mydim> c;     //!< coordinate vectors of corners
    mutable FieldMatrix<sgrid_ctype,mydim,mydim> Jinv;           //!< storage for inverse of jacobian
    mutable bool builtinverse;
  };

  //! specialization for dim=0, this is a vertex
  template<int cdim, class GridImp>
  class SGeometry<0,cdim,GridImp> : public GeometryDefaultImplementation<0,cdim,GridImp,SGeometry>
  {
  public:
    //! define type used for coordinates in grid module
    typedef sgrid_ctype ctype;

    //! return the element type identifier
    GeometryType type () const;

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const;

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<sgrid_ctype, cdim>& operator[] (int i) const;

    //! print internal data
    void print (std::ostream& ss, int indent) const;

    //! constructor, makes element from position and direction vectors
    void make (FieldMatrix<sgrid_ctype,1,cdim>& __As);

    //! maps a local coordinate within reference element to global coordinate in element
    FieldVector<sgrid_ctype, 1> global (const FieldVector<sgrid_ctype, 0>& local) const { return this->operator[] (0); }

    //! maps a global coordinate within the element to a local coordinate in its reference element
    FieldVector<sgrid_ctype, 0> local (const FieldVector<sgrid_ctype, 1>& global) const { return FieldVector<sgrid_ctype,0> (0.0); }

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
       For this specialisation the integrationElement is always 1.
     */

    //! constructor with bool argument makes reference element if true, uninitialized else
    SGeometry () {};

    /** \brief This dummy routine always returns 1.0.
     *
     * This routine exists so that algorithms that integrate over grid
     * boundaries can also be compiled for 1d-grids.
     */
    sgrid_ctype integrationElement(const FieldVector<sgrid_ctype, 0>& local) const {
      return 1.;
    }

    //! returns always true
    bool checkInside (const FieldVector<sgrid_ctype, 0>& local) const
    {
      return true;
    }

    //! can only be called for dim=dimworld!
    const FieldMatrix<sgrid_ctype,0,0>& jacobianInverseTransposed (const FieldVector<sgrid_ctype, 0>& local) const
    {
      static FieldMatrix<sgrid_ctype,0,0> dummy;
      return dummy;
    }

  protected:
    FieldVector<sgrid_ctype, cdim> s;             //!< position of element
  };

  template <int mydim, int cdim, class GridImp>
  inline std::ostream& operator<< (std::ostream& s, SGeometry<mydim,cdim,GridImp>& e)
  {
    e.print(s,0);
    return s;
  }

  template<int mydim, int cdim, class GridImp>
  class SMakeableGeometry : public Geometry<mydim, cdim, GridImp, SGeometry>
  {
  public:
    SMakeableGeometry() :
      Geometry<mydim, cdim, GridImp, SGeometry>(SGeometry<mydim, cdim, GridImp>())
    {};

    void make (FieldMatrix<sgrid_ctype,mydim+1,cdim>& __As) { this->realGeometry.make(__As); }
  };

  //************************************************************************
  /*! SEntityBase contains the part of SEntity that can be defined
     without specialization. This is the base for all SEntity classes with dim>0.
   */

  template<int codim, int dim, class GridImp>
  class SEntityBase {
    friend class SEntityPointer<codim,GridImp>;
    friend class SIntersectionIterator<GridImp>;
    enum { dimworld = GridImp::dimensionworld };
  public:
    typedef typename GridImp::template Codim<codim>::Geometry Geometry;
    typedef SMakeableGeometry<dim-codim, dimworld, const GridImp> MakeableGeometry;
    typedef typename GridImp::PersistentIndexType PersistentIndexType;

    //! level of this element
    int level () const
    {
      return l;
    }

    //! index is unique and consecutive per level and codim
    //! used for access to degrees of freedom
    int index () const
    {
      return compressedIndex();
    }

    //! global index is calculated from the index and grid size
    int globalIndex() const;

    //! geometry of this entity
    const Geometry& geometry () const;

    //! constructor
    SEntityBase (GridImp* _grid, int _l, int _id);
    SEntityBase ();
    void make (GridImp* _grid, int _l, int _id);

    //! Reinitialization
    void make (int _l, int _id);

    //! globally unique, persistent index
    PersistentIndexType persistentIndex () const
    {
      if (codim!=dim)
      {
        // encode codim, this would actually not be necessary
        // because z is unique in codim
        PersistentIndexType id(codim);

        // encode level
        id = id << sgrid_level_bits;
        id = id+PersistentIndexType(l);

        // encode coordinates
        for (int i=dim-1; i>=0; i--)
        {
          id = id << sgrid_dim_bits;
          id = id+PersistentIndexType(z[i]);
        }

        return id;
      }
      else
      {
        // determine min number of trailing zeroes
        // consider that z is on the doubled grid !
        int trailing = 1000;
        for (int i=0; i<dim; i++)
        {
          // count trailing zeros
          int zeros = 0;
          for (int j=0; j<l; j++)
            if (z[i]&(1<<(j+1)))
              break;
            else
              zeros++;
          trailing = std::min(trailing,zeros);
        }

        // determine the level of this vertex
        int level = l-trailing;

        // encode codim
        PersistentIndexType id(dim);

        // encode level
        id = id << sgrid_level_bits;
        id = id+PersistentIndexType(level);

        // encode coordinates
        for (int i=dim-1; i>=0; i--)
        {
          id = id << sgrid_dim_bits;
          id = id+PersistentIndexType(z[i]>>trailing);
        }

        return id;
      }
    }

    //! consecutive, codim-wise, level-wise index
    int compressedIndex () const
    {
      return id;
    }

    //! consecutive, codim-wise, level-wise index
    int compressedLeafIndex () const
    {
      // codim != dim -> there are no copies of entities
      // maxlevel -> ids are fine
      if (codim<dim || l==grid->maxLevel())
        return id;

      // this is a vertex which is not on the finest level
      // move coordinates up to maxlevel (multiply by 2 for each level
      FixedArray<int,dim> coord;
      for (int k=0; k<dim; k++)
        coord[k] = z[k]*(1<<(grid->maxLevel()-l));

      // compute number with respect to maxLevel
      return grid->n(grid->maxLevel(),coord);
    }

  protected:
    // this is how we implement our elements
    GridImp* grid;       //!< grid containes mapper, geometry, etc.
    int l;               //!< level where element is on
    int id;              //!< my consecutive id
    FixedArray<int,dim> z; //!< my coordinate, number of even components = codim
    mutable MakeableGeometry geo; //!< geometry, is only built on demand
    mutable bool builtgeometry; //!< true if geometry has been constructed
  };


  /**
     A Grid is a container of grid entities. An entity is parametrized by
     the codimension.  An entity of codimension c in dimension d is a d-c
     dimensional object.

     Here: the general template
   */
  template<int codim, int dim, class GridImp>
  class SEntity : public SEntityBase<codim,dim,GridImp>,
                  public EntityDefaultImplementation<codim,dim,GridImp,SEntity>
  {
    enum { dimworld = GridImp::dimensionworld };
  public:
    typedef typename GridImp::template Codim<codim>::Geometry Geometry;
    typedef typename GridImp::template Codim<codim>::LevelIterator LevelIterator;
    typedef typename GridImp::template Codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename GridImp::template Codim<0>::HierarchicIterator HierarchicIterator;

    // disambiguate member functions with the same name in both bases
    //! level of this element
    int level () const {return SEntityBase<codim,dim,GridImp>::level();}

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index () const {return SEntityBase<codim,dim,GridImp>::index();}

    //! geometry of this entity
    const Geometry& geometry () const { return SEntityBase<codim,dim,GridImp>::geometry(); }

    //! only interior entities
    PartitionType partitionType () const { return InteriorEntity; }

    // specific to SEntity
    //! constructor
    SEntity (GridImp* _grid, int _l, int _id) : SEntityBase<codim,dim,GridImp>::SEntityBase(_grid,_l,_id) {};
  };

  /**
     A Grid is a container of grid entities. An entity is parametrized
     by the codimension.  An entity of codimension c in dimension d is a
     d-c dimensional object.

     Entities of codimension 0 ("elements") are defined through template
     specialization. Note that this specialization has an extended
     interface compared to the general case

     Entities of codimension 0 allow to visit all neighbors, where a
     neighbor is an entity of codimension 0 which has a common entity of
     codimension 1 with the entity.  These neighbors are accessed via an
     iterator. This allows the implementation of non-matching meshes. The
     number of neigbors may be different from the number of faces/edges
     of an element!
   */

  static FixedArray <int,2> zrefGlob;
  static FixedArray <int,2> zentityGlob;

  /**
     A Grid is a container of grid entities. An entity is parametrized by
     the codimension.  An entity of codimension c in dimension d is a d-c
     dimensional object.

     Entities of codimension=0 ("Cells") are defined through template
     specialization. Note that this specialization has an extended
     interface compared to the general case
   */
  template<int dim, class GridImp>
  class SEntity<0,dim,GridImp> : public SEntityBase<0,dim,GridImp>,
                                 public EntityDefaultImplementation<0,dim,GridImp,SEntity>
  {
    enum { dimworld = GridImp::dimensionworld };
  public:
    typedef typename GridImp::template Codim<0>::Geometry Geometry;
    typedef SMakeableGeometry<dim, dimworld, const GridImp> MakeableGeometry;
    template <int cd>
    struct Codim
    {
      typedef typename GridImp::template Codim<cd>::EntityPointer EntityPointer;
    };
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;
    typedef typename GridImp::template Codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename GridImp::template Codim<0>::HierarchicIterator HierarchicIterator;
    typedef typename GridImp::PersistentIndexType PersistentIndexType;

    //! make HierarchicIterator a friend
    friend class SHierarchicIterator<GridImp>;

    // disambiguate member functions with the same name in both bases
    //! level of this element
    int level () const {return SEntityBase<0,dim,GridImp>::level();}

    //! index is unique and consecutive per level and codim
    //! used for access to degrees of freedom
    int index () const {return SEntityBase<0,dim,GridImp>::index();}

    //! only interior entities
    PartitionType partitionType () const { return InteriorEntity; }

    //! geometry of this entity
    const Geometry& geometry () const {return SEntityBase<0,dim,GridImp>::geometry();}

    /**
       Intra-element access to entities of codimension cc > codim.
       Return number of entities with codimension cc.
     */
    template<int cc> int count () const;

    /**
       Provide access to mesh entity i of given codimension. Entities
       are numbered 0 ... count<cc>()-1
     */
    template<int cc> typename Codim<cc>::EntityPointer entity (int i) const;

    //! subentity compressed index
    template<int cc>
    int subCompressedIndex (int i) const
    {
      if (cc==0) return this->compressedIndex();
      return this->grid->template getRealEntity<cc>(*entity<cc>(i)).compressedIndex();
    }

    //! subentity compressed index
    template<int cc>
    int subCompressedLeafIndex (int i) const
    {
      if (cc==0) return this->compressedLeafIndex();
      return this->grid->template getRealEntity<cc>(*entity<cc>(i)).compressedLeafIndex();
    }

    //! subentity persistent index
    template<int cc>
    PersistentIndexType subPersistentIndex (int i) const
    {
      if (cc==0) return this->persistentIndex();
      return this->grid->template getRealEntity<cc>(*entity<cc>(i)).persistentIndex();
    }

    /**
       Intra-level access to intersections with neighboring elements.  A
       neighbor is an entity of codimension 0 which has an entity of
       codimension 1 in commen with this entity. Access to neighbors is
       provided using iterators. This allows meshes to be
       nonmatching. Returns iterator referencing the first neighbor.
     */
    IntersectionIterator ibegin () const;
    //! Reference to one past the last intersection
    IntersectionIterator iend () const;

    /**
       @brief Inter-level access to father element on coarser grid.

       Assumes that meshes are nested.
     */
    EntityPointer father () const;

    //! return true if the entity is leaf
    bool isLeaf () const
    {
      return ( this->grid->maxLevel() == level() );
    }

    /**
       @brief Location of this element relative to the reference element element of the father.

       This is sufficient to interpolate all dofs in conforming case.
       Nonconforming may require access to neighbors of father and
       computations with local coordinates.  On the fly case is somewhat
       inefficient since dofs are visited several times.  If we store
       interpolation matrices, this is tolerable. We assume that
       on-the-fly implementation of numerical algorithms is only done for
       simple discretizations.  Assumes that meshes are nested.
     */
    const Geometry& geometryInFather () const;

    /**
       @brief Inter-level access to son elements on higher levels<=maxLevel.

       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
     */
    HierarchicIterator hbegin (int maxLevel) const;

    //! Returns iterator to one past the last son
    HierarchicIterator hend (int maxLevel) const;

    // members specific to SEntity
    //! constructor
    SEntity (GridImp* _grid, int _l, int _id) :
      SEntityBase<0,dim,GridImp>::SEntityBase(_grid,_l,_id)
    {
      built_father = false;
    }

    SEntity ()
    {
      built_father = false;
    }

    void make (GridImp* _grid, int _l, int _id)
    {
      SEntityBase<0,dim,GridImp>::make(_grid,_l,_id);
      built_father = false;
    }

    //! Reinitialization
    void make (int _l, int _id)
    {
      SEntityBase<0,dim,GridImp>::make(_l,_id);
      built_father = false;
    }

  private:

    mutable bool built_father;
    mutable int father_id;
    mutable MakeableGeometry in_father_local;
    void make_father() const;
  };

  /**
     A Grid is a container of grid entities. An entity is parametrized
     by the codimension.  An entity of codimension c in dimension d is a
     d-c dimensional object.

     Entities of codimension=dimension ("vertices") are defined through
     template specialization. Note that this specialization has an
     extended interface compared to the general case
   */
  template<int dim, class GridImp>
  class SEntity<dim,dim,GridImp> : public SEntityBase<dim,dim,GridImp>,
                                   public EntityDefaultImplementation <dim,dim,GridImp,SEntity>
  {
    enum { dimworld = GridImp::dimensionworld };
  public:
    typedef typename GridImp::template Codim<dim>::Geometry Geometry;
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;

    // disambiguate member functions with the same name in both bases
    //! level of this element
    int level () const {return SEntityBase<dim,dim,GridImp>::level();}

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index () const {return SEntityBase<dim,dim,GridImp>::index();}

    //! only interior entities
    PartitionType partitionType () const { return InteriorEntity; }

    //! geometry of this entity
    const Geometry& geometry () const {return SEntityBase<dim,dim,GridImp>::geometry();}

    /**
       @brief Location of this vertex within a mesh entity of
       codimension 0 on the coarse grid.

       This can speed up on-the-fly interpolation for linear conforming
       elements Possibly this is sufficient for all applications we want
       on-the-fly.
     */
    EntityPointer ownersFather () const;

    //! local coordinates within father
    const FieldVector<sgrid_ctype, dim>& positionInOwnersFather () const;

    // members specific to SEntity
    //! constructor
    SEntity (GridImp* _grid, int _l, int _id) : SEntityBase<dim,dim,GridImp>::SEntityBase(_grid,_l,_id)
    {
      built_father = false;
    }

    //! Reinitialization
    void make (int _l, int _id)
    {
      SEntityBase<dim,dim,GridImp>::make(_l,_id);
      built_father = false;
    }

  private:
    mutable bool built_father;
    mutable int father_id;
    mutable FieldVector<sgrid_ctype, dim> in_father_local;
    void make_father() const;
  };

  template<int codim, int dim, class GridImp>
  class SMakeableEntity :
    public GridImp::template Codim<codim>::Entity
  {
  public:
    SMakeableEntity(GridImp* _grid, int _l, int _id) :
      GridImp::template Codim<codim>::Entity (SEntity<codim, dim, GridImp>(_grid,_l,_id))
    {};
    SMakeableEntity(const SEntity<codim, dim, GridImp>& e) :
      GridImp::template Codim<codim>::Entity (e)
    {};
    void make (int _l, int _id) { this->realEntity.make(_l, _id); }
  };

  //************************************************************************
  /*! Mesh entities of codimension 0 ("elements") allow to visit all entities of
     codimension 0 obtained through nested, hierarchic refinement of the entity.
     Iteration over this set of entities is provided by the HIerarchicIterator,
     starting from a given entity.
     This is redundant but important for memory efficient implementations of unstructured
     hierarchically refined meshes.
   */
  struct SHierarchicStackElem {
    int l;
    int id;
    SHierarchicStackElem () : l(-1), id(-1) {}
    SHierarchicStackElem (int _l, int _id) {l=_l; id=_id;}
    bool operator== (const SHierarchicStackElem& s) const {return !operator!=(s);}
    bool operator!= (const SHierarchicStackElem& s) const {return l!=s.l || id!=s.id;}
  };

  template<class GridImp>
  class SHierarchicIterator :
    public Dune::SEntityPointer <0,GridImp>,
    public HierarchicIteratorDefaultImplementation <GridImp,SHierarchicIterator>
  {
    friend class SHierarchicIterator<const GridImp>;
    enum { dim = GridImp::dimension };
    enum { dimworld = GridImp::dimensionworld };
  public:
    typedef typename GridImp::template Codim<0>::Entity Entity;
    typedef typename GridImp::ctype ctype;

    //! increment
    void increment();

    /*! constructor. Here is how it works: If with_sons is true, push start
       element and all its sons on the stack, so the initial element is popped
       last. For an end iterator, push the starting element and no sons. Then
       the iteration will stop when both iterators have the same id AND the
       stack is empty
     */
    SHierarchicIterator (GridImp* _grid,
                         const Dune::SEntity<0,GridImp::dimension,GridImp>& _e,
                         int _maxLevel, bool makeend) :
      Dune::SEntityPointer<0,GridImp>(_grid,_e.level(),_e.index())
    {
      // without sons, we are done
      // (the end iterator is equal to the calling iterator)
      if (makeend) return;

      // remember element where begin has been called
      orig_l = this->e.level();
      orig_id = _grid->template getRealEntity<0>(this->e).index();

      // push original element on stack
      SHierarchicStackElem originalElement(orig_l, orig_id);
      stack.push(originalElement);

      // compute maxLevel
      maxLevel = std::min(_maxLevel,this->grid->maxLevel());

      // ok, push all the sons as well
      push_sons(orig_l,orig_id);

      // and pop the first son
      increment();
    }

  private:
    int maxLevel;              //!< maximum level of elements to be processed
    int orig_l, orig_id;       //!< element where begin was called (the root of the tree to be processed)

    FiniteStack<SHierarchicStackElem,GridImp::MAXL> stack;    //!< stack holding elements to be processed

    void push_sons (int level, int fatherid); //!< push all sons of this element on the stack
  };

  //************************************************************************
  /*! Mesh entities of codimension 0 ("elements") allow to visit all neighbors, where
     a neighbor is an entity of codimension 0 which has a common entity of codimension 1 with the entity.
     These neighbors are accessed via a IntersectionIterator. This allows the implementation of
     non-matching meshes. The number of neigbors may be different from the number of faces/edges
     of an element!
   */
  template<class GridImp>
  class SIntersectionIterator :
    public IntersectionIteratorDefaultImplementation <GridImp,SIntersectionIterator>
  {
    enum { dim=GridImp::dimension };
    enum { dimworld=GridImp::dimensionworld };
  public:
    typedef typename GridImp::template Codim<0>::Entity Entity;
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;
    typedef typename GridImp::template Codim<1>::Geometry Geometry;
    typedef typename GridImp::template Codim<1>::LocalGeometry LocalGeometry;
    //! know your own dimension
    enum { dimension=dim };
    //! know your own dimension of world
    enum { dimensionworld=dimworld };
    //! define type used for coordinates in grid module
    typedef typename GridImp::ctype ctype;

    //! ask for level of intersection
    int level () const;

    //! equality
    bool equals(const SIntersectionIterator<GridImp>& i) const;
    //! increment
    void increment();

    //! return EntityPointer to the Entity on the inside of this intersection
    //! (that is the Entity where we started this Iterator)
    EntityPointer inside() const;

    //! return EntityPointer to the Entity on the outside of this intersection
    //! (that is the neighboring Entity)
    EntityPointer outside() const;

    //! return true if intersection is with boundary.
    bool boundary () const;

    int boundaryId () const {
      if (boundary()) return count + 1;
      return 0;
    };
    //! return true if neighbor on this level exists
    bool neighbor () const;

    //! return outer normal
    FieldVector<ctype, GridImp::dimensionworld> outerNormal (const FieldVector<ctype, GridImp::dimension-1>& local) const
    {
      return unitOuterNormal(local);
    }
    //! return unit outer normal
    FieldVector<ctype, GridImp::dimensionworld> unitOuterNormal (const FieldVector<ctype, GridImp::dimension-1>& local) const
    {
      // while we are at it, compute normal direction
      FieldVector<sgrid_ctype, dimworld> normal(0.0);
      if (count%2)
        normal[count/2] =  1.0; // odd
      else
        normal[count/2] = -1.0; // even

      return normal;
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of the element where iteration started.
     */
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

    //! constructor
    SIntersectionIterator (GridImp* _grid, const SEntity<0,dim,GridImp >* _self, int _count);

    //! assignment operator
    SIntersectionIterator& operator = (const SIntersectionIterator& it)
    {
      assert(grid == it.grid);

      /* Assert same Iterator Context */
      if (! self.equals(it.self))
        DUNE_THROW(GridError, "assignment of SIntersectionIterator "
                   << "with different inside Entity");
      if (partition != it.partition)
        DUNE_THROW(GridError, "assignment of SIntersectionIterator "
                   << "with different partition");
      // (zred == it.zred) if (self == it.self)

      /* Assign current position and update ne */
      ne = it.ne;
      count = it.count;
      make(count);

      return *this;
    }

  private:
    void make (int _count) const;               //!< reinitialze iterator with given neighbor
    void makeintersections () const;            //!< compute intersections
    const EntityPointer self;                   //!< EntityPointer for myself
    mutable EntityPointer ne;                   //!< EntityPointer for neighbor
    const GridImp * grid;                       //!< Pointer to the grid
    const int partition;                        //!< partition number of self, needed for coordinate expansion
    const FixedArray<int,dim> zred;               //!< reduced coordinates of myself, allows easy computation of neighbors
    mutable int count;                            //!< number of neighbor
    mutable bool valid_count;                     //!< true if count is in range
    mutable bool valid_nb;                        //!< true if nb is initialized
    mutable bool is_on_boundary;                  //!< true if neighbor is otside the domain
    mutable bool built_intersections;             //!< true if all intersections have been built
    mutable SMakeableGeometry<dim-1,dim,const GridImp> is_self_local;    //!< intersection in own local coordinates
    mutable SMakeableGeometry<dim-1,dimworld,const GridImp> is_global;   //!< intersection in global coordinates, map consistent with is_self_local
    mutable SMakeableGeometry<dim-1,dim,const GridImp> is_nb_local;      //!< intersection in neighbors local coordinates
  };

  //************************************************************************

  /*! Acts as a pointer to an  entities of a given codimension.
   */
  template<int codim, class GridImp>
  class SEntityPointer :
    public EntityPointerDefaultImplementation <codim, GridImp,
        Dune::SEntityPointer<codim,GridImp> >
  {
    enum { dim = GridImp::dimension };
    friend class SIntersectionIterator<GridImp>;
  public:
    typedef SEntityPointer<codim,GridImp> Base;
    typedef typename GridImp::template Codim<codim>::Entity Entity;

    //! equality
    bool equals(const SEntityPointer<codim,GridImp>& i) const;
    //! dereferencing
    Entity& dereference() const;
    //! ask for level of entity
    int level () const;

    //! constructor
    SEntityPointer (GridImp * _grid, int _l, int _id) :
      grid(_grid), l(_l), id(_id), e(_grid,_l,_id) { }

    //! constructor
    SEntityPointer (const SEntity<codim,dim,GridImp> & _e) :
      grid(_e.grid), l(_e.l), id(_e.id), e(_e) { }

  protected:
    GridImp* grid;               //!< my grid
    int l;                       //!< level where element is on
    mutable int id;              //!< my consecutive id
    mutable SMakeableEntity<codim,dim,GridImp> e; //!< virtual entity
  };

  //************************************************************************

  /*! Enables iteration over all entities of a given codimension and level of a grid.
   */
  template<int codim, PartitionIteratorType pitype, class GridImp>
  class SLevelIterator :
    public Dune::SEntityPointer <codim,GridImp>,
    public LevelIteratorDefaultImplementation <codim,pitype,GridImp,SLevelIterator>
  {
    friend class SLevelIterator<codim, pitype,const GridImp>;
    enum { dim = GridImp::dimension };
  public:
    typedef typename GridImp::template Codim<codim>::Entity Entity;

    //! increment
    void increment();

    //! constructor
    SLevelIterator (GridImp * _grid, int _l, int _id) :
      Dune::SEntityPointer<codim,GridImp>(_grid,_l,_id) {}
  };


  //========================================================================
  /*!
     \brief implementation of index set

   */
  //========================================================================

  template <class GridImp>
  struct SGridLevelIndexSetTypes
  {
    //! The types
    template<int cd>
    struct Codim
    {
      template<PartitionIteratorType pitype>
      struct Partition
      {
        typedef typename GridImp::Traits::template Codim<cd>::template Partition<pitype>::LevelIterator Iterator;
      };
    };
  };

  template<class GridImp>
  class SGridLevelIndexSet : public IndexSetDefaultImplementation<GridImp,SGridLevelIndexSet<GridImp>,SGridLevelIndexSetTypes<GridImp> >
  {
    typedef IndexSet<GridImp,SGridLevelIndexSet<GridImp>,SGridLevelIndexSetTypes<GridImp> > Base;
  public:

    //! constructor stores reference to a grid and level
    SGridLevelIndexSet (const GridImp& g, int l) : grid(g), level(l)
    {
      // contains a single element type;
      for (int codim=0; codim<=GridImp::dimension; codim++)
        mytypes[codim].push_back(GeometryType(GeometryType::cube,GridImp::dimension-codim));
    }

    //! get index of an entity
    template<int cd>
    int index (const typename GridImp::Traits::template Codim<cd>::Entity& e) const
    {
      return grid.template getRealEntity<cd>(e).compressedIndex();
    }

    //! get index of subentity of a codim 0 entity
    template<int cc>
    int subIndex (const typename GridImp::Traits::template Codim<0>::Entity& e, int i) const
    {
      return grid.template getRealEntity<0>(e).template subCompressedIndex<cc>(i);
    }

    //! get number of entities of given type and level (the level is known to the object)
    int size (GeometryType type) const
    {
      return grid.size(level,GridImp::dimension-type.dim());
    }

    //! deliver all geometry types used in this grid
    const std::vector<GeometryType>& geomTypes (int codim) const
    {
      return mytypes[codim];
    }

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Base::template Codim<cd>::template Partition<pitype>::Iterator begin () const
    {
      return grid.template lbegin<cd,pitype>(level);
    }

    //! Iterator to one past the last entity of given codim on level for partition type
    template<int cd, PartitionIteratorType pitype>
    typename Base::template Codim<cd>::template Partition<pitype>::Iterator end () const
    {
      return grid.template lend<cd,pitype>(level);
    }

  private:
    const GridImp& grid;
    int level;
    std::vector<GeometryType> mytypes[GridImp::dimension+1];
  };

  // Leaf Index Set

  template <class GridImp>
  struct SGridLeafIndexSetTypes
  {
    //! The types
    template<int cd>
    struct Codim
    {
      template<PartitionIteratorType pitype>
      struct Partition
      {
        typedef typename GridImp::Traits::template Codim<cd>::template Partition<pitype>::LevelIterator Iterator;
      };
    };
  };

  template<class GridImp>
  class SGridLeafIndexSet : public IndexSetDefaultImplementation<GridImp,SGridLeafIndexSet<GridImp>,SGridLeafIndexSetTypes<GridImp> >
  {
    typedef IndexSet<GridImp,SGridLeafIndexSet<GridImp>,SGridLeafIndexSetTypes<GridImp> > Base;
    enum {dim = RemoveConst<GridImp>::Type::dimension};
  public:

    //! constructor stores reference to a grid and level
    SGridLeafIndexSet (const GridImp& g) : grid(g)
    {
      // contains a single element type;
      for (int codim=0; codim<=dim; codim++)
        mytypes[codim].push_back(GeometryType(GeometryType::cube,dim-codim));
    }

    //! get index of an entity
    /*
       We use the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
     */
    template<int cd>
    int index (const typename RemoveConst<GridImp>::Type::Traits::template Codim<cd>::Entity& e) const
    {
      return grid.template getRealEntity<cd>(e).compressedLeafIndex();
    }

    //! get index of subentity of a codim 0 entity
    /*
       We use the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
     */
    template<int cc>
    int subIndex (const typename RemoveConst<GridImp>::Type::Traits::template Codim<0>::Entity& e, int i) const
    {
      return grid.template getRealEntity<0>(e).template subCompressedLeafIndex<cc>(i);
    }

    //! get number of entities of given type
    int size (GeometryType type) const
    {
      return grid.size(grid.maxLevel(),GridImp::dimension-type.dim());
    }

    //! deliver all geometry types used in this grid
    const std::vector<GeometryType>& geomTypes (int codim) const
    {
      return mytypes[codim];
    }

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Base::template Codim<cd>::template Partition<pitype>::Iterator begin () const
    {
      return grid.template lbegin<cd,pitype>(grid.maxLevel());
    }

    //! Iterator to one past the last entity of given codim on level for partition type
    template<int cd, PartitionIteratorType pitype>
    typename Base::template Codim<cd>::template Partition<pitype>::Iterator end () const
    {
      return grid.template lend<cd,pitype>(grid.maxLevel());
    }

  private:
    const GridImp& grid;
    std::vector<GeometryType> mytypes[dim+1];
  };


  //========================================================================
  /*!
     \brief persistent, globally unique Ids

   */
  //========================================================================

  template<class GridImp>
  class SGridGlobalIdSet :
    public IdSetDefaultImplementation<GridImp,SGridGlobalIdSet<GridImp>, typename RemoveConst<GridImp>::Type::PersistentIndexType>
    /*
       We used the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
     */
  {
  public:
    //! define the type used for persisitent indices
    /*
       We use the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
     */
    typedef typename RemoveConst<GridImp>::Type::PersistentIndexType IdType;

    //! constructor stores reference to a grid
    SGridGlobalIdSet (const GridImp& g) : grid(g) {}

    //! get id of an entity
    /*
       We use the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
     */
    template<int cd>
    IdType id (const typename RemoveConst<GridImp>::Type::Traits::template Codim<cd>::Entity& e) const
    {
      return grid.template getRealEntity<cd>(e).persistentIndex();
    }

    //! get id of subentity
    /*
       We use the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
     */
    template<int cc>
    IdType subId (const typename RemoveConst<GridImp>::Type::Traits::template Codim<0>::Entity& e, int i) const
    {
      return grid.template getRealEntity<0>(e).template subPersistentIndex<cc>(i);
    }

  private:
    const GridImp& grid;
  };


  template<int dim, int dimworld>
  struct SGridFamily
  {
    typedef GridTraits<dim,dimworld,Dune::SGrid<dim,dimworld>,
        SGeometry,SEntity,
        SEntityPointer,SLevelIterator,
        SIntersectionIterator,SHierarchicIterator,
        SLevelIterator,
        SGridLevelIndexSet<const SGrid<dim,dimworld> >,
        SGridLevelIndexSetTypes<const SGrid<dim,dimworld> >,
        SGridLeafIndexSet<const SGrid<dim,dimworld> >,
        SGridLeafIndexSetTypes<const SGrid<dim,dimworld> >,
        SGridGlobalIdSet<const SGrid<dim,dimworld> >,
        bigunsignedint<dim*sgrid_dim_bits+sgrid_level_bits+sgrid_codim_bits>,
        SGridGlobalIdSet<const SGrid<dim,dimworld> >,
        bigunsignedint<dim*sgrid_dim_bits+sgrid_level_bits+sgrid_codim_bits>,
        CollectiveCommunication<Dune::SGrid<dim,dimworld> > >
    Traits;
  };


  //************************************************************************
  /**
     \brief [<em> provides \ref Dune::Grid </em>]
     \brief A structured mesh in d dimensions consisting of "cubes".
     \ingroup GridImplementations

          This module describes the pilot implementation of the Dune grid interface.
          It implements the grid interface for simple structured meshes.

          The following class diagram shows how the classes are related with
          each other:

          \image html sgridclasses.png "Class diagram for classes in the grid interface"
          \image latex sgridclasses.eps "Class diagram for classes in the grid interface" width=\textwidth

          Short description of the classes:

          - SGeometry is a class template providing the geometric part of a grid entity, i.e. a general polyhedron
          with a mapping from a reference polyhedron to the actual polyhedron.

          - SLevelIterator is a class template which allows to iterate over all grid entities of a given
          codimension and level.

          - SEntity is a class template realizing the grid entities. Grid entities are the constituents
          of a grid. Grid entities of codimension 0 and codimension dim are defines through specialization.
          Entities can be used as template parameters to generic algorithms. Each entity must therefore
          provide the nested classes Geometry, LevelIterator, HierarchicIterator and IntersectionIterator.
          Geometry and LevelIterator are derived from the classes SELement and SLevelIterator.
          Note that entities of codimension 0 and dim have an extended interface.

          - SEntity::IntersectionIterator provides access to all entities of codimension 0 sharing an object of codimension 1
          with the given entity of codimension 0. This interface covers nonmatching grids.

          - SEntity::HierarchicIterator provides access to the sons of an entity of codimension 0.

          - SGrid is conceptualized as a container of grid entities of various codimensions. Since grids
          are used as template parameters to generic algorithms they must include the nested classes
          LevelIterator and Entity which are derived from SLevelIterator and SEntity.

     A Grid is a container of grid entities. Given a dimension dim these entities have a
     codimension codim with 0 <= codim <= dim.

     The Grid is assumed to be hierachically refined and nested. It enables iteration over
     entities of a given level and codimension.

     The grid can consist of several subdomains and it can be non-matching.

     All information is provided to allocate degrees of freedom in appropriate vector
     data structures (which are not part of this module).
   */
  template<int dim, int dimworld>
  class SGrid : public GridDefaultImplementation <dim,dimworld,sgrid_ctype,SGridFamily<dim,dimworld> >
  {
  public:
    typedef SGridFamily<dim,dimworld> GridFamily;
    typedef bigunsignedint<dim*sgrid_dim_bits+sgrid_level_bits+sgrid_codim_bits> PersistentIndexType;

    // need for friend declarations in entity
    typedef SGridLevelIndexSet<SGrid<dim,dimworld> > LevelIndexSetType;
    typedef SGridLeafIndexSet<SGrid<dim,dimworld> > LeafIndexSetType;
    typedef SGridGlobalIdSet<SGrid<dim,dimworld> > GlobalIdSetType;

    typedef typename SGridFamily<dim,dimworld>::Traits Traits;

    //! maximum number of levels allowed
    enum { MAXL=32 };

    //! define type used for coordinates in grid module
    typedef sgrid_ctype ctype;

    //! return GridIdentifierType of Grid, in this case SGrid_Id
    GridIdentifier type() const { return SGrid_Id; };

    // constructors

    /*! @brief Make an SGrid from extend and number of cells per direction

       \param[in] N_ number of cells in each direction on coarsest level
       \param[in] H_ extend of the unit cube in each dimension

       Note: The origin of the cube is always at (0,0,...,0), only the extend is given.
     */
    SGrid (const int* N_, const sgrid_ctype* H_);

    /*! @brief Make an SGrid from position, extend and number of cells per direction

       \param[in] N_ number of cells in each direction on coarsest level
       \param[in] L_ position of origin of the cube
       \param[in] H_ extend of the unit cube in each dimension

     */
    SGrid (const int* N_, const sgrid_ctype* L_, const sgrid_ctype* H_);

    /*! @brief Make an SGrid from position, extend and number of cells per direction

       \param[in] N_ number of cells in each direction on coarsest level
       \param[in] L_ position of origin of the cube
       \param[in] H_ extend of the unit cube in each dimension

       Note: This constructor uses FieldVectors instead of built-in arrays. This is compatible
          with the YaspGrid class.
     */
    SGrid (FieldVector<int,dim> N_, FieldVector<sgrid_ctype,dim> L_, FieldVector<sgrid_ctype,dim> H_);

    //! @brief empty constructor making grid of unit square discretized with one cell
    SGrid ();

    /*! Return maximum level defined in this grid. Levels are numbered
          0 ... maxLevel with 0 the coarsest level.   */
    int maxLevel() const;

    //! Iterator to first entity of given codim on level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template Codim<cd>::template Partition<pitype>::LevelIterator lbegin (int level) const;

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template Codim<cd>::template Partition<pitype>::LevelIterator lend (int level) const;

    //! Iterator to first entity of given codim on level
    template<int cd>
    typename Traits::template Codim<cd>::template Partition<All_Partition>::LevelIterator lbegin (int level) const
    {
      return lbegin<cd,All_Partition>(level);
    }

    //! one past the end on this level
    template<int cd>
    typename Traits::template Codim<cd>::template Partition<All_Partition>::LevelIterator lend (int level) const
    {
      return lend<cd,All_Partition>(level);
    }

    //! return LeafIterator which points to the first entity
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template Codim<cd>::template Partition<pitype>::LeafIterator leafbegin () const;

    //! one past the end on the leaf level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template Codim<cd>::template Partition<pitype>::LeafIterator leafend () const;

    //! return LeafIterator which points to the first entity
    template<int cd>
    typename Traits::template Codim<cd>::template Partition<All_Partition>::LeafIterator leafbegin () const
    {
      return leafbegin<cd,All_Partition>();
    }

    //! return LeafIterator which points behind the last entity
    template<int cd>
    typename Traits::template Codim<cd>::template Partition<All_Partition>::LeafIterator leafend () const
    {
      return leafend<cd,All_Partition>();
    }

    /*! The communication interface
          @param T: array class holding data associated with the entities
          @param P: type used to gather/scatter data in and out of the message buffer
          @param codim: communicate entites of given codim
          @param if: one of the predifined interface types, throws error if it is not implemented
          @param level: communicate for entities on the given level

          Implements a generic communication function sending an object of type P for each entity
       in the intersection of two processors. P has two methods gather and scatter that implement
       the protocol. Therefore P is called the "protocol class".
     */
    template<class T, template<class> class P, int codim>
    void communicate (T& t, InterfaceType iftype, CommunicationDirection dir, int level)
    {
      // SGrid is sequential and has no periodic boundaries, so do nothing ...
      return;
    }

    //! number of grid entities per level and codim
    int size (int level, int codim) const;

    //! number of leaf entities per codim in this process
    int size (int codim) const
    {
      return size(maxLevel(),codim);
    }

    //! number of entities per level and geometry type in this process
    int size (int level, GeometryType type) const
    {
      return (type.isCube()) ? size(level,dim-type.dim()) : 0;
    }

    //! number of leaf entities per codim and geometry type in this process
    int size (GeometryType type) const
    {
      return size(maxLevel(),type);
    }


    //! number of grid entities of all level for given codim
    int global_size (int codim) const;

    //! return size (= distance in graph) of overlap region
    int overlapSize (int level, int codim)
    {
      return 0;
    }

    //! return size (= distance in graph) of ghost region
    int ghostSize (int level, int codim)
    {
      return 0;
    }

    // these are all members specific to sgrid

    /** \brief Refine mesh globally by one refCount levels */
    void globalRefine (int refCount);

    /** \brief Get number of elements in each coordinate direction */
    const FixedArray<int, dim>& dims(int level) const {
      return N[level];
    }

    /** \brief Get lower left corner */
    const FieldVector<sgrid_ctype, dimworld>& lowerLeft() const {
      return low;
    }

    /** \brief Get upper right corner */
    FieldVector<sgrid_ctype, dimworld> upperRight() const {
      return H;
    }

    //! map adapt to global refine
    bool adapt ()
    {
      globalRefine(1);
      return true;
    }


    //! map expanded coordinates to position
    FieldVector<sgrid_ctype, dim> pos (int level, FixedArray<int,dim>& z) const;

    //! compute codim from coordinate
    int calc_codim (int level, const FixedArray<int,dim>& z) const;

    //! compute number from expanded coordinate
    int n (int level, const FixedArray<int,dim> z) const;

    //! compute coordinates from number and codimension
    FixedArray<int,dim> z (int level, int i, int codim) const;

    //! compress from expanded coordinates to grid for a single partition number
    FixedArray<int,dim> compress (int level, const FixedArray<int,dim>& z) const;

    //! expand with respect to partition number
    FixedArray<int,dim> expand (int level, const FixedArray<int,dim>& r, int b) const;

    /*! There are \f$2^d\f$ possibilities of having even/odd coordinates.
          The binary representation is called partition number.
     */
    int partition (int level, const FixedArray<int,dim>& z) const;

    //! given reduced coordinates of an element, determine if element is in the grid
    bool exists (int level, const FixedArray<int,dim>& zred) const;


    // The new index sets from DDM 11.07.2005
    const typename Traits::GlobalIdSet& globalIdSet() const
    {
      return theglobalidset;
    }

    const typename Traits::LocalIdSet& localIdSet() const
    {
      return theglobalidset;
    }

    const typename Traits::LevelIndexSet& levelIndexSet(int level) const
    {
      assert(level>=0 && level<=maxLevel());
      return *(indexsets[level]);
    }

    const typename Traits::LeafIndexSet& leafIndexSet() const
    {
      return theleafindexset;
    }

    // dummy parallel functions

    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir, int level) const
    {}

    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir) const
    {}

    const CollectiveCommunication<SGrid>& comm () const
    {
      return ccobj;
    }

    //! return size (= distance in graph) of overlap region
    int overlapSize (int level, int codim) const
    {
      return 0;
    }

    //! return size (= distance in graph) of overlap region
    int overlapSize (int codim) const
    {
      return 0;
    }

    //! return size (= distance in graph) of ghost region
    int ghostSize (int level, int codim) const
    {
      return 0;
    }

    //! return size (= distance in graph) of ghost region
    int ghostSize (int codim) const
    {
      return 0;
    }

    SIntersectionIterator<const SGrid<dim, dimworld> >&
    getRealIntersectionIterator(typename Traits::IntersectionIterator& it)
    {
      return this->getRealImplementation(it);
    }

    const SIntersectionIterator<const SGrid<dim, dimworld> >&
    getRealIntersectionIterator(const typename Traits::IntersectionIterator& it) const
    {
      return this->getRealImplementation(it);
    }


  private:

    CollectiveCommunication<SGrid> ccobj;

    std::vector<SGridLevelIndexSet<const SGrid<dim,dimworld> >*> indexsets;
    SGridLeafIndexSet<const SGrid<dim,dimworld> > theleafindexset;
    SGridGlobalIdSet<const SGrid<dim,dimworld> > theglobalidset;

    // Index classes need access to the real entity
    friend class Dune::SGridLevelIndexSet<Dune::SGrid<dim,dimworld> >;
    friend class Dune::SGridLeafIndexSet<Dune::SGrid<dim,dimworld> >;
    friend class Dune::SGridGlobalIdSet<Dune::SGrid<dim,dimworld> >;
    friend class Dune::SIntersectionIterator<Dune::SGrid<dim,dimworld> >;
    friend class Dune::SHierarchicIterator<Dune::SGrid<dim,dimworld> >;
    friend class Dune::SEntity<0,dim,Dune::SGrid<dim,dimworld> >;

    friend class Dune::SGridLevelIndexSet<const Dune::SGrid<dim,dimworld> >;
    friend class Dune::SGridLeafIndexSet<const Dune::SGrid<dim,dimworld> >;
    friend class Dune::SGridGlobalIdSet<const Dune::SGrid<dim,dimworld> >;
    friend class Dune::SIntersectionIterator<const Dune::SGrid<dim,dimworld> >;
    friend class Dune::SHierarchicIterator<const Dune::SGrid<dim,dimworld> >;
    friend class Dune::SEntity<0,dim,const Dune::SGrid<dim,dimworld> >;

    template<int codim>
    SEntity<codim,dim,const SGrid<dim,dimworld> >& getRealEntity(typename Traits::template Codim<codim>::Entity& e )
    {
      return this->getRealImplementaion(e);
    }

    template<int codim>
    const SEntity<codim,dim,const SGrid<dim,dimworld> >& getRealEntity(const typename Traits::template Codim<codim>::Entity& e ) const
    {
      return this->getRealImplementation(e);
    }

    template<int codim_, int dim_, class GridImp_, template<int,int,class> class EntityImp_>
    friend class Entity;

    // diasable copy and assign
    SGrid(const SGrid &) {};
    SGrid & operator = (const SGrid &) { return *this; };
    // generate SGrid
    void makeSGrid (const int* N_,  const sgrid_ctype* L_, const sgrid_ctype* H_);

    int L;                        // number of levels in hierarchic mesh 0<=level<L
    FieldVector<sgrid_ctype, dim> low;   // lower left corner of the grid
    FieldVector<sgrid_ctype, dim> H;     // length of cube per direction
    FixedArray<int,dim> N[MAXL];       // number of elements per direction
    FieldVector<sgrid_ctype, dim> h[MAXL]; // mesh size per direction
    mutable CubeMapper<dim> mapper[MAXL]; // a mapper for each level

    // faster implemantation od subIndex
    mutable FixedArray <int,dim> zrefStatic;   // for subIndex of SEntity
    mutable FixedArray <int,dim> zentityStatic; // for subIndex of SEntity
  };

  namespace Capabilities
  {

    template<int dim,int dimw>
    struct hasLeafIterator< SGrid<dim,dimw> >
    {
      static const bool v = true;
    };

    template<int dim, int dimw, int cdim>
    struct hasEntity< SGrid<dim,dimw>, cdim>
    {
      static const bool v = true;
    };

    template<int dim,int dimw>
    struct hasLeafIterator< const SGrid<dim,dimw> >
    {
      static const bool v = true;
    };

    template<int dim, int dimw>
    struct isLevelwiseConforming< SGrid<dim,dimw> >
    {
      static const bool v = true;
    };

    template<int dim, int dimw>
    struct hasHangingNodes< SGrid<dim,dimw> >
    {
      static const bool v = false;
    };

  } // end namespace Capabilities

} // end namespace Dune

#include "sgrid/sgrid.cc"

#endif
