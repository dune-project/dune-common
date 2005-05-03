// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_HIERARCHICITERATOR_HH
#define DUNE_GRID_HIERARCHICITERATOR_HH

#include "entitypointer.hh"

namespace Dune {

  /**
     @brief Enables iteration over all codim zero entities
     in a subtree

     Mesh entities of codimension 0 ("elements") allow to visit all
     entities of codimension 0 obtained through nested, hierarchic
     refinement of the entity.  Iteration over this set of entities is
     provided by the HierarchicIterator, starting from a given entity.
     This is redundant but important for memory efficient
     implementations of unstructured hierarchically refined meshes.

     @ingroup GridInterface
   */
  template<class GridImp, template<class> class HierarchicIteratorImp>
  class HierarchicIterator :
    public EntityPointer<GridImp, HierarchicIteratorImp<GridImp> >
  {
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;
    /**
       @brief Preincrement operator.

       @note Forwarded to LevelIteratorImp.increment()
     */
    HierarchicIterator& operator++()
    {
      this->realIterator.increment();
      return *this;
    }

    /**
       @brief Postincrement operator.

       @note Forwarded to LevelIteratorImp.increment()
     */
    HierarchicIterator& operator++(int)
    {
      this->realIterator.operator++();
      return *this;
    }

    /**
       @brief copy constructor from HierarchicIteratorImp
     */
    HierarchicIterator (const HierarchicIteratorImp<const GridImp> & i) :
      EntityPointer<GridImp,HierarchicIteratorImp<GridImp> >(i) {};
  };

  /**********************************************************************/
  /**
     @brief Interface Definition for HierarchicIteratorImp

     @ingroup GridDevel
   */
  template<class GridImp, template<class> class HierarchicIteratorImp>
  class HierarchicIteratorInterface
  {
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;

    /**
       @brief coordinate type of this Grid
     */
    typedef typename GridImp::ctype ctype;

    /**
       @brief prefix increment

       implement this in LevelIteratorImp to increment your EntityPointerImp
     */
    void increment() { return asImp().increment(); }

  private:
    //  Barton-Nackman trick
    HierarchicIteratorImp<GridImp>& asImp ()
    {return static_cast<HierarchicIteratorImp<GridImp>&>(*this);}
    const HierarchicIteratorImp<GridImp>& asImp () const
    {return static_cast<const HierarchicIteratorImp<GridImp>&>(*this);}
  };

  //**********************************************************************
  /**
     @brief Default Implementations for HierarchicIteratorImp

     @ingroup GridDevel
   */
  template<class GridImp, template<class> class HierarchicIteratorImp>
  class HierarchicIteratorDefault
    : public HierarchicIteratorInterface <GridImp,HierarchicIteratorImp>
  {
  private:
    // Barton-Nackman trick
    HierarchicIteratorImp<GridImp>& asImp ()
    {return static_cast<HierarchicIteratorImp<GridImp>&>(*this);}
    const HierarchicIteratorImp<GridImp>& asImp () const
    {return static_cast<const HierarchicIteratorImp<GridImp>&>(*this);}
  };

}

#endif // DUNE_GRID_HIERARCHICITERATOR_HH
