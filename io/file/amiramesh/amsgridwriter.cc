// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// ///////////////////////////////////////////////
// Specialization of the AmiraMesh writer for SGrid<3,3> and SGrid<2,2>
// ///////////////////////////////////////////////

#include <dune/grid/sgrid.hh>

#if 0
namespace Dune {

  template <>
  template <class DiscFuncType>
  class AmiraMeshWriter<SGrid<3,3>, DiscFuncType> {

  public:

    static void write(const SGrid<3,3>& grid,
                      const DiscFuncType& sol,
                      const std::string& filename);


    AmiraMeshWriter() {}

  };

}

//template<>
void Dune::AmiraMeshWriter<Dune::SimpleGrid<3,3>, double>::write(const Dune::SimpleGrid<3,3>& grid,
                                                                 const Array<double>& sol,
                                                                 const std::string& filename)
{
  printf("This is the AmiraMesh writer for SimpleGrid<3,3>!\n");

  int maxlevel = grid.maxlevel();
  const levelinfo<3>* li = grid.get_levelinfo(maxlevel);

  // determine current time
  time_t time_val = time(NULL);
  struct tm* localtime_val = localtime(&time_val);
  const char* asctime_val = asctime(localtime_val);

  // write the amiramesh header
  FILE* fp = fopen(filename.c_str(), "w");

  fprintf(fp, "# AmiraMesh 3D ASCII 2.0\n");
  fprintf(fp, "# CreationDate: %s\n\n\n", asctime_val);

  fprintf(fp, "define Lattice %d %d %d\n\n", li->ne[0], li->ne[1], li->ne[2]);

  fprintf(fp, "Parameters {\n");
  // SimpleGrids always have the unit bounding box
  fprintf(fp, "    BoundingBox 0 1 0 1 0 1,\n");
  fprintf(fp, "    CoordType \"uniform\",\n");
  fprintf(fp, "    Content \"%dx%dx%d double, uniform coordinates\"\n",
          li->ne[0], li->ne[1], li->ne[2]);
  fprintf(fp, "}\n\n");

  fprintf(fp, "Lattice { double Data } @1\n\n");

  fprintf(fp, "# Data section follows\n");
  fprintf(fp, "@1\n");

  //
  Array<double>::Iterator iter = sol.begin();
  for (; iter!=sol.end(); ++iter) {
    fprintf(fp, "%f\n", *iter);
  }

  fclose(fp);

}
#endif

namespace Dune {

  template<>
  template <class DiscFuncType>
  class AmiraMeshWriter<SGrid<2,2>, DiscFuncType> {

  public:

    static void write(const SGrid<2,2>& grid,
                      const DiscFuncType& sol,
                      const std::string& filename);


    AmiraMeshWriter() {}

  };

}


template<> template <class DiscFuncType>
void Dune::AmiraMeshWriter<Dune::SGrid<2,2>, DiscFuncType>::
write(const Dune::SGrid<2,2>& grid,
      const DiscFuncType& sol,
      const std::string& filename)
{
  std::cout << "This is the AmiraMesh writer for SGrid<2,2>!" << std::endl;

  int maxlevel = grid.maxlevel();
  //const Dune::levelinfo<2>* li = grid.get_levelinfo(maxlevel);

  // determine current time
  time_t time_val = time(NULL);
  struct tm* localtime_val = localtime(&time_val);
  const char* asctime_val = asctime(localtime_val);

  // write the amiramesh header
  FILE* fp = fopen(filename.c_str(), "w");

  fprintf(fp, "# AmiraMesh 3D ASCII 2.0\n");
  fprintf(fp, "# CreationDate: %s\n\n\n", asctime_val);

  fprintf(fp, "define Lattice %d %d %d\n\n", grid.dims(maxlevel)[0]+1, grid.dims(maxlevel)[1]+1, 1);

  fprintf(fp, "Parameters {\n");

  fprintf(fp, "    BoundingBox %g %g %g %g 0 1,\n",
          grid.lowerLeft()[0], grid.upperRight()[0],
          grid.lowerLeft()[1], grid.upperRight()[1]);
  fprintf(fp, "    CoordType \"uniform\",\n");
  fprintf(fp, "    Content \"%dx%dx%d double, uniform coordinates\"\n",
          grid.dims(maxlevel)[0]+1, grid.dims(maxlevel)[1]+1, 1);
  fprintf(fp, "}\n\n");

  fprintf(fp, "Lattice { double Data } @1\n\n");

  fprintf(fp, "# Data section follows\n");
  fprintf(fp, "@1\n");

  //
  typedef typename DiscFuncType::DofIteratorType DofIterator;
  DofIterator it    = sol.dbegin(maxlevel);
  DofIterator endIt = sol.dend(maxlevel);

  for (; it!=endIt; ++it) {
    fprintf(fp, "%f\n", *it);
  }

  fclose(fp);
}
