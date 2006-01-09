// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <fstream>

// ///////////////////////////////////////////////////////////
//   Write the leaf level of the grid as an Alberta file
// ///////////////////////////////////////////////////////////
template<class GridType>
void Dune::ALUGridWriter<GridType>::writeGrid(const GridType& grid,
                                              const std::string filename, int precision)
{
  const int dim      = GridType::dimension;
  const int dimworld = GridType::dimensionworld;

  assert( (dim==3) && (dim == dimworld));
  //     DUNE_THROW(IOError, "You can only write grids in ALUGrid format if dim==dimworld==3");

  typedef typename GridType::Traits::LeafIndexSet IndexSetType;
  const IndexSetType& indexSet = grid.leafIndexSet();

  // Make sure the grid contains only tetrahedra.
  //if (indexSet.geomTypes(0).size()!=1 || (indexSet.geomTypes(0)[0] != simplex)
  //    || (indexSet.geomTypes(0)[0] != cube) )
  //    DUNE_THROW(IOError, "You can only write simplicial or cube grids in ALUGrid format!");

  // ////////////////////////////////////////////////////
  //   Open output file
  // ////////////////////////////////////////////////////

  std::ofstream ofile(filename.c_str());
  //assert
  //if(!ofile)
  //  DUNE_THROW(IOError, "Couldn't open file '" << filename );

  // ////////////////////////////////////////////////////
  //   Write file header
  // ////////////////////////////////////////////////////
  if(indexSet.geomTypes(0)[0] == simplex)
  {
    writeTetra(grid,precision,ofile);
    ofile.close();
    std::cout << "ALUGridWriter::write: Grid written successfully to: " << filename << std::endl
              << std::endl;
    return;
  }

  if(indexSet.geomTypes(0)[0] == cube)
  {
    writeCube(grid,precision,ofile);
    ofile.close();
    std::cout << "ALUGridWriter::write: Grid written successfully to: " << filename << std::endl
              << std::endl;
    return;
  }

  DUNE_THROW(IOError,"GeometryType not supported by ALUGrid");
  return;
}

