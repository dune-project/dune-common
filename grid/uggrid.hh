// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRID_HH
#define DUNE_UGGRID_HH


//#include <vector>
//#include <assert.h>

// All UG includes have to be includes via this file
// for easier parsing by undefAllMacros.pl
#include "uggrid/ugincludes.hh"

// undef stuff defined by UG
#include "uggrid/ug_undefs.hh"

#include "../common/array.hh"
#include "common/grid.hh"

namespace Dune
{
  /** @defgroup UGGrid UGGrid Module

     This is the implementation of the grid interface
     using the UG grid management system.

     @{
   */

  /** \brief The type used by UG to store coordinates */
  typedef double UGCtype;


  // forward declarations
  class UGMarkerVector;

  template<int codim, int dim, int dimworld> class UGGridEntity;
  template<int codim, int dim, int dimworld> class UGGridLevelIterator;

  template<int dim, int dimworld>            class UGGridElement;
  template<int dim, int dimworld>            class UGGridBoundaryEntity;
  template<int dim, int dimworld>            class UGGridHierarchicIterator;
  template<int dim, int dimworld>            class UGGridIntersectionIterator;
  template<int dim, int dimworld>            class UGGrid;

  template <class Object> class UGGridMemory;

  // singleton holding reference elements
  template<int dim> struct UGGridReferenceElement;


#include "uggrid/uggridelement.hh"
#include "uggrid/uggridentity.hh"



  //**********************************************************************
  //
  // --UGGridHierarchicIterator
  // --HierarchicIterator
  /*!
     Mesh entities of codimension 0 ("elements") allow to visit all entities of
     codimension 0 obtained through nested, hierarchic refinement of the entity.
     Iteration over this set of entities is provided by the HIerarchicIterator,
     starting from a given entity.
     This is redundant but important for memory efficient implementations of unstru
     hierarchically refined meshes.
   */

  template<int dim, int dimworld>
  class UGGridHierarchicIterator :
    public HierarchicIteratorDefault <dim,dimworld, UGCtype,
        UGGridHierarchicIterator,UGGridEntity>
  {
  public:
    //! know your own dimension
    //enum { dimension=dim };

    //! know your own dimension of world
    //enum { dimensionworld=dimworld };

#if 1

#if 0
    //! the normal Constructor
    UGGridHierarchicIterator(UGGrid<dim,dimworld> &grid,
                             ALBERT TRAVERSE_STACK *travStack, int actLevel, int maxLevel);
#endif

    //! the default Constructor
    UGGridHierarchicIterator(UGGrid<dim,dimworld> &grid,
                             int actLevel,int maxLevel);
#else
    //! the normal Constructor
    UGGridHierarchicIterator(UGGrid<dim,dimworld> &grid,
                             ALBERT TRAVERSE_STACK *travStack, int travLevel);

    //! the default Constructor
    UGGridHierarchicIterator(UGGrid<dim,dimworld> &grid);
#endif

    //! prefix increment
    UGGridHierarchicIterator& operator ++();

    //! postfix increment
    UGGridHierarchicIterator& operator ++(int i);

    //! equality
    bool operator== (const UGGridHierarchicIterator& i) const;

    //! inequality
    bool operator!= (const UGGridHierarchicIterator& i) const;

    //! dereferencing
    UGGridEntity<0,dim,dimworld>& operator*();

    //! arrow
    UGGridEntity<0,dim,dimworld>* operator->();

  private:
    //! implement with virtual element
    UGGridEntity<0,dim,dimworld> virtualEntity_;

    //! know the grid were im comming from
    UGGrid<dim,dimworld> &grid_;

    //! the actual Level of this Hierarichic Iterator
    int level_;

    //! max level to go down
    int maxlevel_;

#if 0
    //! we need this for Albert traversal, and we need ManageTravStack, which
    //! does count References when copied
    ALBERT ManageTravStack manageStack_;

    //! The nessesary things for Albert
    ALBERT EL_INFO * recursiveTraverse(ALBERT TRAVERSE_STACK * stack);
#endif

    //! make empty HierarchicIterator
    void makeIterator();
  };


#define NEIGH_DEBUG

