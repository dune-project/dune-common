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
  template<class GridImp, template<class> class LeafIteratorImp>
  class LeafIterator :
    public EntityPointer<GridImp, LeafIteratorImp<GridImp> >
  {
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;

    /** @brief Preincrement operator. */
    LeafIterator& operator++()
    {
      this->realIterator.increment();
      return *this;
    }

    /** @brief Postincrement operator. */
    LeafIterator operator++(int)
    {
      const LeafIterator tmp(*this);
      this->realIterator.increment();
      return tmp;
    }

    /** @brief copy constructor from LevelIteratorImp */
    LeafIterator (const LeafIteratorImp<const GridImp> & i) :
      EntityPointer<GridImp, LeafIteratorImp<GridImp> >(i) {};
  };

  /**********************************************************************/
  /**
     @brief Interface Definition for LeafIteratorImp

     @ingroup GridDevel
   */
  template<class GridImp, template<class> class LeafIteratorImp>
  class LeafIteratorInterface
  {
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;

    //! increment
    void increment()
    {
      asImp().increment();
    }

  private:
    //!  Barton-Nackman trick
    LeafIteratorImp<GridImp>& asImp ()
    {return static_cast<LeafIteratorImp<GridImp>&>(*this);}
    const LeafIteratorImp<GridImp>& asImp () const
    {return static_cast<const LeafIteratorImp<GridImp>&>(*this);}
  };

  //**********************************************************************
  /**
     @brief Default Implementations for LevelIteratorImp

     @ingroup GridDevel
   */
  template<class GridImp, template<class> class LeafIteratorImp>
  class LeafIteratorDefault
    : public LeafIteratorInterface <GridImp,LeafIteratorImp>
  {
  private:
    //!  Barton-Nackman trick
    LeafIteratorImp<GridImp>& asImp ()
    {return static_cast<LeafIteratorImp<GridImp>&>(*this);}
    const LeafIteratorImp<GridImp>& asImp () const
    {return static_cast<const LeafIteratorImp<GridImp>&>(*this);}
  };

  //**********************************************************************
  /**
     @brief Generic Implementation of a LeafIteratorImp

     @ingroup GridDevel
   */
  template<class GridImp>
  class GenericLeafIterator :
    public GridImp::template codim<0>::EntityPointer::base,
    public LeafIteratorDefault<GridImp, GenericLeafIterator>
  {
    friend class GenericLeafIterator<const GridImp>;
    typedef typename GridImp::template codim<0>::EntityPointer::base EntityPointerImp;
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef typename GridImp::template codim<0>::LevelIterator LevelIterator;
    typedef typename GridImp::template codim<0>::HierarchicIterator HierarchicIterator;
    //! increment
    void increment()
    {
      while(true)
      {
        if (hit != hend) ++hit;
        if (hit == hend)
        {
          if (lit != lend) ++lit;
          if (lit == lend)
          {
            // end iterator is lend<0>(0)
            static_cast<EntityPointerImp&>(*this) = lit.realIterator;
            return;
          }
          else
          {
            hit = lit->hbegin(maxlevel);
            hend = lit->hend(maxlevel);
            if (hit == hend)
            {
              // level 0 is leaf
              static_cast<EntityPointerImp&>(*this) = lit.realIterator;
              return;
            }
          }
        }
        if(hit->level() == maxlevel)
        {
          // assign hit to this
          static_cast<EntityPointerImp&>(*this) = hit.realIterator;
          return;
        }
        if(hit->isLeaf())
        {
          // assign hit to this
          static_cast<EntityPointerImp&>(*this) = hit.realIterator;
          return;
        }
      }
    }

    GenericLeafIterator(GridImp & g, int maxl, bool end) :
      EntityPointerImp(g.template lbegin<0>(0).realIterator),
      grid(g), maxlevel(maxl),
      lit(grid.template lbegin<0>(0)),
      lend(grid.template lend<0>(0)),
      hit(lit->hbegin(maxlevel)), // wird hier wirklich die Reihenfolge eingehalten ?
      hend(lit->hend(maxlevel))
    {
      if (end)
      {
        // end iterator is lend<0>(0)
        lit = grid.template lend<0>(0);
        static_cast<EntityPointerImp&>(*this) = lit.realIterator;
        return;
      }
      if (hit != hend)
      {
        // find first leaf
        if (hit->level() != maxlevel && !hit->isLeaf())         // PB 25.04.05: Added this, otherwise grids with one level do not work!
          increment();

        // assign
        static_cast<EntityPointerImp&>(*this) = hit.realIterator;
      }
      else
      {
        // level 0 is leaf
        static_cast<EntityPointerImp&>(*this) = lit.realIterator;
      }
      return;
    }

    GenericLeafIterator(const GenericLeafIterator & rhs) :
      EntityPointerImp(rhs),
      grid(rhs.grid), maxlevel(rhs.maxlevel),
      lit(rhs.lit), lend(rhs.lend),
      hit(rhs.hit), hend(rhs.hend)
    {}

  private:
    GridImp & grid;
    const int maxlevel;
    LevelIterator lit;
    LevelIterator lend;
    HierarchicIterator hit;
    HierarchicIterator hend;
  };

}

#endif // DUNE_GRID_LEAFITERATOR_HH