template<class GridType>
void Dune::ALUGridWriter<GridType>::
writeTetra(const GridType& grid,int precision,
           std::ostream & ofile)
{
  ofile << "!Tetraeder" << std::endl << std::endl;
  ofile.precision(precision);

  // ///////////////////////////////////////////////////
  //   Write vertex positions
  // ///////////////////////////////////////////////////
  typedef typename GridType::Traits::LeafIndexSet IndexSetType;
  const IndexSetType & indexSet = grid.leafIndexSet();

  const int dimworld = GridType::dimensionworld;
  typedef typename GridType::ctype coordType;

  {
    int vxsize = indexSet.size(dim);
    ofile << vxsize << std::endl;

    // write coordinates of the vertices
    std::vector< FieldVector<coordType,dim> > vxvec ( vxsize );

    typedef typename IndexSetType::template Codim<dim>::template Partition<All_Partition>::Iterator VertexIterator;
    VertexIterator vEndIt  = indexSet.template end<dim,All_Partition>();
    for(VertexIterator vIt = indexSet.template begin<dim,All_Partition>();
        vIt!=vEndIt; ++vIt)
    {
      int vxidx = indexSet.index( *vIt );
      FieldVector<coordType,dim> &v = vxvec[vxidx];
      const FieldVector<coordType,dim> & p = vIt->geometry()[0];
      for(int i=0; i<dimworld; i++) v[i] = p[i];
    }

    // write vertices to file
    for(int i=0; i<vxsize; i++)
    {
      ofile << vxvec[i] << std::endl;
    }
    ofile << std::endl;
  }

  // ///////////////////////////////////////////////////
  //   Write elements
  // ///////////////////////////////////////////////////
  typedef typename IndexSetType::template Codim<0>::template Partition<All_Partition>::Iterator ElementIterator;

  ofile << indexSet.size(0) << std::endl;
  ElementIterator eEndIt  = indexSet.template end  <0,All_Partition>();
  for(ElementIterator eIt = indexSet.template begin<0,All_Partition>();
      eIt!=eEndIt; ++eIt)
  {
    // only works for pure simplex grids
    assert( eIt->geometry().type() == simplex );
    for(int i=0; i<eIt->template count<dim>(); ++i)
    {
      ofile << indexSet.template subIndex<dim>(*eIt,i) << "  ";
    }
    ofile << std::endl;
  }
  ofile << std::endl;

  // ///////////////////////////////////////////////////
  //   Write boundary faces
  // /////////////////////////////////////////////////////

  typedef typename GridType::template Codim<0>::Entity::IntersectionIterator NeighborIterator;
  typedef typename GridType::template Codim<0>::Entity Entity;
  typedef typename GridType::ctype coordType;

  // count boundary faces
  int bndFaces = 0;
  for(ElementIterator eIt = indexSet.template begin<0,All_Partition>();
      eIt!=eEndIt; ++eIt)
  {
    const Entity & en = *eIt;

    NeighborIterator nEndIt = en.iend();
    for(NeighborIterator nIt = en.ibegin();
        nIt!=nEndIt; ++nIt)
    {
      if(nIt.boundary())
      {
        bndFaces++;
      }
    }
  }
  ofile << bndFaces << std::endl;

  for(ElementIterator eIt = indexSet.template begin<0,All_Partition>();
      eIt!=eEndIt; ++eIt)
  {
    const Entity & en = *eIt;
    const ReferenceElement< coordType, dim > & refElem =
      ReferenceElements< coordType, dim >::general(en.geometry().type());

    NeighborIterator nEndIt = en.iend();
    for(NeighborIterator nIt = en.ibegin();
        nIt!=nEndIt; ++nIt)
    {
      if(nIt.boundary())
      {
        int bndId = (nIt.boundaryId() < 0) ? nIt.boundaryId() : -nIt.boundaryId();
        int subEntity = nIt.numberInSelf();
        int vxSize = refElem.size(subEntity,1,dim);
        ofile << bndId << "  " << vxSize << "  ";
        for(int i=0; i<vxSize; ++i)
        {
          int vx = refElem.subEntity(subEntity , 1 , i , dim );
          int index = indexSet.template subIndex<dim> (en,vx);
          ofile << index << "  ";
        }
        ofile << std::endl;
      }
    }
  }
  ofile << std::endl;

  // ///////////////////////////////////////////////////
  //   Vertices parallel identification
  // /////////////////////////////////////////////////////

  for (int i=0; i<indexSet.size(dim); ++i)
    ofile << i << " -1" << std::endl;

}

