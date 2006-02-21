// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRID_HH
#define DUNE_UGGRID_HH

/** \file
 * \brief The UGGrid class
 */

#include <dune/common/capabilities.hh>
#include <dune/grid/common/grid.hh>
#include <dune/grid/common/boundarysegment.hh>
#include <dune/common/misc.hh>
#include <dune/common/collectivecommunication.hh>

/* The following lines including the necessary UG headers are somewhat
   tricky.  Here's what's happening:
   UG can support two- and three-dimensional grids.  You choose be setting
   either _2 oder _3 while compiling.  This changes all sorts of stuff, in
   particular data structures in the headers.
   UG was never supposed to provide 2d and 3d grids at the same time.
   However, when compiling it as c++, the dimension-dependent parts are
   wrapped up cleanly in the namespaces UG2d and UG3d, respectively.  That
   way it is possible to link together the UG lib for 2d and the one for 3d.
   But we also need the headers twice!  Once with _2 set and once with _3!
   So here we go:*/

/** \todo Defining __PC__ here is certainly not the perfect way... */
/* We're still forced to include compiler.h, which expects the architecture
   as a macro.  This sucks, but currently there's no better way.  We
   choose __PC__ and hope for the best. */
#define __PC__
/* The following define tells the UG headers that we want access to a few
   special fields, for example the extra index fields in the element data structures. */
#define FOR_DUNE

// Set UG's space-dimension flag to 2d
#define _2
// And include all necessary UG headers
#include "uggrid/ugincludes.hh"

// Wrap a few large UG macros by functions before they get undef'ed away.
// Here: The 2d-version of the macros
#include "uggrid/ugfunctions.hh"

// UG defines a whole load of preprocessor macros.  ug_undefs.hh undefines
// them all, so we don't get name clashes.
#include "uggrid/ug_undefs.hh"
#undef _2

/* Now we're done with 2d, and we can do the whole thing over again for 3d */

/* All macros set by UG have been unset.  This includes the macros that ensure
   single inclusion of headers.  We can thus include them again.  However, we
   only want to include those headers again that contain dimension-dependent stuff.
   Therefore, we set a few single-inclusion defines manually before including
   ugincludes.hh again.
 */
//#define __COMPILER__
#define __HEAPS__
#define __UGENV__
#define __PARGM_H__
#define __DEVICESH__
#define __SM__

#define _3
#include "uggrid/ugincludes.hh"

// Wrap a few large UG macros by functions before they get undef'ed away.
// This time it's the 3d-versions.
#include "uggrid/ugfunctions3d.hh"

// undef all macros defined by UG
#include "uggrid/ug_undefs.hh"

#undef _3
#undef __PC__
#undef FOR_DUNE

/** \todo Remove this once getChildrenOfSubface is gone */
#include <dune/common/array.hh>

// The components of the UGGrid interface
#include "uggrid/uggridgeometry.hh"
#include "uggrid/uggridentity.hh"
#include "uggrid/uggridentitypointer.hh"
#include "uggrid/ugintersectionit.hh"
#include "uggrid/uggridleveliterator.hh"
#include "uggrid/uggridleafiterator.hh"
#include "uggrid/uggridhieriterator.hh"
#include "uggrid/uggridindexsets.hh"

namespace Dune {

  template<int dim, int dimworld>
  struct UGGridFamily
  {
    typedef GridTraits<dim,dimworld,Dune::UGGrid<dim,dimworld>,
        UGGridGeometry,
        UGGridEntity,
        UGGridEntityPointer,
        UGGridLevelIterator,
        UGGridIntersectionIterator,
        UGGridHierarchicIterator,
        UGGridLeafIterator,
        UGGridLevelIndexSet< const UGGrid<dim,dimworld> >,
        UGGridLevelIndexSetTypes< const UGGrid<dim,dimworld> >,
        UGGridLeafIndexSet< const UGGrid<dim,dimworld> >,
        UGGridLeafIndexSetTypes< const UGGrid<dim,dimworld> >,
        UGGridGlobalIdSet< const UGGrid<dim,dimworld> >,
        unsigned int,
        UGGridLocalIdSet< const UGGrid<dim,dimworld> >,
        unsigned int> Traits;
  };