  /** \todo Please doc me! */
  template<int dim, int dimworld>
  class UGGridBoundaryEntity
    : public BoundaryEntityDefault <dim,dimworld, UGCtype,
          UGGridElement,UGGridBoundaryEntity>
  {
    friend class UGGridIntersectionIterator<dim,dimworld>;
  public:
    UGGridBoundaryEntity () : _geom (false) , /* _elInfo ( NULL ), */
                              _neigh (-1) {};

    //! return type of boundary , i.e. Neumann, Dirichlet ...
    BoundaryType type ()
    {
#ifdef NEIGH_DEBUG
      if(_elInfo->boundary[_neigh] == NULL)
      {
        std::cerr << "No Boundary, fella! \n";
        abort();
      }
#endif
      return (( _elInfo->boundary[_neigh]->bound < 0 ) ? Neumann : Dirichlet );
    }

    //! return identifier of boundary segment, number
    int id ()
    {
#ifdef NEIGH_DEBUG
      if(_elInfo->boundary[_neigh] == NULL)
      {
        std::cerr << "No Boundary, fella! \n";
        abort();
      }
#endif
      return _elInfo->boundary[_neigh]->bound;
    }

    //! return true if geometry of ghost cells was filled
    bool hasGeometry () { return _geom.builtGeom(_elInfo,0,0,0); }

    //! return geometry of the ghost cell
    UGGridElement<dim,dimworld> geometry ()
    {
      return _geom;
    }

  private:
#if 0
    // set elInfo
    void setElInfo(ALBERT EL_INFO * elInfo, int nb)
    {
      _neigh = nb;
      if(elInfo)
        _elInfo = elInfo;
      else
        _elInfo = NULL;
    };
#endif

    int _neigh;

    // UGGrid<dim,dimworld> & _grid;
    UGGridElement<dim,dimworld> _geom;

#if 0
    // cooresponding el_info
    ALBERT EL_INFO * _elInfo;
#endif
  };

#include "uggrid/ugintersectionit.hh"
#include "uggrid/uggridleveliterator.hh"

  //**********************************************************************
  //
  // --UGGrid
  //
  //**********************************************************************

