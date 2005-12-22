// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <fstream>

// ///////////////////////////////////////////////////////////
//   Write the leaf level of the grid as an Alberta file
// ///////////////////////////////////////////////////////////
template<class GridType>
void Dune::AlbertaWriter<GridType>::writeGrid(const GridType& grid,
                                              const std::string& filename)
{
  const int dim      = GridType::dimension;
  const int dimworld = GridType::dimensionworld;

  if ((dim!=2 && dim!=3) || dim != dimworld)
    DUNE_THROW(IOError, "You can only write grids in Alberta format if dim==dimworld==2"
               << " or dim==dimworld==3.");

  typedef typename GridType::Traits::LeafIndexSet IndexSetType;
  typedef typename IndexSetType::template Codim<0>::template Partition<All_Partition>::Iterator ElementIterator;
  typedef typename IndexSetType::template Codim<dim>::template Partition<All_Partition>::Iterator VertexIterator;

  const IndexSetType& indexSet = grid.leafIndexSet();

  // Make sure the grid contains only tetrahedra.
  if (indexSet.geomTypes(0).size()!=1 || indexSet.geomTypes(0)[0] != simplex)
    DUNE_THROW(IOError, "You can only write simplicial grids in Alberta format!");

  // ////////////////////////////////////////////////////
  //   Open output file
  // ////////////////////////////////////////////////////

  std::ofstream ofile(filename.c_str());
  /** \todo Make sure the file is really open! */

  // ////////////////////////////////////////////////////
  //   Write file header
  // ////////////////////////////////////////////////////

  ofile << "DIM: "           << dim << std::endl;
  ofile << "DIM_OF_WORLD: " << dimworld << std::endl;
  ofile << std::endl;
  ofile << "number of vertices: " << indexSet.size(dim) << std::endl;
  ofile << "number of elements: " << indexSet.size(0)   << std::endl;
  ofile << std::endl;

  // ///////////////////////////////////////////////////
  //   Write vertex positions
  // ///////////////////////////////////////////////////

  ofile << "vertex coordinates:" << std::endl;

  VertexIterator vIt    = grid.template leafbegin<dim>();
  VertexIterator vEndIt = grid.template leafend<dim>();

  for (; vIt!=vEndIt; ++vIt)
    ofile << vIt->geometry()[0] << std::endl;

  // ///////////////////////////////////////////////////
  //   Write elements
  // ///////////////////////////////////////////////////

  ofile << "element vertices:" << std::endl;

  ElementIterator eIt    = grid.template leafbegin<0>();
  ElementIterator eEndIt = grid.template leafend<0>();

  for (; eIt!=eEndIt; ++eIt) {

    for (int i=0; i<4; i++)
      ofile << indexSet.template subIndex<dim>(*eIt,i) << " ";

    ofile << std::endl;

  }

  // ///////////////////////////////////////////////////
  //   Write boundary ids
  // ///////////////////////////////////////////////////

  typedef typename GridType::template Codim<0>::Entity::IntersectionIterator NeighborIterator;

  for (eIt = grid.template leafbegin<0>(); eIt!=eEndIt; ++eIt) {

    NeighborIterator nIt    = eIt->ibegin();
    NeighborIterator nEndIt = eIt->iend();

    for (; nIt!=nEndIt; ++nIt)
      ofile << ((int)nIt.boundary()) << " ";

    ofile << std::endl;

  }

  std::cout << "Grid written successfully to: " << filename << std::endl;
}
