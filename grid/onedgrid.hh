// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONE_D_GRID_HH
#define DUNE_ONE_D_GRID_HH

#include <vector>

#include <dune/common/misc.hh>
#include <dune/common/capabilities.hh>
#include <dune/grid/common/grid.hh>
#include <dune/common/simplevector.hh>


/** \file
 * \brief The OneDGrid class
 */

namespace Dune
{
  /** \brief The type used by to store coordinates */
  typedef double OneDCType;


  // forward declarations
  template<int codim, int dim, class GridImp> class OneDGridEntity;
  template<int codim, class GridImp> class OneDGridEntityPointer;
  template<int codim, PartitionIteratorType pitype, class GridImp> class OneDGridLevelIterator;

  template<int mydim, int coordworld, class GridImp>            class OneDGridGeometry;
  template<class GridImp>            class OneDGridHierarchicIterator;
  template<class GridImp>            class OneDGridIntersectionIterator;
  template<int dim, int dimworld>            class OneDGrid;

  template<int codim>                        class OneDGridLevelIteratorFactory;

}  // namespace Dune

#include "onedgrid/onedgridentity.hh"
#include "onedgrid/onedgridentitypointer.hh"
#include "onedgrid/onedgridgeometry.hh"
#include "onedgrid/onedintersectionit.hh"
#include "onedgrid/onedgridleveliterator.hh"
#include "onedgrid/onedgridleafiterator.hh"
#include "onedgrid/onedgridhieriterator.hh"
#include "onedgrid/onedgridindexsets.hh"

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

  template<int dim, int dimw>
  struct OneDGridFamily
  {
    typedef GridTraits<dim,dimw,Dune::OneDGrid<dim,dimw>,
        OneDGridGeometry,
        OneDGridEntity,
        OneDGridEntityPointer,
        OneDGridLevelIterator,
        OneDGridIntersectionIterator,
        OneDGridHierarchicIterator,
        OneDGridLeafIterator,
        OneDGridLevelIndexSet<const OneDGrid<dim,dimw> >,
        OneDGridLevelIndexSetTypes<const OneDGrid<dim,dimw> >,
        OneDGridLeafIndexSet<const OneDGrid<dim,dimw> >,
        OneDGridLeafIndexSetTypes<const OneDGrid<dim,dimw> >,
        OneDGridIdSet<const OneDGrid<dim,dimw> >,
        unsigned int,
        OneDGridIdSet<const OneDGrid<dim,dimw> >,
        unsigned int> Traits;
  };

  //**********************************************************************
  //
  // --OneDGrid
  //
  //**********************************************************************

  /**
     \brief [<em> provides \ref Dune::Grid </em>]
     Onedimensional adaptive grid
     \ingroup GridImplementations

     This implementation of the grid interface provides one-dimensional
     grids only.  No matter what the values of dim and dimworld may be,
     you'll always get a 1D-grid in a 1D-world.  Unlike SGrid, however,
     which can also be instantiated in 1D, the OneDGrid is nonuniform
     and provides local mesh refinement and coarsening.
   */
  template <int dim, int dimworld>
  class OneDGrid : public GridDefault <dim, dimworld,OneDCType,OneDGridFamily<dim,dimworld> >
  {

    friend class OneDGridLevelIteratorFactory <0>;
    friend class OneDGridLevelIteratorFactory <1>;
    friend class OneDGridEntity <0,dim,OneDGrid>;
    friend class OneDGridEntity <dim,dim,OneDGrid>;
    friend class OneDGridHierarchicIterator<OneDGrid>;
    friend class OneDGridIntersectionIterator<OneDGrid>;

    friend class OneDGridLevelIndexSet<const OneDGrid<dim,dimworld> >;
    friend class OneDGridLeafIndexSet<const OneDGrid<dim,dimworld> >;
    friend class OneDGridIdSet<const OneDGrid<dim,dimworld> >;

    template <int codim_, PartitionIteratorType PiType_, class GridImp_>
    friend class OneDGridLeafIterator;

    template<int codim_, int dim_, class GridImp_, template<int,int,class> class EntityImp_>
    friend class Entity;

    /** \brief OneDGrid is only implemented for 1d */
    CompileTimeChecker< (dim==1 && dimworld==1) >   Use_OneDGrid_only_for_1d;


    // **********************************************************
    // The Interface Methods
    // **********************************************************

  public:

    /** \brief Provides the standard grid types */
    typedef typename OneDGridFamily<dim,dimworld>::Traits Traits;

    /** \brief Constructor with an explicit set of coordinates */
    OneDGrid(const SimpleVector<OneDCType>& coords);

    /** \brief Constructor for a uniform grid */
    OneDGrid(int numElements, double leftBoundary, double rightBoundary);

    //! Destructor
    ~OneDGrid();

    /** \brief Return maximum level defined in this grid.

       Levels are numbered 0 ... maxlevel with 0 the coarsest level.
     */
    int maxLevel() const {return vertices.size()-1;}

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

    //! Iterator to first entity of given codim on leaf level
    template<int codim>
    typename Traits::template Codim<codim>::LeafIterator leafbegin () const;

    //! one past the end on leaf level
    template<int codim>
    typename Traits::template Codim<codim>::LeafIterator leafend () const;

    //! Iterator to first entity of given codim on level
    template<int codim, PartitionIteratorType PiType>
    typename Traits::template Codim<codim>::template Partition<PiType>::LeafIterator leafbegin() const {
      DUNE_THROW(NotImplemented, "Parallel leafbegin");
    }

    //! one past the end on this level
    template<int codim, PartitionIteratorType PiType>
    typename Traits::template Codim<codim>::template Partition<PiType>::LeafIterator leafend() const {
      DUNE_THROW(NotImplemented, "Parallel leafend");
    }

    /** \brief Number of grid entities per level and codim
     */
    int size (int level, int codim) const {
      if (codim<0 || codim>1)
        DUNE_THROW(GridError, "There are no codim " << codim << " entities in a OneDGrid!");

      if (codim==0)
        return elements[level].size();

      return vertices[level].size();
    }



    //! number of leaf entities per codim in this process
    int size (int codim) const
    {
      DUNE_THROW(NotImplemented, "not implemented");
      return 0;
    }

    //! number of entities per level, codim and geometry type in this process
    int size (int level, int codim, GeometryType type) const
    {
      DUNE_THROW(NotImplemented, "not implemented");
      return 0;
    }

    //! number of leaf entities per codim and geometry type in this process
    int size (int codim, GeometryType type) const
    {
      DUNE_THROW(NotImplemented, "not implemented");
      return 0;
    }

    /** \brief The processor overlap for parallel computing.  Always zero because
        this is a strictly sequential grid */
    int overlapSize(int codim) const {
      return 0;
    }

    /** \brief The processor ghost overlap for parallel computing.  Always zero because
        this is a strictly sequential grid */
    int ghostSize(int codim) const {
      return 0;
    }

    /** \brief The processor overlap for parallel computing.  Always zero because
        this is a strictly sequential grid */
    int overlapSize(int level, int codim) const {
      return 0;
    }

    /** \brief The processor ghost overlap for parallel computing.  Always zero because
        this is a strictly sequential grid */
    int ghostSize(int level, int codim) const {
      return 0;
    }

    /** \brief Get the set of global ids */
    const typename Traits::GlobalIdSet& globalIdSet() const
    {
      return idSet_;
    }

    /** \brief Get the set of local ids */
    const typename Traits::LocalIdSet& localIdSet() const
    {
      return idSet_;
    }

    /** \brief Get an index set for the given level */
    const typename Traits::LevelIndexSet& levelIndexSet(int level) const
    {
      if (! levelIndexSets_[level]) {
        levelIndexSets_[level] =
          new OneDGridLevelIndexSet<const OneDGrid<dim,dimworld> >;
        levelIndexSets_[level]->update(*this, level);
      }

      return * levelIndexSets_[level];
    }

    /** \brief Get an index set for the leaf level */
    const typename Traits::LeafIndexSet& leafIndexSet() const
    {
      return leafIndexSet_;
    }


    /** \brief Mark entity for refinement
     *
     * \param refCount if >0 mark for refinement, if <0 mark for coarsening
     *
     * \return false, which is not compliant with the official specification!
     */
    bool mark(int refCount, const typename Traits::template Codim<0>::EntityPointer& e );

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

    OneDGridIntersectionIterator<const OneDGrid<dim, dimworld> >&
    getRealIntersectionIterator(typename Traits::IntersectionIterator& it) {
      return it.realIterator;
    }

    const OneDGridIntersectionIterator<const OneDGrid<dim, dimworld> >&
    getRealIntersectionIterator(const typename Traits::IntersectionIterator& it) const {
      return it.realIterator;
    }


  private:

    /** \brief Update all indices and ids */
    void setIndices();

    template <int cd>
    OneDGridEntity<cd,dim,const OneDGrid>& getRealEntity(typename Traits::template Codim<cd>::Entity& entity) {
      return entity.realEntity;
    }

    template <int cd>
    const OneDGridEntity<cd,dim,const OneDGrid>& getRealEntity(const typename Traits::template Codim<cd>::Entity& entity) const {
      return entity.realEntity;
    }

    unsigned int getNextFreeId(int codim) {
      return (codim==0) ? freeElementIdCounter_++ : freeVertexIdCounter_;
    }

    //! The type of grid refinement currently in use
    RefinementType refinementType_;

    OneDEntityImp<0>* getLeftUpperVertex(const OneDEntityImp<1>* eIt);

    OneDEntityImp<0>* getRightUpperVertex(const OneDEntityImp<1>* eIt);

    /** \brief Returns an iterator the the first element on the left of
        the input element which has sons.
     */
    OneDEntityImp<1>* getLeftNeighborWithSon(OneDEntityImp<1>* eIt);

    // The vertices of the grid hierarchy
    std::vector<List<OneDEntityImp<0> > > vertices;

    // The elements of the grid hierarchy
    std::vector<List<OneDEntityImp<1> > > elements;

    // Our set of level indices
    mutable std::vector<OneDGridLevelIndexSet<const OneDGrid<dim,dimworld> >* > levelIndexSets_;

    OneDGridLeafIndexSet<const OneDGrid<dim,dimworld> > leafIndexSet_;

    OneDGridIdSet<const OneDGrid<dim,dimworld> > idSet_;

    unsigned int freeVertexIdCounter_;

    unsigned int freeElementIdCounter_;

  }; // end Class OneDGrid

  namespace Capabilities
  {

    template<int dim, int dimw, int cdim>
    struct hasEntity< OneDGrid<dim,dimw>, cdim >
    {
      static const bool v = true;
    };

#if 0  // to be implemented
    template<int dim,int dimw>
    struct hasLeafIterator< OneDGrid<dim,dimw> >
    {
      static const bool v = true;
    };

    template<int dim,int dimw>
    struct isParallel< OneDGrid<dim,dimw> >
    {
      static const bool v = true;
    };
#endif

  }

} // namespace Dune

#endif
