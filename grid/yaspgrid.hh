// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __YASPGRID_HH__
#define __YASPGRID_HH__

#include <iostream>

#include "common/grid.hh"     // the grid base classes
#include "yaspgrid/grids.hh"  // the yaspgrid base classes
#include "../common/stack.hh" // the stack class
#include "../common/capabilities.hh" // the capabilities
#include "../common/helpertemplates.hh"

/*! \file yaspgrid.hh
   Yasppergrid stands for yet another structured parallel grid.
   It will implement the dune grid interface for structured grids with codim 0
   and dim, with arbitrary overlap, parallel features with two overlap
   models, periodic boundaries and fast a implementation allowing on-the-fly computations.
 */

namespace Dune {

  /** @defgroup YaspGrid Yet Another Structured Parallel Grid (YaspGrid)
      \ingroup GridCommon

     Yaspgrid stands for yet another structured parallel grid.
     It will implement the dune grid interface for structured grids with codim 0
     and dim, with arbitrary overlap, parallel features with two overlap
     models, periodic boundaries and fast a implementation allowing on-the-fly computations.


     History:
     - started on July 31, 2004 by PB based on abstractions developed in summer 2003

     @{
   */

  //************************************************************************
  /*! define name for floating point type used for coordinates in yaspgrid.
     You can change the type for coordinates by changing this single typedef.
   */
  typedef double yaspgrid_ctype;

  static const yaspgrid_ctype yasptolerance=1E-13; // tolerance in coordinate computations

  //************************************************************************
  // forward declaration of templates

  template<int dim, int dimworld>               class YaspGrid;
  template<int mydim, int cdim, class GridImp>  class YaspGeometry;
  template<int codim, int dim, class GridImp>   class YaspEntity;
  template<int codim, PartitionIteratorType pitype, class GridImp> class YaspLevelIterator;
  template<class GridImp>            class YaspIntersectionIterator;
  template<class GridImp>            class YaspHierarchicIterator;
  template<class GridImp>            class YaspBoundaryEntity;


  //========================================================================
  // The reference elements
#if 0
  /** Singleton holding reference elements */
  template<int dim>
  struct YaspReferenceElement {
    static FieldVector<yaspgrid_ctype, dim> midpoint; // data neded for the refelem below
    static FieldVector<yaspgrid_ctype, dim> extension; // data needed for the refelem below
    static YaspGeometry<dim,dim> refelem;
  };

  // initialize static variable with bool constructor (which makes reference elements)
  template<int dim>
  YaspGeometry<dim,dim> YaspReferenceElement<dim>::refelem(YaspReferenceElement<dim>::midpoint,
                                                           YaspReferenceElement<dim>::extension);
  template<int dim>
  FieldVector<yaspgrid_ctype, dim> YaspReferenceElement<dim>::midpoint(0.5);

  template<int dim>
  FieldVector<yaspgrid_ctype, dim> YaspReferenceElement<dim>::extension(1.0);

  template<int dim>
  class YaspFatherRelativeLocalElement {
  public:
    static FieldVector<yaspgrid_ctype, dim> midpoint;  // data neded for the refelem below
    static FieldVector<yaspgrid_ctype, dim> extension; // data needed for the refelem below
    static YaspGeometry<dim,dim> element;
    static YaspGeometry<dim,dim>& getson (int i)
    {
      for (int k=0; k<dim; k++)
        if (i&(1<<k))
          midpoint[k] = 0.75;
        else
          midpoint[k] = 0.25;
      return element;
    }
  };

  // initialize static variable with bool constructor (which makes reference elements)
  template<int dim>
  YaspGeometry<dim,dim> YaspFatherRelativeLocalElement<dim>::element(YaspFatherRelativeLocalElement<dim>::midpoint,
                                                                     YaspFatherRelativeLocalElement<dim>::extension);
  template<int dim>
  FieldVector<yaspgrid_ctype, dim> YaspFatherRelativeLocalElement<dim>::midpoint(0.25);

  template<int dim>
  FieldVector<yaspgrid_ctype, dim> YaspFatherRelativeLocalElement<dim>::extension(0.5);

#endif
  //========================================================================
  /*!
     YaspGeometry realizes the concept of the geometric part of a mesh entity.

     We have specializations for dim==dimworld (elements),
     dim = dimworld-1 (faces) and dim=0 (vertices).
     The general version throws a GridError on construction.
   */
  //========================================================================

  template<int mydim, int cdim, class GridImp>
  class YaspSpecialGeometry : public Geometry<mydim, cdim, GridImp, YaspGeometry>
  {
    //! define type used for coordinates in grid module
    typedef typename GridImp::ctype ctype;
  public:
    YaspSpecialGeometry(const FieldVector<ctype, cdim>& p, const FieldVector<ctype, cdim>& h, int& m) :
      Geometry<mydim, cdim, GridImp, YaspGeometry>(YaspGeometry<mydim, cdim, GridImp>(p,h,m))
    {}
    YaspSpecialGeometry() :
      Geometry<mydim, cdim, GridImp, YaspGeometry>(YaspGeometry<mydim, cdim, GridImp>(false))
    {};
  };

  template<int mydim, class GridImp>
  class YaspSpecialGeometry<mydim,mydim,GridImp> : public Geometry<mydim, mydim, GridImp, YaspGeometry>
  {
    //! define type used for coordinates in grid module
    typedef typename GridImp::ctype ctype;
  public:
    YaspSpecialGeometry(const FieldVector<ctype, mydim>& p, const FieldVector<ctype, mydim>& h) :
      Geometry<mydim, mydim, GridImp, YaspGeometry>(YaspGeometry<mydim, mydim, GridImp>(p,h))
    {}
    YaspSpecialGeometry() :
      Geometry<mydim, mydim, GridImp, YaspGeometry>(YaspGeometry<mydim, mydim, GridImp>(false))
    {};
  };

  template<int cdim, class GridImp>
  class YaspSpecialGeometry<0,cdim,GridImp> : public Geometry<0, cdim, GridImp, YaspGeometry>
  {
    //! define type used for coordinates in grid module
    typedef typename GridImp::ctype ctype;
  public:
    YaspSpecialGeometry(const FieldVector<ctype, cdim>& p) :
      Geometry<0, cdim, GridImp, YaspGeometry>(YaspGeometry<0, cdim, GridImp>(p))
    {}
    YaspSpecialGeometry() :
      Geometry<0, cdim, GridImp, YaspGeometry>(YaspGeometry<0, cdim, GridImp>(false))
    {};
  };

