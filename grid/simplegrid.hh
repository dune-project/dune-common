// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __SIMPLEGRID_HH__
#define __SIMPLEGRID_HH__

#include "../common/matvec.hh"
#include "../common/Stack.hh"
#include "common/grid.hh"
#include <iostream>
#include <assert.h>

/*! \file simplegrid.hh
   This is the implementation of the grid interface for a structured grid
   supporting only elements (codim=0) and vertices (codim=dim).
   Most functions are defined through template specialization to get
   a maximum of performance.
 */

namespace Dune {

  /** @defgroup SimpleGrid SimpleGrid
      @ingroup GridCommon

          This module describes the pilot implementation of the Dune grid interface.
          It implements the grid interface for simple structured meshes.

          @{
   */


  //************************************************************************
  /*! define name for floating point type used for coordinates in sgrid.
     You can change the type for coordinates by changing this single typedef.
   */
  typedef double simplegrid_ctype;
  typedef simplegrid_ctype sgrid_ctype;

  //************************************************************************
  // forward declaration of templates

  template<int dim, int dimworld>            class SimpleElement;
  template<int codim, int dim, int dimworld> class SimpleEntity;
  template<int dim, int dimworld>            class SimpleGrid;
  template<int codim, int dim, int dimworld> class SimpleLevelIterator;
  template<int dim, int dimworld>            class SimpleIntersectionIterator;
  template<int dim, int dimworld>            class SimpleHierarchicIterator;

  /** \todo Please doc me! */
  template<int dim>
  struct levelinfo {

    /** \brief Level of this info struct */
    int level;

    /** \brief Number of elements per level and direction */
    int ne[dim];

    /** \brief Number of vertices per level and direction */
    int nv[dim];

    /** \brief Mesh size for each direction */
    simplegrid_ctype h[dim];

    /** \brief Offset of element neighbor in lexicographic ordering */
    int ne_offset[dim];

    /** \brief Offset of vertex neighbor in lexicographic ordering */
    int nv_offset[dim];

    /** \brief Total number of elements on this level */
    int nelements;

    /** \brief Total number of vertices on this level */
    int nvertices;

    /** \brief Integration element codim 0 */
    simplegrid_ctype volume;

    /** \brief integration element codim 1 */
    simplegrid_ctype ie_codim_1[2*dim];

    /** \todo Please doc me! */
    simplegrid_ctype h_face[2*dim][dim-1];

    /** \todo Please doc me! */
    short dir_face[2*dim][dim-1];

    /** \todo Please doc me! */
    simplegrid_ctype facevol[dim];
  };

  //=======================================================================================================
  // The reference elements

  /** Singleton holding reference elements */
  template<int dim>
  struct SimpleReferenceElement {
    static levelinfo<dim> li;
    static SimpleElement<dim,dim> refelem;
  };

  // initialize static variable with bool constructor (which makes reference elements)
  template<int dim>
  SimpleElement<dim,dim> SimpleReferenceElement<dim>::refelem(SimpleReferenceElement<dim>::li);

  template<int dim>
  levelinfo<dim> SimpleReferenceElement<dim>::li;

  //************************************************************************
  /*!
     SimpleElement realizes the concept of the geometric part of a mesh entity.

     We have specializations for dim==dimworld (elements), dim==0 (vertices).
     The general version is assumed to be a codimension 1 object dim=dimworld-1.
     Its geometry can then be computed from the codimension 0 object and an integer face specification.
   */
  template<int dim, int dimworld>
  class SimpleElement : public ElementDefault<dim,dimworld,simplegrid_ctype,SimpleElement>
  {
  public:
    //! define type used for coordinates in grid module
    typedef simplegrid_ctype ctype;

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
    Vec<dimworld,simplegrid_ctype>& operator[] (int i)
    {
      int lk=0;

      for (int k=0; k<dimworld; k++)
      {
        corn(k) = e.s[k];
        if (k!=c)
        {
          if (i&(1<<lk)) corn(k) += e.li->h[k];
          lk++;
        }
      }
      corn(c) += d*e.li->h[c];
      return corn;
    }

    /*! return reference element corresponding to this element. If this is
          a reference element then self is returned. A reference to a reference
          element is returned. Usually, the implementation will store the finite
          set of reference elements as global variables.
     */
    static SimpleElement<dim,dim>& refelem ()
    {
      return SimpleReferenceElement<dim>::refelem;
    }

    //! maps a local coordinate within reference element to global coordinate in element
    Vec<dimworld,simplegrid_ctype> global (const Vec<dim,simplegrid_ctype>& local)
    {
      Vec<dimworld,simplegrid_ctype> g;
      int lk=0;
      for (int k=0; k<dimworld; k++)
      {
        g(k) = e.s[k];
        if (k!=c)
        {
          g(k) += local.read(lk)*e.li->h[k];
          lk++;
        }
      }
      g(c) += d*e.li->h[c];
      return g;
    }

