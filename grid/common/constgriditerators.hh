// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_CONSTGRIDITERATORS_HH__
#define __DUNE_CONSTGRIDITERATORS_HH__

#include <string>
#include <dune/common/matvec.hh>
#include <dune/common/exceptions.hh>

namespace Dune {

  /** @defgroup GridCommon Grid Interface

     These Iterators define wrapper for const versions
     of all interface grid iterators.

     @{
   */

  //************************************************************************
  // C O N S T  L E V E L I T E R A T O R
  //************************************************************************

  /** \brief Enables iteration over all entities of a given codimension and level of a grid.
   */
  template <class LevelIteratorImp>
  class ConstLevelIteratorWrapper
  {
    typedef ConstLevelIteratorWrapper<LevelIteratorImp> ConstLevelIteratorType;
  public:
    struct Traits
    {
      /** \todo Please doc me! */
      typedef typename LevelIteratorImp:: Traits :: CoordType CoordType;

      /** \todo Please doc me! */
      typedef const typename LevelIteratorImp :: Traits :: Entity Entity;

      //! Please doc me!
      typedef const typename LevelIteratorImp :: Traits :: LevelIterator LevelIterator;

      //! Please doc me!
      typedef const typename LevelIteratorImp :: Traits :: InteriorLevelIterator InteriorLevelIterator;

      //! Please doc me!
      typedef const typename LevelIteratorImp :: Traits :: InteriorBorderLevelIterator InteriorBorderLevelIterator;

      //! Please doc me!
      typedef const typename LevelIteratorImp :: Traits :: OverlapLevelIterator OverlapLevelIterator;

      //! Please doc me!
      typedef const typename LevelIteratorImp :: Traits :: OverlapFrontLevelIterator OverlapFrontLevelIterator;

      //! Please doc me!
      typedef const typename LevelIteratorImp :: Traits :: GhostLevelIterator GhostLevelIterator;
    };

    //! know your own codimension
    enum { codimension    = LevelIteratorImp :: codimension };

    //! know your own dimension
    enum { dimension      = LevelIteratorImp :: dimension };

    //! know your own dimension of world
    enum { dimensionworld = LevelIteratorImp :: dimensionworld };

    //! copy the normal LevelIterator, this is ok because the normal
    //! interface method lbegin uses copy aswell
    ConstLevelIteratorWrapper ( LevelIteratorImp & lit);

    //! prefix increment
    ConstLevelIteratorType & operator ++ ();

    //! equality
    bool operator== (const ConstLevelIteratorType & i) const;

    //! inequality
    bool operator!= (const ConstLevelIteratorType & i) const;

    //! dereferencing
    typename Traits :: Entity & operator*() const;

    //! arrow
    typename Traits :: Entity * operator->() const;

    //! ask for level of entity
    int level () const;
  protected:
    LevelIteratorImp it_;

  };

  template <class LevelIteratorImp>
  inline ConstLevelIteratorWrapper<LevelIteratorImp> ::
  ConstLevelIteratorWrapper( LevelIteratorImp & lit ) : it_ ( lit ) {}

  template <class LevelIteratorImp>
  inline ConstLevelIteratorWrapper<LevelIteratorImp> &
  ConstLevelIteratorWrapper<LevelIteratorImp> :: operator++()
  {
    ++it_;
    return (*this);
  }

  template <class LevelIteratorImp>
  inline bool ConstLevelIteratorWrapper<LevelIteratorImp> ::
  operator == ( const ConstLevelIteratorWrapper<LevelIteratorImp> & i) const
  {
    return it_ == i.it_;
  }

  template <class LevelIteratorImp>
  inline bool ConstLevelIteratorWrapper<LevelIteratorImp> ::
  operator != ( const ConstLevelIteratorWrapper<LevelIteratorImp> & i) const
  {
    return it_ != i.it_;
  }

  template <class LevelIteratorImp>
  inline typename ConstLevelIteratorWrapper<LevelIteratorImp> :: Traits :: Entity &
  ConstLevelIteratorWrapper<LevelIteratorImp> :: operator * () const
  {
    return *it_;
  }

  template <class LevelIteratorImp>
  inline typename ConstLevelIteratorWrapper<LevelIteratorImp> :: Traits :: Entity *
  ConstLevelIteratorWrapper<LevelIteratorImp> :: operator -> () const
  {
    return it_.operator -> ();
  }

  template <class LevelIteratorImp>
  inline int ConstLevelIteratorWrapper<LevelIteratorImp> ::
  level () const
  {
    return it_.level();
  }

  /** @} */

}


#endif
