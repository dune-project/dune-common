// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_HIERARCHICITERATOR_HH
#define DUNE_GRID_HIERARCHICITERATOR_HH

namespace Dune {

  //************************************************************************
  // H I E R A R C H I C I T E R A T O R
  //************************************************************************

  /*! Mesh entities of codimension 0 ("elements") allow to visit all entities of
     codimension 0 obtained through nested, hierarchic refinement of the entity.
     Iteration over this set of entities is provided by the HierarchicIterator,
     starting from a given entity.
     This is redundant but important for memory efficient implementations of unstructured
     hierarchically refined meshes.
   */
  template<class GridImp, template<class> class HierarchicIteratorImp>
  class HierarchicIterator :
    public Dune::ForwardIteratorFacade<HierarchicIterator<GridImp,HierarchicIteratorImp>,
        typename GridImp::template codim<0>::Entity>
  {
  protected:
    HierarchicIteratorImp<GridImp> realIterator;
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;
    //! increment
    void increment()
    {
      realIterator.increment();
    }

    //! equality
    bool equals (const HierarchicIterator<GridImp,HierarchicIteratorImp>& i) const
    {
      return realIterator.equals(i.realIterator);
    }

    //! dereferencing
    Entity& dereference() const
    {
      return realIterator.dereference();
    }

    // copy constructor from HierarchicIteratorImp
    HierarchicIterator (const HierarchicIteratorImp<const GridImp> & i) :
      realIterator(i) {};
  };

  /*
     HierarchicIteratorInterface
   */
  template<class GridImp, template<class> class HierarchicIteratorImp>
  class HierarchicIteratorInterface
  {
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;

    //! increment
    void increment()
    {
      asImp().increment();
    }

    //! equality
    bool equals (const HierarchicIterator<GridImp,HierarchicIteratorImp>& i) const
    {
      return asImp().equals(i.realIterator);
    }

    //! dereferencing
    Entity& dereference() const
    {
      return asImp().dereference();
    }

  private:
    //!  Barton-Nackman trick
    HierarchicIteratorImp<GridImp>& asImp ()
    {return static_cast<HierarchicIteratorImp<GridImp>&>(*this);}
    const HierarchicIteratorImp<GridImp>& asImp () const
    {return static_cast<const HierarchicIteratorImp<GridImp>&>(*this);}
  };

  //***************************************************************************
  //
  // --HierarchicIteratorDefault
  //
  //! Default implementation of the HierarchicIterator.
  //! This class provides functionality which uses the interface of
  //! HierarchicIterator. For performance implementation the method of this
  //! class should be overloaded if a fast implementation can be done.
  //
  //***************************************************************************
  template<class GridImp, template<class> class HierarchicIteratorImp>
  class HierarchicIteratorDefault
    : public HierarchicIteratorInterface <GridImp,HierarchicIteratorImp>
  {
  private:
    //!  Barton-Nackman trick
    HierarchicIteratorImp<GridImp>& asImp ()
    {return static_cast<HierarchicIteratorImp<GridImp>&>(*this);}
    const HierarchicIteratorImp<GridImp>& asImp () const
    {return static_cast<const HierarchicIteratorImp<GridImp>&>(*this);}
  };

}

#endif // DUNE_GRID_HIERARCHICITERATOR_HH
