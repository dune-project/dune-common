// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_LEVELITERATOR_HH
#define DUNE_GRID_LEVELITERATOR_HH

#include "entitypointer.hh"

namespace Dune
{

  template<int codim, PartitionIteratorType pitype, class GridImp,
      template<int,PartitionIteratorType,class> class LevelIteratorImp>
  class LevelIterator :
    public EntityPointer<GridImp, LevelIteratorImp<codim,pitype,GridImp> >
  {
  public:
    typedef typename GridImp::template codim<codim>::Entity Entity;
    /** @brief Preincrement operator. */
    LevelIterator& operator++()
    {
      this->realIterator.increment();
      return *this;
    }

    /** @brief Postincrement operator. */
    LevelIterator& operator++(int)
    {
      this->realIterator.operator++();
      return *this;
    }

    /** @brief copy constructor from LevelIteratorImp */
    LevelIterator(const LevelIteratorImp<codim,pitype,const GridImp> & i) :
      EntityPointer<GridImp, LevelIteratorImp<codim,pitype,GridImp> >(i) {};

  };

  //************************************************************************
  // L E V E L I T E R A T O R
  //************************************************************************

  /** \brief Enables iteration over all entities of a given codimension and level of a grid.
   */
  template<int codim, PartitionIteratorType pitype, class GridImp,
      template<int,PartitionIteratorType,class> class LevelIteratorImp>
  class LevelIteratorInterface
  {
  public:
    typedef typename GridImp::template codim<codim>::Entity Entity;

    //! define type used for coordinates in grid module
    typedef typename GridImp::ctype ctype;

    //! prefix increment
    void increment() { return asImp().increment(); }

  private:
    //!  Barton-Nackman trick
    LevelIteratorImp<codim,pitype,GridImp>& asImp ()
    {return static_cast<LevelIteratorImp<codim,pitype,GridImp>&>(*this);}
    const LevelIteratorImp<codim,pitype,GridImp>& asImp () const
    {return static_cast<const LevelIteratorImp<codim,pitype,GridImp>&>(*this);}
  };

  //**********************************************************************
  //
  //  --LevelIteratorDefault
  //
  //! Default implementation of LevelIterator.
  //
  //**********************************************************************
  template<int codim, PartitionIteratorType pitype, class GridImp,
      template<int,PartitionIteratorType,class> class LevelIteratorImp>
  class LevelIteratorDefault
    : public LevelIteratorInterface <codim,pitype,GridImp,LevelIteratorImp>
  {
  private:
    //!  Barton-Nackman trick
    LevelIteratorImp<codim,pitype,GridImp>& asImp () {
      return static_cast<LevelIteratorImp<codim,pitype,GridImp>&>(*this);
    }
    const LevelIteratorImp<codim,pitype,GridImp>& asImp () const {
      return static_cast<const LevelIteratorImp<codim,pitype,GridImp>&>(*this);
    }
  }; // end LevelIteratorDefault

}

#endif // DUNE_GRID_LEVELITERATOR_HH
