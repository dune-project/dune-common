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
  /** @defgroup UGGrid UGGrid
      \ingroup GridCommon

     This is the implementation of the grid interface
     using the UG grid management system.

     To use this module you need UG (the tool from second floor -
     http://cox.iwr.uni-heidelberg.de/~ug).

     After compiling UG you must tell %Dune where to find UG, which
     dimension to use and which dimension your world should have:
     <tt> ./autogen.sh [OPTIONS] --with-ug=PATH_TO_UG --with-problem-dim=DIM --with-world-dim=DIMWORLD
     </tt>

     Now you must use the UGGrid with DIM and DIMWORLD, otherwise
     unpredictable results may occur.

     @addtogroup UGGrid
     @{
   */

  /** \brief The type used by UG to store coordinates */
  typedef double UGCtype;


  // forward declarations
  //class UGMarkerVector;

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

}  // namespace Dune

#include "uggrid/uggridelement.hh"
#include "uggrid/uggridentity.hh"
//#include "uggrid/uggridboundent.hh"
#include "uggrid/ugintersectionit.hh"
#include "uggrid/uggridleveliterator.hh"

namespace Dune {

  //**********************************************************************
  //
  // --UGGrid
  //
  //**********************************************************************

  /** \brief The UG %Grid class
   * @addtogroup UGGrid
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

    //! Triggers the grid refinement process
    bool adapt();

    /** \brief Please doc me! */
    GridIdentifier type () { return UGGrid_Id; };

    //**********************************************************
    // End of Interface Methods
    //**********************************************************

    UGCtype getTime () const { return time_; };

    void makeNewUGMultigrid();

    // UG multigrid, which contains the data
    typename UGTypes<dimworld>::MultiGridType* multigrid_;

  public:
    // I need this to store some information that gets passed
    // to the boundary description
    void* extra_boundary_data_;

  private:
    // Each UGGrid object has a unique name to identify it in the
    // UG environment structure
    std::string name;

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

  }; // end Class UGGrid

  /** @} end documentation group */


}; // namespace Dune

#include "uggrid/uggrid.cc"
#endif
