// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_HIERARCHICSEARCH_HH
#define DUNE_GRID_HIERARCHICSEARCH_HH

#include <dune/common/fvector.hh>
#include <dune/grid/common/grid.hh>

namespace Dune
{

  template<class Grid, class IS>
  class HierarchicSearch
  {
    //! get dimension from the grid
    enum {dim=Grid::dimension};

    //! get world dimension from the grid
    enum {dimw=Grid::dimensionworld};

    //! get coord type from the grid
    typedef typename Grid::ctype ct;

    //! get entity from the grid
    typedef typename Grid::template Codim<0>::Entity Entity;

    //! type of EntityPointer
    typedef typename Grid::template Codim<0>::EntityPointer EntityPointer;

    //! type of LevelIterator
    typedef typename Grid::template Codim<0>::LevelIterator LevelIterator;

    //! type of HierarchicIterator
    typedef typename Grid::template Codim<0>::HierarchicIterator HierarchicIterator;

    EntityPointer hFindEntity(const EntityPointer e,
                              const FieldVector<ct,dimw>& global) const
    {
      // loop over all child Entities
      HierarchicIterator it = e->hbegin(e->level()+1);
      HierarchicIterator end = e->hend(e->level()+1);
      for (; it != end; ++it)
      {
        FieldVector<ct,dim> local = it->geometry().local(global);
        if (it->geometry().checkInside(local))
        {
          // return if we found the leaf
          if (is.contains(*it)) return it;
          // else search through the child entites
          else return hFindEntity(it, global);
        }
      }
      DUNE_THROW(Exception, "Unexpected internal Error");
    }

  public:
    HierarchicSearch(const Grid & _g, const IS & _is) : g(_g), is(_is) {};

    EntityPointer findEntity(const FieldVector<ct,dimw>& global) const
    {
      // loop over macro level
      LevelIterator it = g.template lbegin<0>(0);
      LevelIterator end = g.template lend<0>(0);
      for (; it != end; ++it)
      {
        FieldVector<ct,dim> local = it->geometry().local(global);
        if (it->geometry().checkInside(local))
        {
          // return if we found the leaf
          if (is.contains(*it)) return it;
          // else search through the child entites
          else return hFindEntity(it, global);
        }
      }
      DUNE_THROW(GridError,
                 "Coordinate " << global << " is outside the grid");
    }

  private:
    const Grid& g;
    const IS& is;
  };

}; // end namespace Dune

#endif // DUNE_GRID_HIERARCHICSEARCH_HH
