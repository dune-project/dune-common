// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTGRID_HH
#define DUNE_ALBERTGRID_HH

#include "../common/matvec.hh"
#include "../common/misc.hh"
//#include "../common/Stack.hh"
#include "common/grid.hh"


#ifdef __ALBERTNAME__

#define ALBERT Albert::
namespace Albert
{

#else
#define ALBERT
#endif

#define DIM MYDIM
#define DIM_OF_WORLD MYDOW

#ifndef __ALBERTNAME__
  extern "C"
  {
#endif

#include <albert.h>
#include "albertgrid/albertextra.hh"

#ifndef __ALBERTNAME__
} // end extern "C"
#endif

  typedef struct albert_leaf_data
  {
    /// Achtung, Fehler in Albert memory.c,
    /// kleinste Groesse der Leaf Daten 4 Byte,
    /// also ist hier ok,
    S_CHAR reachedFace[4];

  } AlbertLeafData;

  const FE_SPACE *feSpace;

  void initialReached(const EL_INFO *elInfo);
  void setReached(const EL_INFO *elInfo);
  void AlbertLeafRefine(EL *parent, EL *child[2]);
  void AlbertLeafCoarsen(EL *parent, EL *child[2]);
  void initLeafData(LEAF_DATA_INFO *linfo);
  void initDofAdmin(MESH *mesh);

