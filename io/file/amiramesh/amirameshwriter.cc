// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <amiramesh/AmiraMesh.h>

#include <algorithm>

#include <dune/istl/bvector.hh>

#if _3
/** \todo Make sure that the grid is three-dimensional */
template<class GridType>
void Dune::AmiraMeshWriter<GridType>::writeGrid(const GridType& grid,
                                                const std::string& filename)
{
  // Temporary:  we write this level
  int level = grid.maxlevel();

  // Find out whether the grid contains only tetrahedra.  If yes, then
  // it is written in TetraGrid format.  If not, it is written in
  // hexagrid format.
  bool containsOnlyTetrahedra = true;

  typedef typename GridType::template codim<0>::LevelIterator ElementIterator;

  ElementIterator eIt    = grid.template lbegin<0>(level);
  ElementIterator eEndIt = grid.template lend<0>(level);

  for (; eIt!=eEndIt; ++eIt) {
    if (eIt->geometry().type() != tetrahedron) {
      containsOnlyTetrahedra = false;
      break;
    }
  }

  int maxVerticesPerElement = (containsOnlyTetrahedra) ? 4 : 8;

  const int DIM = 3;

  int noOfNodes = grid.size(level, 3);
  int noOfElem  = grid.size(level, 0);

  int i;

  // create amiramesh object
  AmiraMesh am;

  // write grid vertex coordinates
  AmiraMesh::Location* geo_nodes = new AmiraMesh::Location("Nodes", noOfNodes);
  am.insert(geo_nodes);

  AmiraMesh::Data* geo_node_data = new AmiraMesh::Data("Coordinates", geo_nodes,
                                                       McPrimType::mc_float, DIM);
  am.insert(geo_node_data);

  typedef typename GridType::template codim<3>::LevelIterator VertexIterator;
  VertexIterator vertex    = grid.template lbegin<DIM>(level);
  VertexIterator endvertex = grid.template lend<DIM>(level);

  for (; vertex!=endvertex; ++vertex) {

    int index = vertex->index();
    const FieldVector<double, 3>& coords = vertex->geometry()[0];

    ((float*)geo_node_data->dataPtr())[3*index+0] = coords[0];
    ((float*)geo_node_data->dataPtr())[3*index+1] = coords[1];
    ((float*)geo_node_data->dataPtr())[3*index+2] = coords[2];

  }

#if 0
  // handle materials
  HxParamBundle* materials = new HxParamBundle("Materials");

  for (k=0; k<=maxSubDom; k++) {

    char buffer[100];
    sprintf(buffer, "Material%d", k);
    HxParamBundle* newMaterial = new HxParamBundle(buffer);

    HxParameter* newId = new HxParameter("Id", k);
    newMaterial->insert(newId);

    materials->insert(newMaterial);

  }

  am_geometry.parameters.insert(materials);
#endif

  /* write element section to file */
  AmiraMesh::Location* element_loc = NULL;

  if (containsOnlyTetrahedra)
    element_loc = new AmiraMesh::Location("Tetrahedra", noOfElem);
  else
    element_loc = new AmiraMesh::Location("Hexahedra", noOfElem);

  am.insert(element_loc);

  AmiraMesh::Data* element_data = new AmiraMesh::Data("Nodes", element_loc,
                                                      McPrimType::mc_int32, maxVerticesPerElement);
  am.insert(element_data);

  int *dPtr = (int*)element_data->dataPtr();

  eIt    = grid.template lbegin<0>(level);

  if (containsOnlyTetrahedra) {

    for (i=0; eIt!=eEndIt; ++eIt, i++) {

      for (int j=0; j<4; j++)
        dPtr[i*4+j] = eIt->template subIndex<3>(j)+1;

    }

  } else {

    for (i=0; eIt!=eEndIt; ++eIt, i++) {
      switch (eIt->geometry().type()) {

      case hexahedron : {

        const int hexaReordering[8] = {0, 1, 3, 2, 4, 5, 7, 6};
        for (int j=0; j<8; j++)
          dPtr[8*i + j] = eIt->template subIndex<3>(hexaReordering[j])+1;

        break;
      }

      case prism : {
        const int prismReordering[8] = {0, 1, 1, 2, 3, 4, 4, 5};
        for (int j=0; j<8; j++)
          dPtr[8*i + j] = eIt->template subIndex<3>(prismReordering[j])+1;

        break;
      }

      case pyramid : {
        const int pyramidReordering[8] = {0, 1, 2, 3, 4, 4, 4, 4};
        for (int j=0; j<8; j++)
          dPtr[8*i + j] = eIt->template subIndex<3>(pyramidReordering[j])+1;

        break;
      }

      case tetrahedron : {

        const int tetraReordering[8] = {0, 1, 2, 2, 3, 3, 3, 3};
        for (int j=0; j<8; j++)
          dPtr[8*i + j] = eIt->template subIndex<3>(tetraReordering[j])+1;

        break;
      }

      default :
        DUNE_THROW(NotImplemented, "Unknown element type encountered");

      }

    }

  }

  // write material section to grid file
  AmiraMesh::Data* element_materials = new AmiraMesh::Data("Materials", element_loc, McPrimType::mc_uint8, 1);
  am.insert(element_materials);

  //    for(i=0; i<noOfElem; i++)
  //        ((unsigned char*)element_materials->dataPtr())[i] = SUBDOMAIN(elemList[i]);

  for(i=0; i<noOfElem; i++)
    ((unsigned char*)element_materials->dataPtr())[i] = 0;

  if(!am.write(filename.c_str(), 1))
    DUNE_THROW(IOError, "Writing geometry file failed!");

  std::cout << "Grid written successfully to: " << filename << std::endl;
}


