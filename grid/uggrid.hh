// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRID_HH
#define DUNE_UGGRID_HH

/** \file
 * \brief The UGGrid class
 */

#include "../common/capabilities.hh"
#include <dune/grid/common/grid.hh>
#include <dune/common/misc.hh>

// All UG includes have to be includes via the file ugincludes.hh
// for easier parsing by undefAllMacros.pl
#define __PC__  // hack:  choose the architecture
#include "uggrid/ugincludes.hh"
#undef __PC__

// Wrap a few large UG macros by functions before they get undef'ed away
#include "uggrid/ugfunctions.hh"


// undef stuff defined by UG
#include "uggrid/ug_undefs.hh"

#include <dune/common/array.hh>
#include <dune/grid/common/defaultindexsets.hh>

namespace Dune
{
  /** @defgroup UGGrid UGGrid
      \ingroup GridCommon

     This is the implementation of the grid interface
     using the UG grid management system.  UG provides conforming grids
     in two and three space dimensions.  The grids can be mixed, i.e.
     2d grids can contain triangles and quadrilaterals and 3d grid can
     contain tetrahedra and hexahedra and also pyramids and prisms.
     The grid refinement rules are very flexible.  Local adaptive red/green
     refinement is the default, but a special method in the UGGrid class
     allows you to directly access a number of anisotropic refinements rules.
     Last but not least, the UG grid manager is completely parallelized.

     To use this module you need the UG library.  You can obtain it at
     http://cox.iwr.uni-heidelberg.de/~ug .  Unfortunately, the version
     of UG available for download (3.8 at the time of writing) is not new
     enough to be compatible
     with DUNE.  Please write to one of the contact addresses given on
     the page mentioned above and ask for a more recent version of UG.

     UG is built using the standard GNU autotools.  After unpacking
     the tarball, please type
     \verbatim
      ./configure --prefix=my_ug_install_dir CC=g++-3.4
     \endverbatim
     Note that you need to force the build system to compile UG
     as C++, which is a non-default behaviour.  Also, make sure
     that you're compiling it with a compiler that's binary compatible
     to the one you use for DUNE.

     After that it's simply
     \verbatim
      make
      make install
     \endverbatim
     and you're done.

     After compiling UG you must tell %Dune where to find UG, and which
     grid dimension to use:
     \verbatim
     ./autogen.sh [OPTIONS] --with-ug=PATH_TO_UG --with-problem-dim=DIM --with-world-dim=DIMWORLD
     \endverbatim

     As UG only supports 2d-grids in a 2d-world and 3d-grids in a 3d-world,
     whatever you choose for DIM and DIMWORLD in the line above must be equal
     and either 2 or 3.  The two dimension-related arguments must both be
     there because they actually refer to all external grid implementations
     that may require it, and there are some that do not necessarily pose
     restrictions as strict as UG does.

     In your DUNE application, you can only instantiate UGGrid<2,2> or
     UGGrid<3,3>, depending on what you chose above.

     Please send any questions, suggestions, or bug reports to
     sander@math.fu-berlin.de

   */

  /** \brief The type used by UG to store coordinates */
  typedef double UGCtype;


  // forward declarations
  template<int dim, int dimworld>          class UGGrid;
  template<int codim, int dim,
      class GridImp>                  class UGGridEntity;
  template<int codim,
      PartitionIteratorType pitype,
      class GridImp>                  class UGGridLevelIterator;
  template<int mydim, int coorddim,
      class GridImp>                  class UGGridGeometry;
  template<class GridImp>                 class UGGridBoundaryEntity;
  template<class GridImp>                 class UGGridHierarchicIterator;
  template<class GridImp>                 class UGGridIntersectionIterator;

}  // namespace Dune

#include "uggrid/uggridgeometry.hh"
#include "uggrid/uggridentity.hh"
#include "uggrid/uggridentitypointer.hh"
#include "uggrid/uggridboundent.hh"
#include "uggrid/ugintersectionit.hh"
#include "uggrid/uggridleveliterator.hh"
#include "uggrid/uggridhieriterator.hh"
#include "uggrid/ughierarchicindexset.hh"

namespace Dune {

  //**********************************************************************
  //
  // --UGGrid
  //
  //**********************************************************************

  /** \brief The UG %Grid class
   * \ingroup UGGrid
   *
     This is the implementation of the grid interface
     using the UG grid management system.  UG provides conforming grids
     in two and three space dimensions.  The grids can be mixed, i.e.
     2d grids can contain triangles and quadrilaterals and 3d grid can
     contain tetrahedra and hexahedra and also pyramids and prisms.
     The grid refinement rules are very flexible.  Local adaptive red/green
     refinement is the default, but a special method in the UGGrid class
     allows you to directly access a number of anisotropic refinements rules.
     Last but not least, the UG grid manager is completely parallelized.
   */
  template <int dim, int dimworld>
  class UGGrid : public GridDefault  <dim, dimworld,UGCtype, UGGrid<dim,dimworld> >
  {