    //! maps a global coordinate within the element to a local coordinate in its reference element
    Vec<dim,simplegrid_ctype> local (const Vec<dimworld,simplegrid_ctype>& global)
    {
      Vec<dim,simplegrid_ctype> l;   // result
      Vec<dimworld,simplegrid_ctype> rhs;
      int k;
      for (k=0; k<dimworld; k++)
        if (k!=c) rhs(k) = global(k)-e.s[k];
      rhs(c) -= d*e.li->h[k];

      int lk=0;
      for (int k=0; k<dimworld; k++)
        if (k!=c)
        {
          l(lk) = (rhs(k)) / (e.li->h[k]);
          lk++;
        }
      return l;
    }

    bool checkInside (const Vec<dim,simplegrid_ctype>& local)
    {
      // check wether they are in the reference element
      for(int i=0; i<dim; i++)
      {
        if((local.read(i) < 0.0) || (local.read(i) > 1.0 ))
          return false;
      }
      return true;
    }


    /*! Integration over a general element is done by integrating over the reference element
     */
    simplegrid_ctype integration_element (const Vec<dim,simplegrid_ctype>& local)
    {
      return e.li->facevol[c];
    }

    //! constructor without arguments makes uninitialized element
    SimpleElement (SimpleElement<dim+1,dimworld>& _e) : e(_e)
    {  }

    void set_face (int _c, int _d)
    {
      c = _c;
      d = _d;
    }

    //! print function
    void print (std::ostream& ss)
    {
      Vec<dim,simplegrid_ctype> local = 0.5;
      ss << "SimpleElement<"<<dim<<","<<dimworld<< "> ";
      ss << "position " << s << " mesh size " << h << " directions " << dir << " vol " << integration_element(local);
    }

  private:
    SimpleElement<dim+1,dimworld>& e; // my element
    int c;                          // direction
    int d;                          // face number in direction d=0,1
    Vec<dimworld,simplegrid_ctype> corn; //!< coordinate vector of corner
  };



  //! specialize for dim=dimworld
  template<int dim>
  class SimpleElement<dim,dim> : public ElementDefault<dim,dim,simplegrid_ctype,SimpleElement>
  {
  public:
    friend class SimpleElement<dim-1,dim>; // access granted for faces

    //! define type used for coordinates in grid module
    typedef simplegrid_ctype ctype;

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
    Vec<dim,simplegrid_ctype>& operator[] (int i)
    {
      for (int k=0; k<dim; k++)
      {
        c(i) = s[i];
        if (i&(1<<k)) c(i) += li->h[i];
        return c;
      }
    }

    /*! return reference element corresponding to this element. If this is
          a reference element then self is returned. A reference to a reference
          element is returned. Usually, the implementation will store the finite
          set of reference elements as global variables.
     */
    static SimpleElement<dim,dim>& refelem ()
    {
      return SimpleReferenceElement<dim>::refelem;
    }

    //! maps a local coordinate within reference element to global coordinate in element
    Vec<dim,simplegrid_ctype> global (const Vec<dim,simplegrid_ctype>& local)
    {
      Vec<dim,simplegrid_ctype> g;
      for (int k=0; k<dim; k++) g(k) = s[k] + local.read(k)*li->h[k];
      return g;
    }

    //! maps a global coordinate within the element to a local coordinate in its reference element
    Vec<dim,simplegrid_ctype> local (const Vec<dim,simplegrid_ctype>& global)
    {
      Vec<dim,simplegrid_ctype> l;   // result
      for (int k=0; k<dim; k++) l(k) = (global(k)-s[k])/li->h[k];
      return l;
    }

    /*! Integration over a general element is done by integrating over the reference element
     */
    simplegrid_ctype integration_element (const Vec<dim,simplegrid_ctype>& local)
    {
      return li->volume;
    }

    //! can only be called for dim=dim!
    Mat<dim,dim>& Jacobian_inverse (const Vec<dim,simplegrid_ctype>& local)
    {
      for (int i=0; i<dim; ++i)
      {
        Jinv(i) = 0.0;
        Jinv(i,i) = 1.0/li->h[i];
      }
      return Jinv;
    }

    //! constructor without arguments makes uninitialized element
    SimpleElement (levelinfo<dim>& _li, int _id)
    {
      li = &_li;
      id = _id;

      // compute coordinates from id
      int n=id;


      for (int i=dim-1; i>=1; --i)
      {
        coord[i] = n/li->ne_offset[i];
        n = n%li->ne_offset[i];
      }
      coord[0] = n;

      // compute position from id
      for (int i=0; i<dim; ++i) s[i] = coord[i]*li->h[i];
    }

