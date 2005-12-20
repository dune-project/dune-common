// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_TWISTUTILITY_HH
#define DUNE_TWISTUTILITY_HH

#include <dune/grid/alu3dgrid.hh>
#include <dune/grid/albertagrid.hh>
#include <dune/grid/sgrid.hh>

namespace Dune {

  // for structured grids, the twist is always zero
  // ? is this correct
  template <class GridImp>
  class TwistUtility
  {
  public:
    typedef GridImp GridType;
    typedef typename GridImp::Traits::IntersectionIterator IntersectionIterator;
  public:
    TwistUtility(const GridType& grid) :
      grid_(grid)
    {}

    int twistInSelf(IntersectionIterator& it) const {
      //DUNE_THROW(NotImplemented, "Hey, implement it yourself if you need it!");
      return 0;
    }

    int twistInNeighbor(IntersectionIterator& it) const {
      //DUNE_THROW(NotImplemented, "Hey, implement it yourself if you need it!");
      return 0;
    }

  private:
    const GridType& grid_;
  };


  template <int dim, int dimW>
  class TwistUtility<AlbertaGrid<dim, dimW> >
  {
  public:
    typedef AlbertaGrid<dim, dimW> GridType;
    typedef typename GridType::Traits::IntersectionIterator IntersectionIterator;
  public:
    TwistUtility(const GridType& grid) :
      grid_(grid)
    {}

    int twistInSelf(IntersectionIterator& it) const {
      DUNE_THROW(NotImplemented, "Hey, implement it yourself if you need it!");
      return 0;
    }

    int twistInNeighbor(IntersectionIterator& it) const {
      DUNE_THROW(NotImplemented, "Hey, implement it yourself if you need it!");
      return 0;
    }

  private:
    const GridType& grid_;
  };


  template <int dim, int dimW, ALU3dGridElementType elType>
  class TwistUtility<ALU3dGrid<dim, dimW, elType>  >
  {
  public:
    typedef ALU3dGrid<dim, dimW, elType> GridType;
    typedef typename GridType::Traits::IntersectionIterator IntersectionIterator;
  public:
    TwistUtility(const GridType& grid) :
      grid_(grid)
    {}

    int twistInSelf(const IntersectionIterator& it) const {
      return grid_.getRealIntersectionIterator(it).twistInSelf();
    }

    int twistInNeighbor(const IntersectionIterator& it) const {
      return grid_.getRealIntersectionIterator(it).twistInNeighbor();
    }

  private:
    TwistUtility(const TwistUtility&);
    TwistUtility& operator=(const TwistUtility&);

  private:
    const GridType& grid_;
  };
}

#endif