    friend class UGGridEntity <0,dim,UGGrid<dim,dimworld> >;
    friend class UGGridEntity <dim,dim,UGGrid<dim,dimworld> >;
    friend class UGGridHierarchicIterator<UGGrid<dim,dimworld> >;
    friend class UGGridIntersectionIterator<UGGrid<dim,dimworld> >;

    template<int codim_, int dim_, class GridImp_, template<int,int,class> class EntityImp_>
    friend class Entity;

    friend class UGGridHierarchicIndexSet<UGGrid<dim,dimworld> >;

    /** \brief UGGrid is only implemented for 2 and 3 dimension
     * for 1d use SGrid or OneDGrid  */
    CompileTimeChecker< (dimworld==dim) && ((dim==2) || (dim==3)) >   Use_UGGrid_only_for_2d_and_3d;
    // #ifdef _2
    //   CompileTimeChecker< (dimworld==dim) && (dim==2) >   Use_UGGrid_only_for_2d_when_built_for_2d;
    // #endif
    // #ifdef _3
    //   CompileTimeChecker< (dimworld==dim) && (dim==3) >   Use_UGGrid_only_for_3d_when_built_for_3d;
    // #endif

    //**********************************************************
    // The Interface Methods
    //**********************************************************
  public:
    typedef GridTraits<dim,
        dimworld,
        Dune::UGGrid<dim,dimworld> ,
        UGGridGeometry,
        UGGridEntity,
        UGGridBoundaryEntity,
        UGGridEntityPointer,
        UGGridLevelIterator,
        UGGridIntersectionIterator,
        UGGridHierarchicIterator> Traits;

    typedef UGGridHierarchicIndexSet<Dune::UGGrid<dim,dimworld> > HierarchicIndexSetType;
    typedef DefaultLevelIndexSet<Dune::UGGrid<dim,dimworld> >      LevelIndexSetType;

    /** \brief Constructor with control over UG's memory requirements
     *
     * \param heapSize The size of UG's internal memory in megabytes.  UG allocates
     * memory only once.  I don't know what happens if you create UGGrids with
     * differing heap sizes.
     * \param envHeapSize The size of UG's environment heap, also in megabytes.
     */
    UGGrid(unsigned int heapSize, unsigned int envHeapSize);

    /** \brief Constructor with default memory settings
     *
     * The default values are 500MB for the general heap and 10MB for
     * the environment heap.
     */
    UGGrid();

    //! Desctructor
    ~UGGrid();

    //! Return maximum level defined in this grid. Levels are numbered
    //! 0 ... maxlevel with 0 the coarsest level.
    int maxlevel() const;

    //! Iterator to first entity of given codim on level
    template<int codim>
    typename Traits::template codim<codim>::LevelIterator lbegin (int level) const;

    //! one past the end on this level
    template<int codim>
    typename Traits::template codim<codim>::LevelIterator lend (int level) const;

    //! Iterator to first entity of given codim on level
    template<int codim, PartitionIteratorType PiType>
    typename Traits::template codim<codim>::template partition<PiType>::LevelIterator lbegin (int level) const;

    //! one past the end on this level
    template<int codim, PartitionIteratorType PiType>
    typename Traits::template codim<codim>::template partition<PiType>::LevelIterator lend (int level) const;

    /** \brief Number of grid entities per level and codim
     */
    int size (int level, int codim) const;

#ifdef UGGRID_WITH_INDEX_SETS
    int global_size (int codim) const DUNE_DEPRECATED { return hierarchicIndexSet().size(maxlevel(),codim); }
#else
    int global_size (int codim) const DUNE_DEPRECATED { return size(maxlevel(),codim); }
#endif

    /** \brief Mark entity for refinement
     *
     * This only works for entities of codim 0.
     * The parameter is currently ignored
     *
     * \return <ul>
     * <li> true, if element was marked </li>
     * <li> false, if nothing changed </li>
     * </ul>
     */
    bool mark(int refCount, typename Traits::template codim<0>::EntityPointer & e ) {
      return mark(refCount, *e);
    }

    /** \brief Tempory for Robert */
    bool mark(int refCount, typename Traits::template codim<0>::Entity& e );

    /** \brief Mark method accepting a UG refinement rule
     */
#ifdef _3
    bool mark(typename Traits::template codim<0>::EntityPointer & e, UG3d::RefinementRule rule);
#else
    bool mark(typename Traits::template codim<0>::EntityPointer & e, UG2d::RefinementRule rule);
#endif

    //! Triggers the grid refinement process
    bool adapt();

    /** \brief Clean up refinement markers */
    void postAdapt();