  //**********************************************************************
  //
  // --UGGrid
  //
  //**********************************************************************

  /**
     \brief [<em> provides \ref Dune::Grid </em>]
     \brief Provides the meshes of the finite element toolbox UG.
     \brief (http://cox.iwr.uni-heidelberg.de/~ug).
     \ingroup GridImplementations

     This is the implementation of the grid interface
     using the UG grid management system.  UG provides conforming grids
     in two and three space dimensions.  The grids can be mixed, i.e.
     2d grids can contain triangles and quadrilaterals and 3d grids can
     contain tetrahedra and hexahedra and also pyramids and prisms.
     The grid refinement rules are very flexible.  Local adaptive red/green
     refinement is the default, but a special method in the UGGrid class
     allows you to directly access a number of anisotropic refinements rules.
     Last but not least, the UG grid manager is completely parallelized,
     and you can use boundaries parametrized by either analytical expressions
     or high-resolution piecewise linear surfaces.

     To use this module you need the UG library.  See the
     DUNE installation notes
     on how to obtain and install it.

     In your %Dune application, you can now instantiate objects of the
     type UGGrid<2,2> or UGGrid<3,3>.  You can have more than one, if
     you choose.  However, it is currently not possible to mix 2d and 3d
     UGGrids.

     Please send any questions, suggestions, or bug reports to
     sander@math.fu-berlin.de
   */
  template <int dim, int dimworld>
  class UGGrid : public GridDefaultImplementation  <dim, dimworld, double, UGGridFamily<dim,dimworld> >
  {

    friend class UGGridEntity <0,dim,const UGGrid<dim,dimworld> >;
    friend class UGGridEntity <dim,dim,const UGGrid<dim,dimworld> >;
    friend class UGGridHierarchicIterator<const UGGrid<dim,dimworld> >;
    friend class UGGridIntersectionIterator<const UGGrid<dim,dimworld> >;

    friend class UGGridLevelIndexSet<const UGGrid<dim,dimworld> >;
    friend class UGGridLeafIndexSet<const UGGrid<dim,dimworld> >;
    friend class UGGridGlobalIdSet<const UGGrid<dim,dimworld> >;
    friend class UGGridLocalIdSet<const UGGrid<dim,dimworld> >;

    template <int codim_, PartitionIteratorType PiType_, class GridImp_>
    friend class UGGridLeafIterator;
    template <int codim_, int dim_, class GridImp_, template<int,int,class> class EntityImp_>
    friend class Entity;

    /** \brief UGGrid is only implemented for 2 and 3 dimension */
    CompileTimeChecker< (dimworld==dim) && ((dim==2) || (dim==3)) >   Use_UGGrid_only_for_2d_and_3d;

    //**********************************************************
    // The Interface Methods
    //**********************************************************
  public:
    //! type of the used GridFamily for this grid
    typedef UGGridFamily<dim,dimworld>  GridFamily;

    //! the Traits
    typedef typename UGGridFamily<dim,dimworld>::Traits Traits;

    //! The type used to store coordinates
    typedef double ctype;

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
    int maxLevel() const;

    //! Iterator to first entity of given codim on level
    template<int codim>
    typename Traits::template Codim<codim>::LevelIterator lbegin (int level) const;

    //! one past the end on this level
    template<int codim>
    typename Traits::template Codim<codim>::LevelIterator lend (int level) const;

    //! Iterator to first entity of given codim on level
    template<int codim, PartitionIteratorType PiType>
    typename Traits::template Codim<codim>::template Partition<PiType>::LevelIterator lbegin (int level) const;

