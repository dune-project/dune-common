// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_LEVELITERATOR_HH
#define DUNE_GRID_LEVELITERATOR_HH

#include <dune/common/iteratorfacades.hh>

namespace Dune
{

  template<int codim, PartitionIteratorType pitype, class GridImp,
      template<int,PartitionIteratorType,class> class LevelIteratorImp>
  class LevelIterator :
    public Dune::ForwardIteratorFacade<LevelIterator<codim,pitype,GridImp,LevelIteratorImp>,
        typename GridImp::template codim<codim>::Entity>
  {
  protected:
    LevelIteratorImp<codim,pitype,GridImp> realIterator;
  public:
    typedef typename GridImp::template codim<codim>::Entity Entity;
    //! dereferencing
    Entity & dereference() const
    {
      return realIterator.dereference();
    }
    //! prefix increment
    void increment() { return realIterator.increment(); }
    //! equality
    bool equals(const LevelIterator<codim,pitype,GridImp,LevelIteratorImp> & i) const
    {
      return realIterator.equals(i.realIterator);
    }
    //! ask for level of entity
    int level () const
    {
      return realIterator.level();
    }

    // copy constructor from LevelIteratorImp
    LevelIterator(const LevelIteratorImp<codim,pitype,const GridImp> & i) :
      realIterator(i) {};
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

    //! know your own codimension
    enum { codimension=codim };
    //! know your own dimension
    enum { dimension=GridImp::dimension };
    //! know your own dimension of world
    enum { dimensionworld=GridImp::dimensionworld };

    //! define type used for coordinates in grid module
    typedef typename GridImp::ctype ctype;

    //! prefix increment
    void increment() { return asImp().increment(); }
    //! equality
    bool equals(const LevelIteratorImp<codim,pitype,GridImp>& i) const
    {
      return asImp().equals(i);
    }
    //! dereferencing
    Entity& dereference() const { return asImp().dereference(); }

    //! ask for level of entity
    int level () const { return asImp().level(); }
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