#endif  // #if _3


#ifdef _2
template<class GridType>
void Dune::AmiraMeshWriter<GridType>::writeGrid(const GridType& grid,
                                                const std::string& filename)
{

  // Temporary:  we write this level
  int level = grid.maxlevel();

  // Find out whether the grid contains only triangles.  If yes, then
  // it is written as a HxTriangularGrid.  If not, it cannot be
  // written (so far).
  bool containsOnlyTetrahedra = true;

  typename GridType::template codim<0>::LevelIterator element = grid.template lbegin<0>(level);
  typename GridType::template codim<0>::LevelIterator end     = grid.template lend<0>(level);

  for (; element!=end; ++element) {
    if (element->geometry().type() != triangle) {
      containsOnlyTetrahedra = false;
      break;
    }
  }

  int maxVerticesPerElement = (containsOnlyTetrahedra) ? 3 : 4;

  const int DIM = 2;

  int noOfNodes = grid.size(level, DIM);
  int noOfElem  = grid.size(level, 0);

  int i;

  // Construct the name for the file
  std::string geoFilename(filename);
  geoFilename += ".am";

  // create amiramesh object
  AmiraMesh am_geometry;

  // Set the appropriate content type
  am_geometry.parameters.set("ContentType", "HxTriangularGrid");

  // write grid vertex coordinates
  AmiraMesh::Location* geo_nodes = new AmiraMesh::Location("Nodes", noOfNodes);
  am_geometry.insert(geo_nodes);

  AmiraMesh::Data* geo_node_data = new AmiraMesh::Data("Coordinates", geo_nodes,
                                                       McPrimType::mc_float, DIM);
  am_geometry.insert(geo_node_data);


  typename GridType::template codim<2>::LevelIterator vertex    = grid.template lbegin<DIM>(level);
  typename GridType::template codim<2>::LevelIterator endvertex = grid.template lend<DIM>(level);

  for (; vertex!=endvertex; ++vertex)
  {
    int index = vertex->index();
    const FieldVector<double, DIM>& coords = vertex->geometry()[0];

    ((float*)geo_node_data->dataPtr())[2*index+0] = coords[0];
    ((float*)geo_node_data->dataPtr())[2*index+1] = coords[1];

  }

#if 0
  // handle materials
  HxParamBundle* materials = new HxParamBundle("Materials");

  for (k=0; k<=maxSubDom; k++) {

    char buffer[100];
    sprintf(buffer, "Material%d", k);
    HxParamBundle* newMaterial = new HxParamBundle(buffer);

    HxParameter* newId = new HxParameter("Id", k);
    newMaterial->insert(newId);

    materials->insert(newMaterial);

  }

  am_geometry.parameters.insert(materials);

  ncomp = 0;
  for(i=0; i<NVECTYPES; i++)
    ncomp = std::max(ncomp,VD_NCMPS_IN_TYPE(sol, i));
#endif

  /* write element section to geo - file */
  AmiraMesh::Location* element_loc = NULL;

  if (containsOnlyTetrahedra)
    element_loc = new AmiraMesh::Location("Triangles", noOfElem);
  else
    element_loc = new AmiraMesh::Location("Quadrangles", noOfElem);

  am_geometry.insert(element_loc);

  AmiraMesh::Data* element_data = new AmiraMesh::Data("Nodes", element_loc,
                                                      McPrimType::mc_int32, maxVerticesPerElement);
  am_geometry.insert(element_data);

  int *dPtr = (int*)element_data->dataPtr();

  typename GridType::template codim<0>::LevelIterator element2   = grid.template lbegin<0>(level);
  typename GridType::template codim<0>::LevelIterator endelement = grid.template lend<0>(level);

  for (i=0; element2!=endelement; ++element2, i++) {
    switch (element2->geometry().type()) {

    default :

      for (int j=0; j<element2->geometry().corners(); j++)
        dPtr[i*maxVerticesPerElement+j] = element2->template subIndex<DIM>(j)+1;

      // If the element has less than 8 vertices use the last value
      // to fill up the remaining slots
      for (int j=element2->geometry().corners(); j<maxVerticesPerElement; j++)
        dPtr[i*maxVerticesPerElement+j] = dPtr[i*maxVerticesPerElement+element2->geometry().corners()-1];
    }

  }

  // write material section to geo-file
  AmiraMesh::Data* element_materials = new AmiraMesh::Data("Materials", element_loc, McPrimType::mc_uint8, 1);
  am_geometry.insert(element_materials);

  for(i=0; i<noOfElem; i++)
    ((unsigned char*)element_materials->dataPtr())[i] = 0;


  if(!am_geometry.write(geoFilename.c_str(), 1))
    DUNE_THROW(IOError, "Writing geometry file failed");


  std::cout << "Grid written successfully to: " << geoFilename << std::endl;
}
#endif // #ifdef _2

