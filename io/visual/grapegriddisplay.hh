// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRAPEGRIDDISPLAY_HH
#define DUNE_GRAPEGRIDDISPLAY_HH

#include "grape/grapeinclude.hh"

namespace Dune
{

  template<class GridType>
  class GrapeGridDisplay
  {
    typedef GrapeGridDisplay < GridType > MyDisplayType;
    enum { dim = GridType::dimension };
    enum { dimworld = GridType::dimensionworld };

    // defined in griddisplay.hh
    typedef typename GrapeInterface<dim,dimworld>::DUNE_ELEM DUNE_ELEM;

  public:
    typedef typename GridType::Traits::template codim<0>::LevelIterator LevelIteratorType ;
    typedef typename GridType::LeafIterator LeafIteratorType ;
    //typedef typename GridType::template LeafIteratorDef<Interior_Partition>::LeafIteratorType LeafIteratorType ;
  protected:
    //! the grid we want to display
    GridType &grid_;

    //! my process number
    const int myRank_;

    //! my interal level iterators
    LevelIteratorType *myIt_;
    LevelIteratorType *myEndIt_;

    //! my internal leaf iterators
    LeafIteratorType *myLeafIt_;
    LeafIteratorType *myLeafEndIt_;

    //! store the actual element pointer
    DUNE_ELEM hel_;

    // no better way than this canot export HMESH structure to here
    //! pointer to hmesh
    void *hmesh_;

  public:
    //! Constructor, make a GrapeGridDisplay for given grid
    GrapeGridDisplay(GridType &grid, const int myrank );

    //! Constructor, make a GrapeGridDisplay for given grid
    GrapeGridDisplay(GridType &grid);

    //! Destructor for GrapeGridDisplay
    ~GrapeGridDisplay();

    //! Calls the display of the grid and draws the discrete function
    //! if discretefunction is NULL, then only the grid is displayed
    void display();

    //! return pointer to Grape Hmesh
    void * getHmesh();

    void addMyMeshToTimeScene(void * timescene, double time, int proc)
    {
      GrapeInterface<dim,dimworld>::addHmeshToTimeScene(timescene,time,this->getHmesh(),proc);
    }

    //! return reference to Dune Grid
    GridType& getGrid();

    // internal vec for local to global methods
    FieldVector<double,dimworld> globalVec_;
    FieldVector<double,dim> localVec_;

  private:
    // generate hmesh
    void * setupHmesh();

  protected:
    //****************************************************************
    //
    // --GrapeGridDisplay, Some Subroutines needed for display with GRAPE
    //
    //****************************************************************
    // update element from entity
    template <class GridIteratorType>
    static int el_update (GridIteratorType *, DUNE_ELEM *) ;

    // update child element
    template <class GridIteratorType>
    static int child_update (GridIteratorType *, DUNE_ELEM *) ;

    template <class GridIteratorType>
    static int child_n_update (GridIteratorType *, DUNE_ELEM *) ;

    // first and next macro element via LevelIterator level 0
    int first_leaf (DUNE_ELEM * he) ;
    int next_leaf (DUNE_ELEM * he) ;

    // first and next macro element via LevelIterator level 0
    int first_macro (DUNE_ELEM * he) ;
    int next_macro (DUNE_ELEM * he) ;

    // first and next child via HierarchicIterator with given maxlevel in Grape
    static int first_child (DUNE_ELEM * he) ;
    static int next_child (DUNE_ELEM * he) ;

    static void * copy_iterator (const void * i) ;

    // local to world
    static void ctow (DUNE_ELEM * he, const double * c, double * w);

    // world to local
    static int wtoc (DUNE_ELEM * he, const double * w, double * c);

    // check inside reference element
    static int check_inside (DUNE_ELEM * he, const double * w);

    // dito
    template <class EntityType>
    int  checkInside(EntityType &en, const double * w);

    // dito
    template <class EntityType>
    int  world_to_local(EntityType &en, const double * w, double * c);

    // dito
    template <class EntityType>
    void local_to_world(EntityType &en, const double * c, double * w);

    // wrapper methods for first_child and next_child
    static int first_mac (DUNE_ELEM * he);
    static int next_mac (DUNE_ELEM * he);

    // wrapper methods for first_child and next_child
    static int fst_leaf (DUNE_ELEM * he);
    static int nxt_leaf (DUNE_ELEM * he);

  }; // end class GrapeGridDisplay


} // end namespace Dune


#include "grape/grapegriddisplay.cc"


#endif