  //! The general version implements dimworld==dimworld. If this is not the case an error is thrown
  template<int mydim,int cdim, class GridImp>
  class YaspGeometry : public GeometryDefault<mydim,cdim,GridImp,YaspGeometry>
  {
  public:
    //! define type used for coordinates in grid module
    typedef typename GridImp::ctype ctype;
    //! the Reference Geometry
    typedef Geometry<mydim,mydim,GridImp,Dune::YaspGeometry> ReferenceGeometry;

    //! return the element type identifier
    GeometryType type () const
    {
      switch (mydim)
      {
      case 1 : return line;
      case 2 : return quadrilateral;
      case 3 : return hexahedron;
      default : return unknown;
      }
    }

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const
    {
      return 1<<mydim;
    }

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<ctype, cdim>& operator[] (int i) const
    {
      int bit=0;
      for (int k=0; k<cdim; k++)   // run over all directions in world
      {
        if (k==missing)
        {
          c[k] = midpoint[k];
          continue;
        }
        //k is not the missing direction
        if (i&(1<<bit))          // check whether bit is set or not
          c[k] = midpoint[k]+0.5*extension[k];         // bit is 1 in i
        else
          c[k] = midpoint[k]-0.5*extension[k];         // bit is 0 in i
        bit++;         // we have processed a direction
      }

      return c;
    }

    /*! return reference element corresponding to this element.
            Usually, the implementation will store the finite
            set of reference elements as global variables.
            But why in the hell do we need this reference element?
     */
    static const ReferenceGeometry& refelem ()
    {
#if 0
      return YaspReferenceElement<mydim>::refelem;
#endif
    }

    //! maps a local coordinate within reference element to global coordinate in element
    FieldVector<ctype, cdim> global (const FieldVector<ctype, mydim>& local) const
    {
      FieldVector<ctype, cdim> g;
      int bit=0;
      for (int k=0; k<cdim; k++)
        if (k==missing)
          g[k] = midpoint[k];
        else
        {
          g[k] = midpoint[k] + (local[bit]-0.5)*extension[k];
          bit++;
        }
      return g;
    }

    //! maps a global coordinate within the element to a local coordinate in its reference element
    FieldVector<ctype, mydim> local (const FieldVector<ctype, cdim>& global) const
    {
      FieldVector<ctype, mydim> l; // result
      int bit=0;
      for (int k=0; k<cdim; k++)
        if (k!=missing)
        {
          l[bit] = (global[k]-midpoint[k])/extension[k] + 0.5;
          bit++;
        }
      return l;
    }

    /*! determinant of the jacobian of the mapping
     */
    ctype integrationElement (const FieldVector<ctype, mydim>& local) const
    {
      ctype volume=1.0;
      for (int k=0; k<cdim; k++)
        if (k!=missing) volume *= extension[k];
      return volume;
    }

    //! check whether local is inside reference element
    bool checkInside (const FieldVector<ctype, mydim>& local) const
    {
      for (int i=0; i<mydim; i++)
        if (local[i]<-yasptolerance || local[i]>1+yasptolerance) return false;
      return true;
    }

    //! constructor from (storage for) midpoint and extension and missing direction number
    YaspGeometry (const FieldVector<ctype, cdim>& p, const FieldVector<ctype, cdim>& h, int& m)
      : midpoint(p), extension(h), missing(m)
    {
      if (cdim!=mydim+1)
        DUNE_THROW(GridError, "general YaspGeometry assumes cdim=mydim+1");
    }

    //! print function
    void print (std::ostream& s) const
    {
      s << "YaspGeometry<"<<mydim<<","<<cdim<< "> ";
      s << "midpoint";
      for (int i=0; i<cdim; i++)
        s << " " << midpoint[i];
      s << " extension";
      for (int i=0; i<cdim; i++)
        s << " " << extension[i];
      s << " missing is " << missing;
    }

    const YaspGeometry<mydim,cdim,GridImp>&
    operator = (const YaspGeometry<mydim,cdim,GridImp>& g)
    {
      midpoint = g.midpoint;
      extension = g.extension;
      missing = g.missing;
      c = g.c;
      return *this;
    }

  private:
    // the element is fully defined by its midpoint the extension
    // in each direction and the missing direction.
    // References are used because this information
    // is known outside the element in many cases.
    // Note cdim==cdim+1

    // IMPORTANT midpoint and extension can't be references,
    // because they must stay the same when the iterator changes
    FieldVector<ctype, cdim> midpoint;  // the midpoint
    FieldVector<ctype, cdim> extension; // the extension
    int& missing;                       // the missing, i.e. constant direction

    // In addition we need memory in order to return references.
    // Possibly we should change this in the interface ...
    mutable FieldVector<ctype, cdim> c; // a point
  };



  //! specialize for dim=dimworld, i.e. a volume element
  template<int mydim, class GridImp>
  class YaspGeometry<mydim,mydim,GridImp> : public GeometryDefault<mydim,mydim,GridImp,YaspGeometry>
  {
  public:
    typedef typename GridImp::ctype ctype;
    //! the Reference Geometry
    typedef Geometry<mydim,mydim,GridImp,Dune::YaspGeometry> ReferenceGeometry;

    //! return the element type identifier
    GeometryType type () const
    {
      switch (mydim)
      {
      case 1 : return line;
      case 2 : return quadrilateral;
      case 3 : return hexahedron;
      default : return unknown;
      }
    }

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const
    {
      return 1<<mydim;
    }

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<ctype, mydim>& operator[] (int i) const
    {
      for (int k=0; k<mydim; k++)
        if (i&(1<<k))
          c[k] = midpoint[k]+0.5*extension[k];       // kth bit is 1 in i
        else
          c[k] = midpoint[k]-0.5*extension[k];       // kth bit is 0 in i
      return c;
    }

    /*! return reference element corresponding to this element.
            Usually, the implementation will store the finite
            set of reference elements as global variables.
            But why in the hell do we need this reference element?
     */
    static const ReferenceGeometry& refelem ()
    {
#if 0
      return YaspReferenceElement<mydim>::refelem;
#endif
    }

    //! maps a local coordinate within reference element to global coordinate in element
    FieldVector<ctype, mydim> global (const FieldVector<ctype, mydim>& local) const
    {
      FieldVector<ctype,mydim> g;
      for (int k=0; k<mydim; k++)
        g[k] = midpoint[k] + (local[k]-0.5)*extension[k];
      return g;
    }

    //! maps a global coordinate within the element to a local coordinate in its reference element
    FieldVector<ctype, mydim> local (const FieldVector<ctype,mydim>& global) const
    {
      FieldVector<ctype, mydim> l; // result
      for (int k=0; k<mydim; k++)
        l[k] = (global[k]-midpoint[k])/extension[k] + 0.5;
      return l;
    }