template<class GridType>
template<class DiscFuncType>
void Dune::AmiraMeshWriter<GridType>::writeFunction(const DiscFuncType& f,
                                                    const std::string& filename)
{
  const int noOfNodes = f.getFunctionSpace().size();

  // temporary hack
  const int ncomp = 1;

  // Create AmiraMesh object
  AmiraMesh am;

  // Set the appropriate content type for 2D grid data
  if (GridType::dimension==2)
    am.parameters.set("ContentType", "HxTriangularData");

  AmiraMesh::Location* sol_nodes = new AmiraMesh::Location("Nodes", noOfNodes);
  am.insert(sol_nodes);

  AmiraMesh::Data* nodeData = new AmiraMesh::Data("Data", sol_nodes, McPrimType::mc_double, ncomp);
  am.insert(nodeData);

  AmiraMesh::Field* nodeField = new AmiraMesh::Field(f.name().c_str(), ncomp, McPrimType::mc_double,
                                                     AmiraMesh::t_linear, nodeData);
  am.insert(nodeField);


  // write the data into the AmiraMesh object
  typedef typename DiscFuncType::ConstDofIteratorType DofIterator;
  DofIterator dit = f.dbegin();
  DofIterator ditend = f.dend();

  int i=0;
  for (; dit!=ditend; ++dit, i++) {

    ((double*)nodeData->dataPtr())[i] = *dit;

  }

  // actually save the solution file
  // (the 1 means: ascii)
  if (!am.write(filename.c_str(), 1) )
    DUNE_THROW(IOError, "An error has occured writing file " << filename);

  std::cout << "Solution written successfully to: " << filename << std::endl;
}



template<class GridType>
template<class VectorType>
void Dune::AmiraMeshWriter<GridType>::writeBlockVector(const GridType& grid,
                                                       const VectorType& f,
                                                       const std::string& filename)
{
  int level = grid.maxlevel();

  // Find out whether the grid contains only tetrahedra.  If yes, then
  // it is written in TetraGrid format.  If not, it is written in
  // hexagrid format.
  bool containsOnlyTetrahedra = true;

  typename GridType::template codim<0>::LevelIterator element = grid.template lbegin<0>(level);
  typename GridType::template codim<0>::LevelIterator end     = grid.template lend<0>(level);

  for (; element!=end; ++element) {
    if (element->geometry().type() != tetrahedron) {
      containsOnlyTetrahedra = false;
      break;
    }
  }



  // Get number of components
  const int ncomp = VectorType::block_type::size;

  // Create AmiraMesh object
  AmiraMesh am;

  // Set the appropriate content type for 2D grid data
  if (GridType::dimension==2)
    am.parameters.set("ContentType", "HxTriangularData");

  if (!containsOnlyTetrahedra && GridType::dimension==3) {

    int numElements = grid.size(level, 0);
    AmiraMesh::Location* hexa_loc = new AmiraMesh::Location("Hexahedra", numElements);
    am.insert(hexa_loc);

  }

  AmiraMesh::Location* sol_nodes = new AmiraMesh::Location("Nodes", f.size());
  am.insert(sol_nodes);

  AmiraMesh::Data* nodeData = new AmiraMesh::Data("Data", sol_nodes, McPrimType::mc_double, ncomp);
  am.insert(nodeData);

  AmiraMesh::Field* nodeField;

  if (containsOnlyTetrahedra) {
    nodeField = new AmiraMesh::Field("sol", ncomp, McPrimType::mc_double,
                                     AmiraMesh::t_linear, nodeData);
  } else {

    nodeField = new AmiraMesh::Field("sol", ncomp, McPrimType::mc_double,
                                     AmiraMesh::t_trilinear, nodeData);
  }

  am.insert(nodeField);


  // write the data into the AmiraMesh object
  typedef typename VectorType::ConstIterator Iterator;
  Iterator dit    = f.begin();
  Iterator ditend = f.end();

  int i=0;
  for (; dit!=ditend; ++dit) {

    for (int j=0; j<ncomp; j++)
      ((double*)nodeData->dataPtr())[i++] = (*dit)[j];

  }

  // actually save the solution file
  // (the 1 means: ascii)
  if (!am.write(filename.c_str(), 1) )
    DUNE_THROW(IOError, "An error has occured writing file " << filename);

  std::cout << "Solution written successfully to: " << filename << std::endl;
}
