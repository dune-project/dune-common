// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DOFSTORAGE_HH__
#define __DUNE_DOFSTORAGE_HH__

namespace Dune {

  //*********************************************************************
  //
  //  --DofStorageInterface
  //
  //! Interface for the DofStorage classes. The functionality described
  //! here mus be imlpemented by the Class DofStorageImp. The DofStorageImp
  //! class must be derived from the DofStorageDefault Class. It is
  //! possible to overload the functionality provided by the
  //! DofStorageDefault Class. If it's not overloaded then it will work but
  //! perhaps not efficiently.
  //
  //*********************************************************************
  template <class LocalDofIteratorType, class GlobalDofIteratorType,
      class DofStorageImp>
  class DofStorageInterface
  {
  public:

    //! remember the types of LocalDofIterator and GlobalDofIterator
    //! and the implementation class
    struct Traits
    {
      typedef LocalDofIteratorType LocalDofIterator;
      typedef GlobalDofIteratorType GlobalDofIterator;
      typedef DofStorageImp DofStorage;
    };

    //! return a LocalDofIterator for Entity en, which points
    //! to the first dof of Entity en
    template <class EntityType>
    LocalDofIteratorType beginLocal (EntityType &en)
    {
      return asImp().beginLocal(en);
    };

    //! return a LocalDofIterator for Entity en, which points
    //! behind the last  dof of Entity en
    template <class EntityType>
    LocalDofIteratorType endLocal (EntityType &en)
    {
      return asImp().endLocal(en);
    };

    //! return a GlobalDofIterator for a given grid which points to the
    //! first global dof. The worst case is, that we have to do a grid run
    //! through to go over all dofs
    template <class GridType>
    GlobalDofIteratorType beginGlobal (GridType &grid)
    {
      return asImp().beginGlobal(grid);
    };

    //! return a GlobalDofIterator for a given grid which points behind the
    //! last global dof
    template <class GridType>
    GlobalDofIteratorType endGlobal (GridType &grid)
    {
      return asImp().endGlobal(grid);
    };

  private:
    //! Barton-Nackman trick
    DofStorageImp &asImp() { return static_cast<DofStorageImp&>(*this); }
    const DofStorageImp &asImp() const
    { return static_cast<const DofStorageImp&>(*this); }
  };

  //***********************************************************************
  //
  //  --DofStorageDefault
  //
  //!  The default interface which is used in applications.
  //!  Provides the default implementation for random access to
  //!  global and local dofs
  //
  //***********************************************************************
  template <class DofType, class LocalDofIteratorType, class GlobalDofIteratorType,
      class DofStorageImp>
  class DofStorageDefault
    : public DofStorageInterface < LocalDofIteratorType ,
          GlobalDofIteratorType , DofStorageImp >
  {
  public:

    //! Default implementation of random access to the global dofs
    //! this method can be overloaded in derived class for efficient
    //! implementation
    template <class GridType>
    DofType& global (GridType &grid, int level, int globalNumber)
    {
      GlobalDofIteratorType endit = endGlobal(grid);
      for(GlobalDofIteratorType it = beginGlobal(grid); it != endit; ++it)
      {
        if(it.index() == globalNumber)
          return (*it);
      }
      std::cerr << "DofStorageDefault::global : globalDofNumber out of range ! \n";
      GlobalDofIteratorType it = beginGlobal();
      return (*it);
    };

    //! default implementation of random access to the local dofs
    //! this method can be overloaded in derived class for efficient
    //! implementation
    template <class EntityType>
    DofType& local (EntityType &en, int localNum )
    {
      LocalDofIteratorType endit = endLocal(en);
      for(LocalDofIteratorType it = beginLocal(en); it != endit; ++it)
      {
        if(it.index() == localNum)
          return (*it);
      }
      std::cerr << "DofStorageDefault::local : localDofNumber out of range ! \n";
      LocalDofIteratorType it = beginLocal(en);
      return (*it);
    };

  };

