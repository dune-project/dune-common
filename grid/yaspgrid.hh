// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __YASPGRID_HH__
#define __YASPGRID_HH__

#include <iostream>

#include "common/grid.hh"     // the grid base classes
#include "yaspgrid/grids.hh"  // the yaspgrid base classes

/*! \file yaspgrid.hh
   Yaspgrid stands for yet another structured parallel grid.
   It will implement the dune grid interface for structured grids with codim 0
   and dim, with arbitrary overlap, parallel features with two overlap
   models, periodic boundaries and fast a implementation allowing on-the-fly computations.
 */

namespace Dune {

  /** @defgroup Yaspgrid Yet Another Structured Parallel Grid
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

  template<int dim, int dimworld>            class YaspElement;
  template<int codim, int dim, int dimworld> class YaspEntity;
  template<int dim, int dimworld>            class YaspGrid;
  template<int codim, int dim, int dimworld> class YaspLevelIterator;
  template<int dim, int dimworld>            class YaspIntersectionIterator;
  //template <int dim, int dimworld>           class YaspBoundaryEntity;
  //template<int dim, int dimworld>            class YaspHierarchicIterator;


  //========================================================================
  // The reference elements

  /** Singleton holding reference elements */
  template<int dim>
  struct YaspReferenceElement {
    static Vec<dim,yaspgrid_ctype> midpoint; // data neded for the refelem below
    static Vec<dim,yaspgrid_ctype> extension; // data needed for the refelem below
    static YaspElement<dim,dim> refelem;
  };

  // initialize static variable with bool constructor (which makes reference elements)
  template<int dim>
  YaspElement<dim,dim> YaspReferenceElement<dim>::refelem(YaspReferenceElement<dim>::midpoint,
                                                          YaspReferenceElement<dim>::extension,
                                                          true);
  template<int dim>
  Vec<dim,yaspgrid_ctype> YaspReferenceElement<dim>::midpoint;

  template<int dim>
  Vec<dim,yaspgrid_ctype> YaspReferenceElement<dim>::extension;

  //========================================================================
  /*!
     YaspElement realizes the concept of the geometric part of a mesh entity.

     We have specializations for dim==dimworld (elements),
     dim = dimworld-1 (faces) and dim=0 (vertices).
     The general version throws a GridError on construction.
   */
  //========================================================================

  //! The general version implements dimworld==dimworld. If this is not the case an error is thrown
  template<int dim,int dimworld>
  class YaspElement //: public ElementDefault<dim,dimworld,yaspgrid_ctype,YaspElement>
  {
  public:
    //! define type used for coordinates in grid module
    typedef yaspgrid_ctype ctype;

    //! return the element type identifier
    ElementType type ()
    {
      switch (dim)
      {
      case 1 : return line;
      case 2 : return quadrilateral;
      case 3 : return hexahedron;
      default : return unknown;
      }
    }

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners ()
    {
      return 1<<dim;
    }

    //! access to coordinates of corners. Index is the number of the corner
    Vec<dim,yaspgrid_ctype>& operator[] (int i)
    {
      int bit=0;
      for (int k=0; k<dimworld; k++)   // run over all directions in world
      {
        if (k==missing)
        {
          c(k) = midpoint(k);
          continue;
        }
        //k is not the missing direction
        if (i&(1<<bit))          // check whether bit is set or not
          c(k) = midpoint(k)+0.5*extension(k);         // bit is 1 in i
        else
          c(k) = midpoint(k)-0.5*extension(k);         // bit is 0 in i
        bit++;         // we have processed a direction
      }

      return c;
    }

    /*! return reference element corresponding to this element.
            Usually, the implementation will store the finite
            set of reference elements as global variables.
            But why in the hell do we need this reference element?
     */
    static YaspElement<dim,dim>& refelem ()
    {
      return YaspReferenceElement<dim>::refelem;
    }

    //! maps a local coordinate within reference element to global coordinate in element
    Vec<dimworld,yaspgrid_ctype> global (const Vec<dim,yaspgrid_ctype>& local)
    {
      Vec<dimworld,yaspgrid_ctype> g;
      int bit=0;
      for (int k=0; k<dimworld; k++)
        if (k==missing)
          g(k) = midpoint(k);
        else
        {
          g(k) = midpoint(k) + (local(bit)-0.5)*extension(k);
          bit++;
        }
      return g;
    }

