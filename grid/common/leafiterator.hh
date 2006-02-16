// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_LEAFITERATOR_HH
#define DUNE_GRID_LEAFITERATOR_HH

namespace Dune
{

  /**********************************************************************/
  /**
     @brief Enables iteration over all leaf entities
     of a codimension zero of a grid.

     @ingroup GridInterface
   */
  template<int codim, PartitionIteratorType pitype, class GridImp,
      template<int,PartitionIteratorType,class> class LeafIteratorImp>
  class LeafIterator :
    public EntityPointer<GridImp, LeafIteratorImp<codim,pitype,GridImp> >
  {
  public:
    typedef typename GridImp::template Codim<0>::Entity Entity;

    /** @brief Preincrement operator. */
    LeafIterator& operator++()
    {
      this->realIterator.increment();
      return *this;
    }

    //private:
    //! should not be used
    /** @brief Postincrement operator. */
    LeafIterator operator++(int) DUNE_DEPRECATED
    {
      const LeafIterator tmp(*this);
      this->realIterator.increment();
      return tmp;
    }

  public:
    /** @brief copy constructor from LevelIteratorImp */
    LeafIterator (const LeafIteratorImp<codim, pitype, const GridImp> & i) :
      EntityPointer<GridImp, LeafIteratorImp<codim, pitype, GridImp> >(i) {};
  };

  //**********************************************************************
  /**
     @brief Default Implementations for LevelIteratorImp

     @ingroup GridDevel
   */
  template<int codim, PartitionIteratorType pitype, class GridImp,
      template<int,PartitionIteratorType,class> class LeafIteratorImp>
  class LeafIteratorDefaultImplementation
  {
  private:
    //!  Barton-Nackman trick
    LeafIteratorImp<codim,pitype,GridImp>& asImp ()
    {return static_cast<LeafIteratorImp<codim,pitype,GridImp>&>(*this);}
    const LeafIteratorImp<codim,pitype,GridImp>& asImp () const
    {return static_cast<const LeafIteratorImp<codim,pitype,GridImp>&>(*this);}
  };

}

#endif // DUNE_GRID_LEAFITERATOR_HH