  //*************************************************************************
  //
  //  --LocalDofIteratorInterface
  //
  //! Interface for LocalDofIterator
  //! The functionality defined here must be implemented by the user, i.e
  //! in his LocalDofIteratorImp Class.
  //
  //*************************************************************************
  template <class DofType, class LocalDofIteratorImp>
  class LocalDofIteratorInterface
  {
  public:
    //! remember template parameters
    struct Traits
    {
      typedef DofType Dof;
      typedef LocalDofIteratorImp LocalDofIterator;
    };

    //! return reference to dof
    DofType& operator *()
    {
      return asImp().operator *();
    };

    //! return local dof number of dof
    int index ()  {  return asImp().index();   };

    //! go to next dof
    LocalDofIteratorImp& operator++ ()
    {
      return asImp().operator ++ ();
    };

    //! go to next i steps
    LocalDofIteratorImp& operator++ (int i)
    {
      return asImp().operator ++ (i);
    };

    //! compare with other LocalDofIterators
    bool operator == (const LocalDofIteratorImp& I)
    {
      return asImp().operator == (I);
    };

    //! compare with other LocalDofIterators
    bool operator != (const LocalDofIteratorImp& I)
    {
      return asImp().operator != (I);
    };

  private:
    //! Barton-Nackman trick
    LocalDofIteratorImp &asImp() { return static_cast<LocalDofIteratorImp&>(*this); }
    const LocalDofIteratorImp &asImp() const
    { return static_cast<const LocalDofIteratorImp&>(*this); }
  };

  //************************************************************************
  //
  //  --LocalDofIteratorDefault
  //
  //! Default implementation for some extra functionality for
  //! LocalDofIterator. At the moment there is no extra functionality but
  //! perhaps later.
  //
  //************************************************************************
  template <class DofType, class LocalDofIteratorImp>
  class LocalDofIteratorDefault
    : public LocalDofIteratorInterface < DofType , LocalDofIteratorImp >
  {
  public:
    //! no extra functionality at this moment
  };


  //************************************************************************
  //
  //  --GlobalDofIteratorInterface
  //
  //! Interface for GlobalDofIterator
  //! The functionality defined here must be implemented by the user, i.e
  //! in his GlobalDofIteratorImp Class.
  //
  //***********************************************************************
  template <class DofType, class GlobalDofIteratorImp>
  class GlobalDofIteratorInterface
  {
  public:
    //! return reference to dof
    DofType& operator *()
    {
      return asImp().operator *();
    };

    //! return global dof number of dof
    int index ()  {  return asImp().index();   };

    //! go to next dof
    GlobalDofIteratorImp& operator++ ()
    {
      return asImp().operator ++ ();
    };

    //! go to next i steps
    GlobalDofIteratorImp& operator++ (int i)
    {
      return asImp().operator ++ (i);
    };

    //! compare with other GlobalDofIterators
    bool operator == (const GlobalDofIteratorImp& I)
    {
      return asImp().operator == (I);
    };

    //! compare with other GlobalDofIterators
    bool operator != (const GlobalDofIteratorImp& I)
    {
      return asImp().operator != (I);
    };

  private:
    //! Barton-Nackman trick
    GlobalDofIteratorImp &asImp()
    {
      return static_cast<GlobalDofIteratorImp&>(*this);
    };
    const GlobalDofIteratorImp &asImp() const
    {
      return static_cast<const GlobalDofIteratorImp&>(*this);
    };
  };
  //************************************************************************
  //
  //  --GlobalDofIteratorDefault
  //
  //! Default implementation for some extra functionality for
  //! GlobalDofIterator. At the moment there is no extra functionality but
  //! perhaps later.
  //
  //************************************************************************
  template <class DofType, class GlobalDofIteratorImp>
  class GlobalDofIteratorDefault
    : public GlobalDofIteratorInterface < DofType , GlobalDofIteratorImp >
  {
  public:
    //! no extra functionality at this moment
  }; // end class GlobalDofIteratorDefault


}  // end namespace Dune

#endif