template<class GridType>
void Dune::ALUGridWriter<GridType>::
writeCube(const GridType& grid, int precision,
          std::ostream & ofile)
{
  ofile << "!Hexaeder" << std::endl << std::endl;
  ofile.precision(precision);

  // ///////////////////////////////////////////////////
  //   Write vertex positions
  // ///////////////////////////////////////////////////
  typedef typename GridType::Traits::LeafIndexSet IndexSetType;
  const IndexSetType & indexSet = grid.leafIndexSet();

  const int dimworld = GridType::dimensionworld;
  typedef typename GridType::ctype coordType;

  {
    int vxsize = indexSet.size(dim);
    ofile << vxsize << std::endl;

    // write coordinates of the vertices
    std::vector< FieldVector<coordType,dim> > vxvec ( vxsize );

    typedef typename IndexSetType::template Codim<dim>::template Partition<All_Partition>::Iterator VertexIterator;
    VertexIterator vEndIt  = indexSet.template end<dim,All_Partition>();
    for(VertexIterator vIt = indexSet.template begin<dim,All_Partition>();
        vIt!=vEndIt; ++vIt)
    {
      int vxidx = indexSet.index( *vIt );
      FieldVector<coordType,dim> &v = vxvec[vxidx];
      const FieldVector<coordType,dim> & p = vIt->geometry()[0];
      for(int i=0; i<dimworld; i++) v[i] = p[i];
    }

    // write vertices to file
    for(int i=0; i<vxsize; i++)
    {
      ofile << vxvec[i] << std::endl;
    }
    ofile << std::endl;
  }

  // ///////////////////////////////////////////////////
  //   Write elements
  // ///////////////////////////////////////////////////
  typedef typename IndexSetType::template Codim<0>::template Partition<All_Partition>::Iterator ElementIterator;

  // same as ElementTopologyMapping<hexa>::dune2aluVertex in
  // dune/grid/alu3dgrid/topology.hh
  const int vxMap[8] = {0, 1, 3, 2, 4, 5, 7, 6};

  ofile << indexSet.size(0) << std::endl;
  ElementIterator eEndIt  = indexSet.template end  <0,All_Partition>();
  for(ElementIterator eIt = indexSet.template begin<0,All_Partition>();
      eIt!=eEndIt; ++eIt)
  {
    // only works for pure simplex cube grids
    assert( eIt->geometry().type() == cube );
    for(int i=0; i<eIt->template count<dim>(); ++i)
    {
      // see alu3dgrid/topology.* for definition
      int vx = vxMap[i];
      //int vx = ElementTopologyMapping<hexa>::dune2aluVertex(i);
      ofile << indexSet.template subIndex<dim>(*eIt,vx) << "  ";
    }
    ofile << std::endl;
  }
  ofile << std::endl;

  // ///////////////////////////////////////////////////
  //   Write boundary faces
  // /////////////////////////////////////////////////////

  typedef typename GridType::template Codim<0>::Entity::IntersectionIterator NeighborIterator;
  typedef typename GridType::template Codim<0>::Entity Entity;
  typedef typename GridType::ctype coordType;

  // count boundary faces
  int bndFaces = 0;
  for(ElementIterator eIt = indexSet.template begin<0,All_Partition>();
      eIt!=eEndIt; ++eIt)
  {
    const Entity & en = *eIt;

    NeighborIterator nEndIt = en.iend();
    for(NeighborIterator nIt = en.ibegin();
        nIt!=nEndIt; ++nIt)
    {
      if(nIt.boundary())
      {
        bndFaces++;
      }
    }
  }
  ofile << bndFaces << std::endl;

  for(ElementIterator eIt = indexSet.template begin<0,All_Partition>();
      eIt!=eEndIt; ++eIt)
  {
    const Entity & en = *eIt;
    const ReferenceElement< coordType, dim > & refElem =
      ReferenceElements< coordType, dim >::general(en.geometry().type());

    NeighborIterator nEndIt = en.iend();
    for(NeighborIterator nIt = en.ibegin();
        nIt!=nEndIt; ++nIt)
    {
      if(nIt.boundary())
      {
        int bndId = (nIt.boundaryId() < 0) ? nIt.boundaryId() : -nIt.boundaryId();
        int subEntity = nIt.numberInSelf();
        int vxSize = refElem.size(subEntity,1,dim);
        ofile << bndId << "  " << vxSize << "  ";

        // vertices 2 and 3 have to be spawed
        const int dune2aluVx[4] = {0, 1, 3, 2};
        // same as ElementTopologyMapping<hexa>::faceOrientation in
        // dune/grid/alu3dgrid/topology.hh
        const int faceOrientation[6] = {-1, 1, 1, -1, -1, 1};
        assert( vxSize == 4 );

        // see alu3dgrid/topology.* for definition
        if(faceOrientation[subEntity] < 0)
        ///ElementTopologyMapping<hexa>::faceOrientation(subEntity) < 0)
        {
          for(int i=0; i<vxSize; i++)
          {
            int j = dune2aluVx[i];
            int vx = refElem.subEntity(subEntity , 1 , j , dim );
            int index = indexSet.template subIndex<dim> (en,vx);
            ofile << index << "  ";
          }
        }
        else
        {
          for(int i=vxSize-1; i>=0; i--)
          {
            int j = dune2aluVx[i];
            int vx = refElem.subEntity(subEntity , 1 , j , dim );
            int index = indexSet.template subIndex<dim> (en,vx);
            ofile << index << "  ";
          }
        }
        ofile << std::endl;
      }
    }
  }
  ofile << std::endl;

  // ///////////////////////////////////////////////////
  //   Vertices parallel identification
  // /////////////////////////////////////////////////////

  for (int i=0; i<indexSet.size(dim); ++i)
    ofile << i << " -1" << std::endl;

  return ;
}