    //! one past the end on this level
    template<int codim, PartitionIteratorType PiType>
    typename Traits::template Codim<codim>::template Partition<PiType>::LevelIterator lend (int level) const;

    //! Iterator to first leaf entity of given codim
    template<int codim>
    typename Traits::template Codim<codim>::LeafIterator leafbegin() const {
      return typename Traits::template Codim<codim>::template Partition<All_Partition>::LeafIterator(*this);
    }

    //! one past the end of the sequence of leaf entities
    template<int codim>
    typename Traits::template Codim<codim>::LeafIterator leafend() const {
      return UGGridLeafIterator<codim,All_Partition, const UGGrid<dim,dimworld> >();
    }

    //! Iterator to first leaf entity of given codim
    template<int codim, PartitionIteratorType PiType>
    typename Traits::template Codim<codim>::template Partition<PiType>::LeafIterator leafbegin() const {
      return typename Traits::template Codim<codim>::template Partition<PiType>::LeafIterator(*this);
    }

    //! one past the end of the sequence of leaf entities
    template<int codim, PartitionIteratorType PiType>
    typename Traits::template Codim<codim>::template Partition<PiType>::LeafIterator leafend() const {
      return UGGridLeafIterator<codim,PiType, const UGGrid<dim,dimworld> >();
    }

    /** \brief Number of grid entities per level and codim
     */
    int size (int level, int codim) const;

    //! number of leaf entities per codim in this process
    int size (int codim) const
    {
      if (codim==0)
      {
        if (dim==2)
          return size(codim,GeometryType(GeometryType::simplex,2))
                 + size(codim,GeometryType(GeometryType::cube,2));

        if (dim==3)
          return size(codim,GeometryType(GeometryType::simplex,3))
                 + size(codim,GeometryType(GeometryType::pyramid,3))
                 + size(codim,GeometryType(GeometryType::prism,3))
                 + size(codim,GeometryType(GeometryType::cube,3));
      }
      if (codim==dim)
      {
        return size(codim,GeometryType(0));
      }
      if (codim==dim-1)
      {
        return size(codim,GeometryType(1));
      }
      if (codim==1)
      {
        return size(1,GeometryType(GeometryType::simplex,dim-1))
               + size(1, GeometryType(GeometryType::cube,dim-1));
      }

      DUNE_THROW(NotImplemented, "dim=" << dim << " codim=" << codim);
      return 0;
    }

    //! number of entities per level, codim and geometry type in this process
    int size (int level, int codim, GeometryType type) const
    {
      return this->levelIndexSet(level).size(codim,type);
    }

    //! number of leaf entities per codim and geometry type in this process
    int size (int codim, GeometryType type) const
    {
      return this->leafIndexSet().size(codim,type);
    }

    /** \brief Access to the GlobalIdSet */
    const typename Traits::GlobalIdSet& globalIdSet() const
    {
      return globalIdSet_;
    }

    /** \brief Access to the LocalIdSet */
    const typename Traits::LocalIdSet& localIdSet() const
    {
      return localIdSet_;
    }

    /** \brief Access to the LevelIndexSets */
    const typename Traits::LevelIndexSet& levelIndexSet(int level) const
    {
      if (level<0 || level>maxLevel())
        DUNE_THROW(GridError, "levelIndexSet of nonexisting level " << level << " requested!");
      return *levelIndexSets_[level];
    }

    /** \brief Access to the LeafIndexSet */
    const typename Traits::LeafIndexSet& leafIndexSet() const
    {
      return leafIndexSet_;
    }

    /** @name Grid Refinement Methods */
    /*@{*/

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
    bool mark(int refCount, const typename Traits::template Codim<0>::EntityPointer & e );

