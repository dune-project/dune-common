// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_HIERARCHICITERATOR_HH
#define DUNE_GRID_HIERARCHICITERATOR_HH

#include "entitypointer.hh"

namespace Dune {

  /**
     @brief Enables iteration over all codim zero entities
     in a subtree
     See also the documentation of Dune::EntityPointer.

     Mesh entities of codimension 0 ("elements") allow to visit all
     entities of codimension 0 obtained through nested, hierarchic
     refinement of the entity.  Iteration over this set of entities is
     provided by the HierarchicIterator, starting from a given entity.
     This is redundant but important for memory efficient
     implementations of unstructured hierarchically refined meshes.

     @ingroup GIEntityPointer
   */
  template<class GridImp, template<class> class HierarchicIteratorImp>
  class HierarchicIterator :
    public EntityPointer<GridImp, HierarchicIteratorImp<GridImp> >
  {
  public:
    typedef typename GridImp::template Codim<0>::Entity Entity;
    /**
       @brief Preincrement operator.

       @note Forwarded to LevelIteratorImp.increment()
     */
    HierarchicIterator& operator++()
    {
      this->realIterator.increment();
      return *this;
    }

    //private:
    //! should not be used
    /**
       @brief Postincrement operator.

       @note Forwarded to LevelIteratorImp.increment()
     */
    HierarchicIterator operator++(int) DUNE_DEPRECATED
    {
      const HierarchicIterator tmp(*this);
      this->realIterator.increment();
      return tmp;
    }

  public:
    //===========================================================
    /** @name Implementor interface
     */
    //@{
    //===========================================================

    /** @brief copy constructor from HierarchicIteratorImp
     */
    HierarchicIterator (const HierarchicIteratorImp<const GridImp> & i) :
      EntityPointer<GridImp,HierarchicIteratorImp<GridImp> >(i) {};
    //@}
  };

  //**********************************************************************
  /**
     @brief Default Implementations for HierarchicIteratorImp

     @ingroup GridDevel
   */
  template<class GridImp, template<class> class HierarchicIteratorImp>
  class HierarchicIteratorDefaultImplementation
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
