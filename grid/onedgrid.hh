// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONE_D_GRID_HH
#define DUNE_ONE_D_GRID_HH

#include <vector>

#include <dune/common/matvec.hh>
#include <dune/common/capabilities.hh>
#include <dune/grid/common/grid.hh>
#include <dune/common/simplevector.hh>

/** \file
 * \brief The OneDGrid class
 */

namespace Dune
{
  /** @defgroup OneDGrid OneDGrid
      \ingroup GridCommon

   */

  /** \brief The type used by to store coordinates */
  typedef double OneDCType;

  // forward declarations
  template<int codim, int dim, int dimworld> class OneDGridEntity;
  template<int codim, int dim, int dimworld, PartitionIteratorType pitype> class OneDGridLevelIterator;

  template<int dim, int dimworld>            class OneDGridElement;
  // template<int dim, int dimworld>            class UGGridBoundaryEntity;
  template<int dim, int dimworld>            class OneDGridHierarchicIterator;
  template<int dim, int dimworld>            class OneDGridIntersectionIterator;
  template<int dim, int dimworld>            class OneDGrid;

  // singleton holding reference elements
  //template<int dim> struct UGGridReferenceElement;


}  // namespace Dune

#include "onedgrid/onedgridentity.hh"
#include "onedgrid/onedgridelement.hh"
#include "onedgrid/onedgridboundent.hh"
#include "onedgrid/onedintersectionit.hh"
#include "onedgrid/onedgridleveliterator.hh"
#include "onedgrid/onedgridhieriterator.hh"

namespace Dune {

  //**********************************************************************
  //
  // --OneDGrid
  //
  //**********************************************************************

  /** \brief The 1D-Grid class
   * \ingroup OneDGrid
   *
   * This implementation of the grid interface provides one-dimensional
     grids only.  No matter what the values of dim and dimworld may be,
     you'll always get a 1D-grid in a 1D-world.  Unlike SGrid, however,
     which can also be instantiated in 1D, the OneDGrid is nonuniform
     and provides local mesh refinement and coarsening.
   */
  template <int dim, int dimworld>
  class OneDGrid : public GridDefault  < dim, dimworld,
                       OneDCType, OneDGrid,
                       OneDGridLevelIterator,OneDGridEntity>
  {

    friend class OneDGridEntity <0,dim,dimworld>;
    friend class OneDGridEntity <dim,dim,dimworld>;
    friend class OneDGridHierarchicIterator<dim,dimworld>;
    friend class OneDGridIntersectionIterator<dim,dimworld>;

    /** \brief OneDGrid is only implemented for 1d */
    CompileTimeChecker< (dim==1 && dimworld==1) >   Use_OneDGrid_only_for_1d;

    // **********************************************************
    // The Interface Methods
    // **********************************************************

  public:

    //typedef OneDGridReferenceElement<dim> ReferenceElement;

    /** \brief The leaf iterator type  (currently only a level iterator)
     * \todo Replace this by a true leaf iterator */
    typedef OneDGridLevelIterator<0,dim,dimworld, All_Partition> LeafIterator;


    OneDGrid(const SimpleVector<OneDCType>& coords);

    OneDGrid();

    //! Desctructor
    ~OneDGrid() {}

    //! Return maximum level defined in this grid. Levels are numbered
    //! 0 ... maxlevel with 0 the coarsest level.
    int maxlevel() const {return vertices.size()-1;}

    //! Iterator to first entity of given codim on level
    template<int codim>
    OneDGridLevelIterator<codim,dim,dimworld, All_Partition> lbegin (int level) const;

    //! one past the end on this level
    template<int codim>
    OneDGridLevelIterator<codim,dim,dimworld, All_Partition> lend (int level) const;

#if 0
    //! Iterator to first entity of given codim on level
    template<int codim, PartitionIteratorType PiType>
    OneDGridLevelIterator<codim,dim,dimworld, PiType> lbegin (int level) const;

    //! one past the end on this level
    template<int codim, PartitionIteratorType PiType>
    OneDGridLevelIterator<codim,dim,dimworld, PiType> lend (int level) const;
#endif

    /** \brief Create leaf iterator  (currently only a level iterator)
     * \todo Replace this by a true leaf iterator */
    LeafIterator leafbegin (int level) const {return lbegin<0>(level);}

    /** \brief Create one past last on leaf level  (currently only a level iterator)
     * \todo Replace this by a true leaf iterator */
    LeafIterator leafend (int level) const {return lend<0>(level);}

    /** \brief Number of grid entities per level and codim
     */
    int size (int level, int codim) const {
      if (codim<0 || codim>1)
        DUNE_THROW(GridError, "There are no codim " << codim << " entities in a OneDGrid!");

      if (codim==0)
        return elements[level].size();

      return vertices[level].size();
    }

    //! Triggers the grid refinement process
    bool adapt();

    /** \brief Dynamic type identification */
    GridIdentifier type () { return OneDGrid_Id; }

    // **********************************************************
    // End of Interface Methods
    // **********************************************************

    /** \brief Does one uniform refinement step
     *
     * \param refCount I don't know what this is good for.  It doesn't
     *        actually do anything.
     */
    void globalRefine(int refCount);


  private:

    typedef DoubleLinkedList<OneDGridEntity<1,1,1> > VertexContainer;

    typedef DoubleLinkedList<OneDGridEntity<0,1,1> > ElementContainer;

    VertexContainer::Iterator getLeftUpperVertex(const ElementContainer::Iterator& eIt);

    VertexContainer::Iterator getRightUpperVertex(const ElementContainer::Iterator& eIt);

    /** \brief Returns an iterator the the first element on the left of
        the input element which has sons.
     */
    ElementContainer::Iterator getLeftNeighborWithSon(const ElementContainer::Iterator& eIt);

    // The vertices of the grid hierarchy
    std::vector<DoubleLinkedList<OneDGridEntity<1,1,1> > > vertices;

    // The elements of the grid hierarchy
    std::vector<DoubleLinkedList<OneDGridEntity<0,1,1> > > elements;


  }; // end Class OneDGrid

#include "onedgrid/onedgrid.cc"

#if 0  // to be implemented
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

#endif
