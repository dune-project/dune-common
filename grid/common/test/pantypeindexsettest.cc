// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

/** \file
    \brief A unit test for the class PanTypeIndexSet
 */

#include <config.h>

#include <iostream>
#include <set>

#include <dune/grid/uggrid.hh>
#include <dune/io/file/amirameshreader.hh>
#include <dune/grid/common/pantypeindexset.hh>

using namespace Dune;

template <class GridType, class HostIndexSet>
void checkPanTypeIndexSet(const PanTypeIndexSet<GridType,HostIndexSet>& panTypeIndexSet,
                          const HostIndexSet& hostIndexSet)
{
  // //////////////////////////////////////////////////////////////
  //   Check whether the size methods give the correct result
  // //////////////////////////////////////////////////////////////


  for (int codim=0; codim<=GridType::dimension; codim++) {
    assert(panTypeIndexSet.geomTypes(codim) == hostIndexSet.geomTypes(codim));
    assert(panTypeIndexSet.size(codim) == hostIndexSet.size(codim));

    for (int i=0; i<panTypeIndexSet.geomTypes(codim).size(); i++) {
      NewGeometryType type = panTypeIndexSet.geomTypes(codim)[i];
      assert(panTypeIndexSet.size(codim,type) == hostIndexSet.size(codim,type));
    }

  }

  // ///////////////////////////////////////////////////////////////////////////
  //   Check whether the index for codim 0 is consecutive and zero starting
  // ///////////////////////////////////////////////////////////////////////////
  typedef typename PanTypeIndexSet<GridType,HostIndexSet>::template Codim<0>::template Partition<All_Partition>::Iterator Iterator;

  Iterator eIt    = panTypeIndexSet.template begin<0,All_Partition>();
  Iterator eEndIt = panTypeIndexSet.template end<0,All_Partition>();

  int min = 1;
  int max = 0;
  std::set<int> indices;

  for (; eIt!=eEndIt; ++eIt) {

    int index = panTypeIndexSet.template index<0>(*eIt);

    min = std::min(min, index);
    max = std::max(max, index);

    std::pair<std::set<int>::iterator, bool> status = indices.insert(index);

    if (!status.second)       // not inserted because already existing
      DUNE_THROW(GridError, "PanTypeIndex is not unique!");
  }

  if (min!=0)
    DUNE_THROW(GridError, "PanTypeIndex is not starting from zero!");

  if (max!=panTypeIndexSet.size(0)-1)
    DUNE_THROW(GridError, "PanTypeIndex is not consecutive!");


  // ///////////////////////////////////////////////////////////////////////////
  //   Check whether the index for codim 1 is consecutive and zero starting
  // ///////////////////////////////////////////////////////////////////////////

  eIt = panTypeIndexSet.template begin<0,All_Partition>();

  // Reset the counting variables
  min = 1;
  max = 0;
  indices.clear();

  // Currently there are no iterators over codim-1 entities.  Therefore we have to
  // simulate them with an element iterator and an intersection iterator.

  for (; eIt!=eEndIt; ++eIt) {

    typedef typename GridType::template Codim<0>::Entity::IntersectionIterator IntersectionIterator;

    IntersectionIterator iIt    = eIt->ibegin();
    IntersectionIterator iEndIt = eIt->iend();

    for (; iIt!=iEndIt; ++iIt) {

      // The correctness of the pan-type index for codim 0 has been checked above.
      // So now we can use it.
      if (iIt.neighbor()
          && panTypeIndexSet.template index<0>(*eIt) > panTypeIndexSet.template index<0>(*iIt.outside()))
        continue;

      int index = panTypeIndexSet.template subIndex<1>(*eIt, iIt.numberInSelf());

      //             std::cout << hostIndexSet.template subIndex<1>(*eIt, iIt.numberInSelf())
      //                 << "  Index: " << index << "   type: " << eIt->geometry().type() << std::endl;
      min = std::min(min, index);
      max = std::max(max, index);

      std::pair<std::set<int>::iterator, bool> status = indices.insert(index);

      if (!status.second)         // not inserted because already existing
        DUNE_THROW(GridError, "PanTypeIndex is not unique!");

    }

  }

  if (min!=0)
    DUNE_THROW(GridError, "PanTypeIndex for codim 1 is not starting from zero!");

  if (max!=panTypeIndexSet.size(1)-1)
    DUNE_THROW(GridError, "PanTypeIndex for codim 1 is not consecutive!");

}

/*
   PanTypeIndexSets only do something helpful on grids with more than one
   element type.  So far only UGGrids do this, so we use them to test
   the index set.
 */

int main () try
{

  // ////////////////////////////////////////////////////////////////////////
  //  Do the standard grid test for a 2d UGGrid
  // ////////////////////////////////////////////////////////////////////////

  // extra-environment to check destruction
  {
    typedef UGGrid<2,2> GridType;
    typedef GridType::Traits::LeafIndexSet LeafIndexSetType;
    typedef GridType::Traits::LevelIndexSet LevelIndexSetType;

    GridType grid;
    AmiraMeshReader<GridType>::read(grid, "../../test/ug-testgrid-2.am");

    // create hybrid grid
    grid.mark(1,grid.leafbegin<0>());
    grid.adapt();
    grid.globalRefine(1);

    //       PanTypeIndexSet<GridType, LeafIndexSetType> panTypeLeafIndexSet(grid, grid.leafIndexSet());
    //       checkPanTypeIndexSet(panTypeLeafIndexSet, grid.leafIndexSet());

    for (int i=2; i<=grid.maxLevel(); i++) {
      PanTypeIndexSet<GridType, LevelIndexSetType> panTypeLevelIndexSet(grid, grid.levelIndexSet(i));
      checkPanTypeIndexSet(panTypeLevelIndexSet, grid.levelIndexSet(i));
    }

  }

  // ////////////////////////////////////////////////////////////////////////
  //  Do the standard grid test for a 3d UGGrid
  // ////////////////////////////////////////////////////////////////////////
  {
    typedef UGGrid<3,3> GridType;
    typedef GridType::Traits::LeafIndexSet LeafIndexSetType;
    typedef GridType::Traits::LevelIndexSet LevelIndexSetType;

    GridType grid;
    AmiraMeshReader<GridType>::read(grid, "../../test/ug-testgrid-3.am");

    // create hybrid grid
    grid.mark(1,grid.leafbegin<0>());
    grid.adapt();
    grid.globalRefine(1);

    PanTypeIndexSet<GridType, LeafIndexSetType> panTypeLeafIndexSet(grid, grid.leafIndexSet());
    checkPanTypeIndexSet(panTypeLeafIndexSet, grid.leafIndexSet());

    for (int i=0; i<=grid.maxLevel(); i++) {
      PanTypeIndexSet<GridType, LevelIndexSetType> panTypeLevelIndexSet(grid, grid.levelIndexSet(i));
      checkPanTypeIndexSet(panTypeLevelIndexSet, grid.levelIndexSet(i));
    }

  }

  return 0;

}
catch (Exception &e) {
  std::cerr << e << std::endl;
  return 1;
} catch (...) {
  std::cerr << "Generic exception!" << std::endl;
  return 2;
}