    /*! determinant of the jacobian of the mapping
     */
    ctype integrationElement (const FieldVector<ctype, mydim>& local) const
    {
      ctype volume=1.0;
      for (int k=0; k<mydim; k++) volume *= extension[k];
      return volume;
    }

    //! can only be called for mydim=cdim!
    Mat<mydim,mydim,ctype>& jacobianInverse (const FieldVector<ctype, mydim>& local) const
    {
      for (int i=0; i<mydim; ++i)
      {
        Jinv[i] = 0.0;                        // set column to zero
        Jinv[i][i] = 1.0/extension[i];         // set diagonal element
      }
      return Jinv;
    }

    //! check whether local is inside reference element
    bool checkInside (const FieldVector<ctype, mydim>& local) const
    {
      for (int i=0; i<mydim; i++)
        if (local[i]<-yasptolerance || local[i]>1+yasptolerance) return false;
      return true;
    }

    //! constructor from (storage for) midpoint and extension
    YaspGeometry (const FieldVector<ctype, mydim>& p, const FieldVector<ctype, mydim>& h)
      : midpoint(p), extension(h)
    {}

    //! print function
    void print (std::ostream& s) const
    {
      s << "YaspGeometry<"<<mydim<<","<<mydim<< "> ";
      s << "midpoint";
      for (int i=0; i<mydim; i++)
        s << " " << midpoint[i];
      s << " extension";
      for (int i=0; i<mydim; i++)
        s << " " << extension[i];
    }

    const YaspGeometry<mydim,mydim,GridImp>&
    operator = (const YaspGeometry<mydim,mydim,GridImp>& g)
    {
      midpoint = g.midpoint;
      extension = g.extension;
      Jinv = g.Jinv;
      c = g.c;
      return *this;
    }

  private:
    // the element is fully defined by midpoint and the extension
    // in each direction. References are used because this information
    // is known outside the element in many cases.
    // Note mydim==cdim

    // IMPORTANT midpoint and extension can't be references,
    // because they must stay the same when the iterator changes
    FieldVector<ctype, mydim> midpoint; // the midpoint
    FieldVector<ctype, mydim> extension; // the extension

    // In addition we need memory in order to return references.
    // Possibly we should change this in the interface ...
    mutable Mat<mydim,mydim,ctype> Jinv; // the jacobian inverse
    mutable FieldVector<ctype, mydim> c; // a point
  };




  //! specialization for dim=0, this is a vertex
  template<int cdim, class GridImp>
  class YaspGeometry<0,cdim,GridImp> : public GeometryDefault<0,cdim,GridImp,YaspGeometry>
  {
  public:
    typedef typename GridImp::ctype ctype;

    //! return the element type identifier
    GeometryType type () const
    {
      return vertex;
    }

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners () const
    {
      return 1;
    }

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector<ctype, cdim>& operator[] (int i) const
    {
      return position;
    }

    //! constructor
    YaspGeometry (const FieldVector<ctype, cdim>& p) : position(p)
    {}

    //! print function
    void print (std::ostream& s) const
    {
      s << "YaspGeometry<"<<0<<","<<cdim<< "> ";
      s << "position " << position;
    }

    const YaspGeometry<0,cdim,GridImp>&
    operator = (const YaspGeometry<0,cdim,GridImp>& g)
    {
      position = g.position;
      return *this;
    }

  private:
    // IMPORTANT position can't be references,
    // because they must stay the same when the iterator changes
    FieldVector<ctype, cdim> position; //!< where the vertex is
  };

  // operator<< for all YaspGeometrys
  template <int mydim, int cdim, class GridImp>
  inline
  std::ostream& operator<< (std::ostream& s, YaspGeometry<mydim,cdim,GridImp>& e)
  {
    e.print(s);
    return s;
  }

  //========================================================================
  /*!
     YaspEntity realizes the concept a mesh entity.

     We have specializations for codim==0 (elements) and
     codim=dim (vertices).
     The general version throws a GridError.
   */
  //========================================================================

  template<int codim, int dim, class GridImp>
  class YaspSpecialEntity :
    public GridImp::template codim<codim>::Entity
  {
  public:
    typedef typename GridImp::ctype ctype;

    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;

    YaspSpecialEntity(const YGLI& g, const TSI& it) :
      GridImp::template codim<codim>::Entity (YaspEntity<codim, dim, GridImp>(g,it))
    {};
    YaspSpecialEntity(const YaspEntity<codim, dim, GridImp>& e) :
      GridImp::template codim<codim>::Entity (e)
    {};
    const TSI& transformingsubiterator () const
    {
      return this->realEntity._it;
    }
    const YGLI& gridlevel () const
    {
      return this->realEntity._g;
    }
  };

  template<int codim, int dim, class GridImp>
  class YaspEntity :  public EntityDefault <codim,dim,GridImp,YaspEntity>
  {
  public:
    typedef typename GridImp::ctype ctype;

    typedef typename GridImp::template codim<codim>::Geometry Geometry;
    //! level of this element
    int level () const
    {
      DUNE_THROW(GridError, "YaspEntity not implemented");
    }

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index () const
    {
      DUNE_THROW(GridError, "YaspEntity not implemented");
    }

    //! geometry of this entity
    const Geometry& geometry () const
    {
      DUNE_THROW(GridError, "YaspEntity not implemented");
    }

    //! return partition type attribute
    PartitionType partitionType () const
    {
      DUNE_THROW(GridError, "YaspEntity not implemented");
    }

    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;
    YaspEntity (const YGLI& g, const TSI& it)
    {
      DUNE_THROW(GridError, "YaspEntity not implemented");
    }
  };


  // specialization for codim=0
  template<int dim, class GridImp>
  class YaspEntity<0,dim,GridImp> : public EntityDefault <0,dim,GridImp,YaspEntity>
  {
    enum { dimworld = GridImp::dimensionworld };
  public:
    typedef typename GridImp::ctype ctype;

    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;

    typedef YaspSpecialGeometry<dim-0,dim,GridImp> SpecialGeometry;

    typedef typename GridImp::template codim<0>::Geometry Geometry;
    template <int cd>
    struct codim
    {
      typedef typename GridImp::template codim<cd>::EntityPointer EntityPointer;
    };
    typedef typename GridImp::template codim<0>::EntityPointer EntityPointer;
    typedef typename GridImp::template codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename GridImp::template codim<0>::HierarchicIterator HierarchicIterator;

    //! define type used for coordinates in grid module
    typedef typename YGrid<dim,ctype>::iTupel iTupel;

    // constructor
    YaspEntity (const YGLI& g, const TSI& it)
      : _it(it), _g(g), _geometry(it.position(),it.meshsize())
    {}

    //! level of this element
    int level () const {return _g.level();}

