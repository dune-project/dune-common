// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRID_HH
#define DUNE_UGGRID_HH


#include "../common/matvec.hh"

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
  /** @defgroup UGGrid UGGridImp
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

  /** @} end documentation group */

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
   *
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


    //! UGGrid is only implemented for 2 and 3 dimension
    //! for 1d use SGrid or SimpleGrid
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

    /** \todo Please doc me! */
    enum { numCodim = dim+1 };

    /** \brief Constructor with control over UG's memory requirements
     *
     * \param heap The size of UG's internal memory in megabytes.  UG allocates
     * memory only once.  I don't know what happens if you create UGGrids with
     * differing heap sizes.
     */
    UGGrid(unsigned int heap=500);

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
    // Each UGGrid object has a unique name to identify it in the
    // UG environment structure
    std::string name_;

    // number of maxlevel of the mesh
    int maxlevel_;

    // true if grid was refined
    bool wasChanged_;

    // number of entitys of each level an codim
    Array<int> size_;

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




} // namespace Dune

#include "uggrid/uggrid.cc"
#endif
