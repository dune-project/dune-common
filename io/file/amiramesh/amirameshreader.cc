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
    DUNE_THROW(IOError, "Could not open AmiraMesh file: " << filename);

  float* am_values_float = NULL;
  int i, j;

  f = 0;

  // We allow fields defined on the whole grid and fields defined
  // only on the boundary.  We now check the file and proceed accordingly
  if (!am->findData("Nodes", HxFLOAT, 3, "Data") &&
      !am->findData("Nodes", HxDOUBLE, 3, "Data")) {

    // get the data field
    AmiraMesh::Data* am_ValueData =  am->findData("Nodes", HxFLOAT, 3, "values");
    if (am_ValueData) {

      if (f.size()<am->nElements("Nodes"))
        DUNE_THROW(IOError, "When reading data from a surface field your the "
                   << "array you provide has to have at least the size of the surface!");

      am_values_float = (float*) am_ValueData->dataPtr();

      for (i=0; i<am->nElements("Nodes"); i++) {
        for (j=0; j<3; j++) {
          f[i][j] = am_values_float[i*3+j];
          //std::cout << "size " << f.size() << " i " << i << "  j " << j << std::endl;
        }
      }

    } else {
      am_ValueData =  am->findData("Nodes", HxDOUBLE, 3, "values");
      if (am_ValueData) {

        if (f.size()<am->nElements("Nodes"))
          DUNE_THROW(IOError, "When reading data from a surface field your the "
                     << "array you provide has to have at least the size of the surface!");

        for (i=0; i<3*am->nElements("Nodes"); i++) {
          for (j=0; j<3; j++)
            f[i][j] = am_values_float[i*3+j];
        }

      } else
        DUNE_THROW(IOError, "No data found in the file!");

    }

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

      } else
        DUNE_THROW(IOError, "No data found in the file!");

    }

  }

  std::cout << "Data field " << filename << " loaded successfully!" << std::endl;

}