    //! index is unique and consecutive per level
    int index () const {return _it.superindex();} // superindex works also for iteration over subgrids

    //! return partition type attribute
    PartitionType partitionType () const
    {
      if (_g.cell_interior().inside(_it.coord())) return InteriorEntity;
      if (_g.cell_overlap().inside(_it.coord())) return OverlapEntity;
      return GhostEntity;
    }

    //! geometry of this entity
    const Geometry& geometry () const { return _geometry; }

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
          with codimension cc.
     */
    template<int cc> int count () const
    {
      if (cc==1) return 2*dim;
      if (cc==dim) return 1<<dim;
      DUNE_THROW(GridError, "codim not (yet) implemented");
    }

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
          with codimension cc.
     */
    template<int cc>
    typename codim<cc>::EntityPointer entity (int i) const
    {
#warning please reenable the compile-time error
#if 0
      IsTrue< ( cc == dim || cc == 0 ) >::yes();
#endif
      // coordinates of the cell == coordinates of lower left corner
      if (cc==dim)
      {
        iTupel coord = _it.coord();

        // get corner from there
        for (int k=0; k<dim; k++)
          if (i&(1<<k)) (coord[k])++;

        return YaspLevelIterator<cc,All_Partition,GridImp>(_g,_g.vertex_overlapfront().tsubbegin(coord));
      }
      DUNE_THROW(GridError, "codim not (yet) implemented");
    }

    //! Inter-level access to father element on coarser grid. Assumes that meshes are nested.
    EntityPointer father () const
    {
      // check if coarse level exists
      if (_g.level()<=0)
        DUNE_THROW(GridError, "tried to call father on level 0");

      // yes, get iterator to it
      YGLI cg = _g.coarser();

      // coordinates of the cell
      iTupel coord = _it.coord();

      // get coordinates on next coarser level
      for (int k=0; k<dim; k++) coord[k] = coord[k]/2;

      return YaspLevelIterator<0,All_Partition,GridImp>(cg,cg.cell_overlap().tsubbegin(coord));
    }

    /*! Location of this element relative to the reference element element of the father.
          This is sufficient to interpolate all dofs in conforming case.
          Nonconforming case may require access to neighbors of father and
          computations with local coordinates.
          On the fly case is somewhat inefficient since dofs  are visited several times.
          If we store interpolation matrices, this is tolerable. We assume that on-the-fly
          implementation of numerical algorithms is only done for simple discretizations.
          Assumes that meshes are nested.
     */
    Geometry& geometryInFather () const
    {
      // determine which son we are
      int son = 0;
      for (int k=0; k<dim; k++)
        if (_it.coord(k)%2)
          son += (1<<k);

      // access to one of the 2**dim predefined elements
#if 0
      return YaspFatherRelativeLocalElement<dim>::getson(son);
#endif
    }

    const TSI& transformingsubiterator () const
    {
      return _it;
    }

    const YGLI& gridlevel () const
    {
      return _g;
    }

    //! returns intersection iterator for first intersection
    IntersectionIterator ibegin () const
    {
      return YaspIntersectionIterator<GridImp>(*this,false);
    }

    //! Reference to one past the last neighbor
    IntersectionIterator iend () const
    {
      return YaspIntersectionIterator<GridImp>(*this,true);
    }

    /*! Inter-level access to son elements on higher levels<=maxlevel.
          This is provided for sparsely stored nested unstructured meshes.
          Returns iterator to first son.
     */
    HierarchicIterator hbegin (int maxlevel) const
    {
      return YaspHierarchicIterator<GridImp>(_g,_it,maxlevel);
    }

    //! Returns iterator to one past the last son
    HierarchicIterator hend (int maxlevel) const
    {
      return YaspHierarchicIterator<GridImp>(_g,_it,_g.level());
    }

  private:
    const TSI& _it;         // position in the grid level
    const YGLI& _g;         // access to grid level
    SpecialGeometry _geometry; // the element geometry
  };


  // specialization for codim=dim
  template<int dim, class GridImp>
  class YaspEntity<dim,dim,GridImp> : public EntityDefault <dim,dim,GridImp,YaspEntity>
  {
    enum { dimworld = GridImp::dimensionworld };
  public:
    typedef typename GridImp::ctype ctype;

    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;

    typedef YaspSpecialGeometry<dim-dim,dim,GridImp> SpecialGeometry;

    typedef typename GridImp::template codim<dim>::Geometry Geometry;
    template <int cd>
    struct codim
    {
      typedef typename GridImp::template codim<cd>::EntityPointer EntityPointer;
    };
    typedef typename GridImp::template codim<0>::EntityPointer EntityPointer;

    //! define type used for coordinates in grid module
    typedef typename YGrid<dim,ctype>::iTupel iTupel;

    // constructor
    YaspEntity (const YGLI& g, const TSI& it)
      : _it(it), _g(g), _geometry(it.position())
    {  }

    //! level of this element
    int level () const {return _g.level();}

    //! index is unique and consecutive per level
    int index () const {return _it.superindex();}

    //! geometry of this entity
    const Geometry& geometry () const { return _geometry; }

    //! return partition type attribute
    PartitionType partitionType () const
    {
      if (_g.vertex_interior().inside(_it.coord())) return InteriorEntity;
      if (_g.vertex_interiorborder().inside(_it.coord())) return BorderEntity;
      if (_g.vertex_overlap().inside(_it.coord())) return OverlapEntity;
      if (_g.vertex_overlapfront().inside(_it.coord())) return FrontEntity;
      return GhostEntity;
    }

    /*! Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
          This can speed up on-the-fly interpolation for linear conforming elements
          Possibly this is sufficient for all applications we want on-the-fly.
     */
    EntityPointer ownersFather () const
    {
      // check if coarse level exists
      if (_g.level()<=0)
        DUNE_THROW(GridError, "tried to call father on level 0");

      // yes, get iterator to it
      YGLI cg = _g.coarser();

      // coordinates of the vertex == coordinates of upper right element
      iTupel coord = _it.coord();

      // get coordinates of cell on coarser level
      for (int k=0; k<dim; k++) coord[k] = coord[k]/2;

      // check against boundary
      for (int k=0; k<dim; k++)
        coord[k] = std::min(coord[k],cg.cell_overlap().max(k));

      // return level iterator
      return YaspLevelIterator<0,All_Partition,GridImp>(cg,cg.cell_overlap().tsubbegin(coord));
    }

