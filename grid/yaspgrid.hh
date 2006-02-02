// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_YASPGRID_HH
#define DUNE_YASPGRID_HH

#include <iostream>
#include <vector>
#include <map>

#include "common/grid.hh"     // the grid base classes
#include "yaspgrid/grids.hh"  // the yaspgrid base classes
#include "../common/stack.hh" // the stack class
#include "../common/capabilities.hh" // the capabilities
#include "../common/helpertemplates.hh"
#include "../common/bigunsignedint.hh"
#include "../common/array.hh"
#include "../common/typetraits.hh"
#include "../common/collectivecommunication.hh"
#include "common/indexidset.hh"

#if HAVE_MPI
#include "../common/mpicollectivecommunication.hh"
#endif

/*! \file yaspgrid.hh
   Yasppergrid stands for yet another structured parallel grid.
   It will implement the dune grid interface for structured grids with codim 0
   and dim, with arbitrary overlap, parallel features with two overlap
   models, periodic boundaries and fast a implementation allowing on-the-fly computations.
 */

namespace Dune {

  //************************************************************************
  /*! define name for floating point type used for coordinates in yaspgrid.
     You can change the type for coordinates by changing this single typedef.
   */
  typedef double yaspgrid_ctype;
  static const yaspgrid_ctype yasptolerance=1E-13; // tolerance in coordinate computations

  /* some sizes for building global ids
   */
  const int yaspgrid_dim_bits = 24;   // bits for encoding each dimension
  const int yaspgrid_level_bits = 6;  // bits for encoding level number
  const int yaspgrid_codim_bits = 4;  // bits for encoding codimension


  //************************************************************************
  // forward declaration of templates

  template<int dim, int dimworld>               class YaspGrid;
  template<int mydim, int cdim, class GridImp>  class YaspGeometry;
  template<int codim, int dim, class GridImp>   class YaspEntity;
  template<int codim, class GridImp>            class YaspEntityPointer;
  template<int codim, PartitionIteratorType pitype, class GridImp> class YaspLevelIterator;
  template<class GridImp>            class YaspIntersectionIterator;
  template<class GridImp>            class YaspHierarchicIterator;
  template<class GridImp>            class YaspLevelIndexSet;
  template<class GridImp>            class YaspLeafIndexSet;
  template<class GridImp>            class YaspGlobalIdSet;

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

  //========================================================================
  // The transformation describing the refinement rule

  template<int dim, class GridImp>
  class YaspFatherRelativeLocalElement {
  public:
    static FieldVector<yaspgrid_ctype, dim> midpoint; // data neded for the refelem below
    static FieldVector<yaspgrid_ctype, dim> extension; // data needed for the refelem below
    static YaspSpecialGeometry<dim,dim,GridImp> geo;
    static YaspSpecialGeometry<dim,dim,GridImp>& getson (int i)
    {
      for (int k=0; k<dim; k++)
        if (i&(1<<k))
          midpoint[k] = 0.75;
        else
          midpoint[k] = 0.25;
      return geo;
    }
  };

  // initialize static variable with bool constructor (which makes reference elements)
  template<int dim, class GridImp>
  YaspSpecialGeometry<dim,dim,GridImp>
  YaspFatherRelativeLocalElement<dim,GridImp>::geo(YaspFatherRelativeLocalElement<dim,GridImp>::midpoint,
                                                   YaspFatherRelativeLocalElement<dim,GridImp>::extension);
  template<int dim, class GridImp>
  FieldVector<yaspgrid_ctype,dim> YaspFatherRelativeLocalElement<dim,GridImp>::midpoint(0.25);

  template<int dim, class GridImp>
  FieldVector<yaspgrid_ctype,dim> YaspFatherRelativeLocalElement<dim,GridImp>::extension(0.5);

  //! The general version implements dimworld==dimworld. If this is not the case an error is thrown
  template<int mydim,int cdim, class GridImp>
  class YaspGeometry : public GeometryDefaultImplementation<mydim,cdim,GridImp,YaspGeometry>
  {
  public:
    //! define type used for coordinates in grid module
    typedef typename GridImp::ctype ctype;
    //! the Reference Geometry
    typedef Geometry<mydim,mydim,GridImp,Dune::YaspGeometry> ReferenceGeometry;

    //! return the element type identifier
    NewGeometryType type () const
    {
      return NewGeometryType(NewGeometryType::cube,mydim);
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
  private:
    // the element is fully defined by its midpoint the extension
    // in each direction and the missing direction.
    // References are used because this information
    // is known outside the element in many cases.
    // Note cdim==cdim+1

    // IMPORTANT midpoint and extension are references,
    // YaspGeometry can't be copied
    const FieldVector<ctype, cdim> & midpoint;  // the midpoint
    const FieldVector<ctype, cdim> & extension; // the extension
    int& missing;                       // the missing, i.e. constant direction

    // In addition we need memory in order to return references.
    // Possibly we should change this in the interface ...
    mutable FieldVector<ctype, cdim> c; // a point

    const YaspGeometry<mydim,cdim,GridImp>&
    operator = (const YaspGeometry<mydim,cdim,GridImp>& g);

  };



  //! specialize for dim=dimworld, i.e. a volume element
  template<int mydim, class GridImp>
  class YaspGeometry<mydim,mydim,GridImp> : public GeometryDefaultImplementation<mydim,mydim,GridImp,YaspGeometry>
  {
  public:
    typedef typename GridImp::ctype ctype;
    //! the Reference Geometry
    typedef Geometry<mydim,mydim,GridImp,Dune::YaspGeometry> ReferenceGeometry;

    //! return the element type identifier
    NewGeometryType type () const
    {
      return NewGeometryType(NewGeometryType::cube,mydim);
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
    FieldMatrix<ctype,mydim,mydim>& jacobianInverseTransposed (const FieldVector<ctype, mydim>& local) const
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

  private:
    // the element is fully defined by midpoint and the extension
    // in each direction. References are used because this information
    // is known outside the element in many cases.
    // Note mydim==cdim

    // IMPORTANT midpoint and extension are references,
    // YaspGeometry can't be copied
    const FieldVector<ctype, mydim> & midpoint; // the midpoint
    const FieldVector<ctype, mydim> & extension; // the extension

    // In addition we need memory in order to return references.
    // Possibly we should change this in the interface ...
    mutable FieldMatrix<ctype,mydim,mydim> Jinv; // the jacobian inverse
    mutable FieldVector<ctype, mydim> c; // a point

    // disable copy
    const YaspGeometry<mydim,mydim,GridImp>&
    operator = (const YaspGeometry<mydim,mydim,GridImp>& g);
  };




  //! specialization for dim=0, this is a vertex
  template<int cdim, class GridImp>
  class YaspGeometry<0,cdim,GridImp> : public GeometryDefaultImplementation<0,cdim,GridImp,YaspGeometry>
  {
  public:
    typedef typename GridImp::ctype ctype;