  /** \brief The UG %Grid class
   *
   * \todo Please doc me!
   */
  template <int dim, int dimworld>
  class UGGrid : public GridDefault  < dim, dimworld,
                     UGCtype, UGGrid,
                     UGGridLevelIterator,UGGridEntity>
  {

    friend class UGGridEntity <0,dim,dimworld>;
    //friend class UGGridEntity <1,dim,dimworld>;
    //friend class UGGridEntity <1 << dim-1 ,dim,dimworld>;
    friend class UGGridEntity <dim,dim,dimworld>;

    friend class UGMarkerVector;

    // friends because of fillElInfo
    friend class UGGridLevelIterator<0,dim,dimworld>;
    friend class UGGridLevelIterator<1,dim,dimworld>;
    friend class UGGridLevelIterator<2,dim,dimworld>;
    friend class UGGridLevelIterator<3,dim,dimworld>;
    friend class UGGridHierarchicIterator<dim,dimworld>;

    friend class UGGridIntersectionIterator<dim,dimworld>;


    //! UGGrid is only implemented for 2 and 3 dimension
    //! for 1d use SGrid or SimpleGrid
    CompileTimeChecker< (dimworld==dim) && ((dim==2) || (dim==3)) >   Use_UGGrid_only_for_2d_and_3d;

    //**********************************************************
    // The Interface Methods
    //**********************************************************
  public:

    typedef UGGridReferenceElement<dim> ReferenceElement;

    /** \todo Please doc me! */
    enum { numCodim = dim+1 };

    //! empty Constructor
    UGGrid();

    //! Desctructor
    ~UGGrid();

    //! Return maximum level defined in this grid. Levels are numbered
    //! 0 ... maxlevel with 0 the coarsest level.
    int maxlevel() const;

    //! Iterator to first entity of given codim on level
    template<int codim>
    UGGridLevelIterator<codim,dim,dimworld> lbegin (int level) const;

    //! one past the end on this level
    template<int codim>
    UGGridLevelIterator<codim,dim,dimworld> lend (int level) const;


    /** \brief Number of grid entities per level and codim
     */
    int size (int level, int codim) const;
#if 0
    //! refine all positive marked leaf entities
    //! return true if the grid was changed
    bool refine  ( );

    //! coarsen all negative marked leaf entities
    //! return true if the grid was changed
    bool coarsen ( );

    /** \brief Please doc me! */
    GridIdentifier type () { return UGGrid_Id; };

    //**********************************************************
    // End of Interface Methods
    //**********************************************************

    //! uses the interface, mark on entity and refineLocal
    bool globalRefine(int refCount);

    // write Grid to file
    //void writeGrid(int level=-1);

    /** \brief write Grid to file in specified FileFormatType
     */
    template <FileFormatType ftype>
    bool writeGrid( const char * filename, UGCtype time );

    /** \brief read Grid from file filename and store time of mesh in time
     */
    template <FileFormatType ftype>
    bool readGrid( const char * filename, UGCtype & time );
#endif
    UGCtype getTime () const { return time_; };

    //private:
  public:
    // make the calculation of indexOnLevel and so on.
    // extra method because of Reihenfolge
    void calcExtras();

#if 0
    //! access to mesh pointer, needed by some methods
    ALBERT MESH* getMesh () const { return mesh_; };
#endif

    // UG multigrid, which contains the data
    UG3d::multigrid mesh_;

    /// Pointer to a UG domain
    UG3d::domain* domain_;

    // number of maxlevel of the mesh
    int maxlevel_;

    // true if grid was refined
    bool wasChanged_;

    // number of entitys of each level an codim
    Array<int> size_;

#if 0
    // remember on which level an ALBERT EL lives, is needed for the new
    // fillElInfo method that takes the level of an element into account
    // for calculation of the neighbor realtions
    std::vector<int> neighOnLevel_;

    // this method is new fill_elinfo from ALBERT but here the neighbor
    // relations are calced diffrent, on ervery level there are neighbor
    // realtions ( in ALBERT only on leaf level ), so we needed a new
    // fill_elinfo.
    void fillElInfo(int ichild, int actLevel ,const ALBERT EL_INFO *elinfo_old,
                    ALBERT EL_INFO *elinfo, bool hierachical ) const;

    // needed for VertexIterator, mark on which element a vertex is treated
    UGMarkerVector * vertexMarker_;

    //*********************************************************
    // Methods for mapping the global Index to local on Level
    // contains the index on level for each unique el->index of UG
    Array<int> levelIndex_[dim+1];
    void makeNewSize(Array<int> &a, int newNumberOfEntries);
    void markNew();
    //**********************************************************

    //! map the global index from the UG Mesh to the local index on Level
    template <int codim>
    int indexOnLevel(int globalIndex, int level ) ;

    // pointer to the real number of elements or vertices
    // i.e. points to mesh->n_hier_elements or mesh->n_vertices
    typename std::vector<int *> numberOfEntitys_;

    //! actual time of Grid
    UGCtype time_;

    //***********************************************************************
    //  MemoryManagement for Entitys and Elements
    //**********************************************************************
    typedef MemoryProvider< UGGridEntity<0,dim,dimworld > > EntityProvider;
    typedef MemoryProvider< UGGridElement<dim-1,dimworld> > IntersectionSelfProvider;
    typedef MemoryProvider< UGGridElement<dim-1,dim> >      IntersectionNeighProvider;

    EntityProvider entityProvider_;
    IntersectionSelfProvider interSelfProvider_;
    IntersectionNeighProvider interNeighProvider_;


#endif

  protected:
    /** \brief Number of UGGrids currently in use.
     *
     * This counts the number of UGGrids currently instantiated.  All
     * constructors of UGGrid look at this variable.  If it zero, they
     * initialize UG before proceeding.  Destructors use the same mechanism
     * to safely shut down UG after deleting the last UGGrid object.
     */
    static int numOfUGGrids;

  }; // end Class UGGrid

  // Class to mark the Vertices on the leaf level
  // to visit every vertex only once
  // for the LevelIterator codim == dim
  class UGMarkerVector
  {
    friend class UGGrid<2,2>;
    friend class UGGrid<3,3>;
  public:
    UGMarkerVector () {} ;

#if 0
    bool notOnThisElement(ALBERT EL * el, int level , int vertex);
#endif

    template <class GridType>
    void markNewVertices(GridType &grid);

    void print();

  private:
    // built in array to mark on which element a vertex is reached
    Array<int> vec_;
    // number of vertices
    int numVertex_;
  };

  /** @} end documentation group */


}; // namespace Dune

#include "uggrid/uggrid.cc"

#endif
