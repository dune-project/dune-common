// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "config.h"

#include <dune/grid/albertagrid.hh>
#include <dune/io/file/albertawriter.hh>

using namespace Dune;

int main()
{

  // ////////////////////////////////////////////
  //   Load a 2d test grid
  // ////////////////////////////////////////////
  AlbertaGrid<2,2> albertaGrid("alberta-testgrid-2-2.al");

  // ////////////////////////////////////////////
  //   Save it to a temporary file
  // ////////////////////////////////////////////
  AlbertaWriter<AlbertaGrid<2,2> >::writeGrid(albertaGrid, "temporaryGrid.al");

  // ////////////////////////////////////////////
  //   Check whether written grid is readable
  // ////////////////////////////////////////////
  AlbertaGrid<2,2> testGrid("temporaryGrid.al");

  // ////////////////////////////////////////////
  //   Remove temporary grid
  // ////////////////////////////////////////////
  system("rm temporaryGrid.al");

}
