// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONE_D_GRID_HH
#define DUNE_ONE_D_GRID_HH

#include <vector>

#include <dune/common/fmatrix.hh>
#include <dune/common/misc.hh>
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
  template<int codim, int dim, class GridImp> class OneDGridEntity;
  template<int codim, PartitionIteratorType pitype, class GridImp> class OneDGridLevelIterator;

  template<int mydim, int coordworld, class GridImp>            class OneDGridGeometry;
  template<class GridImp>            class OneDGridHierarchicIterator;
  template<class GridImp>            class OneDGridIntersectionIterator;
  template<int dim, int dimworld>            class OneDGrid;

  template<int codim>                        class OneDGridLevelIteratorFactory;

  // singleton holding reference elements
  //template<int dim> struct UGGridReferenceElement;


}  // namespace Dune

#include "onedgrid/onedgridentity.hh"
#include "onedgrid/onedgridelement.hh"
#include "onedgrid/onedgridboundent.hh"
#include "onedgrid/onedintersectionit.hh"
#include "onedgrid/onedgridleveliterator.hh"
#include "onedgrid/onedgridhieriterator.hh"

#include "onedgrid/onedgridentity.cc"

namespace Dune {

  // A simple double linked list
  template<class T>
  class List
  {

  public:

    List() : numelements(0), begin(0), rbegin(0) {}

    int size() const {return numelements;}

    T* insert_after (T* i, T* t) {

      // Teste Eingabe
      if (i==0 && begin!=0)
        DUNE_THROW(DoubleLinkedListError, "invalid iterator for insert_after");

      // einfuegen
      if (begin==0) {
        // einfuegen in leere Liste
        begin = t;
        rbegin = t;
      }
      else
      {
        // nach Element i.p einsetzen
        t->pred_ = i;
        t->succ_ = i->succ_;
        i->succ_ = t;

        if (t->succ_!=0)
          t->succ_->pred_ = t;

        // tail neu ?
        if (rbegin==i)
          rbegin = t;
      }

      // Groesse und Rueckgabeiterator
      numelements = numelements+1;

      return t;
    }

    T* insert_before (T* i, T* t) {

      // Teste Eingabe
      if (i==0 && begin!=0)
        DUNE_THROW(DoubleLinkedListError,
                   "invalid iterator for insert_before");

      // einfuegen
      if (begin==0)
      {
        // einfuegen in leere Liste
        begin=t;
        rbegin=t;
      }
      else
      {
        // vor Element i.p einsetzen
        t->succ_ = i;
        t->pred_ = i->pred_;
        i->pred_ = t;

        if (t->pred_!=0)
          t->pred_->succ_ = t;
        // head neu ?
        if (begin==i)
          begin = t;
      }

      // Groesse und Rueckgabeiterator
      numelements = numelements+1;
      return t;
    }

    void remove (T* i)
    {
      // Teste Eingabe
      if (i==0)
        return;

      // Ausfaedeln
      if (i->succ_!=0)
        i->succ_->pred_ = i->pred_;
      if (i->pred_!=0)
        i->pred_->succ_ = i->succ_;

      // head & tail
      if (begin==i)
        begin=i->succ_;
      if (rbegin==i)
        rbegin = i->pred_;

      // Groesse
      numelements = numelements-1;
    }


    int numelements;

    T* begin;
    T* rbegin;

  };

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
  class OneDGrid : public GridDefault  < dim, dimworld,OneDCType, OneDGrid>
  {

    friend class OneDGridLevelIteratorFactory <0>;
    friend class OneDGridLevelIteratorFactory <1>;
    friend class OneDGridEntity <0,dim,OneDGrid>;
    friend class OneDGridEntity <dim,dim,OneDGrid>;
    friend class OneDGridHierarchicIterator<OneDGrid>;
    friend class OneDGridIntersectionIterator<OneDGrid>;

    template<int codim_, int dim_, class GridImp_, template<int,int,class> class EntityImp_>
    friend class Entity;