    /** \brief Please doc me! */
    GridIdentifier type () const { return UGGrid_Id; };

    /** \brief Distributes this grid over the available nodes in a distributed machine
     *
       \param minlevel The coarsest grid level that gets distributed
       \param maxlevel does currently get ignored
     */
    void loadBalance(int strategy, int minlevel, int depth, int maxlevel, int minelement);

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
    void communicate (T& t, InterfaceType iftype, CommunicationDirection dir, int level);

    // **********************************************************
    // End of Interface Methods
    // **********************************************************

    const HierarchicIndexSetType& hierarchicIndexSet() const {
      return hierarchicIndexSet_;
    }

    const LevelIndexSetType& levelIndexSet() const {
      if (!levelIndexSet_)
        levelIndexSet_ = new LevelIndexSetType(*this);
      return *levelIndexSet_;
    }

    void createbegin();

    void createend();

    void adaptWithoutClosure();

    void getChildrenOfSubface(typename Traits::template codim<0>::EntityPointer & e,
                              int elementSide,
                              int maxl,
                              Array<FixedArray<unsigned int, 3> >& children) const;

    /** \brief The different forms of grid refinement that UG supports */
    enum RefinementType {
      /** \brief New level consists only of the refined elements */
      LOCAL,
      /** \brief New level consists of the refined elements and the unrefined ones, too */
      COPY
    };

    /** \brief Sets the type of grid refinement */
    void setRefinementType(RefinementType type) {
      refinementType_ = type;
    }

    /** \brief Collapses the grid hierarchy into a single grid */
    void collapse() {
      if (Collapse(multigrid_))
        DUNE_THROW(GridError, "UG" << dim << "d::Collapse() returned error code!");
    }

    /** \brief Read access to the UG-internal grid name */
    const std::string& name() const {return name_;}


    void makeNewUGMultigrid();

    /** \brief Does uniform refinement
     *
     * \param n Number of uniform refinement steps
     */
    void globalRefine(int n);

    // UG multigrid, which contains the data
    typename UGTypes<dimworld>::MultiGridType* multigrid_;

  public:
    // I need this to store some information that gets passed
    // to the boundary description
    void* extra_boundary_data_;

  private:

    template <int cd>
    UGGridEntity<cd,dim,const UGGrid>& getRealEntity(typename Traits::template codim<cd>::Entity& entity) {
      return entity.realEntity;
    }

    template <int cd>
    const UGGridEntity<cd,dim,const UGGrid>& getRealEntity(const typename Traits::template codim<cd>::Entity& entity) const {
      return entity.realEntity;
    }

    void init(unsigned int heapSize, unsigned int envHeapSize);

    void setLocalIndices();

    // Each UGGrid object has a unique name to identify it in the
    // UG environment structure
    std::string name_;

    // number of maxlevel of the mesh
    int maxlevel_;

    // Our hierarchic index set
    HierarchicIndexSetType hierarchicIndexSet_;

    // Our set of level indices
    mutable LevelIndexSetType* levelIndexSet_;


    // true if grid was refined
    //bool wasChanged_;

    // number of entitys of each level an codim
    Array<int> size_;

    //! Marks whether the UG environment heap size is taken from
    //! an existing defaults file or whether the values from
    //! the UGGrid constructor are taken
    bool useExistingDefaultsFile;

    //! The type of grid refinement currently in use
    RefinementType refinementType_;

    //!
    bool omitGreenClosure_;

    /** \brief Number of UGGrids currently in use.
     *
     * This counts the number of UGGrids currently instantiated.  All
     * constructors of UGGrid look at this variable.  If it zero, they
     * initialize UG before proceeding.  Destructors use the same mechanism
     * to safely shut down UG after deleting the last UGGrid object.
     */
    static int numOfUGGrids;

    /** \brief The arguments to UG's newformat command
     *
     * They need to be allocated dynamically, because UG writes into
     * some of them.  That causes the code to crash if it has been
     * compiled with gcc.
     */
    char* newformatArgs[4];

    /** \brief The size of UG's internal heap in megabytes
     *
     * It is handed over to UG for each new multigrid.  I don't know
     * what happens if you hand over differing values.
     */
    unsigned int heapsize;


  }; // end Class UGGrid

#if 0
  namespace Capabilities
  {

    template<int dim,int dimw>
    struct hasLeafIterator< UGGrid<dim,dimw> >
    {
      static const bool v = true;
    };

    template<int dim, int dimw, int cdim>
    struct hasEntity< UGGrid<dim,dimw>, UGGridEntity<cdim,dim,dimw> >
    {
      static const bool v = true;
    };

    template<int dim,int dimw>
    struct isParallel< UGGrid<dim,dimw> >
    {
      static const bool v = true;
    };

  }
#endif

} // namespace Dune

// Include the method definitions
#include "uggrid/uggrid.cc"

#endif