    //! maps a global coordinate within the element to a local coordinate in its reference element
    Vec<dim,yaspgrid_ctype> local (const Vec<dimworld,yaspgrid_ctype>& global)
    {
      Vec<dim,yaspgrid_ctype> l;   // result
      int bit=0;
      for (int k=0; k<dimworld; k++)
        if (k!=missing)
        {
          l(bit) = (global(k)-midpoint(k))/extension(k) + 0.5;
          bit++;
        }
      return l;
    }

    /*! determinant of the jacobian of the mapping
     */
    yaspgrid_ctype integration_element (const Vec<dim,yaspgrid_ctype>& local)
    {
      yaspgrid_ctype volume=1.0;
      for (int k=0; k<dim; k++)
        if (k!=missing) volume *= extension(k);
      return volume;
    }

    //! check whether local is inside reference element
    bool checkInside (const Vec<dim,yaspgrid_ctype>& local)
    {
      for (int i=0; i<dim; i++)
        if (local(i)<-yasptolerance || local(i)>1+yasptolerance) return false;
      return true;
    }

    //! constructor from (storage for) midpoint and extension and missing direction number
    YaspElement (Vec<dimworld,yaspgrid_ctype>& p, Vec<dimworld,yaspgrid_ctype>& h, int& m)
      : midpoint(p), extension(h), missing(m)
    {
      if (dimworld!=dim+1)
        throw GridError("general YaspElement assumes dimworld=dim+1",__FILE__,__LINE__);
    }

    //! print function
    void print (std::ostream& s)
    {
      s << "YaspElement<"<<dim<<","<<dimworld<< "> ";
      s << "midpoint";
      for (int i=0; i<dimworld; i++)
        s << " " << midpoint[i];
      s << " extension";
      for (int i=0; i<dimworld; i++)
        s << " " << extension[i];
      s << " missing is " << missing;
    }

  private:
    // the element is fully defined by its midpoint the extension
    // in each direction and the missing direction.
    // References are used because this information
    // is known outside the element in many cases.
    // Note dimworld==dim+1
    Vec<dimworld,yaspgrid_ctype>& midpoint; // the midpoint
    Vec<dimworld,yaspgrid_ctype>& extension; // the extension
    int& missing;                           // the missing, i.e. constant direction

    // In addition we need memory in order to return references.
    // Possibly we should change this in the interface ...
    Vec<dim,yaspgrid_ctype> c;           // a point
  };



  //! specialize for dim=dimworld, i.e. a volume element
  template<int dim>
  class YaspElement<dim,dim> //: public ElementDefault<dim,dim,yaspgrid_ctype,YaspElement>
  {
  public:
    //! define type used for coordinates in grid module
    typedef yaspgrid_ctype ctype;

    //! return the element type identifier
    ElementType type ()
    {
      switch (dim)
      {
      case 1 : return line;
      case 2 : return quadrilateral;
      case 3 : return hexahedron;
      default : return unknown;
      }
    }

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners ()
    {
      return 1<<dim;
    }

    //! access to coordinates of corners. Index is the number of the corner
    Vec<dim,yaspgrid_ctype>& operator[] (int i)
    {
      for (int k=0; k<dim; k++)
        if (i&(1<<k))
          c(k) = midpoint(k)+0.5*extension(k);       // kth bit is 1 in i
        else
          c(k) = midpoint(k)-0.5*extension(k);       // kth bit is 0 in i
      return c;
    }

    /*! return reference element corresponding to this element.
            Usually, the implementation will store the finite
            set of reference elements as global variables.
            But why in the hell do we need this reference element?
     */
    static YaspElement<dim,dim>& refelem ()
    {
      return YaspReferenceElement<dim>::refelem;
    }

    //! maps a local coordinate within reference element to global coordinate in element
    Vec<dim,yaspgrid_ctype> global (const Vec<dim,yaspgrid_ctype>& local)
    {
      Vec<dim,yaspgrid_ctype> g;
      for (int k=0; k<dim; k++)
        g(k) = midpoint(k) + (local(k)-0.5)*extension(k);
      return g;
    }

    //! maps a global coordinate within the element to a local coordinate in its reference element
    Vec<dim,yaspgrid_ctype> local (const Vec<dim,yaspgrid_ctype>& global)
    {
      Vec<dim,yaspgrid_ctype> l;   // result
      for (int k=0; k<dim; k++)
        l(k) = (global(k)-midpoint(k))/extension(k) + 0.5;
      return l;
    }

