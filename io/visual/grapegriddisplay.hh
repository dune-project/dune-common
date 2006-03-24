// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRAPEGRIDDISPLAY_HH
#define DUNE_GRAPEGRIDDISPLAY_HH

//- system includes
#include <list>

//- Dune includes
#include <dune/grid/common/grid.hh>

//-local includes
#include "grape/grapeinclude.hh"

namespace Dune
{

  // the internal grape partition iterator types
  enum GrapePartitionIteratorType
  {
    g_Interior_Partition       = Interior_Partition,
    g_InteriorBorder_Partition = InteriorBorder_Partition,
    g_Overlap_Partition        = Overlap_Partition,
    g_OverlapFront_Partition   = OverlapFront_Partition,
    g_All_Partition            = All_Partition,
    g_Ghost_Partition          = Ghost_Partition
  };

  // the internal grape partition iterator types
  enum GrapeIteratorType
  {
    g_LeafIterator       = 0,
    g_LevelIterator      = 1,
    g_HierarchicIterator = 2
  };

  template<class GridType>
  class GrapeGridDisplay
  {
    typedef GrapeGridDisplay < GridType > MyDisplayType;
    enum { dim = GridType::dimension };
    enum { dimworld = GridType::dimensionworld };

    // defined in griddisplay.hh
    typedef typename GrapeInterface<dim,dimworld>::DUNE_ELEM DUNE_ELEM;
    typedef typename GrapeInterface<dim,dimworld>::DUNE_DAT DUNE_DAT;

  public:
    typedef GridType MyGridType;

    typedef typename GridType::Traits::template Codim<0>::LevelIterator LevelIteratorType ;
    //typedef typename GridType::Traits::template Codim<0>::template Partition<Ghost_Partition>::LeafIterator LeafIteratorType ;
    typedef typename GridType::Traits::template Codim<0>::template Partition<Interior_Partition>::LeafIterator LeafIteratorType ;

    typedef typename GridType::template Codim<0>:: HierarchicIterator
    HierarchicIteratorType;

    typedef typename GridType::Traits::LocalIdSet LocalIdSetType;
    typedef typename GridType::Traits::LeafIndexSet LeafIndexSetType;

  protected:
    //! the grid we want to display
    const GridType &grid_;

    //! leaf index set of the grid
    const LeafIndexSetType & leafset_;

    //! leaf index set of the grid
    const LocalIdSetType & lid_;

    //! my process number
    const int myRank_;

    //! store the actual element pointer
    DUNE_ELEM hel_;
    DUNE_DAT dune_;

    // no better way than this canot export HMESH structure to here
    //! pointer to hmesh
    void * hmesh_;

    typedef std::list<HierarchicIteratorType *> HierarchicIteratorList;
    typedef typename HierarchicIteratorList::iterator ListIteratorType;
    HierarchicIteratorList hierList_;

  public:
    //! Constructor, make a GrapeGridDisplay for given grid
    inline GrapeGridDisplay(const GridType &grid, const int myrank );

    //! Constructor, make a GrapeGridDisplay for given grid
    inline GrapeGridDisplay(const GridType &grid);

    //! Destructor for GrapeGridDisplay
    inline ~GrapeGridDisplay();

    //! Calls the display of the grid and draws the discrete function
    //! if discretefunction is NULL, then only the grid is displayed
    inline void display();

    //! return rank of this display, for visualisation of parallel grid
    int myRank () const { return myRank_; }

    //! return pointer to Grape Hmesh
    inline void * getHmesh();

    inline void addMyMeshToTimeScene(void * timescene, double time, int proc);

    //! return reference to Dune Grid
    inline const GridType& getGrid() const ;

    bool hasData () { return false; }

    // internal vec for local to global methods
    FieldVector<double,dimworld> globalVec_;
    FieldVector<double,dim> localVec_;

  private:
    // generate hmesh
    inline void * setupHmesh();

  public:
    //****************************************************************
    //
    // --GrapeGridDisplay, Some Subroutines needed for display with GRAPE
    //
    //****************************************************************
    // update element from entity
    template <class EntityPointerType>
    inline int el_update (EntityPointerType *, DUNE_ELEM *) ;

    // update child element
    template <class EntityPointerType>
    inline int child_update (EntityPointerType * , DUNE_ELEM *) ;

    template <class EntityPointerType>
    inline int child_n_update (EntityPointerType *, DUNE_ELEM *) ;

    // first and next macro element via LevelIterator level 0
    template <PartitionIteratorType pitype>
    inline int first_leaf (DUNE_ELEM * he) ;
    template <PartitionIteratorType pitype>
    inline int next_leaf (DUNE_ELEM * he) ;

    // first and next macro element via LevelIterator level
    template <PartitionIteratorType pitype>
    inline int first_level (DUNE_ELEM * he, int level) ;

    template <PartitionIteratorType pitype>
    inline int next_level (DUNE_ELEM * he) ;

    // methods to call for combined display
    inline int firstMacro (DUNE_ELEM * elem) { return dune_.first_macro(elem); }
    inline int nextMacro  (DUNE_ELEM * elem) { return dune_.next_macro(elem);  }
    inline int firstChild (DUNE_ELEM * elem) { return (dune_.first_child) ? dune_.first_child(elem) : 0; }
    inline int nextChild  (DUNE_ELEM * elem) { return (dune_.next_child) ? dune_.next_child(elem) : 0; }