    //! constructor making reference element
    SimpleElement (levelinfo<dim>& _li)
    {
      li = &_li;
      for (int i=0; i<dim; ++i)
      {
        s[i] = 0.0;
        li->h[i] = 1.0;
      }
      li->volume = 1.0;
    }

    //! constructor making uninitialized element
    SimpleElement ()
    {}

    //! print function
    void print (std::ostream& ss)
    {
      Vec<dim,simplegrid_ctype> local = 0.5;
      ss << "SimpleElement<"<<dim<<","<<dim<< "> ";
      ss << "position ";
      for (int i=0; i<dim; i++) ss << s[i] << " ";
      ss << " mesh size ";


      for (int i=0; i<dim; i++) ss << li->h[i] << " ";
      ss << " vol " << integration_element(local);
    }

    //! prefix increment
    SimpleElement<dim,dim>& operator++()
    {
      // increment id;
      ++id;

      // increment coordinate
      for (int i=0; i<dim; i++)
        if (++(coord[i])<li->ne[i])
        {
          s[i] += li->h[i];
          return *this;
        }
        else { coord[i]=0; s[i] = 0.0; }
      return *this;
    }

    void walk (int c, int d)
    {
      id += d*li->ne_offset[c];
      coord[c] += d;
      s[c] += d*li->h[c];
    }

    //! equality
    bool operator== (const SimpleElement<dim,dim>& i) const
    {
      return (id==i.id);
    }

    //! inequality
    bool operator!= (const SimpleElement<dim,dim>& i) const
    {
      return (id!=i.id);
    }

    int level ()
    {
      return li->level;
    }

    int index ()
    {
      return id;
    }

    bool boundary (int c)
    {
      return coord[c]<0 || coord[c]>=li->ne[c] ;
    }

    //   SimpleElement<dim,dim>& operator= (const SimpleElement<dim,dim>& x)
    //   {
    //      id = x.id;
    //      for (int i=0; i<dim; ++i) {coord[i]=x.coord[i]; s[i]=x.s[i];}
    //      li = x.li;
    //      return *this;
    //   }

  private:
    int id;                    // my id
    int coord[dim];            // my integer position
    simplegrid_ctype s[dim];   // my position
    levelinfo<dim>* li;        // access to information common to all elements on a level
    Mat<dim,dim,sgrid_ctype> Jinv; //!< storage for inverse of jacobian

    Vec<dim,simplegrid_ctype> c; // needed for returning references
  };


  //! specialization for dim=0, this is a vertex
  template<int dimworld>
  class SimpleElement<0,dimworld> : public ElementDefault<0,dimworld,simplegrid_ctype,SimpleElement>
  {
  public:
    //! define type used for coordinates in grid module
    typedef simplegrid_ctype ctype;

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
    Vec<dimworld,simplegrid_ctype>& operator[] (int i)
    {
      return s;
    }

    //! constructor
    SimpleElement (levelinfo<dimworld>& _li, int _id) : li(_li)
    {
      id = _id;

      // compute coordinates from id
      int n=id;
      for (int i=dimworld-1; i>=1; --i)
      {
        coord[i] = n/li.nv_offset[i];
        n = n%li.nv_offset[i];
      }
      coord[0] = n;

      // compute position from id
      for (int i=0; i<dimworld; ++i) s(i) = coord[i]*li.h[i];
    }

    //! prefix increment
    SimpleElement<0,dimworld>& operator++()
    {
      // increment id;
      ++id;

      // increment coordinate
      for (int i=0; i<dimworld; i++)
        if (++(coord[i])<li.nv[i])
        {
          s(i) += li.h[i];
          return *this;
        }
        else { coord[i]=0; s(i) = 0.0; }
      return *this;
    }

    //! equality
    bool operator== (const SimpleElement<0,dimworld>& i) const
    {
      return (id==i.id);
    }

    //! inequality
    bool operator!= (const SimpleElement<0,dimworld>& i) const
    {
      return (id!=i.id);
    }

    int level ()
    {
      return li.level;
    }

    int index ()
    {
      return id;
    }

    //! print function
    void print (std::ostream& ss)
    {
      ss << "SimpleElement<"<<0<<","<<dimworld<< "> ";
      ss << "position " << s ;
    }

  private:
    int id;                         //!< my id
    int coord[dimworld];            //!< my integer position
    Vec<dimworld,simplegrid_ctype> s; //!< pos
    levelinfo<dimworld>& li;        //!< common information for all vertices
  };

  template <int dim, int dimworld>
  inline std::ostream& operator<< (std::ostream& s, SimpleElement<dim,dimworld>& e)
  {
    e.print(s);
    return s;
  }