    /*! determinant of the jacobian of the mapping
     */
    yaspgrid_ctype integration_element (const Vec<dim,yaspgrid_ctype>& local)
    {
      yaspgrid_ctype volume=1.0;
      for (int k=0; k<dim; k++) volume *= extension(k);
      return volume;
    }

    //! can only be called for dim=dim!
    Mat<dim,dim>& Jacobian_inverse (const Vec<dim,yaspgrid_ctype>& local)
    {
      for (int i=0; i<dim; ++i)
      {
        Jinv(i) = 0.0;                        // set column to zero
        Jinv(i,i) = 1.0/extension(i);         // set diagonal element
      }
      return Jinv;
    }

    //! check whether local is inside reference element
    bool checkInside (const Vec<dim,yaspgrid_ctype>& local)
    {
      for (int i=0; i<dim; i++)
        if (local(i)<-yasptolerance || local(i)>1+yasptolerance) return false;
      return true;
    }

    //! constructor from (storage for) midpoint and extension
    YaspElement (Vec<dim,yaspgrid_ctype>& p, Vec<dim,yaspgrid_ctype>& h)
      : midpoint(p), extension(h)
    {}

    //! constructor from (storage for) midpoint and extension initializing reference element
    YaspElement (Vec<dim,yaspgrid_ctype>& p, Vec<dim,yaspgrid_ctype>& h, bool b)
      : midpoint(p), extension(h)
    {
      for (int i=0; i<dim; i++)
      {
        midpoint[i] = 0.5;
        extension[i] = 1.0;
      }
    }

    //! print function
    void print (std::ostream& s)
    {
      s << "YaspElement<"<<dim<<","<<dim<< "> ";
      s << "midpoint";
      for (int i=0; i<dim; i++)
        s << " " << midpoint[i];
      s << " extension";
      for (int i=0; i<dim; i++)
        s << " " << extension[i];
    }

  private:
    // the element is fully defined by midpoint and the extension
    // in each direction. References are used because this information
    // is known outside the element in many cases.
    // Note dim==dimworld
    Vec<dim,yaspgrid_ctype>& midpoint; // the midpoint
    Vec<dim,yaspgrid_ctype>& extension; // the extension

    // In addition we need memory in order to return references.
    // Possibly we should change this in the interface ...
    Mat<dim,dim,yaspgrid_ctype> Jinv;     // the jacobian inverse
    Vec<dim,yaspgrid_ctype> c;         // a point
  };




  //! specialization for dim=0, this is a vertex
  template<int dimworld>
  class YaspElement<0,dimworld> //: public ElementDefault<0,dimworld,yaspgrid_ctype,YaspElement>
  {
  public:
    //! define type used for coordinates in grid module
    typedef yaspgrid_ctype ctype;

    //! return the element type identifier
    ElementType type ()
    {
      return vertex;
    }

    //! return the number of corners of this element. Corners are numbered 0...n-1
    int corners ()
    {
      return 1;
    }

    //! access to coordinates of corners. Index is the number of the corner
    Vec<dimworld,yaspgrid_ctype>& operator[] (int i)
    {
      return position;
    }

    //! constructor
    YaspElement (Vec<dimworld,yaspgrid_ctype>& p) : position(p)
    {}

    //! print function
    void print (std::ostream& s)
    {
      s << "YaspElement<"<<0<<","<<dimworld<< "> ";
      s << "position " << position;
    }

  private:
    Vec<dimworld,yaspgrid_ctype>& position; //!< where the vertex is
  };

  // operator<< for all YaspElements
  template <int dim, int dimworld>
  inline std::ostream& operator<< (std::ostream& s, YaspElement<dim,dimworld>& e)
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


  template<int codim, int dim, int dimworld>
  class YaspEntity //:  public EntityDefault <codim,dim,dimworld,yaspgrid_ctype,YaspEntity,YaspElement,
                   //				   YaspLevelIterator,YaspIntersectionIterator,YaspHierarchicIterator>
  {
  public:
    //! define type used for coordinates in grid module
    typedef yaspgrid_ctype ctype;

    //! level of this element
    int level ()
    {
      throw GridError("YaspEntity not implemented",__FILE__,__LINE__);
    }

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index ()
    {
      throw GridError("YaspEntity not implemented",__FILE__,__LINE__);
    }

    //! geometry of this entity
    YaspElement<dim-codim,dimworld>& geometry ()
    {
      throw GridError("YaspEntity not implemented",__FILE__,__LINE__);
    }
  };


