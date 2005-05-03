// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_YASPGRID_HH
#define DUNE_YASPGRID_HH

#include <iostream>

#include "common/grid.hh"     // the grid base classes
#include "yaspgrid/grids.hh"  // the yaspgrid base classes
#include "../common/stack.hh" // the stack class
#include "../common/capabilities.hh" // the capabilities
#include "../common/helpertemplates.hh"
#include "../common/bigunsignedint.hh"

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

  /*! type used for the persistent indices
   */

  // globally define the persistent index type
  typedef bigunsignedint<64> yaspgrid_persistentindextype;



  //************************************************************************
  // forward declaration of templates

  template<int dim, int dimworld>               class YaspGrid;
  template<int mydim, int cdim, class GridImp>  class YaspGeometry;
  template<int codim, int dim, class GridImp>   class YaspEntity;
  template<int codim, class GridImp>            class YaspEntityPointer;
  template<int codim, PartitionIteratorType pitype, class GridImp> class YaspLevelIterator;
  template<class GridImp>            class YaspLeafIterator;
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
#warning Yasp::ReferenceGeometry not working!
      DUNE_THROW(NotImplemented, "Yasp::ReferenceGeometry");
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
    FieldMatrix<ctype,mydim,mydim>& jacobianInverse (const FieldVector<ctype, mydim>& local) const
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
      return this->realEntity.transformingsubiterator();
    }
    const YGLI& gridlevel () const
    {
      return this->realEntity.gridlevel();
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

    //! define the type used for persisitent indices
    typedef yaspgrid_persistentindextype PersistentIndexType;

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
      DUNE_THROW(GridError, "codim not (yet) implemented");
    }

    /*! Intra-element access to subentities of codimension cc > codim.
     */
    template<int cc>
    typename codim<cc>::EntityPointer entity (int i) const
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
      DUNE_THROW(NotImplemented," geometryInFather");
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
    friend class GridImp::IndexType; // needs access to the private index methods

    //! globally unique, persistent index
    PersistentIndexType persistentIndex () const
    {
      // get coordinate and size of global grid
      const iTupel& size =  _g.cell_global().size();
      int coord[dim];

      // correction for periodic boundaries
      for (int i=0; i<dim; i++)
      {
        coord[i] = _it.coord(i);
        if (coord[i]<0) coord[i] += size[i];
        if (coord[i]>=size[i]) coord[i] -= size[i];
      }

      // make one number from coordinate
      PersistentIndexType number1(coord[dim-1]);
      for (int i=dim-2; i>=0; i--)
        number1 = (number1*size[i])+coord[i];

      // encode codim and level
      PersistentIndexType number2((_g.level()<<4));

      return number1|(number2<<52);
    }

    //! consecutive, codim-wise, level-wise index
    int compressedIndex () const
    {
      _it.superindex();
    }

    //! subentity persistent index
    template<int cc>
    PersistentIndexType subPersistentIndex (int i) const
    {
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

        // make one number from coordinate
        PersistentIndexType number1(coord[dim-1]);
        for (int k=dim-2; k>=0; k--)
          number1 = (number1*(_g.cell_global().size(k)+1))+coord[k];

        // encode codim and level
        PersistentIndexType number2((_g.level()<<4)+cc);

        return number1|(number2<<52);
      }

      if (cc==1)   // faces, i.e. for dim=2 codim=1 is treated as a face
      {
        // Idea: direction ivar varies, all others are fixed, i.e. 2 possibilities per direction

        // ivar is the direction that varies
        int ivar=i/2;

        // compute position from cell position
        if (i%2) coord[ivar] += 1;

        // do lexicographic numbering
        PersistentIndexType index(coord[dim-1]);
        for (int k=dim-2; k>=0; --k)
          if (k==ivar)
            index = (index*(_g.cell_global().size(k)+1))+coord[k];             // one more
          else
            index = (index*(_g.cell_global().size(k)))+coord[k];

        // add size of all subsets for smaller directions
        for (int j=0; j<ivar; j++)
        {
          PersistentIndexType n(_g.cell_global().size(j)+1);
          for (int l=0; l<dim; l++)
            if (l!=j) n = n*_g.cell_global().size(l);
          index = index+n;
        }

        // encode codim and level
        PersistentIndexType modifier((_g.level()<<4)+cc);

        return index|(modifier<<52);
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
        PersistentIndexType index(coord[dim-1]);
        for (int k=dim-2; k>=0; --k)
          if (k!=ifix)
            index = (index*(_g.cell_global().size(k)+1))+coord[k];             // one more
          else
            index = (index*(_g.cell_global().size(k)))+coord[k];

        // add size of all subsets for smaller directions
        for (int j=dim-1; j>ifix; j--)
        {
          PersistentIndexType n(_g.cell_overlap().size(j));
          for (int l=0; l<dim; l++)
            if (l!=j) n = n*(_g.cell_global().size(l)+1);
          index = index+n;
        }

        // encode codim and level
        PersistentIndexType modifier((_g.level()<<4)+cc);

        return index|(modifier<<52);
      }

      DUNE_THROW(GridError, "codim not (yet) implemented");
    }

    //! subentity compressed index
    template<int cc>
    int subCompressedIndex (int i) const
    {
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

      DUNE_THROW(GridError, "codim not (yet) implemented");
    }


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

    //! define the type used for persisitent indices
    typedef yaspgrid_persistentindextype PersistentIndexType;

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
    friend class GridImp::IndexType; // needs access to the private index methods

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

      // make one number from coordinate
      PersistentIndexType number1(coord[dim-1]);
      for (int i=dim-2; i>=0; i--)
        number1 = (number1*size[i])+coord[i];

      // encode codim and level
      PersistentIndexType number2((_g.level()<<4)+dim);

      return number1|(number2<<52);
    }

    //! consecutive, codim-wise, level-wise index
    int compressedIndex () const { return _it.superindex();}

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
  class YaspIntersectionIterator :
    public YaspEntityPointer<0,GridImp>,
    public IntersectionIteratorDefault<GridImp,YaspIntersectionIterator>
  {
    enum { dim=GridImp::dimension };
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ctype;
    YaspIntersectionIterator();
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
#if 0
      // The transforming iterator can be safely moved beyond the boundary.
      // So we only have to compare against the cell_global grid
      if (this->_it.coord(_dir)>=_myself.gridlevel().cell_overlap().min(_dir)
          &&
          this->_it.coord(_dir)<=_myself.gridlevel().cell_overlap().max(_dir))
        return true;
      else
        return false;
#endif
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

  private:
    int _count;                           //!< valid neighbor count in 0 .. 2*dim-1
    int _dir;                             //!< count/2
    int _face;                            //!< count%2
    const YaspEntity<0,dim,GridImp>&
    _myself;                              //!< reference to myself
    FieldVector<ctype, dim> _pos_self_local; //!< center of face in own local coordinates
    FieldVector<ctype, dim> _pos_nb_local; //!< center of face in neighbors local coordinates
    FieldVector<ctype, dimworld>_pos_world;   //!< center of face in world coordinates
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
  class YaspHierarchicIterator :
    public YaspEntityPointer<0,GridImp>,
    public HierarchicIteratorDefault <GridImp,YaspHierarchicIterator>
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
    public EntityPointerDefault<codim,GridImp,
        Dune::YaspEntityPointer<codim,GridImp> >
  {
    //! know your own dimension
    enum { dim=GridImp::dimension };
    //! know your own dimension of world
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ctype;
  public:
    typedef typename GridImp::template codim<codim>::Entity Entity;
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
    public LevelIteratorDefault<codim,pitype,GridImp,YaspLevelIterator>
  {
    //! know your own dimension
    enum { dim=GridImp::dimension };
    //! know your own dimension of world
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ctype;
  public:
    typedef typename GridImp::template codim<codim>::Entity Entity;
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
     YaspLeafIterator enables iteration over entities of one leaf level

     We have specializations for codim==0 (elements) and
     codim=dim (vertices).
     The general version throws a GridError.
   */
  //========================================================================

  template <class GridImp>
  class YaspLeafIterator : public YaspLevelIterator<0,All_Partition,GridImp>
  {
    //! know your own dimension
    enum { dim=GridImp::dimension };
    typedef typename GridImp::ctype ctype;
  public:
    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,ctype>::TransformingSubIterator TSI;
    YaspLeafIterator(const YGLI & g, const TSI & it) :
      YaspLevelIterator<0,All_Partition,GridImp>(g,it)
    {}
    YaspLeafIterator(const YaspLeafIterator& i) :
      YaspLevelIterator<0,All_Partition,GridImp>(i)
    {}
  };

  //========================================================================
  /*!
     YaspIndex provides mappers with index information for an entity

     Numbering of the entities on one level which can also be used as leaf index
   */
  //========================================================================

  // Index class for level indices, we implement it with methods in the entity
  template<class GridImp>
  class YaspIndex
  {
  public:
    //! define the type used for persisitent indices
    typedef yaspgrid_persistentindextype PersistentIndexType;

    //! globally unique, persistent index
    template<int cd>
    PersistentIndexType persistent (const typename GridImp::Traits::template codim<cd>::Entity& e) const
    {
      return grid.template getRealEntity<cd>(e).persistentIndex();
    }

    //! consecutive, codim-wise and geometrytype-wise index
    template<int cd>
    int compressed (const typename GridImp::Traits::template codim<cd>::Entity& e) const
    {
      return grid.template getRealEntity<cd>(e).compressedIndex();
    }

    //! subentity persistent index
    template<int cc>
    PersistentIndexType subpersistent (const typename GridImp::Traits::template codim<0>::Entity& e, int i) const
    {
      return grid.template getRealEntity<0>(e).template subPersistentIndex<cc>(i);
    }

    //! subentity compressed index
    template<int cc>
    int subcompressed (const typename GridImp::Traits::template codim<0>::Entity& e, int i) const
    {
      return grid.template getRealEntity<0>(e).template subCompressedIndex<cc>(i);
    }

    YaspIndex (const GridImp& g) : grid(g) {}

  private:
    const GridImp& grid;
  };


  //************************************************************************
  /*!
     \brief [<em> provides \ref Dune::Grid </em>]
     \brief Provides a distributed structured cube mesh.
     \ingroup GridInterface

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
    public GridDefault<dim,dimworld,yaspgrid_ctype,YaspGrid<dim,dimworld> >,
    public MultiYGrid<dim,yaspgrid_ctype>
  {
    typedef const YaspGrid<dim,dimworld> GridImp;
  public:
    typedef GridTraits<dim,dimworld,Dune::YaspGrid<dim,dimworld>,
        YaspGeometry,YaspEntity,YaspBoundaryEntity,
        YaspEntityPointer,YaspLevelIterator,
        YaspIntersectionIterator,YaspHierarchicIterator,
        YaspLeafIterator> Traits;

    typedef YaspIndex<YaspGrid<dim,dimworld> > IndexType;

    //! define type used for coordinates in grid module
    typedef yaspgrid_ctype ctype;

    //! define the type used for persisitent indices
    typedef yaspgrid_persistentindextype PersistentIndexType;

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
              Dune::FieldVector<bool, dim> periodic, int overlap) :
      MultiYGrid<dim,ctype>(comm,L,s,periodic,overlap),yi(*this)
    {
      setsizes();
    }

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
      setsizes();
    }

    //! refine the grid refCount times. What about overlap?
    void refine (bool b)
    {
      MultiYGrid<dim,ctype>::refine(b);
      setsizes();
    }

    //! one past the end on this level
    template<int cd, PartitionIteratorType pitype>
    typename Traits::template codim<cd>::template partition<pitype>::LevelIterator lbegin (int level) const
    {
      IsTrue< ( cd == dim || cd == 0 ) >::yes();
      YGLI g = MultiYGrid<dim,ctype>::begin(level);
      if (level<0 || level>maxlevel()) DUNE_THROW(RangeError, "level out of range");
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
      if (level<0 || level>maxlevel()) DUNE_THROW(RangeError, "level out of range");
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
      if (level<0 || level>maxlevel()) DUNE_THROW(RangeError, "level out of range");
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
      if (level<0 || level>maxlevel()) DUNE_THROW(RangeError, "level out of range");
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

    //! return LeafIterator which points to the first entity in maxLevel
    typename Traits::LeafIterator leafbegin(int maxLevel) const
    {
      int level = std::min(maxLevel,maxlevel());
      YGLI g = MultiYGrid<dim,ctype>::begin(level);
      return YaspLeafIterator<const GridImp>(g,g.cell_overlap().tsubbegin());
    };

    //! return LeafIterator which points behind the last entity in maxLevel
    typename Traits::LeafIterator leafend(int maxLevel) const
    {
      int level = std::min(maxLevel,maxlevel());
      YGLI g = MultiYGrid<dim,ctype>::begin(level);
      return YaspLeafIterator<const GridImp>(g,g.cell_overlap().tsubend());
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

    //! number of entities per level and codim in this process
    int size (int level, int codim) const
    {
      return sizes[level][codim];
    }

    //! number of leaf entities per codim in this process
    int size (int codim) const
    {
      return sizes[maxlevel()][codim];
    }

    //! number of entities per level, codim and geometry type in this process
    int size (int level, int codim, GeometryType type) const
    {
      if (type==hypercube) return sizes[level][codim];
      switch (dim-codim)
      {
      case 0 :
        if (type==vertex) return sizes[level][codim];
        break;

      case 1 :
        if (type==line) return sizes[level][codim];
        break;

      case 2 :
        if (type==quadrilateral) return sizes[level][codim];
        if (type==iso_quadrilateral) return sizes[level][codim];
        break;

      case 3 :
        if (type==hexahedron) return sizes[level][codim];
        break;
      }
      return 0;
    }

    //! number of leaf entities per codim and geometry type in this process
    int size (int codim, GeometryType type) const
    {
      return size(maxlevel(),codim,type);
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
      typedef typename std::deque<IS>::const_iterator ISIT;
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

    // the index methods
    const IndexType& leafindex () const
    {
      return yi;
    }

    const IndexType& levelindex () const
    {
      return yi;
    }

    const IndexType& savedleafindex () const
    {
      return yi;
    }

    const IndexType& savedlevelindex () const
    {
      return yi;
    }

  private:
    IndexType yi;

    // Index classes need access to the real entity
    friend class Dune::YaspIndex<Dune::YaspGrid<dim,dimworld> >;

    template<int codim>
    YaspEntity<codim,dim,const YaspGrid<dim,dimworld> >&
    getRealEntity(typename Traits::template codim<codim>::Entity& e )
    {
      return e.realEntity;
    }

    template<int codim>
    const YaspEntity<codim,dim,const YaspGrid<dim,dimworld> >&
    getRealEntity(const typename Traits::template codim<codim>::Entity& e ) const
    {
      return e.realEntity;
    }

    template<int codim_, int dim_, class GridImp_, template<int,int,class> class EntityImp_>
    friend class Entity;

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

    int sizes[MAXL][dim+1]; // total number of entities per level and codim
  };

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