    //! local coordinates within father
    FieldVector<ctype, dim>& positionInOwnersFather () const
    {
      // check if coarse level exists
      if (_g.level()<=0)
        DUNE_THROW(GridError, "tried to call local on level 0");

      // yes, get iterator to it
      YGLI cg = _g.coarser();

      // coordinates of the vertex == coordinates of upper right element
      iTupel coord = _it.coord();

      // get coordinates of cell on coarser level
      for (int k=0; k<dim; k++) coord[k] = coord[k]/2;

      // check against boundary
      for (int k=0; k<dim; k++)
        coord[k] = std::min(coord[k],cg.cell_overlap().max(k));

      // interpolate again, i.e. coord == lower left in 2**dim cells
      for (int k=0; k<dim; k++)
        coord[k] = 2*coord[k];

      // now it is simple ...
      for (int k=0; k<dim; k++)
        loc[k] = 0.5*(_it.coord(k)-coord[k]);   // expr in brackets is in 0..2

      // return result
      return loc;
    }

  private:
    const TSI& _it;              // position in the grid level
    const YGLI& _g;              // access to grid level
    SpecialGeometry _geometry;    // the element geometry
    // temporary object
    mutable FieldVector<ctype, dim> loc; // always computed before being returned
  };


  //========================================================================
  /*!
     YaspBoundaryEntity is not yet implemented
   */
  //========================================================================

  template <class GridImp>
  class YaspBoundaryEntity
    : public BoundaryEntityDefault <GridImp,YaspBoundaryEntity>
  {
  public:
  private:
  };


  //========================================================================
  /*!
     YaspIntersectionIterator enables iteration over intersection with
     neighboring codim 0 entities.
   */
  //========================================================================

  template<class GridImp>
  class YaspIntersectionIterator : public IntersectionIteratorDefault<GridImp,YaspIntersectionIterator>
  {
    enum { dim=GridImp::dimension };
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ctype;
  public:
    // types used from grids
    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;
    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef typename GridImp::template codim<0>::BoundaryEntity BoundaryEntity;
    typedef typename GridImp::template codim<1>::Geometry Geometry;
    typedef typename GridImp::template codim<1>::LocalGeometry LocalGeometry;
    typedef YaspSpecialEntity<0,dim,GridImp> SpecialEntity;
    typedef YaspSpecialGeometry<dim-1,dimworld,GridImp> SpecialGeometry;
    typedef YaspSpecialGeometry<dim-1,dim,GridImp> SpecialLocalGeometry;

    //! increment
    void increment()
    {
      // update count, check end
      _count++;
      if (_count==2*dim) return;   // end iterator reached, we are done

      // update intersection iterator from current position
      if (_face==0)   // direction remains valid
      {
        _face = 1;         // 0->1, _dir remains

        // move transforming iterator
        _itnb.move(_dir,2);         // move two cells in positive direction

        // make up faces
        _pos_self_local[_dir] = 1.0;
        _pos_nb_local[_dir] = 0.0;
        _pos_world[_dir] += _myself.transformingsubiterator().meshsize(_dir);

        // make up unit outer normal direction
        _normal[_dir] = 1.0;
      }
      else   // change direction
      {
        // move transforming iterator
        _itnb.move(_dir,-1);         // move one cell back

        // make up faces
        _pos_self_local[_dir] = 0.5;
        _pos_nb_local[_dir] = 0.5;
        _pos_world[_dir] = _myself.transformingsubiterator().position(_dir);

        // make up unit outer normal direction
        _normal[_dir] = 0.0;

        _face = 0;
        _dir += 1;

        // move transforming iterator
        _itnb.move(_dir,-1);         // move one cell in negative direction

        // make up faces
        _pos_self_local[_dir] = 0.0;
        _pos_nb_local[_dir] = 1.0;
        _pos_world[_dir] -= 0.5*_myself.transformingsubiterator().meshsize(_dir);

        // make up unit outer normal direction
        _normal[_dir] = -1.0;
      }
    }

    //! equality
    bool equals (const YaspIntersectionIterator<GridImp>& i) const
    {
      return (_count==i._count);
    }

    /*! return true if neighbor ist outside the domain. Still the neighbor might
        exist in case of periodic boundary conditions, i.e. true is returned
        if the neighbor is outside the periodic unit cell
     */
    bool boundary () const
    {
      // The transforming iterator can be safely moved beyond the boundary.
      // So we only have to compare against the cell_global grid
      if (   _itnb.coord(_dir)<_myself.gridlevel().cell_global().min(_dir)
             || _itnb.coord(_dir)>_myself.gridlevel().cell_global().max(_dir))
        return true;
      else
        return false;
    }

    //! return true if neighbor across intersection exists in this processor
    bool neighbor () const
    {
      // The transforming iterator can be safely moved beyond the boundary.
      // So we only have to compare against the cell_global grid
      if (_itnb.coord(_dir)>=_myself.gridlevel().cell_overlap().min(_dir)
          && _itnb.coord(_dir)<=_myself.gridlevel().cell_overlap().max(_dir))
        return true;
      else
        return false;
    }

    //! access neighbor, dereferencing
    Entity& dereference() const
    {
      return _nb;
    }


    //! return unit outer normal, this should be dependent on local coordinates for higher order boundary
    FieldVector<ctype, dimworld>& unitOuterNormal (FieldVector<ctype, dim-1>& local) const
    {
      return _normal;
    }