  // specialization for codim=0
  template<int dim, int dimworld>
  class YaspEntity<0,dim,dimworld> //:  public EntityDefault <0,dim,dimworld,yaspgrid_ctype,YaspEntity,YaspElement,
  //	   YaspLevelIterator,YaspIntersectionIterator,YaspHierarchicIterator>
  {
  public:
    typedef typename MultiYGrid<dim,yaspgrid_ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,yaspgrid_ctype>::TransformingSubIterator TSI;

    //! define type used for coordinates in grid module
    typedef yaspgrid_ctype ctype;

    // constructor
    YaspEntity (YGLI& g, TSI& it)
      : _it(it), _g(g), _element(it.position(),it.meshsize())
    {}

    //! level of this element
    int level () {return _g.level();}

    //! index is unique and consecutive per level
    int index () {return _it.superindex();} // superindex works also for iteration over subgrids

    //! geometry of this entity
    YaspElement<dim,dimworld>& geometry () {return _element;}

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
          with codimension cc.
     */
    template<int cc>
    int count ()
    {
      if (cc==dim) return 1<<dim;
      if (cc==1) return 2*dim;
    }

    TSI& transformingsubiterator ()
    {
      return _it;
    }

    YGLI& gridlevel ()
    {
      return _g;
    }

    //! returns intersection iterator for first intersection
    YaspIntersectionIterator<dim,dimworld> ibegin ()
    {
      return YaspIntersectionIterator<dim,dimworld>(*this,false);
    }

    //! Reference to one past the last neighbor
    YaspIntersectionIterator<dim,dimworld> iend ()
    {
      return YaspIntersectionIterator<dim,dimworld>(*this,true);
    }

    //! return partition type attribute
    PartitionType partition_type ()
    {
      if (_g.cell_interior().inside(_it.coord())) return InteriorEntity;
      if (_g.cell_overlap().inside(_it.coord())) return OverlapEntity;
      return GhostEntity;
    }

  private:
    TSI& _it;                         // position in the grid level
    YGLI& _g;                         // access to grid level
    YaspElement<dim,dimworld> _element; // the element geometry
  };


  // specialization for codim=dim
  template<int dim, int dimworld>
  class YaspEntity<dim,dim,dimworld> //:  public EntityDefault <dim,dim,dimworld,yaspgrid_ctype,YaspEntity,YaspElement,
  //	   YaspLevelIterator,YaspIntersectionIterator,YaspHierarchicIterator>
  {
  public:
    typedef typename MultiYGrid<dim,yaspgrid_ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,yaspgrid_ctype>::TransformingSubIterator TSI;

    //! define type used for coordinates in grid module
    typedef yaspgrid_ctype ctype;

    // constructor
    YaspEntity (YGLI& g, TSI& it)
      : _it(it), _g(g), _element(it.position())
    {  }

    //! level of this element
    int level () {return _g.level();}

    //! index is unique and consecutive per level
    int index () {return _it.superindex();}

    //! geometry of this entity
    YaspElement<0,dimworld>& geometry () {return _element;}

    //! return partition type attribute
    PartitionType partition_type ()
    {
      if (_g.vertex_interior().inside(_it.coord())) return InteriorEntity;
      if (_g.vertex_interiorborder().inside(_it.coord())) return BorderEntity;
      if (_g.vertex_overlap().inside(_it.coord())) return OverlapEntity;
      if (_g.vertex_overlapfront().inside(_it.coord())) return FrontEntity;
      return GhostEntity;
    }

  private:
    TSI& _it;                         // position in the grid level
    YGLI& _g;                         // access to grid level
    YaspElement<0,dimworld> _element; // the element geometry
  };


  //========================================================================
  /*!
     YaspIntersectionIterator enables iteration over intersection with
     neighboring codim 0 entities.
   */
  //========================================================================

