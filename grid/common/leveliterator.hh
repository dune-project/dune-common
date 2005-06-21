// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_LEVELITERATOR_HH
#define DUNE_GRID_LEVELITERATOR_HH

#include "entitypointer.hh"

namespace Dune
{

  /**********************************************************************/
  /**
     @brief Enables iteration over all entities
     of a given codimension and level of a grid.

     @ingroup GridInterface
   */
  template<int codim, PartitionIteratorType pitype, class GridImp,
      template<int,PartitionIteratorType,class> class LevelIteratorImp>
  class LevelIterator :
    public EntityPointer<GridImp, LevelIteratorImp<codim,pitype,GridImp> >
  {
  public:
    typedef typename GridImp::template codim<codim>::Entity Entity;
    /**
       @brief Preincrement operator.

       @note Forwarded to LevelIteratorImp.increment()
     */
    LevelIterator& operator++()
    {
      this->realIterator.increment();
      return *this;
    }

    /**
       @brief Postincrement operator.

       @note Forwarded to LevelIteratorImp.increment()
     */
    LevelIterator operator++(int)
    {
      const LevelIterator tmp(*this);
      this->realIterator.increment();
      return tmp;
    }

    /**
       @brief copy constructor from LevelIteratorImp
     */
    LevelIterator(const LevelIteratorImp<codim,pitype,const GridImp> & i) :
      EntityPointer<GridImp, LevelIteratorImp<codim,pitype,GridImp> >(i) {};

  };

  /**********************************************************************/
  /**
     @brief Interface Definition for LevelIteratorImp

     @ingroup GridDevel
   */
  template<int codim, PartitionIteratorType pitype, class GridImp,
      template<int,PartitionIteratorType,class> class LevelIteratorImp>
  class LevelIteratorInterface
  {
  public:
    typedef typename GridImp::template codim<codim>::Entity Entity;

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
    LevelIteratorImp<codim,pitype,GridImp>& asImp ()
    {return static_cast<LevelIteratorImp<codim,pitype,GridImp>&>(*this);}
    const LevelIteratorImp<codim,pitype,GridImp>& asImp () const
    {return static_cast<const LevelIteratorImp<codim,pitype,GridImp>&>(*this);}
  };

  //**********************************************************************
  /**
     @brief Default Implementations for LevelIteratorImp

     @ingroup GridDevel
   */
  template<int codim, PartitionIteratorType pitype, class GridImp,
      template<int,PartitionIteratorType,class> class LevelIteratorImp>
  class LevelIteratorDefault
    : public LevelIteratorInterface <codim,pitype,GridImp,LevelIteratorImp>
  {
  private:
    //  Barton-Nackman trick
    LevelIteratorImp<codim,pitype,GridImp>& asImp () {
      return static_cast<LevelIteratorImp<codim,pitype,GridImp>&>(*this);
    }
    const LevelIteratorImp<codim,pitype,GridImp>& asImp () const {
      return static_cast<const LevelIteratorImp<codim,pitype,GridImp>&>(*this);
    }
  }; // end LevelIteratorDefault

}

#endif // DUNE_GRID_LEVELITERATOR_HH