  const BOUNDARY *initBoundary(MESH *Spmesh, int bound);

#ifdef __ALBERTNAME__
} //end namespace Albert
#endif


  namespace Dune
  {


    typedef ALBERT REAL albertCtype;

    // Class to mark the Vertices on the leaf level
    // to visit every vertex only once
    class AlbertMarkerVector
    {
      int *vec_;
      int numberOfEntries_;
    public:
      AlbertMarkerVector ();
      ~AlbertMarkerVector ();

      void makeNewSize(int newNumberOfEntries);
      void makeSmaller(int newNumberOfEntries);
      bool notOnThisElement(ALBERT EL_INFO * elInfo, int vertex);
      void markNewVertices(ALBERT MESH * mesh);
      void print();
    private:
      void checkMark(ALBERT EL_INFO * elInfo, int vertex);

    };


    template<int dim, int dimworld> class AlbertGridElement;
    template<int codim, int dim, int dimworld> class AlbertGridEntity;
    template<int codim, int dim, int dimworld> class AlbertGridLevelIterator;
    template<int dim, int dimworld> class AlbertGridHierarchicIterator;
    template<int dim, int dimworld> class AlbertGridNeighborIterator;
    template<int dim, int dimworld> class AlbertGrid;


    //**********************************************************************
    //
    // --AlbertGridElement
    /*!
       Defines the geometry part of a mesh entity. Works for all dimensions, element types and dime
       of world. Provides reference element and mapping between local and global coordinates.
       The element may have different implementations because the mapping can be
       done more efficient for structured meshes than for unstructured meshes.

       dim: An element is a polygonal in a hyperplane of dimension dim. 0 <= dim <= 3 is typically
       dim=0 is a point.

       dimworld: Each corner is a point with dimworld coordinates.
     */

    template<int dim, int dimworld>
    class AlbertGridElement :
      public Element<dim,dimworld,albertCtype,AlbertGridElement>
    {
    public:
      //! know dimension
      enum { dimension=dim };

      //! know dimension of world
      enum { dimensionworld=dimworld };

      AlbertGridElement();

      AlbertGridElement(ALBERT EL_INFO *elInfo,
                        unsigned char face=0,
                        unsigned char edge=0,
                        unsigned char vertex=0);

      //! return the element type identifier
      //! line , triangle or tetrahedron, depends on dim
      ElementType type ();

      //! return the number of corners of this element. Corners are numbered 0...n-1
      int corners ();

      //! access to coordinates of corners. Index is the number of the corner
      Vec<dimworld,albertCtype>& operator[] (int i);

      /*! return reference element corresponding to this element. If this is
         a reference element then self is returned.
       */
      static AlbertGridElement<dim,dim>& refelem ();

      //! maps a local coordinate within reference element to
      //! global coordinate in element
      Vec<dimworld,albertCtype>& global (Vec<dim,albertCtype> local);

      //! maps a global coordinate within the element to a
      //! local coordinate in its reference element
      Vec<dim,albertCtype>& local (Vec<dimworld,albertCtype> global);

      //! returns true if the point is in the current element
      bool pointIsInside(const Vec<dimworld,albertCtype> &point);

      /*!
         Copy from Peter Bastian:

         Integration over a general element is done by integrating over the reference element
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
      albertCtype integration_element (const Vec<dim,albertCtype>& local);

      //! can only be called for dim=dimworld!
      Mat<dim,dim>& Jacobian_inverse (const Vec<dim,albertCtype>& local);

      //! print internal data
      void print (std::ostream& ss, int indent);

      //! return unit outer normal of this element, work for Faces nad Edges
      Vec<dimworld,albertCtype>& unit_outer_normal ();

      //! return outer normal of this element, work for Faces nad Edges
      Vec<dimworld,albertCtype>& outer_normal ();

      //***********************************************************************
      //  Methods that not belong to the Interface, but have to be public
      //***********************************************************************
      void builtGeom(ALBERT EL_INFO *elInfo, unsigned char face,
                     unsigned char edge, unsigned char vertex);
      void initGeom();

    private:
      //! maps a barycentric coordinate within element to global coordinate in element
      Vec<dimworld,albertCtype> globalBary (Vec<dim+1,albertCtype> local);

      //! maps a global coordinate within the elements local barycentric
      //! koordinates
      Vec<dim+1,albertCtype> localBary (Vec<dimworld,albertCtype> global);

      // template method for map the vertices of EL_INFO to the actual
      // coords with face_,edge_ and vertex_ , needes for operator []
      template <int cc>
      int mapVertices (int i) { return i; };

      template <> int mapVertices< 1 > (int i)
      { return ((face_+1+i)%N_VERTICES); };

      template <> int mapVertices< 2 << dimworld > (int i)
      { return ((face_+1)+ (edge_+1) +i)%N_VERTICES; };
      template <> int mapVertices< 3 > (int i)
      { return ((face_+1)+ (edge_+1) +(vertex_+1) +i)%N_VERTICES; };

      Mat<dimworld,dim+1,albertCtype> coord_;

      Vec<dimworld,albertCtype> globalCoord_;
      Vec<dim,albertCtype> localCoord_;

      ALBERT EL_INFO * makeEmptyElInfo();

      ALBERT EL_INFO * elInfo_;

      //! Which Face of the Element 0...dim+1
      unsigned char face_;

      //! Which Edge of the Face of the Element 0...dim
      unsigned char edge_;

      //! Which Edge of the Face of the Element 0...dim-1
      unsigned char vertex_;

      Mat<dim,dim,albertCtype> Jinv_; //!< storage for inverse of jacobian
      bool builtinverse;

      Vec<dimworld,albertCtype> outerNormal_;
    };


    //**********************************************************************
    //
    // --AlbertGridEntity
    //
    /*!
       A Grid is a container of grid entities. An entity is parametrized by the codimension.
       An entity of codimension c in dimension d is a d-c dimensional object.

       Here: the general template
     */
    template<int codim, int dim, int dimworld>
    class AlbertGridEntity :
      public Entity<codim,dim,dimworld,albertCtype,
          AlbertGridEntity,AlbertGridElement,AlbertGridLevelIterator,
          AlbertGridNeighborIterator,AlbertGridHierarchicIterator>
    {
    public:
      //! know your own codimension
      enum { codimension=codim };

      //! know your own dimension
      enum { dimension=dim };

      //! know your own dimension of world
      enum { dimensionworld=dimworld };

      //! level of this element
      int level ();

      //! index is unique and consecutive per level and codim used for access to degrees of freedo
      int index ();

      AlbertGridEntity(ALBERT TRAVERSE_STACK * travStack);

      AlbertGridEntity();

      //! geometry of this entity
      AlbertGridElement<dim-codim,dimworld>& geometry ();

      /*! Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
         This can speed up on-the-fly interpolation for linear conforming elements
         Possibly this is sufficient for all applications we want on-the-fly.
       */
      AlbertGridLevelIterator<0,dim,dimworld> father ();

      //! local coordinates within father
      Vec<dim,albertCtype>& local ();


      //***********************************************************************
      //  Methods that not belong to the Interface, but have to be public
      //***********************************************************************
      void setTraverseStack (ALBERT TRAVERSE_STACK *travStack);
      void setElInfo (ALBERT EL_INFO *elInfo, unsigned char face,
                      unsigned char edge, unsigned char vertex );
      ALBERT EL_INFO *getElInfo () const;

    private:
      // private Methods
      void makeDescription();

      template <int cc>
      int indexMap () { return elInfo_->el->index; };
      template <>
      int indexMap<1> () { return (face_*elInfo_->mesh->n_elements)+elInfo_->el->index; };
      template <>
      int indexMap<dim> () { return elInfo_->el->dof[vertex_][0]; };

      // private Members
      ALBERT EL_INFO *elInfo_;
      ALBERT TRAVERSE_STACK * travStack_;

      //! the cuurent geometry
      AlbertGridElement<dim-codim,dimworld> geo_;
      bool builtgeometry_;     //!< true if geometry has been constructed


      Vec<dim,albertCtype> localFatherCoords_;

      //! Which Face of the Element
      unsigned char face_;

      //! Which Edge of the Face of the Element
      unsigned char edge_;

      //! Which Edge of the Face of the Element
      unsigned char vertex_;
    };

    /*!
       A Grid is a container of grid entities. An entity is parametrized by the codimension.
       An entity of codimension c in dimension d is a d-c dimensional object.

       Entities of codimension 0 ("elements") are defined through template specialization. Note
       that this specialization has an extended interface compared to the general case

       Entities of codimension 0  allow to visit all neighbors, where
       a neighbor is an entity of codimension 0 which has a common entity of codimension 1 with the
       These neighbors are accessed via an iterator. This allows the implementation of
       non-matching meshes. The number of neigbors may be different from the number of faces/edges
       of an element!
     */
    //***********************
    //
    //  --AlbertGridEntity
    //
    //***********************
    template<int dim, int dimworld>
    class AlbertGridEntity<0,dim,dimworld> :
      public Entity<0,dim,dimworld,albertCtype,AlbertGridEntity,AlbertGridElement,
          AlbertGridLevelIterator,AlbertGridNeighborIterator,
          AlbertGridHierarchicIterator>
    {
    public:
      typedef AlbertGridNeighborIterator<dim,dimworld> NeighborIterator;


      //! know your own codimension
      enum { codimension=0 };

      //! know your own dimension
      enum { dimension=dim };

      //! know your own dimension of world
      enum { dimensionworld=dimworld };

      AlbertGridEntity();

      AlbertGridEntity(ALBERT TRAVERSE_STACK * travStack);

      //! level of this element
      int level ();

      //! index is unique and consecutive per level and codim used for access to degrees of freedo
      int index ();


      //! geometry of this entity
      AlbertGridElement<dim,dimworld>& geometry ();


      /*! Intra-element access to entities of codimension cc > codim. Return number of entities
         with codimension cc.
       */
      template<int cc> int count   () { return dim+1; }; //!< Default codim 1 Faces
      template<> int count<2<<dim> () { return (dim*2); }; //!< Edges Codim = 2, only 3d
      template<> int count<dim>    () { return dim+1; }; //!< Vertices codim = dim

      /*! Provide access to mesh entity i of given codimension. Entities
         are numbered 0 ... count<cc>()-1
       */
      template<int cc> AlbertGridLevelIterator<cc,dim,dimworld> entity (int i) // 0 <= i < count()
      { // Default == Codim 1 Faces
        AlbertGridLevelIterator<cc,dim,dimworld> tmp(elInfo_,i,0,0);
        return tmp;
      };

      template <> AlbertGridLevelIterator<2 << dim,dim,dimworld> entity<2 << dim> (int i)
      {
        if(i < 3)
        { // 0,1,2
          AlbertGridLevelIterator<2,3,3> tmp(elInfo_,0,i,0);
          return tmp;
        }
        else
        { // 3,4,5
          AlbertGridLevelIterator<2,3,3> tmp(elInfo_,i-2,1,0);
          return tmp;
        }
      };

      // spezialization for dim = dimworld = 3 ind albertgrid.cc
      template<> AlbertGridLevelIterator<dim,dim,dimworld> entity<dim> (int i)
      {
        AlbertGridLevelIterator<dim,dim,dimworld> tmp(elInfo_,0,0,i);
        return tmp;
      };

      /*! Intra-level access to neighboring elements. A neighbor is an entity of codimension 0
         which has an entity of codimension 1 in commen with this entity. Access to neighbors
         is provided using iterators. This allows meshes to be nonmatching. Returns iterator
         referencing the first neighbor.
       */
      AlbertGridNeighborIterator<dim,dimworld> nbegin ();

      //! Reference to one past the last neighbor
      AlbertGridNeighborIterator<dim,dimworld> nend ();

      //! Inter-level access to father element on coarser grid.
      //! Assumes that meshes are nested.
      AlbertGridLevelIterator<0,dim,dimworld> father ();

      /*! Location of this element relative to the reference element element of the father.
         This is sufficient to interpolate all dofs in conforming case.
         Nonconforming may require access to neighbors of father and
         computations with local coordinates.
         On the fly case is somewhat inefficient since dofs  are visited several times.
         If we store interpolation matrices, this is tolerable. We assume that on-the-fly
         implementation of numerical algorithms is only done for simple discretizations.
         Assumes that meshes are nested.
       */
      AlbertGridElement<dim,dim>& father_relative_local ();

      /*! Inter-level access to son elements on higher levels<=maxlevel.
         This is provided for sparsely stored nested unstructured meshes.
         Returns iterator to first son.
       */
      AlbertGridHierarchicIterator < dim,dimworld > hbegin (int maxlevel);

      //! Returns iterator to one past the last son
      AlbertGridHierarchicIterator < dim,dimworld > hend (int maxlevel);

      //************************************************************
      //  Methoden zum Anbinden von Albert
      //************************************************************
      // face, edge and vertex only for codim > 0, in this
      // case jutst to supply the same interface
      void setTraverseStack (ALBERT TRAVERSE_STACK *travStack);
      void setElInfo (ALBERT EL_INFO *elInfo,
                      unsigned char face = 0,
                      unsigned char edge = 0,
                      unsigned char vertex = 0 );
      ALBERT EL_INFO *getElInfo () const;

    private:
      //! make a new AlbertGridEntity
      void makeDescription();

      //! the cuurent geometry
      AlbertGridElement<dim,dimworld> geo_;
      bool builtgeometry_;           //!< true if geometry has been constructed

      //! pointer to the real Albert element data
      ALBERT EL_INFO *elInfo_;

      //! pointer to the Albert TRAVERSE_STACK data
      ALBERT TRAVERSE_STACK * travStack_;

      AlbertGridElement <dim,dim> fatherReLocal_;

    }; // end of AlbertGridEntity codim = 0




    //**********************************************************************
    //
    // --AlbertGridHierarchicIterator
    /*!
       Mesh entities of codimension 0 ("elements") allow to visit all entities of
       codimension 0 obtained through nested, hierarchic refinement of the entity.
       Iteration over this set of entities is provided by the HIerarchicIterator,
       starting from a given entity.
       This is redundant but important for memory efficient implementations of unstru
       hierarchically refined meshes.
     */

    template<int dim, int dimworld>
    class AlbertGridHierarchicIterator :
      public HierarchicIterator<dim,dimworld,albertCtype,
          AlbertGridHierarchicIterator,AlbertGridEntity>
    {
    public:
      //! know your own dimension
      enum { dimension=dim };

      //! know your own dimension of world
      enum { dimensionworld=dimworld };

      // the normal Constructor
      AlbertGridHierarchicIterator(ALBERT TRAVERSE_STACK travStack, int travLevel);

      // the default Constructor
      AlbertGridHierarchicIterator();

      // the Copy Constructor
      AlbertGridHierarchicIterator(const AlbertGridHierarchicIterator& I);

      //! prefix increment
      AlbertGridHierarchicIterator& operator ++();

      //! postfix increment
      AlbertGridHierarchicIterator& operator ++(int i);

      //! equality
      bool operator== (const AlbertGridHierarchicIterator& i) const;

      //! inequality
      bool operator!= (const AlbertGridHierarchicIterator& i) const;

      //! dereferencing
      AlbertGridEntity<0,dim,dimworld>& operator*();
      //  AlbertGrid<dim,dimworld>::Entity<0> & operator*();

      //! arrow
      AlbertGridEntity<0,dim,dimworld>* operator->();

    private:
      //! implement with virtual element
      AlbertGridEntity<0,dim,dimworld> virtualEntity_;


      //! The nessesary things for Albert
      ALBERT EL_INFO * recursiveTraverse(ALBERT TRAVERSE_STACK * stack);
      void makeIterator();

      //! we need this for Albert traversal, and we need Copy of TRAVERSE_STACK
      ALBERT TRAVERSE_STACK travStack_;

    };



    //**********************************************************************
    //
    // --AlbertGridNeighborIterator
    /*!
       Mesh entities of codimension 0 ("elements") allow to visit all neighbors, wh
       a neighbor is an entity of codimension 0 which has a common entity of codimens
       These neighbors are accessed via a NeighborIterator. This allows the implement
       non-matching meshes. The number of neigbors may be different from the number o
       of an element!
     */
    template<int dim, int dimworld>
    class AlbertGridNeighborIterator :
      public NeighborIterator<dim,dimworld,albertCtype,
          AlbertGridNeighborIterator,AlbertGridEntity,
          AlbertGridElement>
    {
    public:
      //! know your own dimension
      enum { dimension=dim };

      //! know your own dimension of world
      enum { dimensionworld=dimworld };

      //! prefix increment
      AlbertGridNeighborIterator& operator ++();

      //! postfix increment
      AlbertGridNeighborIterator& operator ++(int i);

      //! The default Constructor
      AlbertGridNeighborIterator();

      //! The Constructor
      AlbertGridNeighborIterator(ALBERT TRAVERSE_STACK * travStack,
                                 ALBERT EL_INFO *elInfo);

      //! Copy Constructor
      AlbertGridNeighborIterator(const AlbertGridNeighborIterator& I);


      //! equality
      bool operator== (const AlbertGridNeighborIterator& i) const;

      //! inequality
      bool operator!= (const AlbertGridNeighborIterator& i) const;

      //! access neighbor, dereferencing
      AlbertGridEntity<0,dim,dimworld>& operator*();

      //! access neighbor, arrow
      AlbertGridEntity<0,dim,dimworld>* operator->();

      //! return true if intersection is with boundary. \todo connection with
      //! boundary information, processor/outer boundary
      bool boundary ();

      //! return unit outer normal, this should be dependent on local
      //! coordinates for higher order boundary
      Vec<dimworld,albertCtype>& unit_outer_normal (Vec<dim-1,albertCtype>& local);

      //! return unit outer normal, if you know it is constant use this function instead
      Vec<dimworld,albertCtype>& unit_outer_normal ();

      //! return outer normal, this should be dependent on local
      //! coordinates for higher order boundary
      Vec<dimworld,albertCtype>& outer_normal (Vec<dim-1,albertCtype>& local);

      //! return unit outer normal, if you know it is constant use this function instead
      Vec<dimworld,albertCtype>& outer_normal ();

      //! intersection of codimension 1 of this neighbor with element where
      //! iteration started.
      //! Here returned element is in LOCAL coordinates of the element
      //! where iteration started.
      AlbertGridElement<dim-1,dim>& intersection_self_local ();

      //! intersection of codimension 1 of this neighbor with element where iteration started.
      //! Here returned element is in GLOBAL coordinates of the element where iteration started.
      AlbertGridElement<dim-1,dimworld>& intersection_self_global ();

      //! local number of codim 1 entity in self where intersection is contained in
      int number_in_self ();

      //! intersection of codimension 1 of this neighbor with element where iteration started.
      //! Here returned element is in LOCAL coordinates of neighbor
      AlbertGridElement<dim-1,dim>& intersection_neighbor_local ();

      //! intersection of codimension 1 of this neighbor with element where iteration started.
      //! Here returned element is in LOCAL coordinates of neighbor
      AlbertGridElement<dim-1,dimworld>& intersection_neighbor_global ();

      //! local number of codim 1 entity in neighbor where intersection is contained
      int number_in_neighbor ();

    private:
      void makeIterator();
      void initElInfo(ALBERT EL_INFO * elInfo);
      void setNeighInfo(ALBERT EL_INFO * elInfo, int neigh);

      //! implement with virtual element
      AlbertGridEntity<0,dim,dimworld> virtualEntity_;

      Vec<dimworld,albertCtype> outerNormal_;

      AlbertGridElement<dim-1,dim> fakeNeigh_;
      AlbertGridElement<dim-1,dimworld> neighGlob_;

      ALBERT EL_INFO * elInfo_;

      ALBERT EL_INFO neighElInfo_;
      ALBERT EL boundEl_;

      ALBERT TRAVERSE_STACK * travStack_;

      int neighborCount_;
    };



    //**********************************************************************
    //
    // --AlbertGridLevelIterator
    /*!
       Enables iteration over all entities of a given codimension and level of a grid.
     */
    template<int codim, int dim, int dimworld>
    class AlbertGridLevelIterator :
      public LevelIterator<codim,dim,dimworld,albertCtype,
          AlbertGridLevelIterator,AlbertGridEntity>
    {
    public:
      //! know your own codimension
      enum { codimension=dim };

      //! know your own dimension
      enum { dimension=dim };

      //! know your own dimension of world
      enum { dimensionworld=dimworld };

      //! Constructor
      AlbertGridLevelIterator(ALBERT EL_INFO *elInfo,int face=0, int edge=0,int vertex=0);

      //! Constructor
      AlbertGridLevelIterator(ALBERT MESH * mesh, AlbertMarkerVector * vec,
                              int travLevel);

      //! Constructor
      AlbertGridLevelIterator();

      //! prefix increment
      AlbertGridLevelIterator<codim,dim,dimworld>& operator ++();

      //! postfix increment
      AlbertGridLevelIterator<codim,dim,dimworld>& operator ++(int i);

      //! equality
      bool operator== (const AlbertGridLevelIterator<codim,dim,dimworld>& i) const;

      //! inequality
      bool operator!= (const AlbertGridLevelIterator<codim,dim,dimworld>& i) const;

      //! dereferencing
      AlbertGridEntity<codim,dim,dimworld>& operator*() ;

      //! arrow
      AlbertGridEntity<codim,dim,dimworld>* operator->() ;

      //! ask for level of entity
      int level ();

    private:
      // private Methods
      void makeIterator();

      ALBERT EL_INFO * goFirstElement(ALBERT TRAVERSE_STACK *stack,
                                      ALBERT MESH *mesh,
                                      int level, ALBERT FLAGS fill_flag);
      ALBERT EL_INFO * traverseLeafElLevel(ALBERT TRAVERSE_STACK * stack);

      // the default is, go to next entity
      template <int cc>  ALBERT EL_INFO *
      goNextEntity(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
      {
        return goNextElInfo(stack,elinfo_old);
      };

      // codim 1 is always go to next face
      template <> ALBERT EL_INFO *
      goNextEntity<1>(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
      {
        return goNextFace(stack,elinfo_old);
      };

      // Problem, da es diese Methode nur fuer 3D geben soll, denn in 2D sind
      // die Dreieckskanten Faces
      template <> ALBERT EL_INFO *
      goNextEntity<2 << dim>(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
      {
        return goNextEdge(stack,elinfo_old);
      };
      // Problem, da es diese Methode nur fuer 2D und 3D geben soll,
      // denn in 1D sind Vertices auch Faces,
      // template <> ALBERT EL_INFO *
      template <> ALBERT EL_INFO *
      goNextEntity<dim>(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old)
      {
        return goNextVertex(stack,elinfo_old);
      };


      // the real go next methods
      ALBERT EL_INFO * goNextElInfo(ALBERT TRAVERSE_STACK *stack,ALBERT EL_INFO *elinfo_old);
      ALBERT EL_INFO * goNextFace(ALBERT TRAVERSE_STACK *stack,
                                  ALBERT EL_INFO *elInfo);
      ALBERT EL_INFO * goNextEdge(ALBERT TRAVERSE_STACK *stack,
                                  ALBERT EL_INFO *elInfo);
      ALBERT EL_INFO * goNextVertex(ALBERT TRAVERSE_STACK *stack,
                                    ALBERT EL_INFO *elInfo);


      // private Members
      AlbertGridEntity<codim,dim,dimworld> virtualEntity_;

      ALBERT ManageTravStack manageStack_;

      unsigned char face_;
      unsigned char edge_;
      unsigned char vertex_;

      AlbertMarkerVector * vertexMarker_;

    };



    //**********************************************************************
    //
    // --AlbertGrid
    //
    //**********************************************************************
    template <int dim, int dimworld>
    class AlbertGrid : public Grid < dim, dimworld,
                           albertCtype,AlbertGrid,
                           AlbertGridLevelIterator,AlbertGridEntity>
    {
      //**********************************************************
      // The Interface Methods
      //**********************************************************
    public:
      //! know your own dimension
      enum { dimension=dim };

      //! know your own dimension of world
      enum { dimensionworld=dimworld };

      AlbertGrid(char* macroTriangFilename);

      ~AlbertGrid();

      //! Return maximum level defined in this grid. Levels are numbered
      //! 0 ... maxlevel with 0 the coarsest level.
      int maxlevel();

      //! Iterator to first entity of given codim on level
      template<int codim>
      AlbertGridLevelIterator<codim,dim,dimworld> lbegin (int level);

      //! one past the end on this level
      template<int codim>
      AlbertGridLevelIterator<codim,dim,dimworld> lend (int level);

      //! Number of Vertices of the Mesh
      int numberVertices ();

      //! number of grid entities per level and codim
      int size (int level, int codim);

      //! number of grid entities per level and codim
      int hiersize (int level, int codim);

      //**********************************************************
      // End of Interface Methods
      //**********************************************************
      void globalRefine(int refCount);
      void coarsenLocal();
      void refineLocal();
      void writeGrid();

    private:
      //! pointer to an Albert Mesh, which contains the data
      ALBERT MESH *mesh_;
      int maxlevel_;
      AlbertMarkerVector *vertexMarker_;

    }; // end Class AlbertGridGrid


  }; // namespace Dune

#include "albertgrid/albertgrid.cc"

#endif