  template<int dim, int dimworld>
  class YaspIntersectionIterator //: public IntersectionIteratorDefault<dim,dimworld,simplegrid_ctype,
                                 //	YaspIntersectionIterator,YaspEntity,YaspElement,YaspBoundaryEntity>
  {
  public:
    //! define type used for coordinates in grid module
    typedef yaspgrid_ctype ctype;

    // types used from grids
    typedef typename MultiYGrid<dim,yaspgrid_ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,yaspgrid_ctype>::TransformingSubIterator TSI;

    //! prefix increment
    YaspIntersectionIterator<dim,dimworld>& operator++()
    {
      // update count, check end
      _count++;
      if (_count==2*dim) return *this;   // end iterator reached, we are done

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

      return *this;
    }

    //! equality
    bool operator== (const YaspIntersectionIterator<dim,dimworld>& i) const
    {
      return (_count==i._count);
    }

    //! inequality
    bool operator!= (const YaspIntersectionIterator<dim,dimworld>& i) const
    {
      return (_count!=i._count);
    }

    /*! return true if neighbor ist outside the domain. Still the neighbor might
        exist in case of periodic boundary conditions, i.e. true is returned
        if the neighbor is outside the periodic unit cell
     */
    bool boundary ()
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
    bool neighbor ()
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
    YaspEntity<0,dim,dimworld>& operator*()
    {
      return _nb;
    }

    //! access neighbor, arrow
    YaspEntity<0,dim,dimworld>* operator->()
    {
      return &_nb;
    }

    //! return unit outer normal, this should be dependent on local coordinates for higher order boundary
    Vec<dimworld,yaspgrid_ctype>& unit_outer_normal (Vec<dim-1,yaspgrid_ctype>& local)
    {
      return _normal;
    }

    //! return unit outer normal, if you know it is constant use this function instead
    Vec<dimworld,yaspgrid_ctype>& unit_outer_normal ()
    {
      return _normal;
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
          Here returned element is in LOCAL coordinates of the element where iteration started.
     */
    YaspElement<dim-1,dim>& intersection_self_local ()
    {
      return _is_self_local;
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
          Here returned element is in GLOBAL coordinates of the element where iteration started.
     */
    YaspElement<dim-1,dimworld>& intersection_self_global ()
    {
      return _is_global;
    }

    //! local number of codim 1 entity in self where intersection is contained in
    int number_in_self ()
    {
      return _count;
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
          Here returned element is in LOCAL coordinates of neighbor
     */
    YaspElement<dim-1,dim>& intersection_neighbor_local ()
    {
      return _is_nb_local;
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
          Here returned element is in LOCAL coordinates of neighbor
     */
    YaspElement<dim-1,dimworld>& intersection_neighbor_global ()
    {
      return _is_global;
    }

    //! local number of codim 1 entity in neighbor where intersection is contained in
    int number_in_neighbor ()
    {
      return _count + 1-2*_face;
    }

    //! make intersection iterator from entity
    YaspIntersectionIterator (YaspEntity<0,dim,dimworld>& myself, bool toend)
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
    TSI _itnb;                             //!< position of nb in the grid level
    YaspEntity<0,dim,dimworld>& _myself;  //!< reference to myself
    YaspEntity<0,dim,dimworld> _nb;       //!< virtual neighbor entity, built on the fly
    Vec<dim,yaspgrid_ctype> _pos_self_local; //!< center of face in own local coordinates
    Vec<dim,yaspgrid_ctype> _pos_nb_local; //!< center of face in neighbors local coordinates
    Vec<dim,yaspgrid_ctype> _pos_world;   //!< center of face in world coordinates
    Vec<dim,yaspgrid_ctype> _ext_local;   //!< extension of face in local coordinates
    YaspElement<dim-1,dim> _is_self_local; //!< intersection in own local coordinates
    YaspElement<dim-1,dim> _is_nb_local;  //!< intersection in neighbors local coordinates
    YaspElement<dim-1,dimworld> _is_global; //!< intersection in global coordinates
    Vec<dimworld,yaspgrid_ctype> _normal; //!< for returning outer normal
  };



  //========================================================================
  /*!
     YaspLevelIterator enables iteration over entities of one grid level

     We have specializations for codim==0 (elements) and
     codim=dim (vertices).
     The general version throws a GridError.
   */
  //========================================================================

  // the general version
  template<int codim, int dim, int dimworld>
  class YaspLevelIterator //: public LevelIteratorDefault<codim,dim,dimworld,yaspgrid_ctype,YaspLevelIterator,YaspEntity>
  {
  public:
    YaspLevelIterator ()
    {
      throw GridError("YaspLevelIterator not implemented",__FILE__,__LINE__);
    }
  };

  // specialization for codim==0 -- the elements
  template<int dim, int dimworld>
  class YaspLevelIterator<0,dim,dimworld> //: public LevelIteratorDefault<0,dim,dimworld,yaspgrid_ctype,YaspLevelIterator,YaspEntity>
  {
  public:
    typedef typename MultiYGrid<dim,yaspgrid_ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,yaspgrid_ctype>::TransformingSubIterator TSI;

    //! constructor
    YaspLevelIterator (YGLI g, TSI it) : _g(g), _it(it), _entity(_g,_it)
    {  }

    //! prefix increment
    YaspLevelIterator<0,dim,dimworld>& operator++()
    {
      ++_it;
      return *this;
    }

    //! equality
    bool operator== (const YaspLevelIterator<0,dim,dimworld>& i) const
    {
      return _it==i._it;
    }

    //! inequality
    bool operator!= (const YaspLevelIterator<0,dim,dimworld>& i) const
    {
      return (_it!=i._it);
    }

    //! dereferencing
    YaspEntity<0,dim,dimworld>& operator*()
    {
      return _entity;
    }

    //! arrow
    YaspEntity<0,dim,dimworld>* operator->()
    {
      return &_entity;
    }

    //! ask for level of entity
    int level () {return _g.level();}

  private:
    YGLI _g;                          // access to grid level
    TSI _it;                           // position in the grid level
    YaspEntity<0,dim,dimworld> _entity; //!< virtual entity
  };


  // specialization for codim==dim -- the vertices
  template<int dim, int dimworld>
  class YaspLevelIterator<dim,dim,dimworld> //: public LevelIteratorDefault<dim,dim,dimworld,yaspgrid_ctype,YaspLevelIterator,YaspEntity>
  {
  public:
    typedef typename MultiYGrid<dim,yaspgrid_ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,yaspgrid_ctype>::TransformingSubIterator TSI;

    //! constructor
    YaspLevelIterator (YGLI g, TSI it) : _g(g), _it(it), _entity(_g,_it)
    {  }

    //! prefix increment
    YaspLevelIterator<dim,dim,dimworld>& operator++()
    {
      ++_it;
      return *this;
    }

    //! equality
    bool operator== (const YaspLevelIterator<dim,dim,dimworld>& i) const
    {
      return _it==i._it;
    }

    //! inequality
    bool operator!= (const YaspLevelIterator<dim,dim,dimworld>& i) const
    {
      return (_it!=i._it);
    }

    //! dereferencing
    YaspEntity<dim,dim,dimworld>& operator*()
    {
      return _entity;
    }

    //! arrow
    YaspEntity<dim,dim,dimworld>* operator->()
    {
      return &_entity;
    }

    //! ask for level of entity
    int level () {return _g.level();}

  private:
    YGLI _g;                          // access to grid level
    TSI _it;                           // position in the grid level
    YaspEntity<dim,dim,dimworld> _entity; //!< virtual entity
  };


  //========================================================================
  /*!
     YaspInteriorBorderLevelIterator enables iteration over entities up to border of one grid level

     We have specializations for codim==0 (elements) and
     codim=dim (vertices).
     The general version throws a GridError.
   */
  //========================================================================


  // the general version
  template<int codim, int dim, int dimworld>
  class YaspInteriorBorderLevelIterator
  //: public LevelIteratorDefault<codim,dim,dimworld,yaspgrid_ctype,YaspLevelIterator,YaspEntity>
  {
  public:
    YaspInteriorBorderLevelIterator ()
    {
      throw GridError("YaspLevelIterator not implemented",__FILE__,__LINE__);
    }
  };

  //! codim=0 specialization
  template<int dim, int dimworld>
  class YaspInteriorBorderLevelIterator<0,dim,dimworld> : public YaspLevelIterator<0,dim,dimworld>
                                                          //: public LevelIteratorDefault<codim,dim,dimworld,yaspgrid_ctype,YaspLevelIterator,YaspEntity>
  {
  public:
    typedef typename MultiYGrid<dim,yaspgrid_ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,yaspgrid_ctype>::TransformingSubIterator TSI;

    YaspInteriorBorderLevelIterator (YGLI g, TSI it) : YaspLevelIterator<0,dim,dimworld>::YaspLevelIterator(g,it)
    {  }
  };

  //! codim=dim specialization
  template<int dim, int dimworld>
  class YaspInteriorBorderLevelIterator<dim,dim,dimworld> : public YaspLevelIterator<dim,dim,dimworld>
                                                            //: public LevelIteratorDefault<codim,dim,dimworld,yaspgrid_ctype,YaspLevelIterator,YaspEntity>
  {
  public:
    typedef typename MultiYGrid<dim,yaspgrid_ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,yaspgrid_ctype>::TransformingSubIterator TSI;

    YaspInteriorBorderLevelIterator (YGLI g, TSI it) : YaspLevelIterator<dim,dim,dimworld>::YaspLevelIterator(g,it)
    {  }
  };


  //========================================================================
  /*!
     YaspOverlapLevelIterator enables iteration over entities up to overlap of one grid level

     We have specializations for codim==0 (elements) and
     codim=dim (vertices).
     The general version throws a GridError.
   */
  //========================================================================


  // the general version
  template<int codim, int dim, int dimworld>
  class YaspOverlapLevelIterator
  //: public LevelIteratorDefault<codim,dim,dimworld,yaspgrid_ctype,YaspLevelIterator,YaspEntity>
  {
  public:
    YaspOverlapLevelIterator ()
    {
      throw GridError("YaspLevelIterator not implemented",__FILE__,__LINE__);
    }
  };

  //! codim=0 specialization
  template<int dim, int dimworld>
  class YaspOverlapLevelIterator<0,dim,dimworld> : public YaspLevelIterator<0,dim,dimworld>
                                                   //: public LevelIteratorDefault<codim,dim,dimworld,yaspgrid_ctype,YaspLevelIterator,YaspEntity>
  {
  public:
    typedef typename MultiYGrid<dim,yaspgrid_ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,yaspgrid_ctype>::TransformingSubIterator TSI;

    YaspOverlapLevelIterator (YGLI g, TSI it) : YaspLevelIterator<0,dim,dimworld>::YaspLevelIterator(g,it)
    {  }
  };

  //! codim=dim specialization
  template<int dim, int dimworld>
  class YaspOverlapLevelIterator<dim,dim,dimworld> : public YaspLevelIterator<dim,dim,dimworld>
                                                     //: public LevelIteratorDefault<codim,dim,dimworld,yaspgrid_ctype,YaspLevelIterator,YaspEntity>
  {
  public:
    typedef typename MultiYGrid<dim,yaspgrid_ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,yaspgrid_ctype>::TransformingSubIterator TSI;

    YaspOverlapLevelIterator (YGLI g, TSI it) : YaspLevelIterator<dim,dim,dimworld>::YaspLevelIterator(g,it)
    {  }
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
  class YaspGrid //: public GridDefault<dim,dimworld,yaspgrid_ctype,YaspGrid,YaspLevelIterator,YaspEntity>
  {
  public:
    //! maximum number of levels allowed
    enum { MAXL=64 };

    //! define type used for coordinates in grid module
    typedef yaspgrid_ctype ctype;

    //! shorthand for base class data types
    typedef MultiYGrid<dim,ctype> YMG;
    typedef typename MultiYGrid<dim,ctype>::YGridLevelIterator YGLI;
    typedef typename SubYGrid<dim,yaspgrid_ctype>::TransformingSubIterator TSI;

    //! return GridIdentifierType of Grid, i.e. SGrid_Id or AlbertGrid_Id ...
    GridIdentifier type() const { return YaspGrid_Id; };

    /*! Constructor for a YaspGrid, they are all forwarded to the base class
          @param comm MPI communicator where this mesh is distributed to
          @param L extension of the domain
          @param s number of cells on coarse mesh in each direction
       @param periodic tells if direction is periodic or not
       @param size of overlap on coarsest grid (same in all directions)
     */
    YaspGrid (MPI_Comm comm, Dune::Vec<dim,ctype> L, Dune::Vec<dim,int> s, Dune::Vec<dim,bool> periodic, int overlap)
      : _mg(comm,L,s,periodic,overlap)
    {  }

    /*! Return maximum level defined in this grid. Levels are numbered
          0 ... maxlevel with 0 the coarsest level.
     */
    int maxlevel() const {return _mg.maxlevel();} // delegate

    //! refine the grid refCount times. What about overlap?
    void globalRefine (int refCount)
    {
      bool b=false;
      if (refCount>0) b=true;
      _mg.refine(b);
    }

    //! Iterator to first entity of given codim on level
    template<int cd>
    YaspLevelIterator<cd,dim,dimworld> lbegin (int level)
    {
      YGLI g = _mg.begin(level);
      if (cd==0)   // the elements
      {
        return YaspLevelIterator<cd,dim,dimworld>(g,g.cell_overlap().tsubbegin());
      }
      if (cd==dim)   // the vertices
      {
        return YaspLevelIterator<cd,dim,dimworld>(g,g.vertex_overlapfront().tsubbegin());
      }
      throw GridError("YaspLevelIterator with this codim not implemented",__FILE__,__LINE__);
    }

    //! one past the end on this level
    template<int cd>
    YaspLevelIterator<cd,dim,dimworld> lend (int level)
    {
      YGLI g = _mg.begin(level);
      if (cd==0)   // the elements
      {
        return YaspLevelIterator<cd,dim,dimworld>(g,g.cell_overlap().tsubend());
      }
      if (cd==dim)   // the vertices
      {
        return YaspLevelIterator<cd,dim,dimworld>(g,g.vertex_overlapfront().tsubend());
      }
      throw GridError("YaspLevelIterator with this codim not implemented",__FILE__,__LINE__);
    }

    //! Iterator to first entity of given codim on level
    template<int cd>
    YaspInteriorBorderLevelIterator<cd,dim,dimworld> interiorborder_lbegin (int level)
    {
      YGLI g = _mg.begin(level);
      if (cd==0)   // the elements
      {
        return YaspInteriorBorderLevelIterator<cd,dim,dimworld>(g,g.cell_interior().tsubbegin());
      }
      if (cd==dim)   // the vertices
      {
        return YaspInteriorBorderLevelIterator<cd,dim,dimworld>(g,g.vertex_interiorborder().tsubbegin());
      }
      throw GridError("YaspInteriorBorderLevelIterator with this codim not implemented",__FILE__,__LINE__);
    }

    //! Iterator to one past the last entity of given codim on level
    template<int cd>
    YaspInteriorBorderLevelIterator<cd,dim,dimworld> interiorborder_lend (int level)
    {
      YGLI g = _mg.begin(level);
      if (cd==0)   // the elements
      {
        return YaspInteriorBorderLevelIterator<cd,dim,dimworld>(g,g.cell_interior().tsubend());
      }
      if (cd==dim)   // the vertices
      {
        return YaspInteriorBorderLevelIterator<cd,dim,dimworld>(g,g.vertex_interiorborder().tsubend());
      }
      throw GridError("YaspInteriorBorderLevelIterator with this codim not implemented",__FILE__,__LINE__);
    }

    //! Iterator to first entity of given codim on level
    template<int cd>
    YaspOverlapLevelIterator<cd,dim,dimworld> overlap_lbegin (int level)
    {
      YGLI g = _mg.begin(level);
      if (cd==0)   // the elements
      {
        return YaspOverlapLevelIterator<cd,dim,dimworld>(g,g.cell_overlap().tsubbegin());
      }
      if (cd==dim)   // the vertices
      {
        return YaspOverlapLevelIterator<cd,dim,dimworld>(g,g.vertex_overlap().tsubbegin());
      }
      throw GridError("YaspOverlapLevelIterator with this codim not implemented",__FILE__,__LINE__);
    }

    //! Iterator to one past the last entity of given codim on level
    template<int cd>
    YaspOverlapLevelIterator<cd,dim,dimworld> overlap_lend (int level)
    {
      YGLI g = _mg.begin(level);
      if (cd==0)   // the elements
      {
        return YaspOverlapLevelIterator<cd,dim,dimworld>(g,g.cell_overlap().tsubend());
      }
      if (cd==dim)   // the vertices
      {
        return YaspOverlapLevelIterator<cd,dim,dimworld>(g,g.vertex_overlap().tsubend());
      }
      throw GridError("YaspOverlapLevelIterator with this codim not implemented",__FILE__,__LINE__);
    }

    //! number of grid entities per level and codim
    int size (int level, int codim)
    {
      YGLI g = _mg.begin(level);
      if (codim==0)
      {
        return g.cell_overlap().totalsize();
      }
      if (codim==dim)
      {
        return g.vertex_overlapfront().totalsize();
      }
      throw GridError("Yasp does not implement this codim (yet)",__FILE__,__LINE__);
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
      // access to grid level
      YGLI g = _mg.begin(level);

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
        throw GridError("interface communication not implemented",__FILE__,__LINE__);
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
        _mg.torus().send(is->rank,buf,is->grid.totalsize()*sizeof(P<T>));
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
        _mg.torus().recv(is->rank,buf,is->grid.totalsize()*sizeof(P<T>));
      }

      // exchange all buffers now
      _mg.torus().exchange();

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

  private:
    YMG _mg;
  };



  /** @} end documentation group */

} // end namespace


#endif
