// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_DOFMANAGER_HH__
#define __DUNE_DOFMANAGER_HH__

#include <dune/common/Stack.hh>
#include <dune/grid/albertgrid/agindex.hh>

namespace Dune {


  //************************************************************************
  //
  //  --LagrangeMapper
  //
  //! This Class knows what the space dimension is and how to map for a
  //! given grid entity from local dof number to global dof number
  //
  //************************************************************************
  template <int polOrd, int dimrange>
  class LagrangeMapper
    : public MapperDefault < LagrangeMapper <polOrd,dimrange> >
  {
    int numLocalDofs_;
  public:
    LagrangeMapper ( int numLocalDofs ) : numLocalDofs_ (numLocalDofs) {}

    template <class GridType>
    void resize( GridType & grid ) const {}
    void insert (int num) const {}
    void finish() const {}
    void isNew (int num) const {}

    //! default is Lagrange with polOrd = 1
    template <class GridType>
    int size (const GridType &grid , int level ) const
    {
      // return number of vertices * dimrange
      return (dimrange*grid.size( level , GridType::dimension ));
    };

    //! map Entity an local Dof number to global Dof number
    //! for Lagrange with polOrd = 1
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      enum { codim = EntityType::dimension };
      // Gaussklammer
      int locNum = (int) localNum / dimrange;
      int locDim = localNum % dimrange;

      // get global vertex number
      return (dimrange*en.template subIndex<codim>(locNum)) + locDim;
    };

  };

  template <int dimrange>
  class LagrangeMapper<0,dimrange>
    : public MapperDefault < LagrangeMapper <0,dimrange> >
  {
    mutable SerialIndexSet indexSet_;
  public:
    LagrangeMapper ( int numDofs ) {}

    template <class GridType>
    void resize( GridType & grid ) const
    {
      indexSet_.resize( grid.global_size(0) );
    }

    //! default is Lagrange with polOrd = 0
    template <class GridType>
    int size (const GridType &grid , int level ) const
    {
      // return number of vertices
      //return dimrange*grid.size( level , 0 );
      //return indexSet_.size();
      return grid.global_size(0);
    }

    void insert (int num) const
    {
      indexSet_.insert(num);
    }

    void finish() const
    {
      indexSet_.finish();
    }

    void isNew (int num) const
    {
      return indexSet_.isNew(num);
    }

    //! map Entity an local Dof number to global Dof number
    //! for Lagrange with polOrd = 0
    template <class EntityType>
    int mapToGlobal (EntityType &en, int localNum ) const
    {
      // get global vertex number
      //return indexSet_[en.global_index()];
      return en.global_index();
    };

  };

} // end namespace Dune

#endif