    //! return the element type identifier
    NewGeometryType type () const
    {
      return NewGeometryType(NewGeometryType::cube,0);
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

  private:
    // IMPORTANT position is a reference,
    // YaspGeometry can't be copied
    const FieldVector<ctype, cdim> & position; //!< where the vertex is

    const YaspGeometry<0,cdim,GridImp>&
    operator = (const YaspGeometry<0,cdim,GridImp>& g);
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
    public GridImp::template Codim<codim>::Entity
  {
  public:
    typedef typename GridImp::ctype ctype;

    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;

    YaspSpecialEntity(const YGLI& g, const TSI& it) :
      GridImp::template Codim<codim>::Entity (YaspEntity<codim, dim, GridImp>(g,it))
    {};
    YaspSpecialEntity(const YaspEntity<codim, dim, GridImp>& e) :
      GridImp::template Codim<codim>::Entity (e)
    {};
    const TSI& transformingsubiterator () const
    {
      return this->realEntity.transformingsubiterator();
    }
    const YGLI& gridlevel () const
    {
      return this->realEntity.gridlevel();
    }
  };

  template<int codim, int dim, class GridImp>
  class YaspEntity
    :  public EntityDefaultImplementation <codim,dim,GridImp,YaspEntity>
  {
  public:
    typedef typename GridImp::ctype ctype;

    typedef typename GridImp::template Codim<codim>::Geometry Geometry;
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

    // IndexSets needs access to the private index methods
    friend class Dune::YaspLevelIndexSet<GridImp>;
    friend class Dune::YaspLeafIndexSet<GridImp>;
    friend class Dune::YaspGlobalIdSet<GridImp>;
    typedef typename GridImp::PersistentIndexType PersistentIndexType;

    //! globally unique, persistent index
    PersistentIndexType persistentIndex () const
    {
      DUNE_THROW(GridError, "YaspEntity not implemented");
    }

    //! consecutive, codim-wise, level-wise index
    int compressedIndex () const
    {
      DUNE_THROW(GridError, "YaspEntity not implemented");
    }

    //! consecutive, codim-wise, level-wise index
    int compressedLeafIndex () const
    {
      DUNE_THROW(GridError, "YaspEntity not implemented");
    }
  };


  // specialization for codim=0
  template<int dim, class GridImp>
  class YaspEntity<0,dim,GridImp>
    : public EntityDefaultImplementation <0,dim,GridImp,YaspEntity>
  {
    enum { dimworld = GridImp::dimensionworld };
  public:
    typedef typename GridImp::ctype ctype;

    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;

    typedef YaspSpecialGeometry<dim-0,dim,GridImp> SpecialGeometry;

    typedef typename GridImp::template Codim<0>::Geometry Geometry;
    template <int cd>
    struct Codim
    {
      typedef typename GridImp::template Codim<cd>::EntityPointer EntityPointer;
    };
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;
    typedef typename GridImp::template Codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename GridImp::template Codim<0>::HierarchicIterator HierarchicIterator;

    //! define the type used for persisitent indices
    typedef typename GridImp::PersistentIndexType PersistentIndexType;

    //! define type used for coordinates in grid module
    typedef typename YGrid<dim,ctype>::iTupel iTupel;

    // constructor
    YaspEntity (const YGLI& g, const TSI& it)
      : _it(it), _g(g), _geometry(it.position(),it.meshsize())
    {}

    //! level of this element
    int level () const {return _g.level();}

    //! index is unique and consecutive per level
    int index () const { return _it.superindex();} // superindex works also for iteration over subgrids

    //! globalIndex is unique and consecutive per global level
    int globalIndex () const {
      return _g.cell_global().index(_it.coord());
    }

    //! return partition type attribute
    PartitionType partitionType () const
    {
      if (_g.cell_interior().inside(_it.coord())) return InteriorEntity;
      if (_g.cell_overlap().inside(_it.coord())) return OverlapEntity;
      return GhostEntity;
    }

    //! geometry of this entity
    const Geometry& geometry () const { return _geometry; }

    /*! Return number of subentities with codimension cc.
     */
    template<int cc> int count () const
    {
      if (cc==dim) return 1<<dim;
      if (cc==1) return 2*dim;
      if (cc==dim-1) return dim*(1<<(dim-1));
      if (cc==0) return 1;
      DUNE_THROW(GridError, "codim " << cc << " (dim=" << dim << ") not (yet) implemented");
    }

    /*! Intra-element access to subentities of codimension cc > codim.
     */
    template<int cc>
    typename Codim<cc>::EntityPointer entity (int i) const
    {
      IsTrue< ( cc == dim || cc == 0 ) >::yes();
      // coordinates of the cell == coordinates of lower left corner
      if (cc==dim)
      {
        iTupel coord = _it.coord();

        // get corner from there
        for (int k=0; k<dim; k++)
          if (i&(1<<k)) (coord[k])++;

        return YaspLevelIterator<cc,All_Partition,GridImp>(_g,_g.vertex_overlapfront().tsubbegin(coord));
      }
      if (cc==0)
      {
        return YaspLevelIterator<cc,All_Partition,GridImp>(_g,_it);
      }
      DUNE_THROW(GridError, "codim " << cc << " (dim=" << dim << ") not (yet) implemented");
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
    const Geometry& geometryInFather () const
    {
      // determine which son we are
      int son = 0;
      for (int k=0; k<dim; k++)
        if (_it.coord(k)%2)
          son += (1<<k);

      // configure one of the 2^dim transformations
      return YaspFatherRelativeLocalElement<dim,GridImp>::getson(son);
    }

    const TSI& transformingsubiterator () const
    {
      return _it;
    }

    const YGLI& gridlevel () const
    {
      return _g;
    }

    bool isLeaf() const
    {
      return (_g.level() == _g.mg()->maxlevel());
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
    // IndexSets needs access to the private index methods
    friend class Dune::YaspLevelIndexSet<GridImp>;
    friend class Dune::YaspLeafIndexSet<GridImp>;
    friend class Dune::YaspGlobalIdSet<GridImp>;

    //! globally unique, persistent index
    PersistentIndexType persistentIndex () const
    {
      // get size of global grid
      const iTupel& size =  _g.cell_global().size();

      // get coordinate correction for periodic boundaries
      int coord[dim];
      for (int i=0; i<dim; i++)
      {
        coord[i] = _it.coord(i);
        if (coord[i]<0) coord[i] += size[i];
        if (coord[i]>=size[i]) coord[i] -= size[i];
      }

      // encode codim
      PersistentIndexType id(0);

      // encode level
      id = id << yaspgrid_level_bits;
      id = id+PersistentIndexType(_g.level());


      // encode coordinates
      for (int i=dim-1; i>=0; i--)
      {
        id = id << yaspgrid_dim_bits;
        id = id+PersistentIndexType(coord[i]);
      }

      return id;
    }

    //! consecutive, codim-wise, level-wise index
    int compressedIndex () const
    {
      return _it.superindex();
    }

    //! consecutive, codim-wise, level-wise index
    int compressedLeafIndex () const
    {
      return _it.superindex();
    }

    //! subentity persistent index
    template<int cc>
    PersistentIndexType subPersistentIndex (int i) const
    {
      if (cc==0)
        return persistentIndex();

      // get position of cell, note that global origin is zero
      // adjust for periodic boundaries
      int coord[dim];
      for (int k=0; k<dim; k++)
      {
        coord[k] = _it.coord(k);
        if (coord[k]<0) coord[k] += _g.cell_global().size(k);
        if (coord[k]>=_g.cell_global().size(k)) coord[k] -= _g.cell_global().size(k);
      }

      if (cc==dim)
      {
        // transform to vertex coordinates
        for (int k=0; k<dim; k++)
          if (i&(1<<k)) (coord[k])++;

        // determine min number of trailing zeroes
        int trailing = 1000;
        for (int i=0; i<dim; i++)
        {
          // count trailing zeros
          int zeros = 0;
          for (int j=0; j<_g.level(); j++)
            if (coord[i]&(1<<j))
              break;
            else
              zeros++;
          trailing = std::min(trailing,zeros);
        }

        // determine the level of this vertex
        int level = _g.level()-trailing;

        // encode codim
        PersistentIndexType id(dim);

        // encode level
        id = id << yaspgrid_level_bits;
        id = id+PersistentIndexType(level);

        // encode coordinates
        for (int i=dim-1; i>=0; i--)
        {
          id = id << yaspgrid_dim_bits;
          id = id+PersistentIndexType(coord[i]>>trailing);
        }

        return id;
      }

      if (cc==1)   // faces, i.e. for dim=2 codim=1 is treated as a face
      {
        // Idea: Use the doubled grid to assign coordinates to faces

        // ivar is the direction that varies
        int ivar=i/2;

        // compute position from cell position
        for (int k=0; k<dim; k++)
          coord[k] = coord[k]*2 + 1;         // the doubled grid
        if (i%2)
          coord[ivar] += 1;
        else
          coord[ivar] -= 1;

        // encode codim
        PersistentIndexType id(1);

        // encode level
        id = id << yaspgrid_level_bits;
        id = id+PersistentIndexType(_g.level());

        // encode coordinates
        for (int i=dim-1; i>=0; i--)
        {
          id = id << yaspgrid_dim_bits;
          id = id+PersistentIndexType(coord[i]);
        }

        return id;
      }

      if (cc==dim-1)   // edges, exist only for dim>2
      {
        // Idea: direction i is fixed, all others are vary, i.e. 2^(dim-1) possibilities per direction

        // number of entities per direction
        int m=1<<(dim-1);

        // ifix is the direction that is fixed
        int ifix=(dim-1)-(i/m);

        // compute position from cell position
        int bit=1;
        for (int k=0; k<dim; k++)
        {
          coord[k] = coord[k]*2+1;               // cell position in doubled grid
          if (k==ifix) continue;
          if ((i%m)&bit) coord[k] += 1;else coord[k] -= 1;
          bit *= 2;
        }

        // encode codim
        PersistentIndexType id(dim-1);

        // encode level
        id = id << yaspgrid_level_bits;
        id = id+PersistentIndexType(_g.level());

        // encode coordinates
        for (int i=dim-1; i>=0; i--)
        {
          id = id << yaspgrid_dim_bits;
          id = id+PersistentIndexType(coord[i]);
        }

        return id;
      }

      DUNE_THROW(GridError, "codim " << cc << " (dim=" << dim << ") not (yet) implemented");
    }

    //! subentity compressed index
    template<int cc>
    int subCompressedIndex (int i) const
    {
      if (cc==0)
        return compressedIndex();

      // get cell position relative to origin of local cell grid
      iTupel coord;
      for (int k=0; k<dim; ++k)
        coord[k] = _it.coord(k)-_g.cell_overlap().origin(k);

      if (cc==dim)   // vertices
      {
        // transform cell coordinate to corner coordinate
        for (int k=0; k<dim; k++)
          if (i&(1<<k)) (coord[k])++;

        // do lexicographic numbering
        int index = coord[dim-1];
        for (int k=dim-2; k>=0; --k)
          index = (index*(_g.cell_overlap().size(k)+1))+coord[k];
        return index;
      }

      if (cc==1)   // faces, i.e. for dim=2 codim=1 is treated as a face
      {
        // Idea: direction ivar varies, all others are fixed, i.e. 2 possibilities per direction

        // ivar is the direction that varies
        int ivar=i/2;

        // compute position from cell position
        if (i%2) coord[ivar] += 1;

        // do lexicographic numbering
        int index = coord[dim-1];
        for (int k=dim-2; k>=0; --k)
          if (k==ivar)
            index = (index*(_g.cell_overlap().size(k)+1))+coord[k];             // one more
          else
            index = (index*(_g.cell_overlap().size(k)))+coord[k];

        // add size of all subsets for smaller directions
        for (int j=0; j<ivar; j++)
        {
          int n=_g.cell_overlap().size(j)+1;
          for (int l=0; l<dim; l++)
            if (l!=j) n *= _g.cell_overlap().size(l);
          index += n;
        }

        return index;
      }

      if (cc==dim-1)   // edges, exist only for dim>2
      {
        // Idea: direction i is fixed, all others are vary, i.e. 2^(dim-1) possibilities per direction

        // number of entities per direction
        int m=1<<(dim-1);

        // ifix is the direction that is fixed
        int ifix=(dim-1)-(i/m);

        // compute position from cell position
        int bit=1;
        for (int k=0; k<dim; k++)
        {
          if (k==ifix) continue;
          if ((i%m)&bit) coord[k] += 1;
          bit *= 2;
        }

        // do lexicographic numbering
        int index = coord[dim-1];
        for (int k=dim-2; k>=0; --k)
          if (k!=ifix)
            index = (index*(_g.cell_overlap().size(k)+1))+coord[k];             // one more
          else
            index = (index*(_g.cell_overlap().size(k)))+coord[k];

        // add size of all subsets for smaller directions
        for (int j=dim-1; j>ifix; j--)
        {
          int n=_g.cell_overlap().size(j);
          for (int l=0; l<dim; l++)
            if (l!=j) n *= _g.cell_overlap().size(l)+1;
          index += n;
        }

        return index;
      }

      DUNE_THROW(GridError, "codim " << cc << " (dim=" << dim << ") not (yet) implemented");
    }

    //! subentity compressed index
    template<int cc>
    int subCompressedLeafIndex (int i) const
    {
      if (cc==0)
        return compressedIndex();

      // get cell position relative to origin of local cell grid
      iTupel coord;
      for (int k=0; k<dim; ++k)
        coord[k] = _it.coord(k)-_g.cell_overlap().origin(k);

      if (cc==dim)   // vertices
      {
        // transform cell coordinate to corner coordinate
        for (int k=0; k<dim; k++)
          if (i&(1<<k)) (coord[k])++;

        // move coordinates up to maxlevel
        for (int k=0; k<dim; k++)
          coord[k] = coord[k]<<(_g.mg()->maxlevel()-_g.level());

        // do lexicographic numbering
        int index = coord[dim-1];
        for (int k=dim-2; k>=0; --k)
          index = (index*(_g.mg()->rbegin().cell_overlap().size(k)+1))+coord[k];
        return index;
      }

      if (cc==1)   // faces, i.e. for dim=2 codim=1 is treated as a face
      {
        // Idea: direction ivar varies, all others are fixed, i.e. 2 possibilities per direction

        // ivar is the direction that varies
        int ivar=i/2;

        // compute position from cell position
        if (i%2) coord[ivar] += 1;

        // do lexicographic numbering
        int index = coord[dim-1];
        for (int k=dim-2; k>=0; --k)
          if (k==ivar)
            index = (index*(_g.cell_overlap().size(k)+1))+coord[k];             // one more
          else
            index = (index*(_g.cell_overlap().size(k)))+coord[k];

        // add size of all subsets for smaller directions
        for (int j=0; j<ivar; j++)
        {
          int n=_g.cell_overlap().size(j)+1;
          for (int l=0; l<dim; l++)
            if (l!=j) n *= _g.cell_overlap().size(l);
          index += n;
        }

        return index;
      }

      if (cc==dim-1)   // edges, exist only for dim>2
      {
        // Idea: direction i is fixed, all others are vary, i.e. 2^(dim-1) possibilities per direction

        // number of entities per direction
        int m=1<<(dim-1);

        // ifix is the direction that is fixed
        int ifix=(dim-1)-(i/m);

        // compute position from cell position
        int bit=1;
        for (int k=0; k<dim; k++)
        {
          if (k==ifix) continue;
          if ((i%m)&bit) coord[k] += 1;
          bit *= 2;
        }

        // do lexicographic numbering
        int index = coord[dim-1];
        for (int k=dim-2; k>=0; --k)
          if (k!=ifix)
            index = (index*(_g.cell_overlap().size(k)+1))+coord[k];             // one more
          else
            index = (index*(_g.cell_overlap().size(k)))+coord[k];

        // add size of all subsets for smaller directions
        for (int j=dim-1; j>ifix; j--)
        {
          int n=_g.cell_overlap().size(j);
          for (int l=0; l<dim; l++)
            if (l!=j) n *= _g.cell_overlap().size(l)+1;
          index += n;
        }

        return index;
      }

      DUNE_THROW(GridError, "codim " << cc << " (dim=" << dim << ") not (yet) implemented");
    }


    const TSI& _it;         // position in the grid level
    const YGLI& _g;         // access to grid level
    SpecialGeometry _geometry; // the element geometry
  };


  // specialization for codim=dim
  template<int dim, class GridImp>
  class YaspEntity<dim,dim,GridImp>
    : public EntityDefaultImplementation <dim,dim,GridImp,YaspEntity>
  {
    enum { dimworld = GridImp::dimensionworld };
  public:
    typedef typename GridImp::ctype ctype;

    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;

    typedef YaspSpecialGeometry<dim-dim,dim,GridImp> SpecialGeometry;

    typedef typename GridImp::template Codim<dim>::Geometry Geometry;
    template <int cd>
    struct Codim
    {
      typedef typename GridImp::template Codim<cd>::EntityPointer EntityPointer;
    };
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;

    //! define the type used for persisitent indices
    typedef typename GridImp::PersistentIndexType PersistentIndexType;

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

    //! globally unique, persistent index
    int globalIndex () const { return _g.cell_global().index(_it.coord()); }


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
    const FieldVector<ctype, dim>& positionInOwnersFather () const
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
    // IndexSets needs access to the private index methods
    friend class Dune::YaspLevelIndexSet<GridImp>;
    friend class Dune::YaspLeafIndexSet<GridImp>;
    friend class Dune::YaspGlobalIdSet<GridImp>;

    //! globally unique, persistent index
    PersistentIndexType persistentIndex () const
    {
      // get coordinate and size of global grid
      const iTupel& size =  _g.vertex_global().size();
      int coord[dim];

      // correction for periodic boundaries
      for (int i=0; i<dim; i++)
      {
        coord[i] = _it.coord(i);
        if (coord[i]<0) coord[i] += size[i];
        if (coord[i]>=size[i]) coord[i] -= size[i];
      }

      // determine min number of trailing zeroes
      int trailing = 1000;
      for (int i=0; i<dim; i++)
      {
        // count trailing zeros
        int zeros = 0;
        for (int j=0; j<_g.level(); j++)
          if (coord[i]&(1<<j))
            break;
          else
            zeros++;
        trailing = std::min(trailing,zeros);
      }

      // determine the level of this vertex
      int level = _g.level()-trailing;

      // encode codim
      PersistentIndexType id(dim);

      // encode level
      id = id << yaspgrid_level_bits;
      id = id+PersistentIndexType(level);

      // encode coordinates
      for (int i=dim-1; i>=0; i--)
      {
        id = id << yaspgrid_dim_bits;
        id = id+PersistentIndexType(coord[i]>>trailing);
      }

      return id;
    }

    //! consecutive, codim-wise, level-wise index
    int compressedIndex () const { return _it.superindex();}

    //! consecutive, codim-wise, level-wise index
    int compressedLeafIndex () const
    {
      if (_g.level()==_g.mg()->maxlevel())
        return _it.superindex();

      // not on leaf level, interpolate to finest grid
      int coord[dim];
      for (int i=0; i<dim; i++) coord[i] = _it.coord(i)-(_g).vertex_overlap().origin(i);

      // move coordinates up to maxlevel (multiply by 2 for each level
      for (int k=0; k<dim; k++)
        coord[k] = coord[k]*(1<<(_g.mg()->maxlevel()-_g.level()));

      // do lexicographic numbering
      int index = coord[dim-1];
      for (int k=dim-2; k>=0; --k)
        index = (index*(_g.mg()->rbegin().cell_overlap().size(k)+1))+coord[k];
      return index;
    }

    const TSI& _it;              // position in the grid level
    const YGLI& _g;              // access to grid level
    SpecialGeometry _geometry;    // the element geometry
    // temporary object
    mutable FieldVector<ctype, dim> loc; // always computed before being returned
  };


  //========================================================================
  /*!
     YaspIntersectionIterator enables iteration over intersection with
     neighboring codim 0 entities.
   */
  //========================================================================

  template<class GridImp>
  class YaspIntersectionIterator :
    public YaspEntityPointer<0,GridImp>,
    public IntersectionIteratorDefaultImplementation<GridImp,YaspIntersectionIterator>
  {
    enum { dim=GridImp::dimension };
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ctype;
    YaspIntersectionIterator();
  public:
    // types used from grids
    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;
    typedef typename GridImp::template Codim<0>::Entity Entity;
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;
    typedef typename GridImp::template Codim<1>::Geometry Geometry;
    typedef typename GridImp::template Codim<1>::LocalGeometry LocalGeometry;
    typedef YaspSpecialEntity<0,dim,GridImp> SpecialEntity;
    typedef YaspSpecialGeometry<dim-1,dimworld,GridImp> SpecialGeometry;
    typedef YaspSpecialGeometry<dim-1,dim,GridImp> SpecialLocalGeometry;

    //! increment
    void increment()
    {
      // check end
      if (_count==2*dim) return;   // end iterator reached, we are done
      // update count, check end
      _count++;

      // update intersection iterator from current position
      if (_face==0)   // direction remains valid
      {
        _face = 1;         // 0->1, _dir remains

        // move transforming iterator
        this->_it.move(_dir,2);         // move two cells in positive direction

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
        this->_it.move(_dir,-1);         // move one cell back
        if (_count==2*dim) return;

        // make up faces
        _pos_self_local[_dir] = 0.5;
        _pos_nb_local[_dir] = 0.5;
        _pos_world[_dir] = _myself.transformingsubiterator().position(_dir);

        // make up unit outer normal direction
        _normal[_dir] = 0.0;

        _face = 0;
        _dir += 1;

        // move transforming iterator
        this->_it.move(_dir,-1);         // move one cell in negative direction

        // make up faces
        _pos_self_local[_dir] = 0.0;
        _pos_nb_local[_dir] = 1.0;
        _pos_world[_dir] -= 0.5*_myself.transformingsubiterator().meshsize(_dir);

        // make up unit outer normal direction
        _normal[_dir] = -1.0;
      }
    }

    /*! return true if neighbor ist outside the domain. Still the neighbor might
        exist in case of periodic boundary conditions, i.e. true is returned
        if the neighbor is outside the periodic unit cell
     */
    bool boundary () const
    {
      // The transforming iterator can be safely moved beyond the boundary.
      // So we only have to compare against the cell_global grid
      return (this->_it.coord(_dir)<_myself.gridlevel().cell_global().min(_dir)
              ||
              this->_it.coord(_dir)>_myself.gridlevel().cell_global().max(_dir));
    }

    //! return true if neighbor across intersection exists in this processor
    bool neighbor () const
    {
      return
        (this->_it.coord(_dir)>=_myself.gridlevel().cell_overlap().min(_dir)
         &&
         this->_it.coord(_dir)<=_myself.gridlevel().cell_overlap().max(_dir));
      for (int d = 0; d < dim; d++)
      {
        if (this->_it.coord(_dir)<_myself.gridlevel().cell_overlap().min(_dir)
            ||
            this->_it.coord(_dir)>_myself.gridlevel().cell_overlap().max(_dir))
          return false;
      }
      return true;
    }

    //! return EntityPointer to the Entity on the inside of this intersection
    //! (that is the Entity where we started this Iterator)
    EntityPointer inside() const
    {
      return _selfp;
    }

    //! return EntityPointer to the Entity on the outside of this intersection
    //! (that is the neighboring Entity)
    EntityPointer outside() const
    {
      return *this;
    }

    //! identifier for boundary segment from macro grid
    //! (attach your boundary condition as needed)
    int boundaryId() const
    {
      if (this->_it.coord(_dir)<_myself.gridlevel().cell_global().min(_dir))
        return 2 * _dir;
      if (this->_it.coord(_dir)>_myself.gridlevel().cell_global().max(_dir))
        return 2 * _dir + 1;
      return 0;
    }

    //! return unit outer normal, this should be dependent on local coordinates for higher order boundary
    FieldVector<ctype, dimworld> outerNormal (const FieldVector<ctype, dim-1>& local) const
    {
      return _normal;
    }


    //! return unit outer normal, this should be dependent on local coordinates for higher order boundary
    FieldVector<ctype, dimworld> unitOuterNormal (const FieldVector<ctype, dim-1>& local) const
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
      : YaspEntityPointer<0,GridImp>(myself.gridlevel(),
                                     myself.transformingsubiterator()),
        _selfp(myself.gridlevel(),
               myself.transformingsubiterator()),
        _myself(myself),
        _pos_self_local(0.5),
        _pos_nb_local(0.5),
        _pos_world(myself.transformingsubiterator().position()),
        _ext_local(1.0),
        _is_self_local(_pos_self_local,_ext_local,_dir),
        _is_nb_local(_pos_nb_local,_ext_local,_dir),
        _is_global(_pos_world,_myself.transformingsubiterator().meshsize(),_dir),
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
      this->_it.move(_dir,-1);

      // make up faces
      _pos_self_local[0] = 0.0;
      _pos_nb_local[0] = 1.0;
      _pos_world[0] -= 0.5*_myself.transformingsubiterator().meshsize(0);

      // make up unit outer normal direction
      _normal[0] = -1.0;
    }

    //! copy constructor
    YaspIntersectionIterator (const YaspIntersectionIterator& it)
      : YaspEntityPointer<0,GridImp>(it._g, it._it),
        _count(it._count),
        _dir(it._dir),
        _face(it._face),
        _selfp(it._selfp),
        _myself(it._myself),
        _pos_self_local(it._pos_self_local),
        _pos_nb_local(it._pos_nb_local),
        _pos_world(it._pos_world),
        _ext_local(it._ext_local),
        // Important: _is_* must be recreated -- not copied!
        _is_self_local(_pos_self_local,_ext_local,_dir),
        _is_nb_local(_pos_nb_local,_ext_local,_dir),
        _is_global(_pos_world,_myself.transformingsubiterator().meshsize(),_dir),
        _normal(it._normal)
    {}

    //! copy constructor
    YaspIntersectionIterator & operator = (const YaspIntersectionIterator& it)
    {
      /* Assert same Iterator Context */
      if (! _selfp.equals(it._selfp))
        DUNE_THROW(GridError, "assignment of YaspIntersectionIterator "
                   << "with different inside Entity");

      /* Copy baseclass */
      YaspEntityPointer<0,GridImp>::operator=(it);

      /* Assign current position */
      _count = it._count;
      _dir = it._dir;
      _face = it._face;
      _pos_self_local = it._pos_self_local;
      _pos_nb_local = it._pos_nb_local;
      _pos_world = it._pos_world;
      _ext_local = it._ext_local;
      _normal = it._normal;

      return *this;
    }

  private:
    /* current position */
    int _count;                           //!< valid neighbor count in 0 .. 2*dim-1
    int _dir;                             //!< count/2
    int _face;                            //!< count%2
    /* neighbouring Entity/Poiner (get automatically updated) */
    const YaspEntityPointer<0,GridImp> _selfp; //!< entitypointer to myself
    const YaspEntity<0,dim,GridImp>&
    _myself;                              //!< reference to myself
    /* precalculated data for current position */
    FieldVector<ctype, dim> _pos_self_local; //!< center of face in own local coordinates
    FieldVector<ctype, dim> _pos_nb_local; //!< center of face in neighbors local coordinates
    FieldVector<ctype, dimworld>_pos_world;   //!< center of face in world coordinates
    FieldVector<ctype, dim> _ext_local;   //!< extension of face in local coordinates
    /* geometry objects (get automatically updated) */
    SpecialLocalGeometry _is_self_local;  //!< intersection in own local coordinates
    SpecialLocalGeometry _is_nb_local;    //!< intersection in neighbors local coordinates
    SpecialGeometry _is_global;           //!< intersection in global coordinates
    /* normal vector */
    FieldVector<ctype, dimworld> _normal; //!< for returning outer normal
  };


  //========================================================================
  /*!
     YaspHierarchicIterator enables iteration over son entities of codim 0
   */
  //========================================================================

  template<class GridImp>
  class YaspHierarchicIterator :
    public YaspEntityPointer<0,GridImp>,
    public HierarchicIteratorDefaultImplementation <GridImp,YaspHierarchicIterator>
  {
    enum { dim=GridImp::dimension };
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ctype;
  public:
    // types used from grids
    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;
    typedef typename GridImp::template Codim<0>::Entity Entity;
    typedef YaspSpecialEntity<0,dim,GridImp> SpecialEntity;

    //! define type used for coordinates in grid module
    typedef typename YGrid<dim,ctype>::iTupel iTupel;

    //! constructor
    YaspHierarchicIterator (const YGLI& g, const TSI& it, int maxlevel) :
      YaspEntityPointer<0,GridImp>(g,it)
    {
      // now iterator points to current cell

      // determine maximum level
      _maxlevel = std::min(maxlevel,this->_g.mg()->maxlevel());

      // if maxlevel not reached then push yourself and sons
      if (this->_g.level()<_maxlevel)
      {
        StackElem se(this->_g);
        se.coord = this->_it.coord();
        stack.push(se);
        push_sons();
      }

      // and make iterator point to first son if stack is not empty
      if (!stack.empty())
        pop_tos();
    }

    //! constructor
    YaspHierarchicIterator (const YaspHierarchicIterator& it) :
      YaspEntityPointer<0,GridImp>(it._g, it._it),
      _maxlevel(it._maxlevel), stack(it.stack)
    {}

    //! increment
    void increment ()
    {
      // sanity check: do nothing when stack is empty
      if (stack.empty()) return;

      // if maxlevel not reached then push sons
      if (this->_g.level()<_maxlevel)
        push_sons();

      // in any case pop one element
      pop_tos();
    }

    void print (std::ostream& s) const
    {
      s << "HIER: " << "level=" << this->_g.level()
        << " position=" << this->_it.coord()
        << " superindex=" << this->_it.superindex()
        << " maxlevel=" << this->_maxlevel
        << " stacksize=" << stack.size()
        << std::endl;
    }

  private:
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
      StackElem se(this->_g.finer());
      for (int i=0; i<(1<<dim); i++)
      {
        for (int k=0; k<dim; k++)
          if (i&(1<<k))
            se.coord[k] = this->_it.coord(k)*2+1;
          else
            se.coord[k] = this->_it.coord(k)*2;
        stack.push(se);
      }
    }

    // make TOS the current element
    void pop_tos ()
    {
      StackElem se = stack.pop();
      this->_g = se.g;
      this->_it.reinit(this->_g.cell_overlap(),se.coord);
    }
  };

  //========================================================================
  /*!
     YaspEntityPointer serves as a Reference or Pointer to a YaspGrid::Entity.
     It can also be initialized from Yasp::LevelIterator, Yasp::LeafIterator,
     Yasp::HierarchicIterator and Yasp::IntersectionIterator.

     We have specializations for codim==0 (elements) and
     codim=dim (vertices).
     The general version throws a GridError.
   */
  //========================================================================
  template<int codim, class GridImp>
  class YaspEntityPointer :
    public EntityPointerDefaultImplementation<codim,GridImp,
        Dune::YaspEntityPointer<codim,GridImp> >
  {
    //! know your own dimension
    enum { dim=GridImp::dimension };
    //! know your own dimension of world
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ctype;
  public:
    typedef typename GridImp::template Codim<codim>::Entity Entity;
    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;
    typedef YaspSpecialEntity<codim,dim,GridImp> SpecialEntity;

    //! constructor
    YaspEntityPointer (const YGLI & g, const TSI & it) : _g(g), _it(it), _entity(_g,_it)
    {
      if (codim>0 && codim<dim)
      {
        DUNE_THROW(GridError, "YaspLevelIterator: codim not implemented");
      }
    }

    //! copy constructor
    YaspEntityPointer (const YaspEntityPointer& rhs) : _g(rhs._g), _it(rhs._it), _entity(_g,_it)
    {
      if (codim>0 && codim<dim)
      {
        DUNE_THROW(GridError, "YaspLevelIterator: codim not implemented");
      }
    }

    //! equality
    bool equals (const YaspEntityPointer& rhs) const
    {
      return (_it==rhs._it && _g == rhs._g);
    }

    //! dereferencing
    Entity& dereference() const
    {
      return _entity;
    }

    //! ask for level of entity
    int level () const {return _g.level();}

    const YaspEntityPointer&
    operator = (const YaspEntityPointer& rhs)
    {
      _g = rhs._g;
      _it = rhs._it;
      /* _entity = i._entity
       * is done implicitely, as the entity is completely
       * defined via the interator it belongs to
       */
      return *this;
    }

  protected:
    YGLI _g;             // access to grid level
    TSI _it;             // position in the grid level
    mutable SpecialEntity _entity; //!< virtual entity
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
  class YaspLevelIterator :
    public YaspEntityPointer<codim,GridImp>,
    public LevelIteratorDefaultImplementation<codim,pitype,GridImp,YaspLevelIterator>
  {
    //! know your own dimension
    enum { dim=GridImp::dimension };
    //! know your own dimension of world
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ctype;
  public:
    typedef typename GridImp::template Codim<codim>::Entity Entity;
    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;
    typedef YaspSpecialEntity<codim,dim,GridImp> SpecialEntity;

    //! constructor
    YaspLevelIterator (const YGLI & g, const TSI & it) :
      YaspEntityPointer<codim,GridImp>(g,it) {}

    //! copy constructor
    YaspLevelIterator (const YaspLevelIterator& i) :
      YaspEntityPointer<codim,GridImp>(i) {}

    //! increment
    void increment()
    {
      ++(this->_it);
    }
  };


  //========================================================================
  /*!
     \brief level-wise, non-persistent, consecutive index

   */
  //========================================================================

  template <class GridImp>
  struct YaspLevelIndexSetTypes
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
  class YaspLevelIndexSet : public IndexSet<GridImp,YaspLevelIndexSet<GridImp>,YaspLevelIndexSetTypes<GridImp> >
  {
    typedef IndexSet<GridImp,YaspLevelIndexSet<GridImp>,YaspLevelIndexSetTypes<GridImp> > Base;
  public:

    //! constructor stores reference to a grid and level
    YaspLevelIndexSet (const GridImp& g, int l) : grid(g), level(l)
    {
      // contains a single element type;
      for (int codim=0; codim<=GridImp::dimension; codim++)
        mytypes[codim].push_back(NewGeometryType(NewGeometryType::cube,GridImp::dimension-codim));
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

    //! get number of entities of given codim, type and level (the level is known to the object)
    int size (int codim, NewGeometryType type) const
    {
      return grid.size(level,codim);
    }

    //! deliver all geometry types used in this grid
    const std::vector<NewGeometryType>& geomTypes (int codim) const
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
    std::vector<NewGeometryType> mytypes[GridImp::dimension+1];
  };


  // Leaf Index Set

  template <class GridImp>
  struct YaspLeafIndexSetTypes
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
  class YaspLeafIndexSet : public IndexSet<GridImp,YaspLeafIndexSet<GridImp>,YaspLeafIndexSetTypes<GridImp> >
  {
    typedef IndexSet<GridImp,YaspLeafIndexSet<GridImp>,YaspLeafIndexSetTypes<GridImp> > Base;
  public:

    //! constructor stores reference to a grid
    YaspLeafIndexSet (const GridImp& g) : grid(g)
    {
      // contains a single element type;
      for (int codim=0; codim<=GridImp::dimension; codim++)
        mytypes[codim].push_back(NewGeometryType(NewGeometryType::cube,GridImp::dimension-codim));
    }

    //! get index of an entity
    /*
       We use the RemoveConst to extract the Type from the mutable class,
       because the const class is not instatiated yet.
     */
    template<int cd>
    int index (const typename RemoveConst<GridImp>::Type::Traits::template Codim<cd>::Entity& e) const
    {
      assert(cd==0 || cd==GridImp::dimension);
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

    //! get number of entities of given codim, type
    int size (int codim, NewGeometryType type) const
    {
      return grid.size(grid.maxLevel(),codim);
    }

    //! deliver all geometry types used in this grid
    const std::vector<NewGeometryType>& geomTypes (int codim) const
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
    std::vector<NewGeometryType> mytypes[GridImp::dimension+1];
  };




  //========================================================================
  /*!
     \brief persistent, globally unique Ids

   */
  //========================================================================

  template<class GridImp>
  class YaspGlobalIdSet : public IdSet<GridImp,YaspGlobalIdSet<GridImp>,
                              typename RemoveConst<GridImp>::Type::PersistentIndexType >
                          /*
                             We used the RemoveConst to extract the Type from the mutable class,
                             because the const class is not instatiated yet.
                           */
  {
  public:
    //! define the type used for persisitent indices
    typedef typename RemoveConst<GridImp>::Type::PersistentIndexType IdType;

    //! constructor stores reference to a grid
    YaspGlobalIdSet (const GridImp& g) : grid(g) {}

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
  struct YaspGridFamily
  {
    typedef GridTraits<dim,dimworld,Dune::YaspGrid<dim,dimworld>,
        YaspGeometry,YaspEntity,
        YaspEntityPointer,YaspLevelIterator,
        YaspIntersectionIterator,YaspHierarchicIterator,
        YaspLevelIterator,
        YaspLevelIndexSet<const YaspGrid<dim,dimworld> >,
        YaspLevelIndexSetTypes<const YaspGrid<dim,dimworld> >,
        YaspLeafIndexSet<const YaspGrid<dim,dimworld> >,
        YaspLeafIndexSetTypes<const YaspGrid<dim,dimworld> >,
        YaspGlobalIdSet<const YaspGrid<dim,dimworld> >,
        bigunsignedint<dim*yaspgrid_dim_bits+yaspgrid_level_bits+yaspgrid_codim_bits>,
        YaspGlobalIdSet<const YaspGrid<dim,dimworld> >,
        bigunsignedint<dim*yaspgrid_dim_bits+yaspgrid_level_bits+yaspgrid_codim_bits> >
    Traits;
  };

  template<int dim, int codim>
  struct YaspCommunicateMeta {
    template<class G, class DataHandle>
    static void comm (const G& g, DataHandle& data, InterfaceType iftype, CommunicationDirection dir, int level)
    {
      if (data.contains(dim,codim))
        g.template communicateCodim<DataHandle,codim>(data,iftype,dir,level);
      YaspCommunicateMeta<dim,codim-1>::comm(g,data,iftype,dir,level);
    }
  };

  template<int dim>
  struct YaspCommunicateMeta<dim,0> {
    template<class G, class DataHandle>
    static void comm (const G& g, DataHandle& data, InterfaceType iftype, CommunicationDirection dir, int level)
    {
      if (data.contains(dim,0))
        g.template communicateCodim<DataHandle,0>(data,iftype,dir,level);
    }
  };


  //************************************************************************
  /*!
     \brief [<em> provides \ref Dune::Grid </em>]
     \brief Provides a distributed structured cube mesh.
     \ingroup GridImplementations

     YaspGrid stands for yet another structured parallel grid.
     It will implement the dune grid interface for structured grids with codim 0
     and dim, with arbitrary overlap, parallel features with two overlap
     models, periodic boundaries and fast a implementation allowing on-the-fly computations.

     \par History:
     \li started on July 31, 2004 by PB based on abstractions developed in summer 2003

     \note The only class intended for public use is \ref Dune::YaspGrid
     itself. All other classes are of no use for an application writer.
   */
  template<int dim, int dimworld>
  class YaspGrid :
    public GridDefaultImplementation<dim,dimworld,yaspgrid_ctype,YaspGridFamily<dim,dimworld> >,
    public MultiYGrid<dim,yaspgrid_ctype>
  {
    typedef const YaspGrid<dim,dimworld> GridImp;
  public:
    //! define type used for coordinates in grid module
    typedef yaspgrid_ctype ctype;

    // define the persistent index type
    typedef bigunsignedint<dim*yaspgrid_dim_bits+yaspgrid_level_bits+yaspgrid_codim_bits> PersistentIndexType;

    //! the GridFamily of this grid
    typedef YaspGridFamily<dim,dimworld> GridFamily;
    //! the Traits
    typedef typename YaspGridFamily<dim,dimworld>::Traits Traits;

    // need for friend declarations in entity
    typedef YaspLevelIndexSet<YaspGrid<dim,dimworld> > LevelIndexSetType;
    typedef YaspLeafIndexSet<YaspGrid<dim,dimworld> > LeafIndexSetType;
    typedef YaspGlobalIdSet<YaspGrid<dim,dimworld> > GlobalIdSetType;

    //! maximum number of levels allowed
    enum { MAXL=64 };

    //! shorthand for base class data types
    typedef MultiYGrid<dim,ctype> YMG;
    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;
    typedef typename MultiYGrid<dim,ctype>::Intersection IS;
    typedef typename std::deque<IS>::const_iterator ISIT;

    //! return GridIdentifierType of Grid, i.e. SGrid_Id or AlbertGrid_Id ...
    GridIdentifier type() const { return YaspGrid_Id; };

    /*! Constructor for a YaspGrid, they are all forwarded to the base class
          @param comm MPI communicator where this mesh is distributed to
          @param L extension of the domain
          @param s number of cells on coarse mesh in each direction
       @param periodic tells if direction is periodic or not
       @param size of overlap on coarsest grid (same in all directions)
     */
#if HAVE_MPI
    YaspGrid (MPI_Comm comm, Dune::FieldVector<ctype, dim> L,
              Dune::FieldVector<int, dim> s,
              Dune::FieldVector<bool, dim> periodic, int overlap)
      : MultiYGrid<dim,ctype>(comm,L,s,periodic,overlap), ccobj(comm), theleafindexset(*this), theglobalidset(*this)
#else
    YaspGrid (Dune::FieldVector<ctype, dim> L,
              Dune::FieldVector<int, dim> s,
              Dune::FieldVector<bool, dim> periodic, int overlap)
      : MultiYGrid<dim,ctype>(L,s,periodic,overlap), theleafindexset(*this), theglobalidset(*this)
#endif
    {
      setsizes();
      indexsets.push_back( new YaspLevelIndexSet<const YaspGrid<dim,dimworld> >(*this,0) );
    }

    /*! Return maximum level defined in this grid. Levels are numbered
          0 ... maxlevel with 0 the coarsest level.
     */
    int maxLevel() const {return MultiYGrid<dim,ctype>::maxlevel();} // delegate

    //! refine the grid refCount times. What about overlap?
    void globalRefine (int refCount)
    {
      bool b=true;
      for (int k=0; k<refCount; k++)
      {
        MultiYGrid<dim,ctype>::refine(b);
        setsizes();
        indexsets.push_back( new YaspLevelIndexSet<const YaspGrid<dim,dimworld> >(*this,maxLevel()) );
      }
    }

    // set options for refinement
    void refineOptions (bool b)
    {
      keep_ovlp=b;
    }

    //! refine the grid refCount times. What about overlap?
    void refine (bool b)
    {
      MultiYGrid<dim,ctype>::refine(b);
      setsizes();
      indexsets.push_back( new YaspLevelIndexSet<const YaspGrid<dim,dimworld> >(*this,maxLevel()) );
    }

    //! map adapt to global refine
    bool adapt ()
    {
      globalRefine(1);
      return true;
    }

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template Codim<cd>::template Partition<pitype>::LevelIterator lbegin (int level) const
    {
      return levelbegin<cd,pitype>(level);
    }

    //! Iterator to one past the last entity of given codim on level for partition type
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template Codim<cd>::template Partition<pitype>::LevelIterator lend (int level) const
    {
      return levelend<cd,pitype>(level);
    }

    //! version without second template parameter for convenience
    template<int cd>
    typename Traits::template Codim<cd>::template Partition<All_Partition>::LevelIterator lbegin (int level) const
    {
      return levelbegin<cd,All_Partition>(level);
    }

    //! version without second template parameter for convenience
    template<int cd>
    typename Traits::template Codim<cd>::template Partition<All_Partition>::LevelIterator lend (int level) const
    {
      return levelend<cd,All_Partition>(level);
    }

    //! return LeafIterator which points to the first entity in maxLevel
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template Codim<cd>::template Partition<pitype>::LeafIterator leafbegin () const
    {
      return levelbegin<cd,pitype>(maxLevel());
    };

    //! return LeafIterator which points behind the last entity in maxLevel
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template Codim<cd>::template Partition<pitype>::LeafIterator leafend () const
    {
      return levelend<cd,pitype>(maxLevel());
    }

    //! return LeafIterator which points to the first entity in maxLevel
    template<int cd>
    typename Traits::template Codim<cd>::template Partition<All_Partition>::LeafIterator leafbegin () const
    {
      return levelbegin<cd,All_Partition>(maxLevel());
    };

    //! return LeafIterator which points behind the last entity in maxLevel
    template<int cd>
    typename Traits::template Codim<cd>::template Partition<All_Partition>::LeafIterator leafend () const
    {
      return levelend<cd,All_Partition>(maxLevel());
    }

    //! return size (= distance in graph) of overlap region
    int overlapSize (int level, int codim) const
    {
      YGLI g = MultiYGrid<dim,ctype>::begin(level);
      return g.overlap();
    }

    //! return size (= distance in graph) of overlap region
    int overlapSize (int codim) const
    {
      YGLI g = MultiYGrid<dim,ctype>::begin(maxLevel());
      return g.overlap();
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

    //! number of entities per level and codim in this process
    int size (int level, int codim) const
    {
      return sizes[level][codim];
    }

    //! number of leaf entities per codim in this process
    int size (int codim) const
    {
      return sizes[maxLevel()][codim];
    }

    //! number of entities per level, codim and geometry type in this process
    int size (int level, int codim, NewGeometryType type) const
    {
      if (type.isCube()) return sizes[level][codim];
      return 0;
    }

    //! number of leaf entities per codim and geometry type in this process
    int size (int codim, NewGeometryType type) const
    {
      return size(maxLevel(),codim,type);
    }

    /*! The communication interface
          @param T: array class holding data associated with the entities
          @param P: type used to gather/scatter data in and out of the message buffer
          @param codim: communicate entites of given codim
          @param if: one of the predefined interface types, throws error if it is not implemented
          @param level: communicate for entities on the given level

          Implements a generic communication function sending an object of type P for each entity
       in the intersection of two processors. P has two methods gather and scatter that implement
       the protocol. Therefore P is called the "protocol class".
     */
#if HAVE_MPI
    template<class T, template<class> class P, int codim>
    void communicate (T& t, InterfaceType iftype, CommunicationDirection dir, int level) const
    {
      IsTrue< ( codim == dim || codim == 0 ) >::yes();
      // access to grid level
      YGLI g = MultiYGrid<dim,ctype>::begin(level);

      // find send/recv lists or throw error
      const std::deque<IS>* sendlist;
      const std::deque<IS>* recvlist;
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
#endif

    /*! The new communication interface

       communicate objects for all codims on a given level
     */
    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir, int level) const
    {
      YaspCommunicateMeta<dim,dim>::comm(*this,data,iftype,dir,level);
    }

    /*! The new communication interface

       communicate objects for all codims on the leaf grid
     */
    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir) const
    {
      YaspCommunicateMeta<dim,dim>::comm(*this,data,iftype,dir,this->maxLevel());
    }

    /*! The new communication interface

       communicate objects for one codim
     */
    template<class DataHandle, int codim>
    void communicateCodim (DataHandle& data, InterfaceType iftype, CommunicationDirection dir, int level) const
    {
      // check input
      if (!data.contains(dim,codim)) return;   // should have been checked outside

      // data types
      typedef typename DataHandle::DataType DataType;

      // access to grid level
      YGLI g = MultiYGrid<dim,ctype>::begin(level);

      // find send/recv lists or throw error
      const std::deque<IS>* sendlist;
      const std::deque<IS>* recvlist;
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

      // Size computation (requires communication if variable size)
      std::map<int,int> send_size;        // map rank to total number of objects (of type DataType) to be sent
      std::map<int,int> recv_size;        // map rank to total number of objects (of type DataType) to be recvd
      std::map<int,size_t*> send_sizes;   // map rank to array giving number of objects per entity to be sent
      std::map<int,size_t*> recv_sizes;   // map rank to array giving number of objects per entity to be recvd
      if (data.fixedsize(dim,codim))
      {
        // fixed size: just take a dummy entity, size can be computed without communication
        for (ISIT is=sendlist->begin(); is!=sendlist->end(); ++is)
        {
          typename Traits::template Codim<codim>::template Partition<All_Partition>::LevelIterator
          it(YaspLevelIterator<codim,All_Partition,GridImp>(g,is->grid.tsubbegin()));
          send_size[is->rank] = is->grid.totalsize() * data.size(*it);
        }
        for (ISIT is=recvlist->begin(); is!=recvlist->end(); ++is)
        {
          typename Traits::template Codim<codim>::template Partition<All_Partition>::LevelIterator
          it(YaspLevelIterator<codim,All_Partition,GridImp>(g,is->grid.tsubbegin()));
          recv_size[is->rank] = is->grid.totalsize() * data.size(*it);
        }
      }
      else
      {
        // variable size case: sender side determines the size
        for (ISIT is=sendlist->begin(); is!=sendlist->end(); ++is)
        {
          // allocate send buffer for sizes per entitiy
          size_t *buf = new size_t[is->grid.totalsize()];
          send_sizes[is->rank] = buf;

          // loop over entities and ask for size
          int i=0; size_t n=0;
          typename Traits::template Codim<codim>::template Partition<All_Partition>::LevelIterator
          it(YaspLevelIterator<codim,All_Partition,GridImp>(g,is->grid.tsubbegin()));
          typename Traits::template Codim<codim>::template Partition<All_Partition>::LevelIterator
          tsubend(YaspLevelIterator<codim,All_Partition,GridImp>(g,is->grid.tsubend()));
          for ( ; it!=tsubend; ++it)
          {
            buf[i] = data.size(*it);
            n += buf[i];
            i++;
          }

          // now we know the size for this rank
          send_size[is->rank] = n;

          // hand over send request to torus class
          MultiYGrid<dim,ctype>::torus().send(is->rank,buf,is->grid.totalsize()*sizeof(size_t));
        }

        // allocate recv buffers for sizes and store receive request
        for (ISIT is=recvlist->begin(); is!=recvlist->end(); ++is)
        {
          // allocate recv buffer
          size_t *buf = new size_t[is->grid.totalsize()];
          recv_sizes[is->rank] = buf;

          // hand over recv request to torus class
          MultiYGrid<dim,ctype>::torus().recv(is->rank,buf,is->grid.totalsize()*sizeof(size_t));
        }

        // exchange all size buffers now
        MultiYGrid<dim,ctype>::torus().exchange();

        // release send size buffers
        for (ISIT is=sendlist->begin(); is!=sendlist->end(); ++is)
          delete[] send_sizes[is->rank];

        // process receive size buffers
        for (ISIT is=recvlist->begin(); is!=recvlist->end(); ++is)
        {
          // get recv buffer
          size_t *buf = recv_sizes[is->rank];

          // compute total size
          size_t n=0;
          for (int i=0; i<is->grid.totalsize(); ++i)
            n += buf[i];

          // ... and store it
          recv_size[is->rank] = n;
        }
      }


      // allocate & fill the send buffers & store send request
      std::map<int,DataType*> sends;   // store pointers to send buffers
      for (ISIT is=sendlist->begin(); is!=sendlist->end(); ++is)
      {
        // allocate send buffer
        DataType *buf = new DataType[send_size[is->rank]];

        // remember send buffer
        sends[is->rank] = buf;

        // make a message buffer
        MessageBuffer<DataType> mb(buf);

        // fill send buffer; iterate over cells in intersection
        typename Traits::template Codim<codim>::template Partition<All_Partition>::LevelIterator
        it(YaspLevelIterator<codim,All_Partition,GridImp>(g,is->grid.tsubbegin()));
        typename Traits::template Codim<codim>::template Partition<All_Partition>::LevelIterator
        tsubend(YaspLevelIterator<codim,All_Partition,GridImp>(g,is->grid.tsubend()));
        for ( ; it!=tsubend; ++it)
          data.gather(mb,*it);

        // hand over send request to torus class
        MultiYGrid<dim,ctype>::torus().send(is->rank,buf,send_size[is->rank]*sizeof(DataType));
      }

      // allocate recv buffers and store receive request
      std::map<int,DataType*> recvs;   // store pointers to send buffers
      for (ISIT is=recvlist->begin(); is!=recvlist->end(); ++is)
      {
        // allocate recv buffer
        DataType *buf = new DataType[recv_size[is->rank]];

        // remember recv buffer
        recvs[is->rank] = buf;

        // hand over recv request to torus class
        MultiYGrid<dim,ctype>::torus().recv(is->rank,buf,recv_size[is->rank]*sizeof(DataType));
      }

      // exchange all buffers now
      MultiYGrid<dim,ctype>::torus().exchange();

      // release send buffers
      for (ISIT is=sendlist->begin(); is!=sendlist->end(); ++is)
        delete[] sends[is->rank];

      // process receive buffers and delete them
      for (ISIT is=recvlist->begin(); is!=recvlist->end(); ++is)
      {
        // get recv buffer
        DataType *buf = recvs[is->rank];

        // make a message buffer
        MessageBuffer<DataType> mb(buf);

        // copy data from receive buffer; iterate over cells in intersection
        if (data.fixedsize(dim,codim))
        {
          typename Traits::template Codim<codim>::template Partition<All_Partition>::LevelIterator
          it(YaspLevelIterator<codim,All_Partition,GridImp>(g,is->grid.tsubbegin()));
          size_t n=data.size(*it);
          typename Traits::template Codim<codim>::template Partition<All_Partition>::LevelIterator
          tsubend(YaspLevelIterator<codim,All_Partition,GridImp>(g,is->grid.tsubend()));
          for ( ; it!=tsubend; ++it)
            data.scatter(mb,*it,n);
        }
        else
        {
          int i=0;
          size_t *sbuf = recv_sizes[is->rank];
          typename Traits::template Codim<codim>::template Partition<All_Partition>::LevelIterator
          it(YaspLevelIterator<codim,All_Partition,GridImp>(g,is->grid.tsubbegin()));
          typename Traits::template Codim<codim>::template Partition<All_Partition>::LevelIterator
          tsubend(YaspLevelIterator<codim,All_Partition,GridImp>(g,is->grid.tsubend()));
          for ( ; it!=tsubend; ++it)
            data.scatter(mb,*it,sbuf[i++]);
          delete[] sbuf;
        }

        // delete buffer
        delete[] buf;
      }
    }

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
      return *(indexsets[level]);
    }

    const typename Traits::LeafIndexSet& leafIndexSet() const
    {
      return theleafindexset;
    }

#if HAVE_MPI
    const CollectiveCommunication<MPI_Comm>& comm () const
    {
      return ccobj;
    }
#else
    const CollectiveCommunication<YaspGrid>& comm () const
    {
      return ccobj;
    }
#endif

    YaspIntersectionIterator<const YaspGrid<dim, dimworld> >&
    getRealIntersectionIterator(typename Traits::IntersectionIterator& it)
    {
      return this->getRealImplementation(it);
    }

    const YaspIntersectionIterator<const YaspGrid<dim, dimworld> >&
    getRealIntersectionIterator(const typename Traits::IntersectionIterator& it) const
    {
      return this->getRealImplementation(it);
    }

  private:

#if HAVE_MPI
    CollectiveCommunication<MPI_Comm> ccobj;
#else
    CollectiveCommunication<YaspGrid> ccobj;
#endif

    std::vector<YaspLevelIndexSet<const YaspGrid<dim,dimworld> >*> indexsets;
    YaspLeafIndexSet<const YaspGrid<dim,dimworld> > theleafindexset;
    YaspGlobalIdSet<const YaspGrid<dim,dimworld> > theglobalidset;

    // Index classes need access to the real entity
    friend class Dune::YaspLevelIndexSet<const Dune::YaspGrid<dim,dimworld> >;
    friend class Dune::YaspLeafIndexSet<const Dune::YaspGrid<dim,dimworld> >;
    friend class Dune::YaspGlobalIdSet<const Dune::YaspGrid<dim,dimworld> >;

    template<int codim>
    YaspEntity<codim,dim,const YaspGrid<dim,dimworld> >&
    getRealEntity(typename Traits::template Codim<codim>::Entity& e )
    {
      return this->getRealImplementation(e);
    }

    template<int codim>
    const YaspEntity<codim,dim,const YaspGrid<dim,dimworld> >&
    getRealEntity(const typename Traits::template Codim<codim>::Entity& e ) const
    {
      return this->getRealImplementation(e);
    }

    template<int codim_, int dim_, class GridImp_, template<int,int,class> class EntityImp_>
    friend class Entity;

    template<class DT>
    class MessageBuffer {
    public:
      // Constructor
      MessageBuffer (DT *p)
      {
        a=p;
        i=0;
        j=0;
      }

      // write data to message buffer, acts like a stream !
      template<class Y>
      void write (const Y& data)
      {
        IsTrue< ( SameType<DT,Y>::value ) >::yes();
        a[i++] = data;
      }

      // read data from message buffer, acts like a stream !
      template<class Y>
      void read (Y& data)
      {
        IsTrue< ( SameType<DT,Y>::value ) >::yes();
        data = a[j++];
      }

    private:
      DT *a;
      int i,j;
    };

    void setsizes ()
    {
      for (YGLI g=MultiYGrid<dim,ctype>::begin(); g!=MultiYGrid<dim,ctype>::end(); ++g)
      {
        // codim 0 (elements)
        sizes[g.level()][0] = 1;
        for (int i=0; i<dim; ++i)
          sizes[g.level()][0] *= g.cell_overlap().size(i);

        // codim 1 (faces)
        if (dim>1)
        {
          sizes[g.level()][1] = 0;
          for (int i=0; i<dim; ++i)
          {
            int s=g.cell_overlap().size(i)+1;
            for (int j=0; j<dim; ++j)
              if (j!=i)
                s *= g.cell_overlap().size(j);
            sizes[g.level()][1] += s;
          }
        }

        // codim dim-1 (edges)
        if (dim>2)
        {
          sizes[g.level()][dim-1] = 0;
          for (int i=0; i<dim; ++i)
          {
            int s=g.cell_overlap().size(i);
            for (int j=0; j<dim; ++j)
              if (j!=i)
                s *= g.cell_overlap().size(j)+1;
            sizes[g.level()][dim-1] += s;
          }
        }

        // codim dim (vertices)
        sizes[g.level()][dim] = 1;
        for (int i=0; i<dim; ++i)
          sizes[g.level()][dim] *= g.vertex_overlapfront().size(i);
      }
    }

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    YaspLevelIterator<cd,pitype,GridImp> levelbegin (int level) const
    {
      IsTrue< ( cd == dim || cd == 0 ) >::yes();
      YGLI g = MultiYGrid<dim,ctype>::begin(level);
      if (level<0 || level>maxLevel()) DUNE_THROW(RangeError, "level out of range");
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
    YaspLevelIterator<cd,pitype,GridImp> levelend (int level) const
    {
      IsTrue< ( cd == dim || cd == 0 ) >::yes();
      YGLI g = MultiYGrid<dim,ctype>::begin(level);
      if (level<0 || level>maxLevel()) DUNE_THROW(RangeError, "level out of range");
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

    int sizes[MAXL][dim+1]; // total number of entities per level and codim
    bool keep_ovlp;
  };

  namespace Capabilities
  {

    template<int dim,int dimw>
    struct hasLeafIterator< YaspGrid<dim,dimw> >
    {
      static const bool v = false;
    };

    template<int dim, int dimw>
    struct hasEntity< YaspGrid<dim,dimw>, 0 >
    {
      static const bool v = true;
    };

    template<int dim, int dimw>
    struct hasEntity< YaspGrid<dim,dimw>, dim >
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