    /** \brief Mark method accepting a UG refinement rule

       \param e Pointer to the element to be marked for refinement
       \param rule One of the UG refinement rules
       \param side If rule==UG2d::BLUE (one quadrilateral is split into two rectangles)
       you can choose the orientation of the cut by setting side==0 or side==1
     */
    bool mark(const typename Traits::template Codim<0>::EntityPointer & e,
              typename UG_NS<dim>::RefinementRule rule,
              int side=0);

    //! Triggers the grid refinement process
    bool adapt();

    /** \brief Clean up refinement markers */
    void postAdapt();
    /*@}*/

    /** \brief Returns UGGrid_Id for run-time type identification */
    GridIdentifier type () const { return UGGrid_Id; };

    /** \brief Size of the overlap on the leaf level */
    unsigned int overlapSize(int codim) const {
      return 0;
    }

    /** \brief Size of the ghost cell layer on the leaf level */
    unsigned int ghostSize(int codim) const {
      return (codim==0) ? 1 : 0;
    }

    /** \brief Size of the overlap on a given level */
    unsigned int overlapSize(int level, int codim) const {
      return 0;
    }

    /** \brief Size of the ghost cell layer on a given level */
    unsigned int ghostSize(int level, int codim) const {
      return (codim==0) ? 1 : 0;
    }

    /** \brief Distributes this grid over the available nodes in a distributed machine
     *
       \param minlevel The coarsest grid level that gets distributed
       \param maxlevel does currently get ignored
     */
    void loadBalance(int strategy, int minlevel, int depth, int maxlevel, int minelement);

    /** \brief The communication interface
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

    /*! The new communication interface

       communicate objects for all codims on a given level
     */
    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir, int level) const
    {}

    template<class DataHandle>
    void communicate (DataHandle& data, InterfaceType iftype, CommunicationDirection dir) const
    {}

    /** dummy collective communication */
    const CollectiveCommunication<UGGrid>& comm () const
    {
      return ccobj;
    }

    // **********************************************************
    // End of Interface Methods
    // **********************************************************

    /** @name Coarse Grid Creation Methods */
    /*@{*/

    /** \brief Start the coarse grid creation process
        \todo This method should delete the grid!
     */
    void createbegin();

    /** \brief End the coarse grid creation process */
    void createend();

    /** \brief Method to insert an arbitrarily shaped boundary segment into a coarse grid
        \param vertices The indices of the vertices of the segment
        \param boundarySegment Class implementing the geometry of the boundary segment.
        The grid object takes control of this object and deallocates it when destructing itself.
     */
    void insertBoundarySegment(const std::vector<int> vertices,
                               const BoundarySegment<dimworld>* boundarySegment);

    /** \brief Insert a vertex into the coarse grid */
    void insertVertex(const FieldVector<double,dimworld>& pos);

    /** \brief Insert an element into the coarse grid
        \param type The GeometryType of the new element
        \param vertices The vertices of the new element, using the DUNE numbering
     */
    void insertElement(GeometryType type,
                       const std::vector<unsigned int>& vertices);

    /*@}*/

    /** \brief Adapt the grid without constructing the green closure

       WARNING: This is a very special method.  Omitting the green closure does
       not mean that UG creates correct nonconforming meshes.  For internal
       reasons (bugs?) though, it allows you to do uniform refinement with
       a few anisotropic refinement rules such as UG3d::PRISM_QUADSECT or
       UG3d::HEX_QUADSECT_0.
     */
    void adaptWithoutClosure();

    /** \brief Rudimentary substitute for a hierarchic iterator on faces
        \param e, elementSide Grid face specified by an element and one of its sides
        \param maxl The finest level that should be traversed by the iterator
        \param children For each subface: element index, elementSide, and level
     */
    void getChildrenOfSubface(typename Traits::template Codim<0>::EntityPointer & e,
                              int elementSide,
                              int maxl,
                              Array<typename Dune::UGGridEntityPointer<0,UGGrid> >& childElements,
                              Array<unsigned char>& childElementSides) const;