    // first and next child via HierarchicIterator with given maxlevel in Grape
    inline int first_child (DUNE_ELEM * he) ;
    inline int next_child (DUNE_ELEM * he) ;

    // fake function for copy iterator
    inline static void * copy_iterator (const void * i) ;

    // local to world
    inline void local2world (DUNE_ELEM * he, const double * c, double * w);

    // world to local
    inline int world2local (DUNE_ELEM * he, const double * w, double * c);

    // check inside reference element
    inline int checkWhetherInside (DUNE_ELEM * he, const double * w);

    //*********************************
    //  wrapper functions
    //*********************************
    // local to world
    inline static void ctow (DUNE_ELEM * he, const double * c, double * w);

    // world to local
    inline static int wtoc (DUNE_ELEM * he, const double * w, double * c);

    // check inside reference element
    inline static int check_inside (DUNE_ELEM * he, const double * w);

    // dito
    template <class EntityType>
    inline int  checkInside(EntityType &en, const double * w);

    // dito
    template <class EntityType>
    inline int  world_to_local(EntityType &en, const double * w, double * c);

    // dito
    template <class EntityType>
    inline void local_to_world(EntityType &en, const double * c, double * w);

    template <PartitionIteratorType pitype>
    inline void selectIterators(DUNE_DAT * dat) const;

    inline void setIterationMethods(DUNE_DAT * dat) const;

    template <PartitionIteratorType pitype>
    struct IterationMethods
    {
      // wrapper methods for first_child and next_child
      inline static int first_mac (DUNE_ELEM * he)
      {
        MyDisplayType & disp = *((MyDisplayType *) he->display);
        return disp.template first_level<pitype>(he,0);
      }

      // wrapper methods for first_child and next_child
      inline static int first_lev (DUNE_ELEM * he)
      {
        MyDisplayType & disp = *((MyDisplayType *) he->display);
        return disp.template first_level<pitype>(he,he->level_of_interest);
      }

      inline static int next_lev  (DUNE_ELEM * he)
      {
        MyDisplayType & disp = *((MyDisplayType *) he->display);
        return disp.template next_level<pitype>(he);
      }

      // wrapper methods for first_child and next_child
      inline static int fst_leaf (DUNE_ELEM * he)
      {
        MyDisplayType & disp = *((MyDisplayType *) he->display);
        return disp.template first_leaf<pitype>(he);
      }
      inline static int nxt_leaf (DUNE_ELEM * he)
      {
        MyDisplayType & disp = *((MyDisplayType *) he->display);
        return disp.template next_leaf<pitype>(he);
      }

      // wrapper methods for first_child and next_child
      inline static int fst_child (DUNE_ELEM * he)
      {
        MyDisplayType & disp = *((MyDisplayType *) he->display);
        return disp.first_child(he);
      }
      inline static int nxt_child (DUNE_ELEM * he)
      {
        MyDisplayType & disp = *((MyDisplayType *) he->display);
        return disp.next_child(he);
      }
    };

    inline static void setIterationModus(DUNE_DAT * dat);

  }; // end class GrapeGridDisplay


  /**************************************************************************/
  //  element types, see dune/grid/common/grid.hh
  // and also geldesc.hh for GR_ElementTypes
  enum GRAPE_ElementType
  {  g_vertex         = GrapeInterface_three_three::gr_vertex
     ,  g_line           = GrapeInterface_three_three::gr_line
     ,  g_triangle       = GrapeInterface_three_three::gr_triangle
     ,  g_quadrilateral  = GrapeInterface_three_three::gr_quadrilateral
     ,  g_tetrahedron    = GrapeInterface_three_three::gr_tetrahedron
     ,  g_pyramid        = GrapeInterface_three_three::gr_pyramid
     ,  g_prism          = GrapeInterface_three_three::gr_prism
     ,  g_hexahedron     = GrapeInterface_three_three::gr_hexahedron
     ,  g_iso_triangle   = GrapeInterface_three_three::gr_iso_triangle
     ,  g_iso_quadrilateral  = GrapeInterface_three_three::gr_iso_quadrilateral
     ,  g_unknown            = GrapeInterface_three_three::gr_unknown};

  //! convert dune geometry types to grape geometry types with numbers
  static inline GRAPE_ElementType convertToGrapeType ( GeometryType type , int dim )
  {
    if(dim < 3)
    {
      if(type.isTriangle()) return g_triangle;
      if(type.isQuadrilateral()) return g_quadrilateral;
      if(type.isVertex()) return g_vertex;
      if(type.isLine()) return g_line;
    }
    else
    {
      if(type.isTetrahedron()) return g_tetrahedron;
      if(type.isHexahedron()) return g_hexahedron;
      if(type.isPyramid()) return g_pyramid;
      if(type.isPrism()) return g_prism;
    }

    std::cerr << "No requested conversion for GeometryType " << type << "!\n";
    return g_unknown;
  }

  // see geldesc.hh for definition of this mapping
  // this is the same for all namespaces (two_two , and two_three, ...)
  static const int * const * vxMap = GrapeInterface_three_three::dune2GrapeVertex;
  static inline int mapDune2GrapeVertex( int geomType , int vx )
  {
    enum { usedTypes = GrapeInterface_three_three::numberOfUsedGrapeElementTypes };
    assert( geomType >= 0 );
    assert( geomType <  usedTypes ); // at the moment only defined from 2 to 7
    return vxMap[geomType][vx];
  }

} // end namespace Dune

#include "grape/grapegriddisplay.cc"

// undefs all defines
#include "grape/grape_undefs.hh"
#endif
