// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

template <class GridType>
template <class DiscFuncType>
void Dune::AmiraMeshReader<GridType>::readFunction(DiscFuncType& f, const std::string& filename)
{
  // f may be a block vector
  const int blocksize = DiscFuncType::block_type::size;

  // /////////////////////////////////////////////////////
  // Load the AmiraMesh file
  AmiraMesh* am = AmiraMesh::read(filename.c_str());

  if(!am)
    DUNE_THROW(IOError, "Could not open AmiraMesh file: " << filename);

  int i, j;

  f = 0;

  // We allow fields defined on the whole grid and fields defined
  // only on the boundary.  We now check the file and proceed accordingly
  if (!am->findData("Nodes", HxFLOAT, blocksize, "Data") &&
      !am->findData("Nodes", HxDOUBLE, blocksize, "Data")) {

    // get the data field
    AmiraMesh::Data* am_ValueData =  am->findData("Nodes", HxFLOAT, blocksize, "values");
    if (am_ValueData) {

      if (f.size()<am->nElements("Nodes"))
        DUNE_THROW(IOError, "When reading data from a surface field the "
                   << "array you provide has to have at least the size of the surface!");

      float* am_values_float = (float*) am_ValueData->dataPtr();

      for (i=0; i<am->nElements("Nodes"); i++) {
        for (j=0; j<blocksize; j++)
          f[i][j] = am_values_float[i*blocksize+j];

      }

    } else {
      am_ValueData =  am->findData("Nodes", HxDOUBLE, blocksize, "values");
      if (am_ValueData) {

        if (f.size()<am->nElements("Nodes"))
          DUNE_THROW(IOError, "When reading data from a surface field your the "
                     << "array you provide has to have at least the size of the surface!");

        for (i=0; i<blocksize*am->nElements("Nodes"); i++) {
          for (j=0; j<blocksize; j++)
            f[i][j] = ((double*)am_ValueData->dataPtr())[i*blocksize+j];
        }

      } else
        DUNE_THROW(IOError, "No data found in the file!");

    }

  } else {

    // get the data field
    AmiraMesh::Data* am_ValueData =  am->findData("Nodes", HxFLOAT, blocksize, "Data");
    if (am_ValueData) {

      float* am_values_float = (float*) am_ValueData->dataPtr();
      f.resize(am->nElements("Nodes"));

      for (i=0; i<am->nElements("Nodes"); i++)
        for (j=0; j<blocksize; j++)
          f[i][j] = am_values_float[i*blocksize+j];

    } else {
      am_ValueData =  am->findData("Nodes", HxDOUBLE, blocksize, "Data");
      if (am_ValueData) {
        f.resize(am->nElements("Nodes"));

        for (i=0; i<am->nElements("Nodes"); i++)
          for (j=0; j<blocksize; j++)
            f[i][j] = ((double*)am_ValueData->dataPtr())[i*blocksize+j];

      } else
        DUNE_THROW(IOError, "No data found in the file!");

    }

  }

  std::cout << "Data field " << filename << " loaded successfully!" << std::endl;

}
