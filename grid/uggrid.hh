// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRID_HH
#define DUNE_UGGRID_HH


#include "../common/matvec.hh"
#include "../common/capabilities.hh"

// All UG includes have to be includes via the file ugincludes.hh
// for easier parsing by undefAllMacros.pl
#define __PC__  // hack:  choose the architecture
//#define _3      // Choose the dimension
#include "uggrid/ugincludes.hh"
#undef __PC__
//#undef _3

// Wrap a few large UG macros by functions before they get undef'ed away
#include "uggrid/ugfunctions.hh"


// undef stuff defined by UG
#include "uggrid/ug_undefs.hh"

#include "../common/array.hh"
#include "common/grid.hh"

namespace Dune
{
  /** @defgroup UGGrid UGGrid
      \ingroup GridCommon

     This is the implementation of the grid interface
     using the UG grid management system.

     To use this module you need UG (the tool from second floor -
     http://cox.iwr.uni-heidelberg.de/~ug).

     For the compilation of UG keep the following in mind!
     In your shell you need set the following variables:

     \verbatim
      export UGROOT=some_path/UG/ug
      export PATH=$PATH:$UGROOT/bin
     \endverbatim

     In the dir $UGROOT (the path above) you find a file named 'ug.conf'.
     There you have to choose for dimension 2 or 3 and furthermore set the
     variable 'IF' to 'S' (default is 'X').

     Then in the file $UGROOT/arch/PC/mk.arch add '-xc++' to the ARCH_CFLAGS
     variable.

     Now your ready to compile UG. Type

     \verbatim
     cd $UGROOT
     ugmake links
     make
     \endverbatim

     After compiling UG you must tell %Dune where to find UG, which
     dimension to use and which dimension your world should have:
     \verbatim
     ./autogen.sh [OPTIONS] --with-ug=PATH_TO_UG --with-problem-dim=DIM --with-world-dim=DIMWORLD
     \endverbatim

     Now you must use the UGGrid with DIM and DIMWORLD, otherwise
     unpredictable results may occur.


     @{
   */

  /** \brief The type used by UG to store coordinates */
  typedef double UGCtype;


  /** @} end documentation group */

  // forward declarations
  template<int codim, int dim, int dimworld> class UGGridEntity;
  template<int codim, int dim, int dimworld, PartitionIteratorType pitype> class UGGridLevelIterator;

  template<int dim, int dimworld>            class UGGridElement;
  template<int dim, int dimworld>            class UGGridBoundaryEntity;
  template<int dim, int dimworld>            class UGGridHierarchicIterator;
  template<int dim, int dimworld>            class UGGridIntersectionIterator;
  template<int dim, int dimworld>            class UGGrid;

  //template <class Object> class UGGridMemory;

  // singleton holding reference elements
  template<int dim> struct UGGridReferenceElement;


}  // namespace Dune

#include "uggrid/uggridelement.hh"
#include "uggrid/uggridentity.hh"
#include "uggrid/uggridboundent.hh"
#include "uggrid/ugintersectionit.hh"
#include "uggrid/uggridleveliterator.hh"
#include "uggrid/uggridhieriterator.hh"

namespace Dune {

  //**********************************************************************
  //
  // --UGGrid
  //
  //**********************************************************************

  /** \brief The UG %Grid class
   * \ingroup UGGrid
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

    // friends because of fillElInfo
    //   friend class UGGridLevelIterator<0,dim,dimworld>;
    //   friend class UGGridLevelIterator<1,dim,dimworld>;
    //   friend class UGGridLevelIterator<2,dim,dimworld>;
    //   friend class UGGridLevelIterator<3,dim,dimworld>;
    friend class UGGridHierarchicIterator<dim,dimworld>;

    friend class UGGridIntersectionIterator<dim,dimworld>;


    /** \brief UGGrid is only implemented for 2 and 3 dimension
     * for 1d use SGrid or SimpleGrid  */
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


    typedef UGGridReferenceElement<dim> ReferenceElement;

    /** \brief The leaf iterator type  (currently only a level iterator)
     * \todo Replace this by a true leaf iterator */
    typedef UGGridLevelIterator<0,dim,dimworld, All_Partition> LeafIterator;

    /** \todo Please doc me! */
    //   enum { numCodim = dim+1 };

    /** \brief Constructor with control over UG's memory requirements
     *
     * \param heapSize The size of UG's internal memory in megabytes.  UG allocates
     * memory only once.  I don't know what happens if you create UGGrids with
     * differing heap sizes.
     * \param envHeapSize The size of UG's environment heap.
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
    UGGridLevelIterator<codim,dim,dimworld, All_Partition> lbegin (int level) const;

    //! one past the end on this level
    template<int codim>
    UGGridLevelIterator<codim,dim,dimworld, All_Partition> lend (int level) const;

    //! Iterator to first entity of given codim on level
    template<int codim, PartitionIteratorType PiType>
    UGGridLevelIterator<codim,dim,dimworld, PiType> lbegin (int level) const;

    //! one past the end on this level
    template<int codim, PartitionIteratorType PiType>
    UGGridLevelIterator<codim,dim,dimworld, PiType> lend (int level) const;

    /** \brief Create leaf iterator  (currently only a level iterator)
     * \todo Replace this by a true leaf iterator */
    LeafIterator leafbegin (int level) const {return lbegin<0>(level);}

    /** \brief Create one past last on leaf level  (currently only a level iterator)
     * \todo Replace this by a true leaf iterator */
    LeafIterator leafend (int level) const {return lend<0>(level);}

    /** \brief Number of grid entities per level and codim
     */
    int size (int level, int codim) const;

    //! Triggers the grid refinement process
    bool adapt();

    /** \brief Please doc me! */
    GridIdentifier type () { return UGGrid_Id; };

    // **********************************************************
    // End of Interface Methods
    // **********************************************************

    /** \brief The different forms of grid refinement that UG supports */
    enum AdaptationType {
      /** \brief New level consists only of the refined elements */
      LOCAL,
      /** \brief New level consists of the refined elements and the unrefined ones, too */
      COPY,
      /** \brief %Grid hierarchy is collapsed into a single grid level after refinement */
      COLLAPSE
    };

    /** \brief Sets the type of grid refinement */
    void setAdaptationType(AdaptationType type);

    /** \brief Read access to the UG-internal grid name */
    const std::string& name() const {return name_;}


    void makeNewUGMultigrid();

    /** \brief Does one uniform refinement step
     *
     * \param refCount I don't know what this is good for.  It doesn't
     *        actually do anything.
     */
    void globalRefine(int refCount);

    // UG multigrid, which contains the data
    typename UGTypes<dimworld>::MultiGridType* multigrid_;

  public:
    // I need this to store some information that gets passed
    // to the boundary description
    void* extra_boundary_data_;

  private:

    void init(unsigned int heapSize, unsigned int envHeapSize);

    // Each UGGrid object has a unique name to identify it in the
    // UG environment structure
    std::string name_;

    // number of maxlevel of the mesh
    int maxlevel_;

    // true if grid was refined
    bool wasChanged_;

    // number of entitys of each level an codim
    Array<int> size_;

    //! Marks whether the UG environment heap size is taken from
    //! an existing defaults file or whether the values from
    //! the UGGrid constructor are taken
    bool useExistingDefaultsFile;

  protected:
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

} // namespace Dune

#include "uggrid/uggrid.cc"
#endif