    //! return unit outer normal, if you know it is constant use this function instead
    FieldVector<ctype, dimworld>& unitOuterNormal () const
    {
      return _normal;
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
          Here returned element is in LOCAL coordinates of the element where iteration started.
     */
    const LocalGeometry& intersectionSelfLocal () const
    {
      return _is_self_local;
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
          Here returned element is in LOCAL coordinates of neighbor
     */
    const LocalGeometry& intersectionNeighborLocal () const
    {
      return _is_nb_local;
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
          Here returned element is in LOCAL coordinates of neighbor
     */
    const Geometry& intersectionGlobal () const
    {
      return _is_global;
    }

    //! local number of codim 1 entity in self where intersection is contained in
    int numberInSelf () const
    {
      return _count;
    }

    //! local number of codim 1 entity in neighbor where intersection is contained in
    int numberInNeighbor () const
    {
      return _count + 1-2*_face;
    }

    //! make intersection iterator from entity
    YaspIntersectionIterator (const YaspEntity<0,dim,GridImp>& myself, bool toend)
      : _itnb(myself.transformingsubiterator()),
        _myself(myself),
        _nb(myself.gridlevel(),_itnb),
        _pos_self_local(0.5),
        _pos_nb_local(0.5),
        _pos_world(myself.transformingsubiterator().position()),
        _ext_local(1.0),
        _is_self_local(_pos_self_local,_ext_local,_dir),
        _is_nb_local(_pos_nb_local,_ext_local,_dir),
        _is_global(_pos_world,myself.transformingsubiterator().meshsize(),_dir),
        _normal(0.0)
    {
      // making an end iterator?
      if (toend)
      {
        // initialize end iterator
        _count = 2*dim;
        return;
      }

      // initialize to first neighbor
      _count = 0;
      _dir = 0;
      _face = 0;

      // move transforming iterator
      _itnb.move(_dir,-1);

      // make up faces
      _pos_self_local[0] = 0.0;
      _pos_nb_local[0] = 1.0;
      _pos_world[0] -= 0.5*_myself.transformingsubiterator().meshsize(0);

      // make up unit outer normal direction
      _normal[0] = -1.0;
    }

  private:
    int _count;                           //!< valid neighbor count in 0 .. 2*dim-1
    int _dir;                             //!< count/2
    int _face;                            //!< count%2
    TSI _itnb;                            //!< position of nb in the grid level
    const YaspEntity<0,dim,GridImp>&
    _myself;                              //!< reference to myself
    mutable SpecialEntity _nb;            //!< virtual neighbor entity, built on the fly
    FieldVector<ctype, dim> _pos_self_local; //!< center of face in own local coordinates
    FieldVector<ctype, dim> _pos_nb_local; //!< center of face in neighbors local coordinates
    FieldVector<ctype, dim> _pos_world;   //!< center of face in world coordinates
    FieldVector<ctype, dim> _ext_local;   //!< extension of face in local coordinates
    SpecialLocalGeometry _is_self_local;  //!< intersection in own local coordinates
    SpecialLocalGeometry _is_nb_local;    //!< intersection in neighbors local coordinates
    SpecialGeometry _is_global;           //!< intersection in global coordinates
    FieldVector<ctype, dimworld> _normal; //!< for returning outer normal
  };


  //========================================================================
  /*!
     YaspHierarchicIterator enables iteration over son entities of codim 0
   */
  //========================================================================

  template<class GridImp>
  class YaspHierarchicIterator : public HierarchicIteratorDefault <GridImp,YaspHierarchicIterator>
  {
    enum { dim=GridImp::dimension };
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ctype;
  public:
    // types used from grids
    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;
    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef YaspSpecialEntity<0,dim,GridImp> SpecialEntity;

    //! define type used for coordinates in grid module
    typedef typename YGrid<dim,ctype>::iTupel iTupel;

    //! constructor
    YaspHierarchicIterator (const YGLI& g, const TSI& it, int maxlevel) : _g(g), _it(it), _entity(_g,_it)
    {
      // now iterator points to current cell

      // determine maximum level
      _maxlevel = std::min(maxlevel,_g.mg()->maxlevel());

      // if maxlevel not reached then push yourself and sons
      if (_g.level()<_maxlevel)
      {
        StackElem se(_g);
        se.coord = _it.coord();
        stack.push(se);
        push_sons();
      }

      // and make iterator point to first son if stack is not empty
      if (!stack.empty())
        pop_tos();
    }

    //! increment
    void increment ()
    {
      // sanity check: do nothing when stack is empty
      if (stack.empty()) return;

      // if maxlevel not reached then push sons
      if (_g.level()<_maxlevel)
        push_sons();

      // in any case pop one element
      pop_tos();
    }

    //! equality is very simple: compare grid level and superindex
    bool equals (const YaspHierarchicIterator<GridImp>& i) const
    {
      if (_g.level()==i._g.level() && _it.superindex()==i._it.superindex())
        return true;
      else
        return false;
    }

    //! dereferencing
    Entity& dereference() const
    {
      return _entity;
    }

    void print (std::ostream& s) const
    {
      s << "HIER: " << "level=" << _g.level()
        << " position=" << _it.coord()
        << " superindex=" << _it.superindex()
        << " maxlevel=" << _maxlevel
        << " stacksize=" << stack.size()
        << std::endl;
    }

    const YaspHierarchicIterator<GridImp>&
    operator = (const YaspHierarchicIterator<GridImp>& i)
    {
      _g = i._g;
      _it = i._it;
      /* _entity = i._entity
       * is done implicitely, as the entity is completely
       * defined via the interator it belongs to
       */
      _maxlevel = i._maxlevel;
      stack = i.stack;
      return *this;
    }

  private:
    YGLI _g;             //!< access to grid level
    TSI _it;             //!< position in the grid level
    mutable SpecialEntity _entity; //!< virtual entity
    int _maxlevel;       //!< maximum level of elements to be processed

    struct StackElem {
      YGLI g;         // grid level of the element
      iTupel coord;   // and the coordinates
      StackElem(YGLI gg) : g(gg) {}
    };
    Stack<StackElem> stack;    //!< stack holding elements to be processed

    // push sons of current element on the stack
    void push_sons ()
    {
      // yes, process all 1<<dim sons
      StackElem se(_g.finer());
      for (int i=0; i<(1<<dim); i++)
      {
        for (int k=0; k<dim; k++)
          if (i&(1<<k))
            se.coord[k] = _it.coord(k)*2+1;
          else
            se.coord[k] = _it.coord(k)*2;
        stack.push(se);
      }
    }

    // make TOS the current element
    void pop_tos ()
    {
      StackElem se = stack.pop();
      _g = se.g;
      _it.reinit(_g.cell_overlap(),se.coord);
    }
  };

  //========================================================================
  /*!
     YaspLevelIterator enables iteration over entities of one grid level

     We have specializations for codim==0 (elements) and
     codim=dim (vertices).
     The general version throws a GridError.
   */
  //========================================================================

  template<int codim, PartitionIteratorType pitype, class GridImp>
  class YaspLevelIterator : public LevelIteratorDefault<codim,pitype,GridImp,YaspLevelIterator>
  {
    //! know your own dimension
    enum { dim=GridImp::dimension };
    //! know your own dimension of world
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::template codim<codim>::Entity Entity;
    typedef typename GridImp::ctype ctype;
  public:
    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;
    typedef YaspSpecialEntity<codim,dim,GridImp> SpecialEntity;

    //! constructor
    YaspLevelIterator (const YGLI & g, const TSI & it) : _g(g), _it(it), _entity(_g,_it)
    {
      if (codim>0 && codim<dim)
      {
        DUNE_THROW(GridError, "YaspLevelIterator: codim not implemented");
      }
    }

    //! copy constructor
    YaspLevelIterator (const YaspLevelIterator& i) : _g(i._g), _it(i._it), _entity(_g,_it)
    {
      if (codim>0 && codim<dim)
      {
        DUNE_THROW(GridError, "YaspLevelIterator: codim not implemented");
      }
    }

    //! increment
    void increment()
    {
      ++_it;
    }

    //! equality
    bool equals (const YaspLevelIterator<codim,pitype,GridImp>& i) const
    {
      return _it==i._it;
    }

    //! dereferencing
    Entity& dereference() const
    {
      return _entity;
    }

    //! ask for level of entity
    int level () const {return _g.level();}

    const YaspLevelIterator<codim,pitype,GridImp>&
    operator = (const YaspLevelIterator<codim,pitype,GridImp>& i)
    {
      _g = i._g;
      _it = i._it;
      /* _entity = i._entity
       * is done implicitely, as the entity is completely
       * defined via the interator it belongs to
       */
      return *this;
    }

  private:
    YGLI _g;             // access to grid level
    TSI _it;             // position in the grid level
    mutable SpecialEntity _entity; //!< virtual entity
  };



  //************************************************************************
  /*!
     A Grid is a container of grid entities. Given a dimension dim these entities have a
     codimension codim with 0 <= codim <= dim.

     The Grid is assumed to be hierachically refined and nested. It enables iteration over
     entities of a given level and codimension.

     The grid can consist of several subdomains and it can be non-matching.

     All information is provided to allocate degrees of freedom in appropriate vector
     data structures (which are not part of this module).
   */
  template<int dim, int dimworld>
  class YaspGrid :
    public GridDefault<dim,dimworld,yaspgrid_ctype,YaspGrid>,
    public MultiYGrid<dim,yaspgrid_ctype>
  {
    typedef const YaspGrid<dim,dimworld> GridImp;
  public:
    typedef GridTraits<dim,dimworld,Dune::YaspGrid,YaspGeometry,YaspEntity,
        YaspBoundaryEntity,YaspLevelIterator,
        YaspIntersectionIterator,YaspHierarchicIterator> Traits;

    //! define type used for coordinates in grid module
    typedef yaspgrid_ctype ctype;

    //! maximum number of levels allowed
    enum { MAXL=64 };

    //! shorthand for base class data types
    typedef MultiYGrid<dim,ctype> YMG;
    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;

    //! return GridIdentifierType of Grid, i.e. SGrid_Id or AlbertGrid_Id ...
    GridIdentifier type() const { return YaspGrid_Id; };

    /*! Constructor for a YaspGrid, they are all forwarded to the base class
          @param comm MPI communicator where this mesh is distributed to
          @param L extension of the domain
          @param s number of cells on coarse mesh in each direction
       @param periodic tells if direction is periodic or not
       @param size of overlap on coarsest grid (same in all directions)
     */
    YaspGrid (MPI_Comm comm, Dune::FieldVector<ctype, dim> L,
              Dune::FieldVector<int, dim> s,
              Dune::FieldVector<bool, dim> periodic, int overlap)
      : MultiYGrid<dim,ctype>(comm,L,s,periodic,overlap)
    {  }

    /*! Return maximum level defined in this grid. Levels are numbered
          0 ... maxlevel with 0 the coarsest level.
     */
    int maxlevel() const {return MultiYGrid<dim,ctype>::maxlevel();} // delegate

    //! refine the grid refCount times. What about overlap?
    void globalRefine (int refCount)
    {
      bool b=false;
      if (refCount>0) b=true;
      MultiYGrid<dim,ctype>::refine(b);
    }

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template codim<cd>::template partition<pitype>::LevelIterator lbegin (int level) const
    {
      IsTrue< ( cd == dim || cd == 0 ) >::yes();
      YGLI g = MultiYGrid<dim,ctype>::begin(level);
      if (cd==0)   // the elements
      {
        if (pitype<=InteriorBorder_Partition)
          return YaspLevelIterator<cd,pitype,GridImp>(g,g.cell_interior().tsubbegin());
        if (pitype<=All_Partition)
          return YaspLevelIterator<cd,pitype,GridImp>(g,g.cell_overlap().tsubbegin());
      }
      if (cd==dim)   // the vertices
      {
        if (pitype==Interior_Partition)
          return YaspLevelIterator<cd,pitype,GridImp>(g,g.vertex_interior().tsubbegin());
        if (pitype==InteriorBorder_Partition)
          return YaspLevelIterator<cd,pitype,GridImp>(g,g.vertex_interiorborder().tsubbegin());
        if (pitype==Overlap_Partition)
          return YaspLevelIterator<cd,pitype,GridImp>(g,g.vertex_overlap().tsubbegin());
        if (pitype<=All_Partition)
          return YaspLevelIterator<cd,pitype,GridImp>(g,g.vertex_overlapfront().tsubbegin());
      }
      DUNE_THROW(GridError, "YaspLevelIterator with this codim or partition type not implemented");
    }

    //! Iterator to one past the last entity of given codim on level for partition type
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template codim<cd>::template partition<pitype>::LevelIterator lend (int level) const
    {
      IsTrue< ( cd == dim || cd == 0 ) >::yes();
      YGLI g = MultiYGrid<dim,ctype>::begin(level);
      if (cd==0)   // the elements
      {
        if (pitype<=InteriorBorder_Partition)
          return YaspLevelIterator<cd,pitype,GridImp>(g,g.cell_interior().tsubend());
        if (pitype<=All_Partition)
          return YaspLevelIterator<cd,pitype,GridImp>(g,g.cell_overlap().tsubend());
      }
      if (cd==dim)   // the vertices
      {
        if (pitype==Interior_Partition)
          return YaspLevelIterator<cd,pitype,GridImp>(g,g.vertex_interior().tsubend());
        if (pitype==InteriorBorder_Partition)
          return YaspLevelIterator<cd,pitype,GridImp>(g,g.vertex_interiorborder().tsubend());
        if (pitype==Overlap_Partition)
          return YaspLevelIterator<cd,pitype,GridImp>(g,g.vertex_overlap().tsubend());
        if (pitype<=All_Partition)
          return YaspLevelIterator<cd,pitype,GridImp>(g,g.vertex_overlapfront().tsubend());
      }
      DUNE_THROW(GridError, "YaspLevelIterator with this codim or partition type not implemented");
    }

    //! version without second template parameter for convenience
    template<int cd>
    typename Traits::template codim<cd>::template partition<All_Partition>::LevelIterator lbegin (int level) const
    {
      IsTrue< ( cd == dim || cd == 0 ) >::yes();
      YGLI g = MultiYGrid<dim,ctype>::begin(level);
      if (cd==0)   // the elements
      {
        return YaspLevelIterator<cd,All_Partition,GridImp>(g,g.cell_overlap().tsubbegin());
      }
      if (cd==dim)   // the vertices
      {
        return YaspLevelIterator<cd,All_Partition,GridImp>(g,g.vertex_overlapfront().tsubbegin());
      }
      DUNE_THROW(GridError, "YaspLevelIterator with this codim or partition type not implemented");
    }

    //! version without second template parameter for convenience
    template<int cd>
    typename Traits::template codim<cd>::template partition<All_Partition>::LevelIterator lend (int level) const
    {
      IsTrue< ( cd == dim || cd == 0 ) >::yes();
      YGLI g = MultiYGrid<dim,ctype>::begin(level);
      if (cd==0)   // the elements
      {
        return YaspLevelIterator<cd,All_Partition,GridImp>(g,g.cell_overlap().tsubend());
      }
      if (cd==dim)   // the vertices
      {
        return YaspLevelIterator<cd,All_Partition,GridImp>(g,g.vertex_overlapfront().tsubend());
      }
      DUNE_THROW(GridError, "YaspLevelIterator with this codim or partition type not implemented");
    }

    //! return size (= distance in graph) of overlap region
    int overlapSize (int level, int codim) const
    {
      YGLI g = MultiYGrid<dim,ctype>::begin(level);
      return g.overlap();
    }

    //! return size (= distance in graph) of ghost region
    int ghostSize (int level, int codim) const
    {
      return 0;
    }

    //! number of grid entities per level and codim
    int size (int level, int codim) const
    {
      YaspGrid<dim, dimworld>* p = const_cast<YaspGrid<dim, dimworld>*>(this);
      YGLI g = p->begin(level);
      if (codim==0)
      {
        return g.cell_overlap().totalsize();
      }
      if (codim==dim)
      {
        return g.vertex_overlapfront().totalsize();
      }
      DUNE_THROW(GridError, "Yasp does not implement this codim (yet)");
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
      IsTrue< ( codim == dim || codim == 0 ) >::yes();
      // access to grid level
      YGLI g = MultiYGrid<dim,ctype>::begin(level);

      // find send/recv lists or throw error
      typedef typename MultiYGrid<dim,ctype>::Intersection IS;
      std::deque<IS>* sendlist;
      std::deque<IS>* recvlist;
      if (codim==0)   // the elements
      {
        if (iftype==InteriorBorder_InteriorBorder_Interface)
          return;         // there is nothing to do in this case
        if (iftype==InteriorBorder_All_Interface)
        {
          sendlist = &g.send_cell_interior_overlap();
          recvlist = &g.recv_cell_overlap_interior();
        }
        if (iftype==Overlap_OverlapFront_Interface || iftype==Overlap_All_Interface || iftype==All_All_Interface)
        {
          sendlist = &g.send_cell_overlap_overlap();
          recvlist = &g.recv_cell_overlap_overlap();
        }
      }
      if (codim==dim)   // the vertices
      {
        if (iftype==InteriorBorder_InteriorBorder_Interface)
        {
          sendlist = &g.send_vertex_interiorborder_interiorborder();
          recvlist = &g.recv_vertex_interiorborder_interiorborder();
        }

        if (iftype==InteriorBorder_All_Interface)
        {
          sendlist = &g.send_vertex_interiorborder_overlapfront();
          recvlist = &g.recv_vertex_overlapfront_interiorborder();
        }
        if (iftype==Overlap_OverlapFront_Interface || iftype==Overlap_All_Interface)
        {
          sendlist = &g.send_vertex_overlap_overlapfront();
          recvlist = &g.recv_vertex_overlapfront_overlap();
        }
        if (iftype==All_All_Interface)
        {
          sendlist = &g.send_vertex_overlapfront_overlapfront();
          recvlist = &g.recv_vertex_overlapfront_overlapfront();
        }
      }
      if (codim>0 && codim<dim)
      {
        DUNE_THROW(GridError, "interface communication not implemented");
      }

      // change communication direction?
      if (dir==BackwardCommunication)
        std::swap(sendlist,recvlist);

      // allocate & fill the send buffers & store send request
      std::vector<P<T>*> sends;   // store pointers to send buffers
      typedef typename std::deque<IS>::iterator ISIT;
      for (ISIT is=sendlist->begin(); is!=sendlist->end(); ++is)
      {
        // allocate send buffer
        P<T> *buf = new P<T>[is->grid.totalsize()];

        // remember send buffer
        sends.push_back(buf);

        // fill send buffer; iterate over cells in intersection
        typename SubYGrid<dim,ctype>::SubIterator subend = is->grid.subend();
        for (typename SubYGrid<dim,ctype>::SubIterator i=is->grid.subbegin(); i!=subend; ++i)
          buf[i.index()].gather(t,i.superindex());

        // hand over send request to torus class
        MultiYGrid<dim,ctype>::torus().send(is->rank,buf,is->grid.totalsize()*sizeof(P<T>));
      }

      // allocate recv buffers and store receive request
      std::vector<P<T>*> recvs;   // pointers to receive buffers
      for (ISIT is=recvlist->begin(); is!=recvlist->end(); ++is)
      {
        // allocate recv buffer
        P<T> *buf = new P<T>[is->grid.totalsize()];

        // remember recv buffer
        recvs.push_back(buf);

        // hand over recv request to torus class
        MultiYGrid<dim,ctype>::torus().recv(is->rank,buf,is->grid.totalsize()*sizeof(P<T>));
      }

      // exchange all buffers now
      MultiYGrid<dim,ctype>::torus().exchange();

      // release send buffers
      for (int i=0; i<sends.size(); i++)
        delete[] sends[i];

      // process receive buffers and delete them
      int i=0;
      for (ISIT is=recvlist->begin(); is!=recvlist->end(); ++is)
      {
        // get recv buffer
        P<T> *buf = recvs[i++];

        // copy data from receive buffer; iterate over cells in intersection
        typename SubYGrid<dim,ctype>::SubIterator subend = is->grid.subend();
        for (typename SubYGrid<dim,ctype>::SubIterator i=is->grid.subbegin(); i!=subend; ++i)
          buf[i.index()].scatter(t,i.superindex());

        // delete buffer
        delete[] buf;
      }
    }

    // implement leaf communication. Problem: supply vector of vectors

  private:
    //  YMG _mg;
  };

  /** @} end documentation group */

  namespace Capabilities
  {

    template<int dim,int dimw>
    struct hasLeafIterator< YaspGrid<dim,dimw> >
    {
      static const bool v = false;
    };

    template<int dim, int dimw>
    struct hasEntity< YaspGrid<dim,dimw>, YaspEntity<0,dim,YaspGrid<dim,dimw> > >
    {
      static const bool v = true;
    };

    template<int dim, int dimw>
    struct hasEntity< YaspGrid<dim,dimw>, YaspEntity<dim,dim,YaspGrid<dim,dimw> > >
    {
      static const bool v = true;
    };

    template<int dim, int dimw>
    struct isParallel< YaspGrid<dim,dimw> >
    {
      static const bool v = true;
    };

  }

} // end namespace


#endif
