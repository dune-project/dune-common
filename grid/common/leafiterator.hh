// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_LEAFITERATOR_HH
#define DUNE_GRID_LEAFITERATOR_HH

namespace Dune
{

  template<class GridImp, template<class> class LeafIteratorImp>
  class LeafIterator :
    public Dune::ForwardIteratorFacade<LeafIterator<GridImp,LeafIteratorImp>,
        typename GridImp::template codim<0>::Entity>
  {
  protected:
    LeafIteratorImp<GridImp> realIterator;
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;

    //! increment
    void increment()
    {
      realIterator.increment();
    }

    //! equality
    bool equals (const LeafIterator<GridImp,LeafIteratorImp>& i) const
    {
      return realIterator.equals(i.realIterator);
    }

    //! dereferencing
    Entity& dereference() const
    {
      return realIterator.dereference();
    }
    //! ask for level of entity
    int level () const
    {
      return realIterator.level();
    }

    // copy constructor from LeafIteratorImp
    LeafIterator (const LeafIteratorImp<const GridImp> & i) : realIterator(i) {};
  };

  /*
     LeafIteratorInterface
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

    //! equality
    bool equals (const LeafIterator<GridImp,LeafIteratorImp>& i) const
    {
      return asImp().equals(i.realIterator);
    }

    //! dereferencing
    Entity& dereference() const
    {
      return asImp().dereference();
    }

    //! ask for level of entity
    int level () const { return asImp().level(); }
  private:
    //!  Barton-Nackman trick
    LeafIteratorImp<GridImp>& asImp ()
    {return static_cast<LeafIteratorImp<GridImp>&>(*this);}
    const LeafIteratorImp<GridImp>& asImp () const
    {return static_cast<const LeafIteratorImp<GridImp>&>(*this);}
  };

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

  template<class GridImp>
  class GenericLeafIterator : public LeafIteratorDefault<GridImp, GenericLeafIterator>
  {
    friend class GenericLeafIterator<const GridImp>;
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef typename GridImp::template codim<0>::LevelIterator LevelIterator;
    typedef typename GridImp::template codim<0>::HierarchicIterator HierarchicIterator;
    //! increment
    void increment()
    {
      while(true)
      {
        ++hit;
        if (hit == hend)
        {
          ++lit;
          if (lit != lend)
          {
            hit = lit->hbegin(maxlevel);
            hend = lit->hend(maxlevel);
          }
        }
        if(hit->level() == maxlevel) break;
        if(hit->isLeaf()) break;
      }
    }
    //! equality
    bool equals (const GenericLeafIterator<GridImp>& i) const
    {
      if (lit == lend) return (lit == i.lit);
      if (i.lit == i.lend) return (lit == i.lit);
      return (lit == i.lit) && (hit == i.hit);
    }
    //! dereferencing
    Entity& dereference() const
    {
      return *hit;
    }
    //! ask for level of entity
    int level () const
    {
      return hit->level();
    }

    GenericLeafIterator(GridImp & g, int maxl, bool end) :
      grid(g), maxlevel(maxl),
      lit(grid.template lbegin<0>(0)),
      lend(grid.template lend<0>(0)),
      hit(lit->hbegin(maxlevel)),
      hend(lit->hend(maxlevel))
    {
      if (end)
      {
        lit = grid.template lend<0>(0);
      }
    }

    //   GenericLeafIterator(const GenericLeafIterator<typename RemoveConst<GridImp>::Type> & i) :
    //     grid(i.grid), maxlevel(i.maxlevel),
    //     lit(i.lit), lend(i.lend), hit(i.hit), hend(i.hend)
    //     {}

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
