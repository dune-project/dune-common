// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

template <class GridType>
template <class DiscFuncType>
void Dune::AmiraMeshReader<GridType>::readFunction(DiscFuncType& f, const std::string& filename)
{
  // /////////////////////////////////////////////////////
  // Load the AmiraMesh file
  AmiraMesh* am = AmiraMesh::read(filename.c_str());

  if(!am)
    DUNE_THROW(IOError, "Could not open AmiraMesh file");

  float* am_values_float = NULL;
  int i, j;

  // We allow fields defined on the whole grid and fields defined
  // only on the boundary.  We now check the file and proceed accordingly
  if (!am->findData("Nodes", HxFLOAT, 3, "Data") &&
      !am->findData("Nodes", HxDOUBLE, 3, "Data")) {

    DUNE_THROW(IOError, "Only the reading of functions defined on volumetric"
               << "grids is currently supported!");

#if 0
    // get the data field
    AmiraMesh::Data* am_ValueData =  am->findData("Nodes", HxFLOAT, 3, "values");
    if (am_ValueData) {
      am_values_float = (float*) am_ValueData->dataPtr();


      BoundaryData = (DOUBLE*)malloc(am->nElements("Nodes")*3*sizeof(DOUBLE));
      for (i=0; i<3*am->nElements("Nodes"); i++) {
        BoundaryData[i] = am_values_float[i];
      }
    } else {
      am_ValueData =  am->findData("Nodes", HxDOUBLE, 3, "values");
      if (am_ValueData)
        BoundaryData = (DOUBLE*) am_ValueData->takeDataPtr();
      else
        throw("No data found in the file!");

    }
#endif
  } else {

    std::cout << "Loading boundary conditions from a tetragrid field!" << std::endl;

    // get the data field
    AmiraMesh::Data* am_ValueData =  am->findData("Nodes", HxFLOAT, 3, "Data");
    if (am_ValueData) {
      am_values_float = (float*) am_ValueData->dataPtr();
      f.resize(am->nElements("Nodes"));

      for (i=0; i<am->nElements("Nodes"); i++)
        for (j=0; j<3; j++)
          f[i][j] = am_values_float[i*3+j];

    } else {
      am_ValueData =  am->findData("Nodes", HxDOUBLE, 3, "Data");
      if (am_ValueData) {
        f.resize(am->nElements("Nodes"));

        for (i=0; i<am->nElements("Nodes"); i++)
          for (j=0; j<3; j++)
            f[i][j] = ((double*)am_ValueData->dataPtr())[i*3+j];

        //BoundaryData = (DOUBLE*) am_ValueData->takeDataPtr();
      } else
        DUNE_THROW(IOError, "No data found in the file!");

    }

  }

  std::cout << "BoundaryData loaded successfully!" << std::endl;

}