    /** \brief The different forms of grid refinement that UG supports */
    enum RefinementType {
      /** \brief New level consists only of the refined elements and the closure*/
      LOCAL,
      /** \brief New level consists of the refined elements and the unrefined ones, too */
      COPY
    };

    /** \brief Sets the type of grid refinement */
    void setRefinementType(RefinementType type) {
      refinementType_ = type;
    }

    /** \brief Collapses the grid hierarchy into a single grid level*/
    void collapse() {
      if (Collapse(multigrid_))
        DUNE_THROW(GridError, "UG" << dim << "d::Collapse() returned error code!");
    }

    /** \brief Sets a vertex to a new position

       Changing a vertex' position changes its position on all grid levels!*/
    void setPosition(typename Traits::template Codim<dim>::EntityPointer& e,
                     const FieldVector<double, dimworld>& pos);

    /** \brief Does uniform refinement
     *
     * \param n Number of uniform refinement steps
     */
    void globalRefine(int n);

  private:
    /** \brief UG multigrid, which contains the actual grid hierarchy structure */
    typename UGTypes<dimworld>::MultiGridType* multigrid_;

    /** \brief The classes implementing the geometry of the boundary segments */
    std::vector<const BoundarySegment<dimworld>*> boundarySegments_;

    /** \brief Buffer for the vertices of each explicitly given boundary segment */
    std::vector<FixedArray<unsigned int, dim*2-2> > boundarySegmentVertices_;

    CollectiveCommunication<UGGrid> ccobj;

    // Start up the UG system
    void init(unsigned int heapSize, unsigned int envHeapSize);

    /** \brief Convenience method to insert a linear boundary segment into a UG coarse grid
        \param vertices The indices of the vertices of the segment
        \param coordinates The coordinates of the vertices of the segment
     */
    void insertLinearSegment(const std::vector<int>& vertices,
                             const std::vector<FieldVector<double,dimworld> >& coordinates,
                             unsigned int segmentIndex);

    // Recomputes entity indices after the grid was changed
    void setIndices();

    // Each UGGrid object has a unique name to identify it in the
    // UG environment structure
    std::string name_;

    // Our set of level indices
    std::vector<UGGridLevelIndexSet<const UGGrid<dim,dimworld> >*> levelIndexSets_;

    UGGridLeafIndexSet<const UGGrid<dim,dimworld> > leafIndexSet_;

    UGGridGlobalIdSet<const UGGrid<dim,dimworld> > globalIdSet_;

    UGGridLocalIdSet<const UGGrid<dim,dimworld> > localIdSet_;

    //! Marks whether the UG environment heap size is taken from
    //! an existing defaults file or whether the values from
    //! the UGGrid constructor are taken
    static bool useExistingDefaultsFile;

    //! The type of grid refinement currently in use
    RefinementType refinementType_;

    //!
    bool omitGreenClosure_;

    /** \brief While inserting the elements this array records the number of
        vertices of each element. */
    std::vector<unsigned char> elementTypes_;

    /** \brief While inserting the elements this array records the vertices
        of the elements. */
    std::vector<unsigned int> elementVertices_;

    /** \brief Buffer the vertices until createend() is called */
    std::vector<FieldVector<double, dimworld> > vertexPositions_;

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

    template<int dim, int dimw>
    struct hasEntity< UGGrid<dim,dimw>, 0>
    {
      static const bool v = true;
    };

    template<int dim, int dimw>
    struct hasEntity< UGGrid<dim,dimw>, dim>
    {
      static const bool v = true;
    };

    template<int dim,int dimw>
    struct isParallel< UGGrid<dim,dimw> >
    {
      static const bool v = true;
    };

    template<int dim, int dimw>
    struct isLevelwiseConforming< UGGrid<dim,dimw> >
    {
      static const bool v = true;
    };

    template<int dim, int dimw>
    struct hasHangingNodes< UGGrid<dim,dimw> >
    {
      static const bool v = false;
    };

  }

} // namespace Dune

#endif