  //***********************************************************************
  //
  //  SimpleBoundaryEntity
  //
  //***********************************************************************
  template <int dim, int dimworld>
  class SimpleBoundaryEntity
    : public BoundaryEntityDefault <dim,dimworld,simplegrid_ctype,SimpleElement,SimpleBoundaryEntity>
  {
  public:
    //! make default Entity
    SimpleBoundaryEntity() {};

    //! return type of boundary segment
    BoundaryType type () { return Neumann; };

    //! return true if ghost cell was calced
    bool hasGeometry () { return false; };

    //! return outer ghost cell
    SimpleElement<dim,dimworld> & geometry()
    {};

    //! return outer barycenter of ghost cell
    Vec<dimworld,simplegrid_ctype> & outerPoint () {};
  private:
  };


  //************************************************************************
  /*!
     IntersectionIterator
   */

  template<int dim, int dimworld>
  class SimpleIntersectionIterator : public IntersectionIteratorDefault<dim,dimworld,simplegrid_ctype,
                                         SimpleIntersectionIterator,SimpleEntity,SimpleElement,SimpleBoundaryEntity>
  {
  public:
    //! define type used for coordinates in grid module
    typedef simplegrid_ctype ctype;

    //! prefix increment
    SimpleIntersectionIterator<dim,dimworld>& operator++()
    {
      // we have a valid count
      int c = count/2;
      int d = 2*(count%2)-1;
      count++;

      if (d==-1)
      {
        nb_geo.walk(c,2);
        is_self_local.set_face(c,1);
        is_nb_local.set_face(c,0);
        is_global.set_face(c,0);
        normal(c) = 1;
        return *this;
      }
      else
      {
        nb_geo.walk(c,-1);
        normal(c) = 0;
        c = count/2;
        if (count>=2*dim) return *this;
        normal(c) = -1;
        nb_geo.walk(c,-1);
        is_self_local.set_face(c,0);
        is_nb_local.set_face(c,1);
        is_global.set_face(c,1);
        return *this;
      }
    }

    //! equality
    bool operator== (const SimpleIntersectionIterator<dim,dimworld>& i) const
    {
      return (count==i.count);
    }

    //! inequality
    bool operator!= (const SimpleIntersectionIterator<dim,dimworld>& i) const
    {
      return (count!=i.count);
    }

    //! return true if intersection is with boundary. \todo connection with boundary information, processor/outer boundary
    bool boundary ()
    {
      return nb_geo.boundary(count/2) ;
    }

    //! return true if neihgbor across intersection exists. \todo connection with boundary information, processor/outer boundary
    bool neighbor ()
    {
      return !(nb_geo.boundary(count/2)) ;
    }

    SimpleBoundaryEntity<dim,dimworld>& boundaryEntity ()
    {
      //return bndEntity;
    };

    //! access neighbor, dereferencing
    SimpleEntity<0,dim,dimworld>& operator*()
    {
      return nb;
    }

    //! access neighbor, arrow
    SimpleEntity<0,dim,dimworld>* operator->()
    {
      return &nb;
    }

    //! return unit outer normal, this should be dependent on local coordinates for higher order boundary
    Vec<dimworld,sgrid_ctype>& unit_outer_normal (Vec<dim-1,sgrid_ctype>& local)
    {
      return normal;
    }

    //! return unit outer normal, if you know it is constant use this function instead
    Vec<dimworld,sgrid_ctype>& unit_outer_normal ()
    {
      return normal;
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of the element where iteration started.
     */
    SimpleElement<dim-1,dim>& intersection_self_local ()
    {
      return is_self_local;
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in GLOBAL coordinates of the element where iteration started.
     */
    SimpleElement<dim-1,dimworld>& intersection_self_global ()
    {
      return is_global;
    }

    //! local number of codim 1 entity in self where intersection is contained in
    int number_in_self ()
    {
      return count;
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of neighbor
     */
    SimpleElement<dim-1,dim>& intersection_neighbor_local ()
    {
      return is_nb_local;
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of neighbor
     */
    SimpleElement<dim-1,dimworld>& intersection_neighbor_global ()
    {
      return is_global;
    }

    //! local number of codim 1 entity in neighbor where intersection is contained in
    int number_in_neighbor ()
    {
      return (count/2)*2 + (1-count%2);
    }

    //! constructor
    SimpleIntersectionIterator (SimpleElement<dim,dimworld>& _self, int _count) :
      nb_geo(_self),    // Kopie erstellen
      nb(nb_geo),          // entity kennt geometrie
      is_self_local(SimpleReferenceElement<dim>::refelem),
      is_nb_local(SimpleReferenceElement<dim>::refelem),
      is_global(nb_geo),
      normal(0.0)
    {
      count = _count;
      int c = _count/2;
      int d = _count%2;
      if (c>=dim) return;
      is_self_local.set_face(c,d);
      is_nb_local.set_face(c,1-d);
      is_global.set_face(c,1-d);
      nb_geo.walk(c,2*d-1);
      normal(c) = 2*d-1;
    }

    SimpleIntersectionIterator (const SimpleIntersectionIterator<dim,dimworld>& i) : nb(nb_geo), is_global(nb_geo)
    { }

    SimpleIntersectionIterator () :
      nb(nb_geo), is_global(nb_geo),
      is_self_local(SimpleReferenceElement<dim>::refelem),
      is_nb_local(SimpleReferenceElement<dim>::refelem)
    { }

    void make (SimpleElement<dim,dimworld>& _self, int _count)
    {
      count = _count;
      int c = _count/2;
      int d = _count%2;
      if (c>=dim) return;
      nb_geo = _self;
      is_self_local.set_face(c,d);
      is_nb_local.set_face(c,1-d);
      is_global.set_face(c,1-d);
      nb_geo.walk(c,2*d-1);
      normal(c) = 2*d-1;
    }

  private:
    //SimpleBoundaryEntity<dim,dimworld> bndEntity;
    int count;                             //!< neighbor count
    SimpleElement<dim,dimworld> nb_geo;    //!< intersection in own local coordinates
    SimpleEntity<0,dim,dimworld> nb;       //!< virtual neighbor entity, built on the fly
    SimpleElement<dim-1,dim> is_self_local; //!< intersection in own local coordinates
    SimpleElement<dim-1,dim> is_nb_local;  //!< intersection in neighbors local coordinates
    SimpleElement<dim-1,dimworld> is_global; //!< intersection in global coordinates
    Vec<dimworld,sgrid_ctype> normal;
  };

  template<int dim, int dimworld>
  class SimpleHierarchicIterator
  {
  public:
  };

  //************************************************************************
  /*!
     SimpleEntity : only specialization with codim=0 and codim=dim can be used
     We do not support these entities here.
   */

  template<int codim, int dim, int dimworld>
  class SimpleEntity : public EntityDefault<codim,dim,dimworld,simplegrid_ctype,SimpleEntity,SimpleElement,
                           SimpleLevelIterator,SimpleIntersectionIterator,SimpleHierarchicIterator>
  {
    SimpleEntity () {}
  };

  // codimension 0 -- the elements
  template<int dim, int dimworld>
  class SimpleEntity<0,dim,dimworld> : public EntityDefault<0,dim,dimworld,simplegrid_ctype,SimpleEntity,SimpleElement,
                                           SimpleLevelIterator,SimpleIntersectionIterator,SimpleHierarchicIterator>

  {
  public:
    friend class SimpleIntersectionIterator<dim,dimworld>;

    //! level of this element
    int level ()
    {
      return geo.level();
    }

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index ()
    {
      return geo.index();
    }

    //! geometry of this entity
    SimpleElement<dim,dimworld>& geometry ()
    {
      return geo;
    }

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
          with codimension cc.
     */
    template<int cc> int count () {return -1;}
    template<> int count<1>   () {return 2*dim;}
    template<> int count<dim> () {return 1<<dim;}

    /*! Provide access to mesh entity i of given codimension. Entities
          are numbered 0 ... count<cc>()-1
     */
    template<int cc>
    SimpleLevelIterator<cc,dim,dimworld> entity (int i)
    {
      assert(cc==dim);     // support only vertices
      return SimpleLevelIterator<cc,dim,dimworld>();
    }
    template<>
    SimpleLevelIterator<dim,dim,dimworld> entity<dim> (int i)
    {
      // THIS FUNCTION IS NOT IMPLEMENTED CORRECTLY
      //      int n=0;
      //      for (int k=0; k<dim; ++k)
      //        {
      //              n += coord[k]*li->nv_offset[k];
      //              if (i&(1<<k))
      //                n += li->nv_offset[k];
      //        }
      //      return SimpleLevelIterator<dim,dim,dimworld>(li,n);
    }

    /*! Intra-level access to neighboring elements. A neighbor is an entity of codimension 0
          which has an entity of codimension 1 in commen with this entity. Access to neighbors
          is provided using iterators. This allows meshes to be nonmatching. Returns iterator
          referencing the first neighbor.
     */
    SimpleIntersectionIterator<dim,dimworld> ibegin ()
    {
      return SimpleIntersectionIterator<dim,dimworld>(geo,0);
    }
    void ibegin (SimpleIntersectionIterator<dim,dimworld>& i)
    {
      i.make(geo,0);
    }

    //! Reference to one past the last neighbor
    SimpleIntersectionIterator<dim,dimworld> iend ()
    {
      return SimpleIntersectionIterator<dim,dimworld>(geo,2*dim);
    }
    void iend (SimpleIntersectionIterator<dim,dimworld>& i)
    {
      i.make(geo,2*dim);
    }

    //! Inter-level access to father element on coarser grid. Assumes that meshes are nested.
    SimpleLevelIterator<0,dim,dimworld> father ()
    {
      SimpleLevelIterator<0,dim,dimworld>();
    }

    /*! Location of this element relative to the reference element element of the father.
          This is sufficient to interpolate all dofs in conforming case.
          Nonconforming may require access to neighbors of father and
          computations with local coordinates.
          On the fly case is somewhat inefficient since dofs  are visited several times.
          If we store interpolation matrices, this is tolerable. We assume that on-the-fly
          implementation of numerical algorithms is only done for simple discretizations.
          Assumes that meshes are nested.
     */
    SimpleElement<dim,dim>& father_relative_local ()
    {
      SimpleElement<dim,dim>();
    }

    /*! Inter-level access to son elements on higher levels<=maxlevel.
          This is provided for sparsely stored nested unstructured meshes.
          Returns iterator to first son.
     */
    SimpleHierarchicIterator<dim,dimworld> hbegin (int maxlevel)
    {
      return SimpleHierarchicIterator<dim,dimworld>();
    }

    //! Returns iterator to one past the last son
    SimpleHierarchicIterator<dim,dimworld> hend (int maxlevel)
    {
      return SimpleHierarchicIterator<dim,dimworld>();
    }

    // constructor
    SimpleEntity (SimpleElement<dim,dimworld>& _geo) : geo(_geo)
    {  }

  private:
    SimpleElement<dim,dimworld>& geo; // reference to geometry which is updated outside of here
  };


  // codimension dim -- the vertices
  template<int dim, int dimworld>
  class SimpleEntity<dim,dim,dimworld> : public EntityDefault<dim,dim,dimworld,simplegrid_ctype,SimpleEntity,SimpleElement,
                                             SimpleLevelIterator,SimpleIntersectionIterator,SimpleHierarchicIterator>
  {
  public:
    // disambiguate member functions with the same name in both bases
    //! level of this element
    int level () {return geo.level();}

    //! index is unique and consecutive per level and codim used for access to degrees of freedom
    int index () {return geo.index();}

    //! geometry of this entity
    SimpleElement<0,dimworld>& geometry () {return geo;}

    /*! Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
          This can speed up on-the-fly interpolation for linear conforming elements
          Possibly this is sufficient for all applications we want on-the-fly.
     */
    SimpleLevelIterator<0,dim,dimworld> father ()
    {
      return SimpleLevelIterator<0,dim,dimworld>();   // not implemented yet
    }

    //! local coordinates within father
    Vec<dim,simplegrid_ctype>& local () {return loc;} // not implemented yet

    // members specific to SimpleEntity
    SimpleEntity (SimpleElement<0,dimworld>& _geo) : geo(_geo)
    {  }

  private:
    SimpleElement<0,dimworld>& geo; // reference to geometry which is updated outside of here
    Vec<dim,simplegrid_ctype> loc; // needed for returning a reference
  };

  //************************************************************************
  /*! Enables iteration over all entities of a given codimension and level of a grid.
     General version is dummy, only specializations for codim=0 and codim=dim can be used
   */
  template<int codim, int dim, int dimworld>
  class SimpleLevelIterator : public LevelIteratorDefault<codim,dim,dimworld,simplegrid_ctype,SimpleLevelIterator,SimpleEntity>
  {
    SimpleLevelIterator () {}
  };

  // specialization for codim==0 -- the elements
  template<int dim, int dimworld>
  class SimpleLevelIterator<0,dim,dimworld> : public LevelIteratorDefault<0,dim,dimworld,simplegrid_ctype,SimpleLevelIterator,SimpleEntity>
  {
  public:
    //! prefix increment
    SimpleLevelIterator<0,dim,dimworld>& operator++()
    {
      ++elem;
      return *this;
    }

    //! equality
    bool operator== (const SimpleLevelIterator<0,dim,dimworld>& i) const
    {
      return elem==i.elem;
    }

    //! inequality
    bool operator!= (const SimpleLevelIterator<0,dim,dimworld>& i) const
    {
      return (elem!=i.elem);
    }

    //! dereferencing
    SimpleEntity<0,dim,dimworld>& operator*()
    {
      return enty;
    }

    //! arrow
    SimpleEntity<0,dim,dimworld>* operator->()
    {
      return &enty;
    }

    //! ask for level of entity
    int level () {return elem.level();}

    //! constructor
    SimpleLevelIterator (levelinfo<dim>& li, int id) : elem(li,id),enty(elem)
    {  }

    //! copy constructor
    SimpleLevelIterator (const SimpleLevelIterator<0,dim,dimworld>& i) : elem(i.elem),enty(elem)
    {  }

  private:
    SimpleElement<dim,dimworld> elem; //!< my entity
    SimpleEntity<0,dim,dimworld> enty; //!< virtual entity
  };


  // specialization for codim==dim -- the vertices
  template<int dim, int dimworld>
  class SimpleLevelIterator<dim,dim,dimworld> : public LevelIteratorDefault<dim,dim,dimworld,simplegrid_ctype,SimpleLevelIterator,SimpleEntity>
  {
  public:
    //! prefix increment
    SimpleLevelIterator<dim,dim,dimworld>& operator++()
    {
      ++elem;   // go to next
      return *this;
    }

    //! equality
    bool operator== (const SimpleLevelIterator<dim,dim,dimworld>& i) const
    {
      return elem==i.elem;
    }

    //! inequality
    bool operator!= (const SimpleLevelIterator<dim,dim,dimworld>& i) const
    {
      return elem!=i.elem;
    }

    //! dereferencing
    SimpleEntity<dim,dim,dimworld>& operator*()
    {
      return enty;
    }

    //! arrow
    SimpleEntity<dim,dim,dimworld>* operator->()
    {
      return &enty;
    }

    //! ask for level of entity
    int level () {return elem.level();}

    //! constructor
    SimpleLevelIterator (levelinfo<dim>& li, int id) : elem(li,id),enty(elem)
    {  }

    //! copy constructor
    SimpleLevelIterator (const SimpleLevelIterator<dim,dim,dimworld>& i) : elem(i.elem),enty(elem)
    {  }

  private:
    SimpleElement<0,dimworld> elem;    //!< my entity
    SimpleEntity<dim,dim,dimworld> enty; //!< virtual entity
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
  class SimpleGrid : public GridDefault<dim,dimworld,simplegrid_ctype,SimpleGrid,SimpleLevelIterator,SimpleEntity>
  {
  public:
    //! maximum number of levels allowed
    enum { MAXL=64 };

    //! define type used for coordinates in grid module
    typedef simplegrid_ctype ctype;

    //! return GridIdentifierType of Grid, i.e. SGrid_Id or AlbertGrid_Id ...
    GridIdentifier type() const { return SimpleGrid_id; };

    /*! Return maximum level defined in this grid. Levels are numbered
       0 ... maxlevel with 0 the coarsest level.
     */
    int maxlevel() const {return L-1;}

    //! Iterator to first entity of given codim on level
    template<int cd>
    SimpleLevelIterator<cd,dim,dimworld> lbegin (int level)
    {
      assert(cd==0 || cd==dim)
      return SimpleLevelIterator<cd,dim,dimworld>(li[level],-1);
    }
    template<>
    SimpleLevelIterator<0,dim,dimworld> lbegin (int level)
    {
      return SimpleLevelIterator<0,dim,dimworld>(li[level],0);
    }
    template<>
    SimpleLevelIterator<dim,dim,dimworld> lbegin (int level)
    {
      return SimpleLevelIterator<dim,dim,dimworld>(li+level,0);
    }

    //! one past the end on this level
    template<int cd>
    SimpleLevelIterator<cd,dim,dimworld> lend (int level)
    {
      assert(cd==0 || cd==dim)
      return SimpleLevelIterator<cd,dim,dimworld>(li[level],-1);
    }
    template<>
    SimpleLevelIterator<0,dim,dimworld> lend (int level)
    {
      return SimpleLevelIterator<0,dim,dimworld>(li[level],li[level].nelements);
    }
    template<>
    SimpleLevelIterator<dim,dim,dimworld> lend (int level)
    {
      return SimpleLevelIterator<dim,dim,dimworld>(li[level],li[level].nvertices);
    }

    //! number of grid entities per level and codim
    int size (int level, int codim) const
    {
      if (codim==0)
      {
        return li[level].nelements;
      }
      if (codim==dim)
      {
        return li[level].nvertices;
      }
      std::cout << "No Entitys of Codim these codim " << codim << " \n";
      return -1;
    }

    // these are all members specific to sgrid

    /*! constructor, subject to change!
       @param H_ size of domain
       @param N_ coarse grid size, #elements in one direction
       \todo The following parameter doesn't actually exist!
       @param L_ number of levels 0,...,L_-1, maxlevel = L_-1
     */
    SimpleGrid (int* N_, double* H_)
    {
      L=1;
      li[0].level = 0;
      for (int i=0; i<dim; ++i)
      {
        li[0].ne[i] = N_[i];
        li[0].nv[i] = N_[i]+1;
        li[0].h[i] = H_[i]/N_[i];
      }
      li[0].ne_offset[0] = 1;
      li[0].nv_offset[0] = 1;
      for (int i=1; i<dim; ++i)
      {
        li[0].ne_offset[i] = li[0].ne_offset[i-1]*li[0].ne[i-1];
        li[0].nv_offset[i] = li[0].nv_offset[i-1]*li[0].nv[i-1];
      }
      li[0].volume = 1.0;
      for (int i=0; i<dim; ++i) li[0].volume*=li[0].h[i];
      for (int c=0; c<2*dim; c++)
      {
        li[0].ie_codim_1[c] = 1.0;
        for (int i=0; i<dim; ++i)
          if (i!=c/2) li[0].ie_codim_1[c] *= li[0].h[i];
      }
      for (int c=0; c<2*dim; c++)
      {
        int n=0;
        for (int i=0; i<dim; ++i)
          if (i!=c/2)
          {
            li[0].h_face[c][n] = li[0].h[i];
            li[0].dir_face[c][n] = i;
            n++;
          }
      }
      for (int i=0; i<dim; ++i) li[0].facevol[i]=1.0;
      for (int i=0; i<dim; ++i)
        for (int j=0; j<dim; ++j)
          if (j!=i) li[0].facevol[i] *= li[0].h[j];

      std::cout << "level=" << L-1 << " size=(" << li[L-1].ne[0];
      for (int i=1; i<dim; i++) std::cout << "," <<  li[L-1].ne[i];
      std::cout << ")" << std::endl;

      // calc number of elemnts
      li[L-1].nelements = 1;
      for (int i=0; i<dim; i++)
        li[L-1].nelements *= li[L-1].ne[i];

      // calc number of elemnts
      li[L-1].nvertices = 1;
      for (int i=0; i<dim; i++)
        li[L-1].nvertices *= (li[L-1].ne[i]+1);
    }

    /// \todo Doc me!!
    void globalRefine (int refCount)
    {
      // refine grid once
      if (L>=MAXL) return;
      L++;
      li[L-1].level = L-1;
      for (int i=0; i<dim; ++i)
      {
        li[L-1].ne[i] = 2*li[L-2].ne[i];
        li[L-1].nv[i] = li[L-1].ne[i]+1;
        li[L-1].h[i] = 0.5*li[L-2].h[i];
      }
      li[L-1].ne_offset[0] = 1;
      li[L-1].nv_offset[0] = 1;
      for (int i=1; i<dim; ++i)
      {
        li[L-1].ne_offset[i] = li[L-1].ne_offset[i-1]*li[L-1].ne[i-1];
        li[L-1].nv_offset[i] = li[L-1].nv_offset[i-1]*li[L-1].nv[i-1];
      }
      li[L-1].nelements = li[L-1].ne_offset[dim-1]*li[L-1].ne[dim-1];
      li[L-1].nvertices = li[L-1].nv_offset[dim-1]*li[L-1].nv[dim-1];
      li[L-1].volume = 1.0;
      for (int i=0; i<dim; ++i) li[L-1].volume*=li[L-1].h[i];
      for (int c=0; c<2*dim; c++)
      {
        li[L-1].ie_codim_1[c] = 1.0;
        for (int i=0; i<dim; ++i)
          if (i!=c/2) li[L-1].ie_codim_1[c] *= li[L-1].h[i];
      }
      for (int c=0; c<2*dim; c++)
      {
        int n=0;
        for (int i=0; i<dim; ++i)
          if (i!=c/2)
          {
            li[L-1].h_face[c][n] = li[L-1].h[i];
            li[L-1].dir_face[c][n] = i;
            n++;
          }
      }
      for (int i=0; i<dim; ++i) li[L-1].facevol[i]=1.0;
      for (int i=0; i<dim; ++i)
        for (int j=0; j<dim; ++j)
          if (j!=i) li[L-1].facevol[i] *= li[L-1].h[j];

      std::cout << "level=" << L-1 << " size=(" << li[L-1].ne[0];
      for (int i=1; i<dim; i++) std::cout << "," <<  li[L-1].ne[i];
      std::cout << ")" << std::endl;

      // calc number of elemnts
      li[L-1].nelements = 1;
      for (int i=0; i<dim; i++)
        li[L-1].nelements *= li[L-1].ne[i];

      // calc number of vertices
      li[L-1].nvertices = 1;
      for (int i=0; i<dim; i++)
        li[L-1].nvertices *= (li[L-1].ne[i]+1);
    }

    /** \brief Returns information about a given grid level
     *
     * \todo Make sure the level exists!
     */
    levelinfo<dim>* get_levelinfo (int l) {return &li[l];}

    /** \brief Returns information about a given grid level
     *
     * \todo Make sure the level exists!
     */
    const levelinfo<dim>* get_levelinfo (int l) const {return &li[l];}

    //! write Grid to file filename and store time
    template <FileFormatType ftype>
    bool writeGrid ( const char * filename , sgrid_ctype time)
    {
      return true;
    };

    //! read Grid from file filename and store time of grid in time
    template <FileFormatType ftype>
    bool readGrid ( const char * filename , sgrid_ctype &time )
    {
      return true;
    };

  private:
    int L;                            // number of levels in hierarchic mesh 0<=level<L
    levelinfo<dim> li[MAXL];          // array of info structures
  };


  /** @} end documentation group */

} // end namespace


#endif