    /** \brief OneDGrid is only implemented for 1d */
    CompileTimeChecker< (dim==1 && dimworld==1) >   Use_OneDGrid_only_for_1d;


    // **********************************************************
    // The Interface Methods
    // **********************************************************

  public:
    typedef GridTraits<dim,dimworld,
        Dune::OneDGrid,
        OneDGridGeometry,
        OneDGridEntity,
        OneDGridBoundaryEntity,
        OneDGridLevelIterator,
        OneDGridIntersectionIterator,
        OneDGridHierarchicIterator> Traits;

    /** \brief Constructor with an explicit set of coordinates */
    OneDGrid(const SimpleVector<OneDCType>& coords);

    /** \brief Constructor for a uniform grid */
    OneDGrid(int numElements, double leftBoundary, double rightBoundary);

    OneDGrid();

    //! Desctructor
    ~OneDGrid();

    //! Return maximum level defined in this grid. Levels are numbered
    //! 0 ... maxlevel with 0 the coarsest level.
    int maxlevel() const {return vertices.size()-1;}

    //! Iterator to first entity of given codim on level
    template<int codim>
    typename Traits::template codim<codim>::LevelIterator lbegin (int level) const;

    //! one past the end on this level
    template<int codim>
    typename Traits::template codim<codim>::LevelIterator lend (int level) const;

#if 0
    //! Iterator to first entity of given codim on level
    template<int codim, PartitionIteratorType PiType>
    OneDGridLevelIterator<codim,dim,dimworld, PiType> lbegin (int level) const;

    //! one past the end on this level
    template<int codim, PartitionIteratorType PiType>
    OneDGridLevelIterator<codim,dim,dimworld, PiType> lend (int level) const;
#endif

    /** \brief Number of grid entities per level and codim
     */
    int size (int level, int codim) const {
      if (codim<0 || codim>1)
        DUNE_THROW(GridError, "There are no codim " << codim << " entities in a OneDGrid!");

      if (codim==0)
        return elements[level].size();

      return vertices[level].size();
    }

    /** \brief Mark entity for refinement
     *
     * \param refCount if >0 mark for refinement, if <0 mark for coarsening
     *
     * \return false, which is not compliant with the official specification!
     */
    bool mark(int refCount, typename Traits::template codim<0>::EntityPointer& e );

    //! Triggers the grid refinement process
    bool adapt();

    /** \brief Dynamic type identification */
    GridIdentifier type () { return OneDGrid_Id; }

    // **********************************************************
    // End of Interface Methods
    // **********************************************************

    /** \brief The different forms of grid refinement supported by OneDGrid */
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

    /** \brief Does one uniform refinement step
     *
     * \param refCount I don't know what this is good for.  It doesn't
     *        actually do anything.
     */
    void globalRefine(int refCount);


  private:

    template <int cd>
    OneDGridEntity<cd,dim,const OneDGrid>& getRealEntity(typename Traits::template codim<cd>::Entity& entity) {
      return entity.realEntity;
    }

    //! The type of grid refinement currently in use
    RefinementType refinementType_;

    OneDGridEntity<1,1,const OneDGrid<dim,dimworld> >* getLeftUpperVertex(const OneDGridEntity<0,1,const OneDGrid<dim,dimworld> >* eIt);

    OneDGridEntity<1,1,const OneDGrid<dim,dimworld> >* getRightUpperVertex(const OneDGridEntity<0,1,const OneDGrid<dim,dimworld> >* eIt);

    /** \brief Returns an iterator the the first element on the left of
        the input element which has sons.
     */
    OneDGridEntity<0,1,const OneDGrid<dim,dimworld> >* getLeftNeighborWithSon(OneDGridEntity<0,1,const OneDGrid<dim,dimworld> >* eIt);

    // The vertices of the grid hierarchy
    std::vector<List<OneDGridEntity<1,1,const OneDGrid<dim,dimworld> > > > vertices;

    // The elements of the grid hierarchy
    std::vector<List<OneDGridEntity<0,1,const OneDGrid<dim,dimworld> > > > elements;


  }; // end Class OneDGrid

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
